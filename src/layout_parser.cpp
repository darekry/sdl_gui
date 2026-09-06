#include "layout_parser.hpp"
#include "anchor.hpp"
#include "arc_container.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "scroll_area.hpp"
#include "tab_control.hpp"
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

    // Docelowy rect od razu w konstruktorze — bez dummy (0,0) + późniejszego
    // setSize() (to dummy zostawiało labelki Buttona na ujemnych coords).
    // Konstrukcja przez WidgetFactory — jeden rejestr typów współdzielony
    // z podglądem edytora, EditorState i C-API (koniec 3 kopii if type==).
    // Tutaj tylko atrybuty wspólne + dzieci strukturalne (zakładki, treść
    // scrolla, kąty łuku); cała reszta propsów w fillPropsFromNode().
    WidgetProps props;
    props.x = getInt(node, "x", 0);
    props.y = getInt(node, "y", 0);
    props.w = getInt(node, "width", 0);
    props.h = getInt(node, "height", 0);
    fillPropsFromNode(node, type, props);

    std::unique_ptr<GUIElement> element = WidgetFactory::create(m_guiManager, type, props);
    if (!element) {
        if (type == "Style" || type == "Item" || type == "Resources" || type == "Option") {
            return nullptr;
        }
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
    else if (type == "TabControl")
    {
        // Taby utworzyła fabryka (fillPropsFromNode); tutaj tylko dzieci
        // strukturalne — panele treści pobrane po indeksie, bez recreate.
        auto* tc = static_cast<TabControl*>(element.get());
        if (isArray(node, "tabs"))
        {
            size_t tabIndex = 0;
            forEachInArray(node, "tabs", [this, tc, &tabIndex](void* tabNode) {
                if (Panel* content = tc->getTabContent(tabIndex)) {
                    if (isArray(tabNode, "children"))
                    {
                        forEachInArray(tabNode, "children", [this, content](void* childNode) {
                            auto childEl = parseNode(childNode);
                            if (childEl) content->addChild(std::move(childEl));
                        });
                    }
                }
                ++tabIndex;
            });
        }
    }
    else
    {
        forEachInArray(node, "children", [this, &element](void* childNode) {
            auto childElement = parseNode(childNode);
            if (childElement) element->addChild(std::move(childElement));
        });
    }

    return element;
}

void LayoutParser::fillPropsFromNode(void* node, const std::string& type, WidgetProps& p) {
    auto splitComma = [](const std::string& s) {
        std::vector<std::string> out;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, ',')) {
            size_t start = item.find_first_not_of(" \t");
            size_t end = item.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                out.push_back(item.substr(start, end - start + 1));
            }
        }
        return out;
    };

    if (type == "Panel") {
        p.draggable = getBool(node, "draggable", false);
    } else if (type == "Button") {
        p.text = getString(node, "text", "");
    } else if (type == "Label") {
        // Label sam wymiaruje się z tekstu — pozycja z atrybutów, rozmiar auto.
        p.text = getString(node, "text", "");
        p.fontSize = getInt(node, "fontSize", -1);
    } else if (type == "Checkbox") {
        p.checked = getBool(node, "checked", false);
    } else if (type == "RadioButton") {
        p.selected = getBool(node, "selected", false);
    } else if (type == "RadioGroup") {
        if (hasNode(node, "optionSpacing")) {
            p.hasOptionSpacing = true;
            p.optionSpacing = getInt(node, "optionSpacing", 40);
        }
        if (hasNode(node, "buttonX") || hasNode(node, "labelX") || hasNode(node, "startY")) {
            p.hasOptionMargins = true;
            p.buttonX = getInt(node, "buttonX", 20);
            p.labelX = getInt(node, "labelX", 45);
            p.startY = getInt(node, "startY", 20);
        }
        if (hasNode(node, "buttonSize") || hasNode(node, "labelFontSize")) {
            p.hasOptionSizes = true;
            p.buttonSize = getInt(node, "buttonSize", 20);
            p.labelFontSize = getInt(node, "labelFontSize", 16);
        }
        if (isArray(node, "options")) {
            forEachInArray(node, "options", [this, &p](void* optNode) {
                WidgetOptionSpec opt;
                opt.text = getString(optNode, "text", "");
                opt.selected = getBool(optNode, "selected", false);
                if (!opt.text.empty()) p.options.push_back(std::move(opt));
            });
        }
    } else if (type == "Slider" || type == "RangeSlider") {
        p.vertical = getString(node, "orientation", "Horizontal") == "Vertical";
        p.minVal = getInt(node, "min", 0);
        p.maxVal = getInt(node, "max", 100);
        p.value = getInt(node, "value", 0);
        p.wheelStep = getInt(node, "wheelStep", 1);
        if (type == "RangeSlider") {
            p.lowerVal = getInt(node, "lower", 0);
            p.upperVal = getInt(node, "upper", 100);
        }
    } else if (type == "StringGrid") {
        p.rowCount = static_cast<size_t>(getInt(node, "rowCount", 5));
        p.colCount = static_cast<size_t>(getInt(node, "colCount", 5));
        p.showRowHeaders = getBool(node, "showRowHeaders", true);
        p.showColumnHeaders = getBool(node, "showColumnHeaders", true);
        p.editable = getBool(node, "editable", true);
        if (hasNode(node, "rowHeight")) p.rowHeight = getInt(node, "rowHeight", 24);
        if (hasNode(node, "headerHeight")) p.headerHeight = getInt(node, "headerHeight", 28);
        if (hasNode(node, "rowHeaderWidth")) p.rowHeaderWidth = getInt(node, "rowHeaderWidth", 50);
        if (hasNode(node, "hScrollEnabled")) {
            p.hasHScroll = true;
            p.hScrollEnabled = getBool(node, "hScrollEnabled", true);
        }
        if (hasNode(node, "vScrollEnabled")) {
            p.hasVScroll = true;
            p.vScrollEnabled = getBool(node, "vScrollEnabled", true);
        }
    } else if (type == "TextInput") {
        if (hasNode(node, "text")) p.text = getString(node, "text");
        p.locked = getBool(node, "locked", false);
    } else if (type == "TextArea") {
        if (hasNode(node, "text")) p.text = getString(node, "text");
        p.fontPath = getString(node, "fontPath", constants::kDefaultFontPath);
        p.fontSize = getInt(node, "fontSize", 16);
        p.wordWrap = getBool(node, "wordWrap", true);
        p.locked = getBool(node, "locked", false);
    } else if (type == "ComboBox") {
        if (isArray(node, "items")) {
            forEachInArray(node, "items", [this, &p](void* itemNode) {
                p.items.push_back(getString(itemNode, "text", ""));
            });
        } else if (hasNode(node, "items")) {
            p.items = splitComma(getString(node, "items", ""));
        }
        if (hasNode(node, "selectedIndex")) {
            p.hasSelectedIndex = true;
            p.selectedIndex = getInt(node, "selectedIndex", -1);
        }
    } else if (type == "TabControl") {
        p.tabHeight = getInt(node, "tabHeight", 30);
        if (isArray(node, "tabs")) {
            forEachInArray(node, "tabs", [this, &p](void* tabNode) {
                WidgetTabSpec tab;
                tab.title = getString(tabNode, "title", "Tab");
                tab.width = getInt(tabNode, "width", 100);
                tab.height = getInt(tabNode, "height", -1);
                p.tabs.push_back(std::move(tab));
            });
        }
    } else if (type == "AnimatedImage") {
        p.path = getString(node, "path", "");
        p.frames = getInt(node, "frames", 1);
        p.rows = getInt(node, "rows", 1);
        p.frameW = getInt(node, "frameW", 0);
        p.frameH = getInt(node, "frameH", 0);
        if (hasNode(node, "frameDuration")) {
            p.frameDuration = getFloat(node, "frameDuration", 1.0f / 12.0f);
        } else {
            p.fps = getFloat(node, "fps", 12.0f);
        }
        p.loop = getBool(node, "loop", true);
        p.useCache = getBool(node, "useCache", true);
        p.preserveAspect = getBool(node, "preserveAspect", true);
        p.scaleMode = getString(node, "scaleMode", "Fit");
        p.autoplay = getBool(node, "autoplay", true);
    } else if (type == "Canvas") {
        // Bez propsów.
    } else if (type == "ProgressBar") {
        p.minF = getFloat(node, "min", 0.0f);
        p.maxF = getFloat(node, "max", 100.0f);
        p.valueF = getFloat(node, "value", 0.0f);
        p.vertical = getString(node, "orientation", "Horizontal") == "Vertical";
        p.showText = getBool(node, "showText", true);
        if (hasNode(node, "textFormat")) {
            p.hasTextFormat = true;
            p.textFormat = getString(node, "textFormat", "%.0f%%");
        }
    } else if (type == "ScrollArea") {
        if (hasNode(node, "contentWidth")) p.contentWidth = getInt(node, "contentWidth", 0);
        if (hasNode(node, "contentHeight")) p.contentHeight = getInt(node, "contentHeight", 0);
        if (hasNode(node, "vScrollEnabled")) {
            p.hasVScroll = true;
            p.vScrollEnabled = getBool(node, "vScrollEnabled", true);
        }
        if (hasNode(node, "hScrollEnabled")) {
            p.hasHScroll = true;
            p.hScrollEnabled = getBool(node, "hScrollEnabled", false);
        }
    } else if (type == "ArcContainer") {
        p.radius = getInt(node, "radius", 100);
        p.startAngle = getFloat(node, "startAngle", 0.0f);
        p.endAngle = getFloat(node, "endAngle", 360.0f);
    } else if (type == "ListView") {
        if (hasNode(node, "rowHeight")) p.rowHeight = getInt(node, "rowHeight", 25);
        if (isArray(node, "items")) {
            forEachInArray(node, "items", [this, &p](void* itemNode) {
                std::string text = getString(itemNode, "text", "");
                if (text.empty()) text = getDirectString(itemNode, "");
                if (!text.empty()) p.items.push_back(text);
            });
        } else if (hasNode(node, "items")) {
            p.items = splitComma(getString(node, "items", ""));
        }
        if (hasNode(node, "selectedIndex")) {
            p.hasSelectedIndex = true;
            p.selectedIndex = getInt(node, "selectedIndex", -1);
        }
    }
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