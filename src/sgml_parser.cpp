#include "sgml_parser.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include <SDL.h>

SGMLParser::SGMLParser(GUIManager& guiManager) : m_guiManager(guiManager) {}

std::unique_ptr<GUIElement> SGMLParser::loadLayout(const std::string& file_path) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(file_path.c_str()) != tinyxml2::XML_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load layout file: %s", file_path.c_str());
        return nullptr;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to parse layout file: No root element in %s", file_path.c_str());
        return nullptr;
    }

    return parseNode(root);
}

std::unique_ptr<GUIElement> SGMLParser::parseNode(tinyxml2::XMLElement* xmlNode) {
    std::map<std::string, std::function<std::unique_ptr<GUIElement>(tinyxml2::XMLElement*)>> factory;

    factory["Panel"] = [this](tinyxml2::XMLElement* el) {
        return std::make_unique<Panel>(m_guiManager, 0, 0, 0, 0);
    };

    factory["Button"] = [this](tinyxml2::XMLElement* el) {
        const char* text = el->Attribute("text");
        return std::make_unique<Button>(m_guiManager, 0, 0, 0, 0, text ? text : "");
    };

    std::string tagName = xmlNode->Name();
    std::unique_ptr<GUIElement> element = nullptr;

    if (factory.count(tagName)) {
        element = factory[tagName](xmlNode);
    } else {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown GUI element type: %s", tagName.c_str());
        return nullptr;
    }

    if (element) {
        const char* id = xmlNode->Attribute("id");
        if (id) {
            element->setID(id);
        }

        int x = 0, y = 0, width = 100, height = 100;
        xmlNode->QueryIntAttribute("x", &x);
        xmlNode->QueryIntAttribute("y", &y);
        xmlNode->QueryIntAttribute("width", &width);
        xmlNode->QueryIntAttribute("height", &height);
        element->setPosition(x, y);
        element->setSize(width, height);

        for (tinyxml2::XMLElement* childNode = xmlNode->FirstChildElement(); childNode != nullptr; childNode = childNode->NextSiblingElement()) {
            auto childElement = parseNode(childNode);
            if (childElement) {
                element->addChild(std::move(childElement));
            }
        }
    }

    return element;
}