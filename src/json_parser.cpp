#include "json_parser.hpp"
#include <SDL2/SDL.h>

import std.compat;

namespace
{
    json::Value parseJson(const std::string& content)
    {
        size_t pos = 0;
        auto skipWs = [&] { while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos]))) ++pos; };
        auto peek = [&] { return pos < content.size() ? content[pos] : '\0'; };
        auto get = [&] { return pos < content.size() ? content[pos++] : '\0'; };

        std::function<json::Value()> parseValue = [&]() -> json::Value {
            skipWs();
            char c = peek();
            if (c == 'n') { pos += 4; return nullptr; }
            if (c == 't') { pos += 4; return true; }
            if (c == 'f') { pos += 5; return false; }
            if (c == '"') {
                ++pos;
                std::string result;
                while (pos < content.size()) {
                    char ch = get();
                    if (ch == '"') break;
                    if (ch == '\\') {
                        char esc = get();
                        switch (esc) {
                            case '"': result += '"'; break;
                            case '\\': result += '\\'; break;
                            case '/': result += '/'; break;
                            case 'b': result += '\b'; break;
                            case 'f': result += '\f'; break;
                            case 'n': result += '\n'; break;
                            case 'r': result += '\r'; break;
                            case 't': result += '\t'; break;
                            case 'u': {
                                std::string hex = content.substr(pos, 4); pos += 4;
                                int cp = std::stoi(hex, nullptr, 16);
                                if (cp < 0x80) result += static_cast<char>(cp);
                                else if (cp < 0x800) { result += static_cast<char>(0xC0 | (cp >> 6)); result += static_cast<char>(0x80 | (cp & 0x3F)); }
                                else { result += static_cast<char>(0xE0 | (cp >> 12)); result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); result += static_cast<char>(0x80 | (cp & 0x3F)); }
                                break;
                            }
                            default: result += esc;
                        }
                    } else result += ch;
                }
                return result;
            }
            if (c == '[') {
                ++pos;
                json::Array arr;
                skipWs();
                if (peek() == ']') { ++pos; return arr; }
                while (true) {
                    arr.push_back(parseValue());
                    skipWs();
                    if (peek() == ']') { ++pos; break; }
                    if (peek() == ',') { ++pos; skipWs(); } else break;
                }
                return arr;
            }
            if (c == '{') {
                ++pos;
                json::Object obj;
                skipWs();
                if (peek() == '}') { ++pos; return obj; }
                while (true) {
                    skipWs();
                    if (peek() != '"') break;
                    ++pos;
                    std::string key;
                    while (pos < content.size()) {
                        char ch = get();
                        if (ch == '"') break;
                        if (ch == '\\') get();
                        else key += ch;
                    }
                    skipWs();
                    if (peek() == ':') ++pos;
                    obj[key] = parseValue();
                    skipWs();
                    if (peek() == '}') { ++pos; break; }
                    if (peek() == ',') ++pos; else break;
                }
                return obj;
            }
            if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
                size_t start = pos;
                if (peek() == '-') ++pos;
                while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos;
                bool isFloat = false;
                if (peek() == '.') { isFloat = true; ++pos; while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos; }
                if (peek() == 'e' || peek() == 'E') { isFloat = true; ++pos; if (peek() == '+' || peek() == '-') ++pos; while (std::isdigit(static_cast<unsigned char>(peek()))) ++pos; }
                std::string numStr = content.substr(start, pos - start);
                return isFloat ? json::Value(std::stod(numStr)) : json::Value(std::stoi(numStr));
            }
            return nullptr;
        };

        skipWs();
        return parseValue();
    }
}

JsonParser::JsonParser(GUIManager& guiManager)
    : LayoutParser(guiManager)
{
}

bool JsonParser::loadFile(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load JSON file: %s", file_path.c_str());
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    m_root = parseJson(buffer.str());
    return !m_root.isNull();
}

void* JsonParser::getRootNode()
{
    return static_cast<void*>(&m_root);
}

bool JsonParser::hasNode(void* node, const std::string& key)
{
    auto* v = static_cast<json::Value*>(node);
    return v && v->isObject() && v->getObject().count(key) > 0;
}

std::string JsonParser::getString(void* node, const std::string& key, const std::string& defaultVal)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return defaultVal;
    const auto& val = v->getObject().at(key);
    return val.isString() ? val.getString() : defaultVal;
}

int JsonParser::getInt(void* node, const std::string& key, int defaultVal)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return defaultVal;
    const auto& val = v->getObject().at(key);
    return val.isInt() ? val.getInt() : (val.isDouble() ? static_cast<int>(val.getDouble()) : defaultVal);
}

float JsonParser::getFloat(void* node, const std::string& key, float defaultVal)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return defaultVal;
    const auto& val = v->getObject().at(key);
    return val.isDouble() ? static_cast<float>(val.getDouble()) : (val.isInt() ? static_cast<float>(val.getInt()) : defaultVal);
}

bool JsonParser::getBool(void* node, const std::string& key, bool defaultVal)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return defaultVal;
    const auto& val = v->getObject().at(key);
    return val.isBool() ? val.getBool() : defaultVal;
}

bool JsonParser::isArray(void* node, const std::string& key)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return false;
    return v->getObject().at(key).isArray();
}

void JsonParser::forEachInArray(void* node, const std::string& key, std::function<void(void*)> callback)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return;
    const auto& arr = v->getObject().at(key);
    if (!arr.isArray()) return;
    for (auto& item : const_cast<json::Array&>(arr.getArray()))
        callback(static_cast<void*>(&item));
}

void* JsonParser::getChild(void* node, const std::string& key)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count(key)) return &m_nullValue;
    return const_cast<void*>(static_cast<const void*>(&v->getObject().at(key)));
}

std::string JsonParser::getNodeName(void* node)
{
    auto* v = static_cast<json::Value*>(node);
    if (!v || !v->isObject() || !v->getObject().count("type")) return "";
    const auto& val = v->getObject().at("type");
    return val.isString() ? val.getString() : "";
}