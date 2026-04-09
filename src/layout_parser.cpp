#include "layout_parser.hpp"
#include "animated_image.hpp"
#include "button.hpp"
#include "canvas.hpp"
#include "checkbox.hpp"
#include "combobox.hpp"
#include "gui_manager.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "slider.hpp"
#include "string_grid.hpp"
#include "tab_control.hpp"
#include "text_area.hpp"
#include "text_input.hpp"
#include <SDL2/SDL.h>

import std.compat;

LayoutParser::LayoutParser(GUIManager& guiManager)
    : m_guiManager(guiManager)
{
}

std::unique_ptr<GUIElement> LayoutParser::loadLayout(const std::string& file_path)
{
    if (!loadFile(file_path))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load layout file: %s", file_path.c_str());
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

    if (type == "Panel")
    {
        auto p = std::make_unique<Panel>(m_guiManager, 0, 0, 0, 0);
        p->setDraggable(getBool(node, "draggable", false));
        element = std::move(p);
    }
    else if (type == "Button")
    {
        element = std::make_unique<Button>(m_guiManager, 0, 0, 0, 0, getString(node, "text", ""));
    }
    else if (type == "Label")
    {
        element = std::make_unique<Label>(m_guiManager, 0, 0, getString(node, "text", ""), getInt(node, "fontSize", -1));
    }
    else if (type == "Checkbox")
    {
        auto c = std::make_unique<Checkbox>(m_guiManager, 0, 0, 0, 0);
        c->setChecked(getBool(node, "checked", false));
        element = std::move(c);
    }
    else if (type == "RadioButton")
    {
        auto rb = std::make_unique<RadioButton>(m_guiManager, 0, 0, 0, 0);
        rb->setSelected(getBool(node, "selected", false));
        element = std::move(rb);
    }
    else if (type == "RadioGroup")
    {
        element = std::make_unique<RadioGroup>(m_guiManager, 0, 0, 0, 0);
    }
    else if (type == "Slider")
    {
        Orientation orientation = getString(node, "orientation", "Horizontal") == "Vertical" ? Orientation::Vertical : Orientation::Horizontal;
        element = std::make_unique<Slider>(m_guiManager, 0, 0, 100, 20, getInt(node, "min", 0), getInt(node, "max", 100), getInt(node, "value", 0), orientation);
    }
    else if (type == "StringGrid")
    {
        auto grid = std::make_unique<StringGrid>(m_guiManager, 0, 0, 400, 300, static_cast<size_t>(getInt(node, "rowCount", 5)), static_cast<size_t>(getInt(node, "colCount", 5)));
        grid->setShowRowHeaders(getBool(node, "showRowHeaders", true));
        grid->setShowColumnHeaders(getBool(node, "showColumnHeaders", true));
        grid->setEditable(getBool(node, "editable", true));
        element = std::move(grid);
    }
    else if (type == "TextInput")
    {
        auto ti = std::make_unique<TextInput>(m_guiManager, 0, 0, 100, 30);
        if (hasNode(node, "text")) ti->setText(std::string_view(getString(node, "text")));
        ti->setLocked(getBool(node, "locked", false));
        element = std::move(ti);
    }
    else if (type == "TextArea")
    {
        auto ta = std::make_unique<TextArea>(m_guiManager, 0, 0, 200, 150, getString(node, "fontPath", "assets/fonts/font.ttf"), getInt(node, "fontSize", 16));
        if (hasNode(node, "text")) ta->setText(std::string_view(getString(node, "text")));
        ta->setWordWrap(getBool(node, "wordWrap", true));
        element = std::move(ta);
    }
    else if (type == "ComboBox")
    {
        auto cb = std::make_unique<ComboBox>(m_guiManager, 0, 0, 150, 30);
        if (isArray(node, "items"))
        {
            forEachInArray(node, "items", [this, &cb](void* itemNode) {
                cb->addItem(getString(itemNode, "text", ""));
            });
        }
        element = std::move(cb);
    }
    else if (type == "TabControl")
    {
        auto tc = std::make_unique<TabControl>(m_guiManager, 0, 0, 200, 200, getInt(node, "tabHeight", 30));
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
        auto ai = std::make_unique<AnimatedImage>(m_guiManager, 0, 0, 100, 100);
        if (hasNode(node, "path"))
            ai->setSpriteSheet(getString(node, "path"), getInt(node, "frames", 1), getInt(node, "rows", 1), getInt(node, "frameW", 0), getInt(node, "frameH", 0));
        ai->setFPS(getFloat(node, "fps", 12.0f));
        ai->setLoop(getBool(node, "loop", true));
        if (getBool(node, "autoplay", true)) ai->play();
        element = std::move(ai);
    }
    else if (type == "Canvas")
    {
        element = std::make_unique<Canvas>(m_guiManager, 0, 0, 100, 100);
    }
    else if (type == "Style" || type == "Item" || type == "Resources")
    {
        return nullptr;
    }
    else
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown GUI element type: %s", type.c_str());
        return nullptr;
    }

    if (element)
    {
        if (hasNode(node, "id")) element->setID(getString(node, "id"));
        element->setPosition(getInt(node, "x", element->getX()), getInt(node, "y", element->getY()));
        element->setSize(getInt(node, "width", element->getWidth()), getInt(node, "height", element->getHeight()));
        
        if (hasNode(node, "visible")) element->setVisible(getBool(node, "visible", true));
        if (hasNode(node, "enabled")) element->setEnabled(getBool(node, "enabled", true));

        if (isArray(node, "styles"))
        {
            forEachInArray(node, "styles", [this, &element](void* styleNode) {
                parseStyle(styleNode, element.get());
            });
        }

        if (type != "TabControl")
        {
            forEachInArray(node, "children", [this, &element](void* childNode) {
                auto childElement = parseNode(childNode);
                if (childElement) element->addChild(std::move(childElement));
            });
        }
    }

    return element;
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
    if (!styleNode || !element) return;

    std::string stateStr = getString(styleNode, "state", "Normal");
    ElementState state = ElementState::Normal;
    if (stateStr == "Hover") state = ElementState::Hover;
    else if (stateStr == "Pressed") state = ElementState::Pressed;
    else if (stateStr == "Disabled") state = ElementState::Disabled;

    Style style;
    if (hasNode(styleNode, "backgroundColor")) { auto c = parseColor(getString(styleNode, "backgroundColor")); if (c) style.backgroundColor = c; }
    if (hasNode(styleNode, "textColor")) { auto c = parseColor(getString(styleNode, "textColor")); if (c) style.textColor = c; }
    if (hasNode(styleNode, "borderColor")) { auto c = parseColor(getString(styleNode, "borderColor")); if (c) style.borderColor = c; }
    if (hasNode(styleNode, "borderWidth")) style.borderWidth = getInt(styleNode, "borderWidth", 0);
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