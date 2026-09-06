#include "layout_parser.hpp"
#include "anchor.hpp"
#include "animated_image.hpp"
#include "arc_container.hpp"
#include "button.hpp"
#include "canvas.hpp"
#include "checkbox.hpp"
#include "combobox.hpp"
#include "gui_manager.hpp"
#include "label.hpp"
#include "list_view.hpp"
#include "panel.hpp"
#include "progress_bar.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "range_slider.hpp"
#include "scroll_area.hpp"
#include "slider.hpp"
#include "string_grid.hpp"
#include "tab_control.hpp"
#include "text_area.hpp"
#include "text_input.hpp"
#include <SDL3/SDL.h>

#include "constants.hpp"
#include "std.hpp"

LayoutParser::LayoutParser(GUIManager& guiManager)
    : m_guiManager(guiManager)
{
}

std::unique_ptr<GUIElement> LayoutParser::loadLayout(const std::string& file_path)
{
    if (!loadFile(file_path))
    {
        LOG_ERROR("LayoutParser", "Failed to load layout file: {}", file_path);
        return nullptr;
    }

    void* root = getRootNode();
    if (!root) return nullptr;

    if (hasNode(root, "resources"))
        parseResources(getChild(root, "resources"));

    if (hasNode(root, "root"))
        return parseNode(getChild(root, "root"));

    return parseNode(root);
}

std::unique_ptr<GUIElement> LayoutParser::parseNode(void* node)
{
    if (!node) return nullptr;

    std::string type = getNodeName(node);
    if (type.empty()) return nullptr;

    std::unique_ptr<GUIElement> element = nullptr;

    // Docelowy rect od razu w konstruktorze — bez dummy (0,0) + późniejszego
    // setSize() (to dummy zostawiało labelki Buttona na ujemnych coords).
    const int x = getInt(node, "x", 0);
    const int y = getInt(node, "y", 0);
    const int w = getInt(node, "width", 0);
    const int h = getInt(node, "height", 0);

    if (type == "Panel")
    {
        auto p = std::make_unique<Panel>(m_guiManager, x, y, w, h);
        p->setDraggable(getBool(node, "draggable", false));
        element = std::move(p);
    }
    else if (type == "Button")
    {
        element = std::make_unique<Button>(m_guiManager, x, y, w, h, getString(node, "text", ""));
    }
    else if (type == "Label")
    {
        // Label sam wymiaruje się z tekstu — pozycja z atrybutów, rozmiar auto.
        element = std::make_unique<Label>(m_guiManager, x, y, getString(node, "text", ""), getInt(node, "fontSize", -1));
    }
    else if (type == "Checkbox")
    {
        auto c = std::make_unique<Checkbox>(m_guiManager, x, y, w, h);
        c->setChecked(getBool(node, "checked", false));
        element = std::move(c);
    }
    else if (type == "RadioButton")
    {
        auto rb = std::make_unique<RadioButton>(m_guiManager, x, y, w, h);
        rb->setSelected(getBool(node, "selected", false));
        element = std::move(rb);
    }
    else if (type == "RadioGroup")
    {
        auto rg = std::make_unique<RadioGroup>(m_guiManager, x, y, w, h);
        
        // Parse option configuration
        if (hasNode(node, "optionSpacing")) rg->setOptionSpacing(getInt(node, "optionSpacing", 40));
        if (hasNode(node, "buttonX") || hasNode(node, "labelX") || hasNode(node, "startY"))
            rg->setOptionMargins(getInt(node, "buttonX", 20), getInt(node, "labelX", 45), getInt(node, "startY", 20));
        if (hasNode(node, "buttonSize") || hasNode(node, "labelFontSize"))
            rg->setOptionSizes(getInt(node, "buttonSize", 20), getInt(node, "labelFontSize", 16));
        
        // Parse options array
        if (isArray(node, "options"))
        {
            forEachInArray(node, "options", [this, &rg](void* optNode) {
                std::string text = getString(optNode, "text", "");
                bool selected = getBool(optNode, "selected", false);
                if (!text.empty())
                    rg->addOption(text, selected);
            });
        }
        
        element = std::move(rg);
    }
    else if (type == "Slider")
    {
        Orientation orientation = getString(node, "orientation", "Horizontal") == "Vertical" ? Orientation::Vertical : Orientation::Horizontal;
        auto slider = std::make_unique<Slider>(m_guiManager, x, y, w > 0 ? w : 100, h > 0 ? h : 20, getInt(node, "min", 0), getInt(node, "max", 100), getInt(node, "value", 0), orientation);
        slider->setWheelStep(getInt(node, "wheelStep", 1));
        element = std::move(slider);
    }
    else if (type == "RangeSlider")
    {
        Orientation orientation = getString(node, "orientation", "Horizontal") == "Vertical" ? Orientation::Vertical : Orientation::Horizontal;
        auto rangeSlider = std::make_unique<RangeSlider>(m_guiManager, x, y, w > 0 ? w : 100, h > 0 ? h : 20, getInt(node, "min", 0), getInt(node, "max", 100), getInt(node, "lower", 0), getInt(node, "upper", 100), orientation);
        rangeSlider->setWheelStep(getInt(node, "wheelStep", 1));
        element = std::move(rangeSlider);
    }
    else if (type == "StringGrid")
    {
        auto grid = std::make_unique<StringGrid>(m_guiManager, x, y, w > 0 ? w : 400, h > 0 ? h : 300, static_cast<size_t>(getInt(node, "rowCount", 5)), static_cast<size_t>(getInt(node, "colCount", 5)));
        grid->setShowRowHeaders(getBool(node, "showRowHeaders", true));
        grid->setShowColumnHeaders(getBool(node, "showColumnHeaders", true));
        grid->setEditable(getBool(node, "editable", true));
        if (hasNode(node, "rowHeight")) grid->setRowHeight(getInt(node, "rowHeight", 24));
        if (hasNode(node, "headerHeight")) grid->setHeaderHeight(getInt(node, "headerHeight", 28));
        if (hasNode(node, "rowHeaderWidth")) grid->setRowHeaderWidth(getInt(node, "rowHeaderWidth", 50));
        if (hasNode(node, "hScrollEnabled")) grid->setHorizontalScrollEnabled(getBool(node, "hScrollEnabled", true));
        if (hasNode(node, "vScrollEnabled")) grid->setVerticalScrollEnabled(getBool(node, "vScrollEnabled", true));
        element = std::move(grid);
    }
    else if (type == "TextInput")
    {
        auto ti = std::make_unique<TextInput>(m_guiManager, x, y, w > 0 ? w : 100, h > 0 ? h : 30);
        if (hasNode(node, "text")) ti->setText(std::string_view(getString(node, "text")));
        ti->setLocked(getBool(node, "locked", false));
        element = std::move(ti);
    }
    else if (type == "TextArea")
    {
        auto ta = std::make_unique<TextArea>(m_guiManager, x, y, w > 0 ? w : 200, h > 0 ? h : 150, getString(node, "fontPath", constants::kDefaultFontPath), getInt(node, "fontSize", 16));
        if (hasNode(node, "text")) ta->setText(std::string_view(getString(node, "text")));
        ta->setWordWrap(getBool(node, "wordWrap", true));
        ta->setLocked(getBool(node, "locked", false));
        element = std::move(ta);
    }
    else if (type == "ComboBox")
    {
        auto cb = std::make_unique<ComboBox>(m_guiManager, x, y, w > 0 ? w : 150, h > 0 ? h : 30);
        if (isArray(node, "items"))
        {
            forEachInArray(node, "items", [this, &cb](void* itemNode) {
                cb->addItem(getString(itemNode, "text", ""));
            });
        }
        else if (hasNode(node, "items"))
        {
            std::string itemsAttr = getString(node, "items", "");
            if (!itemsAttr.empty())
            {
                std::stringstream ss(itemsAttr);
                std::string item;
                while (std::getline(ss, item, ','))
                {
                    size_t start = item.find_first_not_of(" \t");
                    size_t end = item.find_last_not_of(" \t");
                    if (start != std::string::npos && end != std::string::npos)
                        cb->addItem(item.substr(start, end - start + 1));
                }
            }
        }
        if (hasNode(node, "selectedIndex")) cb->setSelectedIndex(getInt(node, "selectedIndex", -1));
        element = std::move(cb);
    }
    else if (type == "TabControl")
    {
        auto tc = std::make_unique<TabControl>(m_guiManager, x, y, w > 0 ? w : 200, h > 0 ? h : 200, getInt(node, "tabHeight", 30));
        if (isArray(node, "tabs"))
        {
            forEachInArray(node, "tabs", [this, &tc](void* tabNode) {
                Panel* content = tc->addTab(getString(tabNode, "title", "Tab"), getInt(tabNode, "width", 100), getInt(tabNode, "height", -1));
                if (isArray(tabNode, "children"))
                {
                    forEachInArray(tabNode, "children", [this, content](void* childNode) {
                        auto childEl = parseNode(childNode);
                        if (childEl) content->addChild(std::move(childEl));
                    });
                }
            });
        }
        element = std::move(tc);
    }
    else if (type == "AnimatedImage")
    {
        auto ai = std::make_unique<AnimatedImage>(m_guiManager, x, y, w > 0 ? w : 100, h > 0 ? h : 100);
        if (hasNode(node, "path"))
            ai->setSpriteSheet(getString(node, "path"), getInt(node, "frames", 1), getInt(node, "rows", 1), getInt(node, "frameW", 0), getInt(node, "frameH", 0));
        if (hasNode(node, "frameDuration"))
            ai->setFrameDuration(getFloat(node, "frameDuration", 1.0f / 12.0f));
        else
            ai->setFPS(getFloat(node, "fps", 12.0f));
        ai->setLoop(getBool(node, "loop", true));
        ai->setUseCache(getBool(node, "useCache", true));
        ai->setPreserveAspect(getBool(node, "preserveAspect", true));
        if (hasNode(node, "scaleMode"))
        {
            std::string scaleModeStr = getString(node, "scaleMode", "Fit");
            if (scaleModeStr == "Center")
                ai->setScaleMode(AnimatedImage::ScaleMode::Center);
            else if (scaleModeStr == "None")
                ai->setScaleMode(AnimatedImage::ScaleMode::None);
            else
                ai->setScaleMode(AnimatedImage::ScaleMode::Fit);
        }
        if (getBool(node, "autoplay", true)) ai->play();
        element = std::move(ai);
    }
    else if (type == "Canvas")
    {
        element = std::make_unique<Canvas>(m_guiManager, x, y, w > 0 ? w : 100, h > 0 ? h : 100);
    }
    else if (type == "ProgressBar")
    {
        auto pb = std::make_unique<ProgressBar>(m_guiManager, x, y, w > 0 ? w : 200, h > 0 ? h : 30);
        pb->setRange(getFloat(node, "min", 0.0f), getFloat(node, "max", 100.0f));
        pb->setValue(getFloat(node, "value", 0.0f));
        pb->setOrientation(getString(node, "orientation", "Horizontal") == "Vertical" ? Orientation::Vertical : Orientation::Horizontal);
        pb->setShowText(getBool(node, "showText", true));
        if (hasNode(node, "textFormat")) pb->setTextFormat(getString(node, "textFormat", "%.0f%%"));
        element = std::move(pb);
    }
    else if (type == "ScrollArea")
    {
        auto sa = std::make_unique<ScrollArea>(m_guiManager, x, y, w > 0 ? w : 300, h > 0 ? h : 200);
        if (hasNode(node, "contentWidth") || hasNode(node, "contentHeight"))
            sa->setContentSize(getInt(node, "contentWidth", sa->getWidth()), getInt(node, "contentHeight", sa->getHeight()));
        if (hasNode(node, "vScrollEnabled")) sa->setVerticalScroll(getBool(node, "vScrollEnabled", true));
        if (hasNode(node, "hScrollEnabled")) sa->setHorizontalScroll(getBool(node, "hScrollEnabled", false));
        element = std::move(sa);
    }
    else if (type == "ArcContainer")
    {
        element = std::make_unique<ArcContainer>(m_guiManager, x, y, getInt(node, "radius", 100),
            getFloat(node, "startAngle", 0.0f), getFloat(node, "endAngle", 360.0f));
    }
    else if (type == "ListView")
    {
        auto lv = std::make_unique<ListView>(m_guiManager, x, y, w > 0 ? w : 200, h > 0 ? h : 200);
        if (hasNode(node, "rowHeight")) lv->setRowHeight(getInt(node, "rowHeight", 25));
        if (isArray(node, "items"))
        {
            forEachInArray(node, "items", [this, &lv](void* itemNode) {
                std::string text = getString(itemNode, "text", "");
                if (text.empty()) text = getDirectString(itemNode, "");
                if (!text.empty()) lv->addItem(text);
            });
        }
        else if (hasNode(node, "items"))
        {
            std::string itemsAttr = getString(node, "items", "");
            if (!itemsAttr.empty())
            {
                std::stringstream ss(itemsAttr);
                std::string item;
                while (std::getline(ss, item, ','))
                {
                    size_t start = item.find_first_not_of(" \t");
                    size_t end = item.find_last_not_of(" \t");
                    if (start != std::string::npos && end != std::string::npos)
                        lv->addItem(item.substr(start, end - start + 1));
                }
            }
        }
        if (hasNode(node, "selectedIndex"))
            lv->setSelectedRow(static_cast<size_t>(getInt(node, "selectedIndex", -1)));
        element = std::move(lv);
    }
    else if (type == "Style" || type == "Item" || type == "Resources" || type == "Option")
    {
        return nullptr;
    }
    else
    {
        LOG_WARNING("LayoutParser", "Unknown GUI element type: {}", type);
        return nullptr;
    }

    if (hasNode(node, "id")) element->setID(getString(node, "id"));

    // Anchor: jawne tryby per oś + marginesy w px (enum, bez magicznych floatów).
    // Kotwica aplikowana przy addChild/addElement — brak (0,0) przed resize.
    if (hasNode(node, "anchorH") || hasNode(node, "anchorV") ||
        hasNode(node, "marginLeft") || hasNode(node, "marginTop") ||
        hasNode(node, "marginRight") || hasNode(node, "marginBottom"))
    {
        element->setAnchor(parseAnchor(node));
    }
    
    if (hasNode(node, "visible")) element->setVisible(getBool(node, "visible", true));
    if (hasNode(node, "enabled")) element->setEnabled(getBool(node, "enabled", true));
    if (hasNode(node, "tooltip")) element->setTooltip(getString(node, "tooltip"));
    if (hasNode(node, "clipChildren")) element->setClipChildren(getBool(node, "clipChildren", true));
    if (hasNode(node, "rotation")) element->setRotation(static_cast<double>(getFloat(node, "rotation", 0.0f)));
    if (hasNode(node, "rotationCenterX") || hasNode(node, "rotationCenterY"))
        element->setRotationCenter(getInt(node, "rotationCenterX", -1), getInt(node, "rotationCenterY", -1));

    if (isArray(node, "styles"))
    {
        forEachInArray(node, "styles", [this, &element](void* styleNode) {
            parseStyle(styleNode, element.get());
        });
    }

    if (type == "ArcContainer")
    {
        auto* arcContainer = static_cast<ArcContainer*>(element.get());
        forEachInArray(node, "children", [this, arcContainer](void* childNode) {
            auto childElement = parseNode(childNode);
            if (childElement)
            {
                float angle = getFloat(childNode, "angle", 0.0f);
                bool rotateChild = getBool(childNode, "rotateChild", true);
                int offset = getInt(childNode, "offset", 0);
                arcContainer->addChildAtAngle(std::move(childElement), angle, rotateChild, offset);
            }
        });
    }
    else if (type == "ScrollArea")
    {
        std::vector<std::unique_ptr<GUIElement>> contentChildren;
        forEachInArray(node, "children", [this, &contentChildren](void* childNode) {
            auto childElement = parseNode(childNode);
            if (childElement) contentChildren.push_back(std::move(childElement));
        });
        if (!contentChildren.empty())
        {
            auto* sa = static_cast<ScrollArea*>(element.get());
            if (contentChildren.size() == 1)
                sa->setContent(std::move(contentChildren[0]));
            else
            {
                auto panel = std::make_unique<Panel>(m_guiManager, 0, 0, sa->getWidth(), sa->getHeight());
                for (auto& c : contentChildren)
                    panel->addChild(std::move(c));
                sa->setContent(std::move(panel));
            }
        }
    }
    else if (type != "TabControl")
    {
        forEachInArray(node, "children", [this, &element](void* childNode) {
            auto childElement = parseNode(childNode);
            if (childElement) element->addChild(std::move(childElement));
        });
    }

    return element;
}

Anchor LayoutParser::parseAnchor(void* node)
{
    HAnchor h = HAnchor::None;
    VAnchor v = VAnchor::None;

    if (hasNode(node, "anchorH"))
    {
        const std::string mode = getString(node, "anchorH");
        if (mode == "left") h = HAnchor::Left;
        else if (mode == "center") h = HAnchor::Center;
        else if (mode == "right") h = HAnchor::Right;
        else if (mode == "stretch") h = HAnchor::Stretch;
        else if (mode == "none") h = HAnchor::None;
        else LOG_WARNING("LayoutParser", "Unknown anchorH mode: {} (expected none|left|center|right|stretch)", mode);
    }
    if (hasNode(node, "anchorV"))
    {
        const std::string mode = getString(node, "anchorV");
        if (mode == "top") v = VAnchor::Top;
        else if (mode == "center") v = VAnchor::Center;
        else if (mode == "bottom") v = VAnchor::Bottom;
        else if (mode == "stretch") v = VAnchor::Stretch;
        else if (mode == "none") v = VAnchor::None;
        else LOG_WARNING("LayoutParser", "Unknown anchorV mode: {} (expected none|top|center|bottom|stretch)", mode);
    }

    return Anchor::pinned(h, v,
        getInt(node, "marginLeft", 0), getInt(node, "marginTop", 0),
        getInt(node, "marginRight", 0), getInt(node, "marginBottom", 0));
}

void LayoutParser::parseResources(void* resourcesNode)
{
    if (!resourcesNode) return;

    if (isArray(resourcesNode, "fonts"))
    {
        forEachInArray(resourcesNode, "fonts", [this](void* fontNode) {
            std::string path = getString(fontNode, "path", "");
            if (!path.empty()) m_guiManager.getFontManager().loadFont(path, getInt(fontNode, "size", 16));
        });
    }
    if (isArray(resourcesNode, "textures"))
    {
        forEachInArray(resourcesNode, "textures", [this](void* texNode) {
            std::string path = getString(texNode, "path", "");
            if (path.empty()) return;
            auto tex = m_guiManager.getTextureManager().loadTexture(path);
            if (hasNode(texNode, "key") && tex)
                m_guiManager.getTextureManager().addTexture(getString(texNode, "key"), tex);
        });
    }
}

void LayoutParser::parseStyle(void* styleNode, GUIElement* element)
{
    std::string stateStr = getString(styleNode, "state", "Normal");
    ElementState state = ElementState::Normal;
    if (stateStr == "Hover") state = ElementState::Hover;
    else if (stateStr == "Pressed") state = ElementState::Pressed;
    else if (stateStr == "Disabled") state = ElementState::Disabled;

    Style style;
    if (hasNode(styleNode, "backgroundColor")) { auto c = parseColor(getString(styleNode, "backgroundColor")); if (c) style.backgroundColor = c; }
    if (hasNode(styleNode, "textColor")) { auto c = parseColor(getString(styleNode, "textColor")); if (c) style.textColor = c; }
    if (hasNode(styleNode, "borderColor")) { auto c = parseColor(getString(styleNode, "borderColor")); if (c) style.borderColor = c; }
    if (hasNode(styleNode, "bevel"))
    {
        std::string bevelStr = getString(styleNode, "bevel");
        if (bevelStr == "Raised") applyBevelToStyle(style, BevelType::Raised);
        else if (bevelStr == "Sunken") applyBevelToStyle(style, BevelType::Sunken);
        else LOG_WARNING("LayoutParser", "Unknown bevel type: {} (expected Raised or Sunken)", bevelStr);
    }
    if (hasNode(styleNode, "borderColorOuterTopLeft")) { auto c = parseColor(getString(styleNode, "borderColorOuterTopLeft")); if (c) style.borderColorOuterTopLeft = c; }
    if (hasNode(styleNode, "borderColorOuterBottomRight")) { auto c = parseColor(getString(styleNode, "borderColorOuterBottomRight")); if (c) style.borderColorOuterBottomRight = c; }
    if (hasNode(styleNode, "borderColorInnerTopLeft")) { auto c = parseColor(getString(styleNode, "borderColorInnerTopLeft")); if (c) style.borderColorInnerTopLeft = c; }
    if (hasNode(styleNode, "borderColorInnerBottomRight")) { auto c = parseColor(getString(styleNode, "borderColorInnerBottomRight")); if (c) style.borderColorInnerBottomRight = c; }
    if (hasNode(styleNode, "borderWidth")) style.borderWidth = getInt(styleNode, "borderWidth", 0);
    if (hasNode(styleNode, "borderRadius")) style.borderRadius = getInt(styleNode, "borderRadius", 0);
    if (hasNode(styleNode, "fontSize")) style.fontSize = getInt(styleNode, "fontSize", 0);
    if (hasNode(styleNode, "fontName")) style.fontName = getString(styleNode, "fontName");
    if (hasNode(styleNode, "texture"))
    {
        std::string texKey = getString(styleNode, "texture");
        style.texture = m_guiManager.getTextureManager().hasTexture(texKey) 
            ? m_guiManager.getTextureManager().getTexture(texKey) 
            : m_guiManager.getTextureManager().loadTexture(texKey);
    }
    element->setStyle(state, style);
}

std::optional<SDL_Color> LayoutParser::parseColor(const std::string& colorStr)
{
    if (colorStr.empty()) return std::nullopt;
    std::stringstream ss(colorStr);
    int r, g, b, a = 255;
    char comma;
    if (ss >> r >> comma >> g >> comma >> b)
    {
        ss >> comma >> a;
        return SDL_Color{static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};
    }
    if (colorStr[0] == '#')
    {
        uint32_t val;
        std::stringstream hexSS;
        hexSS << std::hex << colorStr.substr(1);
        hexSS >> val;
        if (colorStr.length() == 7) return SDL_Color{static_cast<Uint8>((val >> 16) & 0xFF), static_cast<Uint8>((val >> 8) & 0xFF), static_cast<Uint8>(val & 0xFF), 255};
        if (colorStr.length() == 9) return SDL_Color{static_cast<Uint8>((val >> 24) & 0xFF), static_cast<Uint8>((val >> 16) & 0xFF), static_cast<Uint8>((val >> 8) & 0xFF), static_cast<Uint8>(val & 0xFF)};
    }
    return std::nullopt;
}