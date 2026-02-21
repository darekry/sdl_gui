#include "sgml_parser.hpp"
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
#include "tab_control.hpp"
#include "text_area.hpp"
#include "text_input.hpp"
#include <SDL2/SDL.h>
#include <functional>
#include <map>
#include <sstream>

SGMLParser::SGMLParser(GUIManager & guiManager)
    : m_guiManager(guiManager)
{
}

std::unique_ptr<GUIElement> SGMLParser::loadLayout(const std::string & file_path)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(file_path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load layout file: %s", file_path.c_str());
        return nullptr;
    }

    tinyxml2::XMLElement * root = doc.RootElement();
    if (!root)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse layout file: No root element in %s", file_path.c_str());
        return nullptr;
    }

    // Check for global resources first
    if (root->Name() != nullptr && std::string(root->Name()) == "Layout")
    {
        tinyxml2::XMLElement * resources = root->FirstChildElement("Resources");
        if (resources != nullptr)
        {
            parseResources(resources);
        }

        // Find the first widget element
        for (tinyxml2::XMLElement * child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
        {
            if (child->Name() != nullptr && std::string(child->Name()) != "Resources")
            {
                return parseNode(child);
            }
        }
    }

    return parseNode(root);
}

std::unique_ptr<GUIElement> SGMLParser::parseNode(tinyxml2::XMLElement * xmlNode)
{
    if (!xmlNode)
        return nullptr;

    std::string tagName = xmlNode->Name();
    std::unique_ptr<GUIElement> element = nullptr;

    if (tagName == "Panel")
    {
        auto p = std::make_unique<Panel>(m_guiManager, 0, 0, 0, 0);
        bool draggable = false;
        xmlNode->QueryBoolAttribute("draggable", &draggable);
        p->setDraggable(draggable);
        element = std::move(p);
    }
    else if (tagName == "Button")
    {
        const char * text = xmlNode->Attribute("text");
        element = std::make_unique<Button>(m_guiManager, 0, 0, 0, 0, text ? text : "");
    }
    else if (tagName == "Label")
    {
        const char * text = xmlNode->Attribute("text");
        int fontSize = -1;
        xmlNode->QueryIntAttribute("fontSize", &fontSize);
        element = std::make_unique<Label>(m_guiManager, 0, 0, text ? text : "", fontSize);
    }
    else if (tagName == "Checkbox")
    {
        auto c = std::make_unique<Checkbox>(m_guiManager, 0, 0, 0, 0);
        bool checked = false;
        xmlNode->QueryBoolAttribute("checked", &checked);
        c->setChecked(checked);
        element = std::move(c);
    }
    else if (tagName == "RadioButton")
    {
        auto rb = std::make_unique<RadioButton>(m_guiManager, 0, 0, 0, 0);
        bool selected = false;
        xmlNode->QueryBoolAttribute("selected", &selected);
        rb->setSelected(selected);
        element = std::move(rb);
    }
    else if (tagName == "RadioGroup")
    {
        element = std::make_unique<RadioGroup>(m_guiManager, 0, 0, 0, 0);
    }
    else if (tagName == "Slider")
    {
        int minValue = 0;
        int maxValue = 100;
        int val = 0;
        xmlNode->QueryIntAttribute("min", &minValue);
        xmlNode->QueryIntAttribute("max", &maxValue);
        xmlNode->QueryIntAttribute("value", &val);
        const char * orient = xmlNode->Attribute("orientation");
        Orientation orientation = Orientation::Horizontal;
        if (orient && std::string(orient) == "Vertical")
        {
            orientation = Orientation::Vertical;
        }
        element = std::make_unique<Slider>(m_guiManager, 0, 0, 100, 20, minValue, maxValue, val, orientation);
    }
    else if (tagName == "TextInput")
    {
        auto ti = std::make_unique<TextInput>(m_guiManager, 0, 0, 100, 30);
        const char * text = xmlNode->Attribute("text");
        if (text)
        {
            ti->setText(std::string_view(text));
        }
        bool locked = false;
        xmlNode->QueryBoolAttribute("locked", &locked);
        ti->setLocked(locked);
        element = std::move(ti);
    }
    else if (tagName == "TextArea")
    {
        const char * fontPathText = xmlNode->Attribute("fontPath");
        int fontSizeText = 16;
        xmlNode->QueryIntAttribute("fontSize", &fontSizeText);
        auto ta = std::make_unique<TextArea>(m_guiManager, 0, 0, 200, 150, fontPathText ? fontPathText : "", fontSizeText);
        const char * text = xmlNode->Attribute("text");
        if (text)
        {
            ta->setText(std::string_view(text));
        }
        bool wrap = true;
        xmlNode->QueryBoolAttribute("wordWrap", &wrap);
        ta->setWordWrap(wrap);
        element = std::move(ta);
    }
    else if (tagName == "ComboBox")
    {
        auto cb = std::make_unique<ComboBox>(m_guiManager, 0, 0, 150, 30);
        for (tinyxml2::XMLElement * itemNode = xmlNode->FirstChildElement("Item"); itemNode != nullptr; itemNode = itemNode->NextSiblingElement("Item"))
        {
            const char * itemText = itemNode->Attribute("text");
            if (itemText)
            {
                cb->addItem(itemText);
            }
        }
        element = std::move(cb);
    }
    else if (tagName == "TabControl")
    {
        int tabHeight = 30;
        xmlNode->QueryIntAttribute("tabHeight", &tabHeight);
        auto tc = std::make_unique<TabControl>(m_guiManager, 0, 0, 200, 200, tabHeight);
        for (tinyxml2::XMLElement * tabNode = xmlNode->FirstChildElement("Tab"); tabNode != nullptr; tabNode = tabNode->NextSiblingElement("Tab"))
        {
            const char * title = tabNode->Attribute("title");
            int width = 100;
            int height = -1;
            tabNode->QueryIntAttribute("width", &width);
            tabNode->QueryIntAttribute("height", &height);

            Panel * content = tc->addTab(title ? title : "Tab", width, height);
            for (tinyxml2::XMLElement * child = tabNode->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
            {
                auto childEl = parseNode(child);
                if (childEl)
                {
                    content->addChild(std::move(childEl));
                }
            }
        }
        element = std::move(tc);
    }
    else if (tagName == "AnimatedImage")
    {
        auto ai = std::make_unique<AnimatedImage>(m_guiManager, 0, 0, 100, 100);
        const char * path = xmlNode->Attribute("path");
        int frames = 1;
        int rows = 1;
        int fw = 0;
        int fh = 0;
        xmlNode->QueryIntAttribute("frames", &frames);
        xmlNode->QueryIntAttribute("rows", &rows);
        xmlNode->QueryIntAttribute("frameW", &fw);
        xmlNode->QueryIntAttribute("frameH", &fh);
        if (path)
        {
            ai->setSpriteSheet(path, frames, rows, fw, fh);
        }
        float fps = 12.0f;
        xmlNode->QueryFloatAttribute("fps", &fps);
        ai->setFPS(fps);
        bool loop = true;
        xmlNode->QueryBoolAttribute("loop", &loop);
        ai->setLoop(loop);

        bool autoplay = true;  // Default to true
        xmlNode->QueryBoolAttribute("autoplay", &autoplay);
        if (autoplay)
        {
            ai->play();
        }

        element = std::move(ai);
    }
    else if (tagName == "Canvas")
    {
        element = std::make_unique<Canvas>(m_guiManager, 0, 0, 100, 100);
    }
    else if (tagName == "Style" || tagName == "Item" || tagName == "Resources")
    {
        return nullptr;  // Skip non-widget elements
    }
    else
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown GUI element type: %s", tagName.c_str());
        return nullptr;
    }

    if (element)
    {
        const char * idAttr = xmlNode->Attribute("id");
        if (idAttr)
        {
            element->setID(idAttr);
        }

        int x = element->getX();
        int y = element->getY();
        int width = element->getWidth();
        int height = element->getHeight();
        xmlNode->QueryIntAttribute("x", &x);
        xmlNode->QueryIntAttribute("y", &y);
        xmlNode->QueryIntAttribute("width", &width);
        xmlNode->QueryIntAttribute("height", &height);
        element->setPosition(x, y);
        element->setSize(width, height);

        bool visibleFlag = true;
        bool enabledFlag = true;
        if (xmlNode->QueryBoolAttribute("visible", &visibleFlag) == tinyxml2::XML_SUCCESS)
        {
            element->setVisible(visibleFlag);
        }
        if (xmlNode->QueryBoolAttribute("enabled", &enabledFlag) == tinyxml2::XML_SUCCESS)
        {
            element->setEnabled(enabledFlag);
        }

        // Parse styles
        for (tinyxml2::XMLElement * styleNode = xmlNode->FirstChildElement("Style"); styleNode != nullptr; styleNode = styleNode->NextSiblingElement("Style"))
        {
            parseStyle(styleNode, element.get());
        }

        // Parse children (only for non-tabs, tab children are handled in TabControl branch)
        if (tagName != "TabControl")
        {
            for (tinyxml2::XMLElement * childNode = xmlNode->FirstChildElement(); childNode != nullptr; childNode = childNode->NextSiblingElement())
            {
                auto childElement = parseNode(childNode);
                if (childElement)
                {
                    element->addChild(std::move(childElement));
                }
            }
        }
    }

    return element;
}

void SGMLParser::parseResources(tinyxml2::XMLElement * resourcesNode)
{
    for (tinyxml2::XMLElement * res = resourcesNode->FirstChildElement(); res != nullptr; res = res->NextSiblingElement())
    {
        std::string resType = res->Name();
        const char * resPath = res->Attribute("path");
        const char * resKey = res->Attribute("key");
        if (resPath == nullptr)
        {
            continue;
        }

        if (resType == "Font")
        {
            int fontSize = 16;
            res->QueryIntAttribute("size", &fontSize);
            m_guiManager.getFontManager().loadFont(resPath, fontSize);
        }
        else if (resType == "Texture")
        {
            auto tex = m_guiManager.getTextureManager().loadTexture(resPath);
            if (resKey != nullptr && tex)
            {
                m_guiManager.getTextureManager().addTexture(resKey, tex);
            }
        }
    }
}

void SGMLParser::parseStyle(tinyxml2::XMLElement * styleNode, GUIElement * element)
{
    const char * stateStr = styleNode->Attribute("state");
    ElementState state = ElementState::Normal;
    if (stateStr)
    {
        std::string s = stateStr;
        if (s == "Hover")
            state = ElementState::Hover;
        else if (s == "Pressed")
            state = ElementState::Pressed;
        else if (s == "Disabled")
            state = ElementState::Disabled;
    }

    Style style;
    if (auto color = parseColor(styleNode->Attribute("backgroundColor")))
        style.backgroundColor = color;
    if (auto color = parseColor(styleNode->Attribute("textColor")))
        style.textColor = color;
    if (auto color = parseColor(styleNode->Attribute("borderColor")))
        style.borderColor = color;

    int borderWidth = 0;
    if (styleNode->QueryIntAttribute("borderWidth", &borderWidth) == tinyxml2::XML_SUCCESS)
        style.borderWidth = borderWidth;

    int fontSize = 0;
    if (styleNode->QueryIntAttribute("fontSize", &fontSize) == tinyxml2::XML_SUCCESS)
        style.fontSize = fontSize;

    const char * fontName = styleNode->Attribute("fontName");
    if (fontName)
        style.fontName = fontName;

    const char * texKey = styleNode->Attribute("texture");
    if (texKey)
    {
        if (m_guiManager.getTextureManager().hasTexture(texKey))
        {
            style.texture = m_guiManager.getTextureManager().getTexture(texKey);
        }
        else
        {
            style.texture = m_guiManager.getTextureManager().loadTexture(texKey);
        }
    }

    element->setStyle(state, style);
}

std::optional<SDL_Color> SGMLParser::parseColor(const char * colorStr)
{
    if (!colorStr)
        return std::nullopt;

    std::string s = colorStr;
    if (s.empty())
        return std::nullopt;

    // Support for R,G,B,A format
    std::stringstream ss(s);
    int r, g, b, a = 255;
    char comma;
    if (ss >> r >> comma >> g >> comma >> b)
    {
        if (ss >> comma >> a)
        {
        }
        return SDL_Color { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };
    }

    // Support for Hex format #RRGGBB or #RRGGBBAA
    if (s[0] == '#')
    {
        uint32_t val;
        std::stringstream hexSS;
        hexSS << std::hex << s.substr(1);
        hexSS >> val;
        if (s.length() == 7)
        {  // #RRGGBB
            return SDL_Color { (Uint8)((val >> 16) & 0xFF), (Uint8)((val >> 8) & 0xFF), (Uint8)(val & 0xFF), 255 };
        }
        else if (s.length() == 9)
        {  // #RRGGBBAA
            return SDL_Color { (Uint8)((val >> 24) & 0xFF), (Uint8)((val >> 16) & 0xFF), (Uint8)((val >> 8) & 0xFF), (Uint8)(val & 0xFF) };
        }
    }

    return std::nullopt;
}