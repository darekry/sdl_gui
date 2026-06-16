#include "sgml_parser.hpp"
#include <SDL3/SDL.h>
#include "logger.hpp"

SGMLParser::SGMLParser(GUIManager& guiManager)
    : LayoutParser(guiManager)
{
}

bool SGMLParser::loadFile(const std::string& file_path)
{
    if (m_doc.LoadFile(file_path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        LOG_ERROR("SGMLParser", "Failed to load XML file: {}", file_path);
        return false;
    }
    m_root = m_doc.RootElement();
    return m_root != nullptr;
}

void* SGMLParser::getRootNode()
{
    if (!m_root) return nullptr;
    if (std::string(m_root->Name()) == "Layout")
    {
        for (tinyxml2::XMLElement* child = m_root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
            if (std::string(child->Name()) != "Resources")
                return child;
        return nullptr;
    }
    return m_root;
}

bool SGMLParser::hasNode(void* node, const std::string& key)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    if (key == "children") return elem->FirstChildElement() != nullptr;
    if (key == "styles") return elem->FirstChildElement("Style") != nullptr;
    if (key == "items") return elem->FirstChildElement("Item") != nullptr;
    if (key == "tabs") return elem->FirstChildElement("Tab") != nullptr;
    if (key == "fonts") return elem->FirstChildElement("Font") != nullptr;
    if (key == "textures") return elem->FirstChildElement("Texture") != nullptr;
    if (key == "resources") return elem->FirstChildElement("Resources") != nullptr;
    if (key == "options") return elem->FirstChildElement("Option") != nullptr;
    return elem->Attribute(key.c_str()) != nullptr;
}

std::string SGMLParser::getString(void* node, const std::string& key, const std::string& defaultVal)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    const char* val = elem->Attribute(key.c_str());
    return val ? std::string(val) : defaultVal;
}

int SGMLParser::getInt(void* node, const std::string& key, int defaultVal)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    int val = defaultVal;
    elem->QueryIntAttribute(key.c_str(), &val);
    return val;
}

float SGMLParser::getFloat(void* node, const std::string& key, float defaultVal)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    float val = defaultVal;
    elem->QueryFloatAttribute(key.c_str(), &val);
    return val;
}

bool SGMLParser::getBool(void* node, const std::string& key, bool defaultVal)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    bool val = defaultVal;
    elem->QueryBoolAttribute(key.c_str(), &val);
    return val;
}

bool SGMLParser::isArray(void* node, const std::string& key)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    if (key == "children") return elem->FirstChildElement() != nullptr;
    if (key == "styles") return elem->FirstChildElement("Style") != nullptr;
    if (key == "items") return elem->FirstChildElement("Item") != nullptr;
    if (key == "tabs") return elem->FirstChildElement("Tab") != nullptr;
    if (key == "fonts") return elem->FirstChildElement("Font") != nullptr;
    if (key == "textures") return elem->FirstChildElement("Texture") != nullptr;
    if (key == "options") return elem->FirstChildElement("Option") != nullptr;
    return false;
}

void SGMLParser::forEachInArray(void* node, const std::string& key, std::function<void(void*)> callback)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    std::string childTag = key;
    if (key == "children") childTag = "";
    else if (key == "styles") childTag = "Style";
    else if (key == "items") childTag = "Item";
    else if (key == "tabs") childTag = "Tab";
    else if (key == "fonts") childTag = "Font";
    else if (key == "textures") childTag = "Texture";
    else if (key == "options") childTag = "Option";

    if (childTag.empty())
    {
        for (tinyxml2::XMLElement* child = elem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
            if (std::string(child->Name()) != "Style")
                callback(child);
    }
    else
    {
        for (tinyxml2::XMLElement* child = elem->FirstChildElement(childTag.c_str()); child != nullptr; child = child->NextSiblingElement(childTag.c_str()))
            callback(child);
    }
}

void* SGMLParser::getChild(void* node, const std::string& key)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    if (key == "resources") return elem->FirstChildElement("Resources");
    if (key == "root") return elem->FirstChildElement();
    return elem->FirstChildElement(key.c_str());
}

std::string SGMLParser::getNodeName(void* node)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    return elem->Name() ? std::string(elem->Name()) : "";
}

std::string SGMLParser::getDirectString(void* node, const std::string& defaultVal)
{
    auto elem = static_cast<tinyxml2::XMLElement*>(node);
    const char* text = elem->GetText();
    if (text) return std::string(text);
    const char* valAttr = elem->Attribute("value");
    if (valAttr) return std::string(valAttr);
    return defaultVal;
}