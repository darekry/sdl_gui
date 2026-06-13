#pragma once

#include <SDL3/SDL_pixels.h>

import std.compat;

class GUIManager;
class GUIElement;

class LayoutParser
{
public:
    LayoutParser(GUIManager& guiManager);
    virtual ~LayoutParser() = default;

    std::unique_ptr<GUIElement> loadLayout(const std::string& file_path);

protected:
    virtual bool loadFile(const std::string& file_path) = 0;
    virtual void* getRootNode() = 0;
    virtual bool hasNode(void* node, const std::string& key) = 0;
    virtual std::string getString(void* node, const std::string& key, const std::string& defaultVal = "") = 0;
    virtual int getInt(void* node, const std::string& key, int defaultVal = 0) = 0;
    virtual float getFloat(void* node, const std::string& key, float defaultVal = 0.0f) = 0;
    virtual bool getBool(void* node, const std::string& key, bool defaultVal = false) = 0;
    virtual bool isArray(void* node, const std::string& key) = 0;
    virtual void forEachInArray(void* node, const std::string& key, std::function<void(void*)> callback) = 0;
    virtual void* getChild(void* node, const std::string& key) = 0;
    virtual std::string getNodeName(void* node) = 0;
    virtual std::string getDirectString(void* node, const std::string& defaultVal = "") = 0;

    GUIManager& m_guiManager;

private:
    std::unique_ptr<GUIElement> parseNode(void* node);
    void parseResources(void* resourcesNode);
    void parseStyle(void* styleNode, GUIElement* element);
    std::optional<SDL_Color> parseColor(const std::string& colorStr);
};