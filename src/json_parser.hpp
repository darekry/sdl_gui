#pragma once

#include "layout_parser.hpp"

import std.compat;

namespace json
{
    struct Value;
    using Array = std::vector<Value>;
    using Object = std::map<std::string, Value>;

    struct Value
    {
        std::variant<std::nullptr_t, bool, int, double, std::string, Array, Object> data;
        Value() : data(nullptr) {}
        Value(std::nullptr_t) : data(nullptr) {}
        Value(bool v) : data(v) {}
        Value(int v) : data(v) {}
        Value(double v) : data(v) {}
        Value(const char* v) : data(std::string(v)) {}
        Value(const std::string& v) : data(v) {}
        Value(Array v) : data(std::move(v)) {}
        Value(Object v) : data(std::move(v)) {}

        bool isNull() const { return data.index() == 0; }
        bool isBool() const { return data.index() == 1; }
        bool isInt() const { return data.index() == 2; }
        bool isDouble() const { return data.index() == 3; }
        bool isString() const { return data.index() == 4; }
        bool isArray() const { return data.index() == 5; }
        bool isObject() const { return data.index() == 6; }

        bool getBool() const { return std::get<bool>(data); }
        int getInt() const { return std::get<int>(data); }
        double getDouble() const { return std::get<double>(data); }
        const std::string& getString() const { return std::get<std::string>(data); }
        const Array& getArray() const { return std::get<Array>(data); }
        const Object& getObject() const { return std::get<Object>(data); }
    };
}

class JsonParser : public LayoutParser
{
public:
    JsonParser(GUIManager& guiManager);

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
    json::Value m_root;
    json::Value m_nullValue;
};