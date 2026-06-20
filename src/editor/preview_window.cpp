#include "preview_window.hpp"
#include "../button.hpp"
#include "../label.hpp"
#include "../checkbox.hpp"
#include "../radio_button.hpp"
#include "../slider.hpp"
#include "../text_input.hpp"
#include "../text_area.hpp"
#include "../combobox.hpp"
#include "../tab_control.hpp"
#include "../animated_image.hpp"
#include "../canvas.hpp"
#include "../string_grid.hpp"
#include "../list_view.hpp"

#include "std.hpp"

// Helper function for safe integer parsing from properties
static int previewParseInt(const std::string& value, int defaultVal) {
    if (value.empty()) return defaultVal;
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultVal;
    }
}

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

void PreviewWindow::refreshAllElements() {
    clearAllWidgets();
    
    const auto& elements = m_state.getElements();
    for (size_t i = 0; i < elements.size(); ++i) {
        createWidgetForElement(i);
    }
}

void PreviewWindow::refreshElement(size_t index) {
    if (index >= m_state.getElements().size()) return;
    
    removeElementWidget(index);
    createWidgetForElement(index);
}

void PreviewWindow::removeElementWidget(size_t index) {
    auto it = m_widgetMap.find(index);
    if (it != m_widgetMap.end()) {
        if (it->second) {
            it->second->markForDeletion();
        }
        m_widgetMap.erase(it);
    }
}

void PreviewWindow::clearAllWidgets() {
    for (auto& [idx, widget] : m_widgetMap) {
        if (widget) {
            widget->markForDeletion();
        }
    }
    m_widgetMap.clear();
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
        auto parentIt = m_state.findElementById(elem.parentId);
        if (parentIt.has_value()) {
            auto widgetIt = m_widgetMap.find(parentIt.value());
            if (widgetIt != m_widgetMap.end() && widgetIt->second) {
                parentWidget = widgetIt->second;
            }
        }
    }
    
    parentWidget->addChild(std::move(widgetPtr));
    
    widget->setPosition(elem.x, elem.y);
    
    m_widgetMap[index] = widget;
}

std::unique_ptr<GUIElement> PreviewWindow::createWidget(const EditorElement& elem) {
    if (elem.type == "Button") {
        std::string text = elem.getProperty("text", "Button");
        return std::make_unique<Button>(m_manager, 0, 0, elem.width, elem.height, text);
    }
    else if (elem.type == "Label") {
        std::string text = elem.getProperty("text", "Label");
        int fontSize = previewParseInt(elem.getProperty("fontSize", "-1"), -1);
        return std::make_unique<Label>(m_manager, 0, 0, text, fontSize);
    }
    else if (elem.type == "Checkbox") {
        auto checkbox = std::make_unique<Checkbox>(m_manager, 0, 0, elem.width, elem.height);
        bool checked = elem.getProperty("checked", "false") == "true";
        checkbox->setChecked(checked);
        return checkbox;
    }
    else if (elem.type == "RadioButton") {
        auto radio = std::make_unique<RadioButton>(m_manager, 0, 0, elem.width, elem.height);
        bool selected = elem.getProperty("selected", "false") == "true";
        radio->setSelected(selected);
        return radio;
    }
    else if (elem.type == "Slider") {
        int min = previewParseInt(elem.getProperty("min", "0"), 0);
        int max = previewParseInt(elem.getProperty("max", "100"), 100);
        int value = previewParseInt(elem.getProperty("value", "50"), 50);
        Orientation orient = elem.getProperty("orientation", "horizontal") == "vertical" 
            ? Orientation::Vertical : Orientation::Horizontal;
        auto slider = std::make_unique<Slider>(m_manager, 0, 0, elem.width, elem.height, min, max, value, orient);
        return slider;
    }
    else if (elem.type == "TextInput") {
        std::string text = elem.getProperty("text", "");
        bool locked = elem.getProperty("locked", "false") == "true";
        auto input = std::make_unique<TextInput>(m_manager, 0, 0, elem.width, elem.height);
        input->setText(text);
        input->setLocked(locked);
        return input;
    }
    else if (elem.type == "TextArea") {
        std::string text = elem.getProperty("text", "");
        std::string fontPath = elem.getProperty("fontPath", "assets/fonts/font.ttf");
        int fontSize = previewParseInt(elem.getProperty("fontSize", "16"), 16);
        auto area = std::make_unique<TextArea>(m_manager, 0, 0, elem.width, elem.height, fontPath, fontSize);
        area->setText(text);
        return area;
    }
    else if (elem.type == "ComboBox") {
        auto combo = std::make_unique<ComboBox>(m_manager, 0, 0, elem.width, elem.height);
        std::string items = elem.getProperty("items", "");
        if (!items.empty()) {
            std::istringstream iss(items);
            std::string item;
            while (std::getline(iss, item, ',')) {
                if (!item.empty()) {
                    combo->addItem(item);
                }
            }
        }
        return combo;
    }
    else if (elem.type == "Panel") {
        auto panel = std::make_unique<Panel>(m_manager, 0, 0, elem.width, elem.height);
        panel->setClipChildren(false);
        return panel;
    }
    else if (elem.type == "TabControl") {
        int tabHeight = previewParseInt(elem.getProperty("tabHeight", "30"), 30);
        auto tabs = std::make_unique<TabControl>(m_manager, 0, 0, elem.width, elem.height, tabHeight);
        std::string tabsStr = elem.getProperty("tabs", "");
        if (!tabsStr.empty()) {
            std::istringstream iss(tabsStr);
            std::string tabName;
            while (std::getline(iss, tabName, ',')) {
                if (!tabName.empty()) {
                    tabs->addTab(tabName);
                }
            }
        }
        return tabs;
    }
    else if (elem.type == "AnimatedImage") {
        auto anim = std::make_unique<AnimatedImage>(m_manager, 0, 0, elem.width, elem.height);
        std::string path = elem.getProperty("path", "");
        int frames = previewParseInt(elem.getProperty("frames", "1"), 1);
        int rows = previewParseInt(elem.getProperty("rows", "1"), 1);
        int frameW = previewParseInt(elem.getProperty("frameW", "0"), 0);
        int frameH = previewParseInt(elem.getProperty("frameH", "0"), 0);
        float fps = previewParseFloat(elem.getProperty("fps", "12"), 12.0f);
        bool loop = elem.getProperty("loop", "true") == "true";
        bool autoplay = elem.getProperty("autoplay", "true") == "true";
        
        if (!path.empty()) {
            anim->setSpriteSheet(path, frames, rows, frameW, frameH);
            anim->setFPS(fps);
            anim->setLoop(loop);
            if (autoplay) anim->play();
        }
        return anim;
    }
    else if (elem.type == "Canvas") {
        return std::make_unique<Canvas>(m_manager, 0, 0, elem.width, elem.height);
    }
    else if (elem.type == "StringGrid") {
        int rowCount = previewParseInt(elem.getProperty("rowCount", "5"), 5);
        int colCount = previewParseInt(elem.getProperty("colCount", "5"), 5);
        bool showRowHeaders = elem.getProperty("showRowHeaders", "true") == "true";
        bool showColHeaders = elem.getProperty("showColumnHeaders", "true") == "true";
        bool editable = elem.getProperty("editable", "false") == "true";
        auto grid = std::make_unique<StringGrid>(m_manager, 0, 0, elem.width, elem.height, 
            static_cast<size_t>(rowCount), static_cast<size_t>(colCount));
        grid->setShowRowHeaders(showRowHeaders);
        grid->setShowColumnHeaders(showColHeaders);
        grid->setEditable(editable);
        return grid;
    }
    else if (elem.type == "ListView") {
        auto list = std::make_unique<ListView>(m_manager, 0, 0, elem.width, elem.height);
        std::string items = elem.getProperty("items", "");
        if (!items.empty()) {
            std::istringstream iss(items);
            std::string item;
            while (std::getline(iss, item, ',')) {
                if (!item.empty()) {
                    list->addItem(item);
                }
            }
        }
        int selectedIndex = previewParseInt(elem.getProperty("selectedIndex", "-1"), -1);
        if (selectedIndex >= 0) {
            list->setSelectedRow(static_cast<size_t>(selectedIndex));
        }
        return list;
    }
    
    return std::make_unique<Panel>(m_manager, 0, 0, elem.width, elem.height);
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

const char* CanvasPanel::getComponentType() const {
    return "CanvasPanel";
}

void CanvasPanel::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
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
    
    auto it = m_previewWindow->m_widgetMap.find(index);
    if (it == m_previewWindow->m_widgetMap.end() || it->second == nullptr) return;
    
    GUIElement* widget = it->second;
    SDL_Point abs = widget->getAbsolutePosition();
    
    const int borderWidth = 3;
    
    SDL_Color highlightColor = {0, 120, 215, 255};
    SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
    
    int x = abs.x - borderWidth;
    int y = abs.y - borderWidth;
    int w = widget->getWidth() + 2 * borderWidth;
    int h = widget->getHeight() + 2 * borderWidth;
    
    for (int i = 0; i < borderWidth; ++i) {
        SDL_Rect r = {x + i, y + i, w - 2*i, h - 2*i};
        SDL_FRect fr = {static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)};
        SDL_RenderRect(renderer, &fr);
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
    
    const EditorElement* elem = m_state.getSelectedElement();
    if (elem) {
        startDrag(mouseX, mouseY);
    }
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
        auto it = m_previewWindow->m_widgetMap.find(selectedIndex);
        if (it != m_previewWindow->m_widgetMap.end() && it->second) {
            it->second->setPosition(newX, newY);
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