#pragma once

#include <SDL2/SDL_pixels.h>
#include "../lib/tinyxml2.h"

import std.compat;

class GUIManager;
class GUIElement;

class SGMLParser
{
public:
    SGMLParser(GUIManager & guiManager);
    std::unique_ptr<GUIElement> loadLayout(const std::string & file_path);

private:
    GUIManager & m_guiManager;
    std::unique_ptr<GUIElement> parseNode(tinyxml2::XMLElement * xmlNode);
    void parseResources(tinyxml2::XMLElement * resourcesNode);
    void parseStyle(tinyxml2::XMLElement * styleNode, GUIElement * element);
    std::optional<SDL_Color> parseColor(const char * colorStr);
};
