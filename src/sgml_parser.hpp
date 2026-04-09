#pragma once

#include "layout_parser.hpp"
#include "../lib/tinyxml2.h"

class SGMLParser : public LayoutParser
{
public:
    SGMLParser(GUIManager& guiManager);

protected:
    bool loadFile(const std::string& file_path) override;
    void* getRootNode() override;
    bool hasNode(void* node, const std::string& key) override;
    std::string getString(void* node, const std::string& key, const std::string& defaultVal) override;
    int getInt(void* node, const std::string& key, int defaultVal) override;
    float getFloat(void* node, const std::string& key, float defaultVal) override;
    bool getBool(void* node, const std::string& key, bool defaultVal) override;
    bool isArray(void* node, const std::string& key) override;
    void forEachInArray(void* node, const std::string& key, std::function<void(void*)> callback) override;
    void* getChild(void* node, const std::string& key) override;
    std::string getNodeName(void* node) override;

private:
    tinyxml2::XMLDocument m_doc;
    tinyxml2::XMLElement* m_root = nullptr;
};