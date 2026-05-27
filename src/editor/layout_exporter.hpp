#pragma once

#include "editor_element.hpp"

import std.compat;
class LayoutExporter {
public:
    static bool saveToXML(const std::vector<EditorElement>& elements, const std::string& filePath);
    static bool saveToJSON(const std::vector<EditorElement>& elements, const std::string& filePath);

private:
    static void addElementToXML(void* doc, void* parent, const EditorElement& element, const std::vector<EditorElement>& allElements);
    static std::string elementToJSON(const EditorElement& element, const std::vector<EditorElement>& allElements);
    static std::vector<size_t> getRootIndices(const std::vector<EditorElement>& elements);
    static std::vector<size_t> getChildIndices(const std::string& parentId, const std::vector<EditorElement>& elements);
    static std::string elementStateToString(ElementState state);
    static std::string colorToString(const SDL_Color& c);
    static bool isNumericAttribute(const std::string& key, const std::string& value);
    static bool isBoolAttribute(const std::string& key);
};