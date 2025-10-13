#ifndef SGML_PARSER_HPP
#define SGML_PARSER_HPP

#include <string>
#include <memory>
#include <functional>
#include <map>
#include "../lib/tinyxml2.h"

class GUIManager;
class GUIElement;

class SGMLParser {
public:
    SGMLParser(GUIManager& guiManager);
    std::unique_ptr<GUIElement> loadLayout(const std::string& file_path);

private:
    GUIManager& m_guiManager;
    std::unique_ptr<GUIElement> parseNode(tinyxml2::XMLElement* xmlNode);
};

#endif // SGML_PARSER_HPP