#include "preview_window.hpp"
#include "../button.hpp"
#include "../checkbox.hpp"
#include "../combobox.hpp"
#include "../constants.hpp"
#include "../label.hpp"
#include "../progress_bar.hpp"
#include "../radio_button.hpp"
#include "../range_slider.hpp"
#include "../scroll_area.hpp"
#include "../slider.hpp"
#include "../text_area.hpp"
#include "../text_input.hpp"
#include "../widget_factory.hpp"

#include "editor_utils.hpp"
#include "std.hpp"

// Helper function for safe float parsing from properties
static float previewParseFloat(const std::string& value, float defaultVal) {
    if (value.empty()) return defaultVal;
    try {
        return std::stof(value);
    } catch (...) {
        return defaultVal;
    }
}

PreviewWindow::PreviewWindow(GUIManager& manager, EditorState& state, int width, int height)
    : m_manager(manager), m_state(state) {
    m_canvas = new CanvasPanel(manager, 0, 0, width, height, state);
    m_canvas->setPreviewWindow(this);
    manager.addElement(std::unique_ptr<GUIElement>(m_canvas));
}

PreviewWindow::~PreviewWindow() {
    clearAllWidgets();
}

void PreviewWindow::refreshElement(size_t index) {
    // Thin wrapper: the map is id-keyed, so even a stale index (e.g. after a
    // delete that shifted the vector) cannot remove the wrong widget.
    // A full diff also prunes widgets whose ids left the document.
    if (index < m_state.getElements().size()) {
        const EditorElement& elem = m_state.getElements()[index];
        GUIElement* widget = findWidgetById(elem.id);
        const std::string key = structureKey(elem);
        auto kit = m_structureKeys.find(elem.id);
        ComponentType expected = componentTypeFromString(elem.type);
        if (expected == ComponentType::Unknown) expected = ComponentType::Panel;  // createWidget fallback
        if (!widget || kit == m_structureKeys.end() || kit->second != key ||
            widget->getComponentTypeId() != expected) {
            removeWidgetById(elem.id);
            createWidgetForElement(index);
        } else {
            updateWidgetForElement(widget, elem);
        }
    }
    syncAll();
}

void PreviewWindow::removeElementWidget(size_t index) {
    // NOTE: EditorWindow fires onElementDeleted AFTER deleteElement(), so the
    // index is already stale here. Remove by id when possible, then run the
    // diff which prunes by document membership — never by position.
    if (index < m_state.getElements().size()) {
        removeWidgetById(m_state.getElements()[index].id);
    }
    syncAll();
}

void PreviewWindow::clearAllWidgets() {
    for (auto& [id, widget] : m_widgetMap) {
        widget->markForDeletion();
    }
    m_widgetMap.clear();
    m_structureKeys.clear();
}

GUIElement* PreviewWindow::findWidgetById(const std::string& id) const {
    auto it = m_widgetMap.find(id);
    return it != m_widgetMap.end() ? it->second : nullptr;
}

void PreviewWindow::syncAll() {
    const auto& elements = m_state.getElements();
    std::unordered_set<std::string> seen;
    seen.reserve(elements.size());

    for (size_t i = 0; i < elements.size(); ++i) {
        const EditorElement& elem = elements[i];
        seen.insert(elem.id);
        const std::string key = structureKey(elem);
        GUIElement* widget = findWidgetById(elem.id);
        auto kit = m_structureKeys.find(elem.id);
        ComponentType expected = componentTypeFromString(elem.type);
        if (expected == ComponentType::Unknown) expected = ComponentType::Panel;  // fallback
        if (!widget || kit == m_structureKeys.end() || kit->second != key ||
            widget->getComponentTypeId() != expected) {
            removeWidgetById(elem.id);
            createWidgetForElement(i);
        } else {
            // Same structure: update in place — focus, scroll position and
            // text selection survive property edits (no remove+create).
            updateWidgetForElement(widget, elem);
        }
    }

    std::vector<std::string> dead;
    for (const auto& [id, widget] : m_widgetMap) {
        if (!seen.contains(id)) dead.push_back(id);
    }
    for (const auto& id : dead) removeWidgetById(id);
}

void PreviewWindow::removeWidgetById(const std::string& id) {
    auto it = m_widgetMap.find(id);
    if (it != m_widgetMap.end()) {
        it->second->markForDeletion();
        m_widgetMap.erase(it);
    }
    m_structureKeys.erase(id);
}

void PreviewWindow::createWidgetForElement(size_t index) {
    const auto& elements = m_state.getElements();
    if (index >= elements.size()) return;

    const EditorElement& elem = elements[index];

    std::unique_ptr<GUIElement> widgetPtr = createWidget(elem);
    if (!widgetPtr) return;

    GUIElement* widget = widgetPtr.get();

    applyElementProperties(widget, elem);
    applyElementStyles(widget, elem);

    GUIElement* parentWidget = m_canvas;
    if (!elem.parentId.empty()) {
        if (auto parentIdx = m_state.findElementById(elem.parentId)) {
            const auto& all = m_state.getElements();
            if (*parentIdx < all.size()) {
                if (GUIElement* pw = findWidgetById(all[*parentIdx].id)) {
                    parentWidget = pw;
                }
            }
        }
    }

    parentWidget->addChild(std::move(widgetPtr));

    widget->setPosition(elem.x, elem.y);

    m_widgetMap[elem.id] = widget;
    m_structureKeys[elem.id] = structureKey(elem);
}

void PreviewWindow::updateWidgetForElement(GUIElement* widget, const EditorElement& elem) {
    applyElementProperties(widget, elem);
    applyElementStyles(widget, elem);

    // Scalar props, applied idempotently. List/structure props are covered
    // by structureKey() and take the recreate path instead.
    WidgetProps props;
    fillPropsFromEditor(elem, props);
    switch (widget->getComponentTypeId()) {
        case ComponentType::Button: {
            for (auto& child : widget->getChildren()) {
                if (child->getComponentTypeId() == ComponentType::Label) {
                    static_cast<Label*>(child.get())->setText(props.text);
                    break;
                }
            }
            break;
        }
        case ComponentType::Label:
            static_cast<Label*>(widget)->setText(props.text);
            break;
        case ComponentType::Checkbox:
            static_cast<Checkbox*>(widget)->setChecked(props.checked);
            break;
        case ComponentType::RadioButton:
            static_cast<RadioButton*>(widget)->setSelected(props.selected);
            break;
        case ComponentType::Slider: {
            auto* s = static_cast<Slider*>(widget);
            s->setRange(props.minVal, props.maxVal);
            s->setValue(props.value);
            s->setWheelStep(props.wheelStep);
            break;
        }
        case ComponentType::RangeSlider: {
            auto* rs = static_cast<RangeSlider*>(widget);
            rs->setRange(props.minVal, props.maxVal);
            rs->setLowerValue(props.lowerVal);
            rs->setUpperValue(props.upperVal);
            rs->setWheelStep(props.wheelStep);
            break;
        }
        case ComponentType::TextInput: {
            auto* ti = static_cast<TextInput*>(widget);
            ti->setText(props.text);
            ti->setLocked(props.locked);
            break;
        }
        case ComponentType::TextArea: {
            auto* ta = static_cast<TextArea*>(widget);
            ta->setText(props.text);
            ta->setWordWrap(props.wordWrap);
            ta->setLocked(props.locked);
            break;
        }
        case ComponentType::ComboBox:
            static_cast<ComboBox*>(widget)->setSelectedIndex(props.selectedIndex);
            break;
        case ComponentType::ProgressBar: {
            auto* pb = static_cast<ProgressBar*>(widget);
            pb->setRange(props.minF, props.maxF);
            pb->setValue(props.valueF);
            pb->setShowText(props.showText);
            break;
        }
        case ComponentType::ScrollArea: {
            auto* sa = static_cast<ScrollArea*>(widget);
            if (props.contentWidth >= 0 || props.contentHeight >= 0) {
                sa->setContentSize(props.contentWidth >= 0 ? props.contentWidth : sa->getWidth(),
                                   props.contentHeight >= 0 ? props.contentHeight : sa->getHeight());
            }
            break;
        }
        default:
            break;
    }
}

void PreviewWindow::fillPropsFromEditor(const EditorElement& elem, WidgetProps& p) {
    p.w = elem.width;
    p.h = elem.height;

    auto getBool = [&elem](const char* key, bool def) {
        return elem.getProperty(key, def ? "true" : "false") == "true";
    };
    auto getInt = [&elem](const char* key, int def) {
        return safeParseInt(elem.getProperty(key, std::to_string(def)), def);
    };
    auto splitItems = [&elem](const char* key) {
        std::vector<std::string> out;
        std::istringstream iss(elem.getProperty(key, ""));
        std::string item;
        while (std::getline(iss, item, ',')) {
            if (!item.empty()) out.push_back(item);
        }
        return out;
    };

    const std::string& type = elem.type;
    if (type == "Button") {
        p.text = elem.getProperty("text", "Button");
    } else if (type == "Label") {
        p.text = elem.getProperty("text", "Label");
        p.fontSize = getInt("fontSize", -1);
    } else if (type == "Checkbox") {
        p.checked = getBool("checked", false);
    } else if (type == "RadioButton") {
        p.selected = getBool("selected", false);
    } else if (type == "RadioGroup") {
        p.hasOptionSpacing = elem.hasProperty("optionSpacing");
        p.optionSpacing = getInt("optionSpacing", 40);
    } else if (type == "Slider" || type == "RangeSlider") {
        p.minVal = getInt("min", 0);
        p.maxVal = getInt("max", 100);
        p.value = getInt("value", 50);
        p.wheelStep = getInt("wheelStep", 1);
        p.vertical = elem.getProperty("orientation", "horizontal") == "vertical";
        if (type == "RangeSlider") {
            p.lowerVal = getInt("lower", 0);
            p.upperVal = getInt("upper", 100);
        }
    } else if (type == "TextInput") {
        p.text = elem.getProperty("text", "");
        p.locked = getBool("locked", false);
    } else if (type == "TextArea") {
        p.text = elem.getProperty("text", "");
        p.fontPath = elem.getProperty("fontPath", constants::kDefaultFontPath);
        p.fontSize = getInt("fontSize", 16);
        p.wordWrap = getBool("wordWrap", true);
        p.locked = getBool("locked", false);
    } else if (type == "ComboBox") {
        p.items = splitItems("items");
        if (elem.hasProperty("selectedIndex")) {
            p.hasSelectedIndex = true;
            p.selectedIndex = getInt("selectedIndex", -1);
        }
    } else if (type == "TabControl") {
        p.tabHeight = getInt("tabHeight", 30);
        for (const auto& title : splitItems("tabs")) {
            WidgetTabSpec tab;
            tab.title = title;
            p.tabs.push_back(std::move(tab));
        }
    } else if (type == "AnimatedImage") {
        p.path = elem.getProperty("path", "");
        p.frames = getInt("frames", 1);
        p.rows = getInt("rows", 1);
        p.frameW = getInt("frameW", 0);
        p.frameH = getInt("frameH", 0);
        p.fps = previewParseFloat(elem.getProperty("fps", "12"), 12.0f);
        p.loop = getBool("loop", true);
        p.autoplay = getBool("autoplay", true);
    } else if (type == "StringGrid") {
        p.rowCount = static_cast<size_t>(getInt("rowCount", 5));
        p.colCount = static_cast<size_t>(getInt("colCount", 5));
        p.showRowHeaders = getBool("showRowHeaders", true);
        p.showColumnHeaders = getBool("showColumnHeaders", true);
        p.editable = getBool("editable", false);
    } else if (type == "ListView") {
        p.items = splitItems("items");
        if (elem.hasProperty("selectedIndex")) {
            p.hasSelectedIndex = true;
            p.selectedIndex = getInt("selectedIndex", -1);
        }
    } else if (type == "ProgressBar") {
        p.minF = previewParseFloat(elem.getProperty("min", "0"), 0.0f);
        p.maxF = previewParseFloat(elem.getProperty("max", "100"), 100.0f);
        p.valueF = previewParseFloat(elem.getProperty("value", "0"), 0.0f);
        p.showText = getBool("showText", true);
        p.vertical = elem.getProperty("orientation", "horizontal") == "vertical";
    } else if (type == "ScrollArea") {
        if (elem.hasProperty("contentWidth")) p.contentWidth = getInt("contentWidth", 0);
        if (elem.hasProperty("contentHeight")) p.contentHeight = getInt("contentHeight", 0);
    } else if (type == "ArcContainer") {
        p.radius = getInt("radius", 100);
        p.startAngle = previewParseFloat(elem.getProperty("startAngle", "0"), 0.0f);
        p.endAngle = previewParseFloat(elem.getProperty("endAngle", "360"), 360.0f);
    }
}

std::string PreviewWindow::structureKey(const EditorElement& elem) {
    std::string key = elem.type;
    key += '\x1f' + elem.getProperty("items", "");
    key += '\x1f' + elem.getProperty("tabs", "");
    key += '\x1f' + elem.getProperty("options", "");
    key += '\x1f' + elem.getProperty("rowCount", "") + 'x' + elem.getProperty("colCount", "");
    key += '\x1f' + elem.getProperty("path", "") + elem.getProperty("frames", "") +
           elem.getProperty("rows", "");
    key += '\x1f' + elem.getProperty("radius", "") + elem.getProperty("startAngle", "") +
           elem.getProperty("endAngle", "");
    key += '\x1f' + elem.getProperty("contentWidth", "") + 'x' +
           elem.getProperty("contentHeight", "");
    key += '\x1f' + elem.getProperty("tabHeight", "");
    return key;
}

std::unique_ptr<GUIElement> PreviewWindow::createWidget(const EditorElement& elem) {
    // Single construction path shared with LayoutParser: the factory owns
    // type knowledge (this also gains preview support for RadioGroup,
    // RangeSlider, ProgressBar, ScrollArea and ArcContainer, which the old
    // if-chain silently downgraded to Panel).
    WidgetProps props;
    fillPropsFromEditor(elem, props);
    if (auto widget = WidgetFactory::create(m_manager, elem.type, props)) {
        if (elem.type == "Panel") {
            static_cast<Panel*>(widget.get())->setClipChildren(false);
        }
        return widget;
    }
    auto panel = std::make_unique<Panel>(m_manager, 0, 0, elem.width, elem.height);
    panel->setClipChildren(false);
    return panel;
}

void PreviewWindow::applyElementProperties(GUIElement* widget, const EditorElement& elem) {
    widget->setID(elem.id);
    widget->setVisible(elem.getProperty("visible", "true") == "true");
    widget->setEnabled(elem.getProperty("enabled", "true") == "true");
    widget->setSize(elem.width, elem.height);
    widget->setPosition(elem.x, elem.y);
}

void PreviewWindow::applyElementStyles(GUIElement* widget, const EditorElement& elem) {
    for (const auto& [state, style] : elem.styles) {
        if (style.backgroundColor) {
            widget->setBackgroundColor(state, *style.backgroundColor);
        }
        if (style.textColor) {
            widget->setTextColor(state, *style.textColor);
        }
        if (style.borderColor && style.borderWidth) {
            widget->setBorder(state, *style.borderColor, *style.borderWidth);
        }
        if (style.borderRadius) {
            widget->setBorderRadius(state, *style.borderRadius);
        }
    }
}

CanvasPanel::CanvasPanel(GUIManager& manager, int x, int y, int width, int height, EditorState& state)
    : Panel(manager, x, y, width, height), m_state(state) {
    setClipChildren(false);
    setBackgroundColor(ElementState::Normal, SDL_Color{245, 245, 245, 255});
    setBorder(ElementState::Normal, SDL_Color{200, 200, 200, 255}, 1);
}

ComponentType CanvasPanel::getComponentTypeId() const {
    return ComponentType::Panel;
}

void CanvasPanel::drawDirect(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    
    if (m_showGrid) {
        drawGrid(renderer);
    }
}

void CanvasPanel::drawGrid(SDL_Renderer* renderer) {
    int gridSize = m_state.getGridSize();
    if (gridSize <= 0) return;
    
    float fGridSize = static_cast<float>(gridSize);
    float fW = static_cast<float>(m_width);
    float fH = static_cast<float>(m_height);
    
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    
    for (float x = 0; x < fW; x += fGridSize) {
        RenderLine(renderer, x, 0.0f, x, fH);
    }
    
    for (float y = 0; y < fH; y += fGridSize) {
        RenderLine(renderer, 0.0f, y, fW, y);
    }
}

void CanvasPanel::renderOverlay(SDL_Renderer* renderer) {
    Panel::renderOverlay(renderer);
    
    if (m_state.hasSelectedElement()) {
        size_t selectedIndex = m_state.getSelectedElementIndex();
        drawSelectionHighlight(renderer, selectedIndex);
    }
}

void CanvasPanel::drawSelectionHighlight(SDL_Renderer* renderer, size_t index) {
    if (m_previewWindow == nullptr) return;

    const auto& elements = m_state.getElements();
    if (index >= elements.size()) return;
    GUIElement* widget = m_previewWindow->findWidgetById(elements[index].id);
    if (!widget) return;
    SDL_Point abs = widget->getAbsolutePosition();
    
    const int borderWidth = 3;
    
    SDL_Color highlightColor = {0, 120, 215, 255};
    SetDrawColor(renderer, highlightColor);
    
    int x = abs.x - borderWidth;
    int y = abs.y - borderWidth;
    int w = widget->getWidth() + 2 * borderWidth;
    int h = widget->getHeight() + 2 * borderWidth;
    
    for (int i = 0; i < borderWidth; ++i) {
        SDL_Rect r = {x + i, y + i, w - 2*i, h - 2*i};
        RenderRect(renderer, r);
    }
}

bool CanvasPanel::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }
    
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        SDL_Point abs = getAbsolutePosition();
        float localX = e.button.x - static_cast<float>(abs.x);
        float localY = e.button.y - static_cast<float>(abs.y);
        
        if (localX < 0 || localX >= static_cast<float>(m_width) || localY < 0 || localY >= static_cast<float>(m_height)) {
            return false;
        }
        
        auto hitElement = m_state.findElementAtPosition(static_cast<int>(e.button.x), static_cast<int>(e.button.y));
        
        if (hitElement.has_value()) {
            handleElementClick(e.button.x, e.button.y, hitElement.value());
            return true;
        } else {
            handleCanvasClick(e.button.x, e.button.y);
            return true;
        }
    }
    else if (e.type == SDL_EVENT_MOUSE_MOTION && m_isDragging) {
        updateDrag(e.motion.x, e.motion.y);
        return true;
    }
    else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_isDragging) {
            endDrag();
            return true;
        }
    }
    
    return Panel::handleEvent(e);
}

void CanvasPanel::handleCanvasClick(float mouseX, float mouseY) {
    int ix = static_cast<int>(mouseX);
    int iy = static_cast<int>(mouseY);
    const std::string& selectedType = m_state.getSelectedWidgetType();
    
    if (!selectedType.empty()) {
        size_t newIndex = m_state.addElement(selectedType, ix, iy);
        
        if (m_previewWindow) {
            m_previewWindow->refreshElement(newIndex);
        }
        
        if (m_previewWindow && m_previewWindow->m_onSelectionChanged) {
            m_previewWindow->m_onSelectionChanged(m_state.getSelectedElementIndex());
        }
    } else {
        m_state.clearSelection();
        
        if (m_previewWindow && m_previewWindow->m_onSelectionChanged) {
            m_previewWindow->m_onSelectionChanged(static_cast<size_t>(-1));
        }
    }
}

void CanvasPanel::handleElementClick(float mouseX, float mouseY, size_t elementIndex) {
    m_state.selectElement(elementIndex);
    
    if (m_previewWindow && m_previewWindow->m_onSelectionChanged) {
        m_previewWindow->m_onSelectionChanged(elementIndex);
    }
    
    startDrag(mouseX, mouseY);
}

void CanvasPanel::startDrag(float mouseX, float mouseY) {
    int ix = static_cast<int>(mouseX);
    int iy = static_cast<int>(mouseY);
    const EditorElement* elem = m_state.getSelectedElement();
    if (!elem) return;
    
    m_isDragging = true;
    m_dragOffsetX = ix - elem->x;
    m_dragOffsetY = iy - elem->y;
    m_dragStartX = elem->x;
    m_dragStartY = elem->y;
    
    m_manager.captureMouse(this);
}

void CanvasPanel::updateDrag(float mouseX, float mouseY) {
    int ix = static_cast<int>(mouseX);
    int iy = static_cast<int>(mouseY);
    if (!m_isDragging) return;
    
    const EditorElement* elem = m_state.getSelectedElement();
    if (!elem) return;
    
    int newX = snapToGrid(ix - m_dragOffsetX);
    int newY = snapToGrid(iy - m_dragOffsetY);
    
    newX = std::max(0, newX);
    newY = std::max(0, newY);
    
    size_t selectedIndex = m_state.getSelectedElementIndex();
    m_state.moveElement(selectedIndex, newX, newY);
    
    if (m_previewWindow) {
        const auto& elements = m_state.getElements();
        if (selectedIndex < elements.size()) {
            if (GUIElement* w = m_previewWindow->findWidgetById(elements[selectedIndex].id)) {
                w->setPosition(newX, newY);
            }
        }
        
        if (m_previewWindow->m_onElementMoved) {
            m_previewWindow->m_onElementMoved(selectedIndex, newX, newY);
        }
    }
}

void CanvasPanel::endDrag() {
    m_isDragging = false;
    m_manager.releaseMouse();
}