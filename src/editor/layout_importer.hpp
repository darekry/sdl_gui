#pragma once

#include "editor_element.hpp"

import std.compat;

class LayoutImporter {
public:
    static std::vector<EditorElement> loadFromXML(const std::string& filePath);
    static std::vector<EditorElement> loadFromJSON(const std::string& filePath);

private:
    static void parseXMLElement(void* elementPtr, std::vector<EditorElement>& elements, const std::string& parentId);
    static void parseJSONElement(const void* valuePtr, std::vector<EditorElement>& elements, const std::string& parentId);
    static Style parseStyleFromXML(void* styleNode);
    static Style parseStyleFromJSON(const void* styleValue);
    static std::optional<SDL_Color> parseColor(const std::string& colorStr);
    static ElementState parseElementState(const std::string& stateStr);
};