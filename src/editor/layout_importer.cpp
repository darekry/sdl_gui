#include "layout_importer.hpp"
#include "editor_utils.hpp"
#include "../../lib/tinyxml2.h"

#include "std.hpp"

// Wyciąga wartość "key": value z fragmentu JSON (obsługuje uciekane cudzysłowy w stringach)
static std::string extractKeyVal(const std::string& objContent, const std::string& key) {
    size_t keyPos = objContent.find('"' + key + '"');
    if (keyPos == std::string::npos) return "";
    size_t colonPos = objContent.find(':', keyPos);
    if (colonPos == std::string::npos) return "";
    size_t valStart = colonPos + 1;
    while (valStart < objContent.size() && std::isspace(static_cast<unsigned char>(objContent[valStart]))) valStart++;

    if (objContent[valStart] == '"') {
        valStart++;
        std::string val;
        while (valStart < objContent.size() && objContent[valStart] != '"') {
            if (objContent[valStart] == '\\' && valStart + 1 < objContent.size()) {
                valStart++;
                val += objContent[valStart++];
            } else {
                val += objContent[valStart++];
            }
        }
        return val;
    }

    std::string val;
    while (valStart < objContent.size() && !std::isspace(static_cast<unsigned char>(objContent[valStart])) && objContent[valStart] != ',' && objContent[valStart] != '}') {
        val += objContent[valStart++];
    }
    return val;
}
std::vector<EditorElement> LayoutImporter::loadFromXML(const std::string& filePath) {
    std::vector<EditorElement> elements;
    
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(filePath.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        return elements;
    }
    
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root) return elements;
    
    const char* rootName = root->Name();
    if (!rootName || std::string(rootName) != "Layout") return elements;
    
    for (tinyxml2::XMLElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
        const char* childName = child->Name();
        if (!childName) continue;
        
        std::string name(childName);
        if (name == "Resources") continue;
        
        parseXMLElement(child, elements, "");
    }
    
    return elements;
}

std::vector<EditorElement> LayoutImporter::loadFromJSON(const std::string& filePath) {
    std::vector<EditorElement> elements;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return elements;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    if (content.empty()) return elements;
    
    size_t pos = 0;
    
    auto skipWhitespace = [&]() {
        while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos]))) pos++;
    };
    
    auto parseString = [&]() -> std::string {
        skipWhitespace();
        if (pos >= content.size() || content[pos] != '"') return "";
        pos++;
        std::string result;
        while (pos < content.size() && content[pos] != '"') {
            if (content[pos] == '\\' && pos + 1 < content.size()) {
                pos++;
                result += content[pos];
            } else {
                result += content[pos];
            }
            pos++;
        }
        if (pos < content.size()) pos++;
        return result;
    };
    
    auto parseNumber = [&]() -> std::string {
        skipWhitespace();
        std::string result;
        while (pos < content.size() && (std::isdigit(static_cast<unsigned char>(content[pos])) || content[pos] == '-' || content[pos] == '+' || content[pos] == '.')) {
            result += content[pos];
            pos++;
        }
        return result;
    };
    
    auto parseValue = [&]() -> std::string {
        skipWhitespace();
        if (pos >= content.size()) return "";
        
        if (content[pos] == '"') return parseString();
        if (content[pos] == '-' || std::isdigit(static_cast<unsigned char>(content[pos]))) return parseNumber();
        if (content.substr(pos, 4) == "true") { pos += 4; return "true"; }
        if (content.substr(pos, 5) == "false") { pos += 5; return "false"; }
        if (content.substr(pos, 4) == "null") { pos += 4; return ""; }
        return "";
    };
    
    auto parseObject = [&](size_t start, const std::string& parentId) {
        EditorElement elem;
        elem.parentId = parentId;
        
        skipWhitespace();
        size_t objStart = start;
        if (content[objStart] != '{') return elem;
        
        size_t bracketCount = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < content.size() && bracketCount > 0) {
            if (content[objEnd] == '{') bracketCount++;
            else if (content[objEnd] == '}') bracketCount--;
            objEnd++;
        }
        
        std::string objContent = content.substr(objStart, objEnd - objStart);
        
        elem.type = extractKeyVal(objContent, "type");
        elem.id = extractKeyVal(objContent, "id");
        elem.x = safeParseInt(extractKeyVal(objContent, "x"), 0);
        elem.y = safeParseInt(extractKeyVal(objContent, "y"), 0);
        elem.width = safeParseInt(extractKeyVal(objContent, "width"), 100);
        elem.height = safeParseInt(extractKeyVal(objContent, "height"), 50);
        
        static const std::set<std::string> standardKeys = {"type", "id", "x", "y", "width", "height", "children", "styles"};
        static const std::set<std::string> numericKeys = {"fontSize", "borderWidth", "borderRadius", "min", "max", "value",
            "rowCount", "colCount", "tabHeight", "rowHeight", "selectedIndex", "frames", "rows", "frameW", "frameH", "fps"};
        static const std::set<std::string> boolKeys = {"draggable", "checked", "selected", "visible", "enabled",
            "showRowHeaders", "showColumnHeaders", "editable", "locked", "wordWrap", "loop", "autoplay"};
        
        size_t searchPos = 0;
        while (searchPos < objContent.size()) {
            size_t keyQuote = objContent.find('"', searchPos);
            if (keyQuote == std::string::npos) break;
            
            size_t keyEndQuote = objContent.find('"', keyQuote + 1);
            if (keyEndQuote == std::string::npos) break;
            
            std::string key = objContent.substr(keyQuote + 1, keyEndQuote - keyQuote - 1);
            
            if (standardKeys.find(key) != standardKeys.end()) {
                searchPos = keyEndQuote + 1;
                continue;
            }
            
            size_t colonPos = objContent.find(':', keyEndQuote);
            if (colonPos == std::string::npos) break;
            
            size_t valStart = colonPos + 1;
            while (valStart < objContent.size() && std::isspace(static_cast<unsigned char>(objContent[valStart]))) valStart++;
            
            std::string value;
            if (objContent[valStart] == '"') {
                valStart++;
                while (valStart < objContent.size() && objContent[valStart] != '"') {
                    if (objContent[valStart] == '\\' && valStart + 1 < objContent.size()) valStart++;
                    value += objContent[valStart++];
                }
            } else if (objContent[valStart] == '-' || std::isdigit(static_cast<unsigned char>(objContent[valStart]))) {
                while (valStart < objContent.size() && (std::isdigit(static_cast<unsigned char>(objContent[valStart])) || objContent[valStart] == '-' || objContent[valStart] == '.')) {
                    value += objContent[valStart++];
                }
            } else if (objContent.substr(valStart, 4) == "true") {
                value = "true";
                valStart += 4;
            } else if (objContent.substr(valStart, 5) == "false") {
                value = "false";
                valStart += 5;
            }
            
            elem.setProperty(key, value);
            searchPos = valStart + 1;
        }
        
        size_t childrenPos = objContent.find("\"children\"");
        if (childrenPos != std::string::npos) {
            size_t arrStart = objContent.find('[', childrenPos);
            if (arrStart != std::string::npos) {
                size_t arrEnd = arrStart + 1;
                size_t arrBracketCount = 1;
                while (arrEnd < objContent.size() && arrBracketCount > 0) {
                    if (objContent[arrEnd] == '[') arrBracketCount++;
                    else if (objContent[arrEnd] == ']') arrBracketCount--;
                    arrEnd++;
                }
                
                size_t childPos = arrStart + 1;
                while (childPos < arrEnd) {
                    while (childPos < arrEnd && std::isspace(static_cast<unsigned char>(objContent[childPos]))) childPos++;
                    if (childPos >= arrEnd) break;
                    if (objContent[childPos] == '{') {
                        size_t childObjStart = childPos;
                        int objBracketCount = 1;
                        size_t childObjEnd = childPos + 1;
                        while (childObjEnd < arrEnd && objBracketCount > 0) {
                            if (objContent[childObjEnd] == '{') objBracketCount++;
                            else if (objContent[childObjEnd] == '}') objBracketCount--;
                            childObjEnd++;
                        }
                        
                        EditorElement childElem;
                        childElem.parentId = elem.id;
                        
                        std::string childObjContent = objContent.substr(childObjStart, childObjEnd - childObjStart);
                        
                        childElem.type = extractKeyVal(childObjContent, "type");
                        childElem.id = extractKeyVal(childObjContent, "id");
                        childElem.x = safeParseInt(extractKeyVal(childObjContent, "x"), 0);
                        childElem.y = safeParseInt(extractKeyVal(childObjContent, "y"), 0);
                        childElem.width = safeParseInt(extractKeyVal(childObjContent, "width"), 100);
                        childElem.height = safeParseInt(extractKeyVal(childObjContent, "height"), 50);
                        
                        elements.push_back(childElem);
                        
                        childPos = childObjEnd + 1;
                    } else {
                        childPos++;
                    }
                }
            }
        }
        
        return elem;
    };
    
    skipWhitespace();
    if (pos >= content.size() || content[pos] != '{') return elements;
    pos++;
    
    while (pos < content.size()) {
        skipWhitespace();
        if (content[pos] == '}') break;
        if (content[pos] == ',') { pos++; continue; }
        
        std::string key = parseString();
        skipWhitespace();
        if (pos < content.size() && content[pos] == ':') pos++;
        skipWhitespace();
        
        if (key == "root") {
            if (content[pos] == '{') {
                EditorElement rootElem = parseObject(pos, "");
                pos++;
                
                if (!rootElem.type.empty()) {
                    elements.push_back(rootElem);
                }
            }
        } else if (key == "resources") {
            skipWhitespace();
            if (content[pos] == '{') {
                int bracketCount = 1;
                pos++;
                while (pos < content.size() && bracketCount > 0) {
                    if (content[pos] == '{') bracketCount++;
                    else if (content[pos] == '}') bracketCount--;
                    pos++;
                }
            }
        } else {
            skipWhitespace();
            if (content[pos] == '{') {
                int bracketCount = 1;
                pos++;
                while (pos < content.size() && bracketCount > 0) {
                    if (content[pos] == '{') bracketCount++;
                    else if (content[pos] == '}') bracketCount--;
                    pos++;
                }
            } else if (content[pos] == '[') {
                int bracketCount = 1;
                pos++;
                while (pos < content.size() && bracketCount > 0) {
                    if (content[pos] == '[') bracketCount++;
                    else if (content[pos] == ']') bracketCount--;
                    pos++;
                }
            } else {
                parseValue();
            }
        }
    }
    
    return elements;
}

void LayoutImporter::parseXMLElement(void* elementPtr, std::vector<EditorElement>& elements, const std::string& parentId) {
    auto* elemNode = static_cast<tinyxml2::XMLElement*>(elementPtr);
    
    EditorElement elem;
    elem.parentId = parentId;
    
    const char* typeName = elemNode->Name();
    if (!typeName) return;
    elem.type = typeName;
    
    const char* idAttr = elemNode->Attribute("id");
    elem.id = idAttr ? idAttr : "";
    
    elem.x = elemNode->IntAttribute("x", 0);
    elem.y = elemNode->IntAttribute("y", 0);
    elem.width = elemNode->IntAttribute("width", 100);
    elem.height = elemNode->IntAttribute("height", 50);
    
    static const std::set<std::string> standardAttrs = {"id", "x", "y", "width", "height"};
    static const std::set<std::string> numericAttrs = {"fontSize", "borderWidth", "borderRadius", "min", "max", "value",
        "rowCount", "colCount", "tabHeight", "rowHeight", "selectedIndex", "frames", "rows", "frameW", "frameH", "fps"};
    
    for (const tinyxml2::XMLAttribute* attr = elemNode->FirstAttribute(); attr; attr = attr->Next()) {
        const char* attrName = attr->Name();
        if (!attrName) continue;
        
        std::string name(attrName);
        if (standardAttrs.find(name) != standardAttrs.end()) continue;
        
        const char* attrValue = attr->Value();
        if (attrValue) {
            elem.setProperty(name, std::string(attrValue));
        }
    }
    
    for (tinyxml2::XMLElement* child = elemNode->FirstChildElement(); child; child = child->NextSiblingElement()) {
        const char* childName = child->Name();
        if (!childName) continue;
        
        std::string name(childName);
        
        if (name == "Style") {
            Style style = parseStyleFromXML(child);
            std::string stateStr = child->Attribute("state") ? child->Attribute("state") : "Normal";
            ElementState state = parseElementState(stateStr);
            elem.setStyle(state, style);
        } else {
            parseXMLElement(child, elements, elem.id);
        }
    }
    
    elements.push_back(elem);
}

Style LayoutImporter::parseStyleFromXML(void* styleNode) {
    auto* node = static_cast<tinyxml2::XMLElement*>(styleNode);
    Style style;
    
    const char* bgColor = node->Attribute("backgroundColor");
    if (bgColor) {
        auto color = parseColor(std::string(bgColor));
        if (color) style.backgroundColor = color;
    }
    
    const char* textColor = node->Attribute("textColor");
    if (textColor) {
        auto color = parseColor(std::string(textColor));
        if (color) style.textColor = color;
    }
    
    const char* borderColor = node->Attribute("borderColor");
    if (borderColor) {
        auto color = parseColor(std::string(borderColor));
        if (color) style.borderColor = color;
    }
    
    int borderWidth = node->IntAttribute("borderWidth", -1);
    if (borderWidth >= 0) style.borderWidth = borderWidth;
    
    int borderRadius = node->IntAttribute("borderRadius", -1);
    if (borderRadius >= 0) style.borderRadius = borderRadius;
    
    int fontSize = node->IntAttribute("fontSize", -1);
    if (fontSize >= 0) style.fontSize = fontSize;
    
    const char* fontName = node->Attribute("fontName");
    if (fontName) style.fontName = std::string(fontName);
    
    return style;
}

std::optional<SDL_Color> LayoutImporter::parseColor(const std::string& colorStr) {
    if (colorStr.empty()) return std::nullopt;
    
    SDL_Color color = {0, 0, 0, 255};
    
    if (colorStr.size() >= 6 && colorStr[0] == '#' && colorStr.size() <= 9) {
        std::string hex = colorStr.substr(1);
        if (hex.size() >= 6) {
            color.r = static_cast<Uint8>(std::stoi(hex.substr(0, 2), nullptr, 16));
            color.g = static_cast<Uint8>(std::stoi(hex.substr(2, 2), nullptr, 16));
            color.b = static_cast<Uint8>(std::stoi(hex.substr(4, 2), nullptr, 16));
            if (hex.size() >= 8) {
                color.a = static_cast<Uint8>(std::stoi(hex.substr(6, 2), nullptr, 16));
            }
        }
        return color;
    }
    
    std::vector<int> parts;
    std::string part;
    for (char c : colorStr) {
        if (c == ',' || c == ' ') {
            if (!part.empty()) {
                try { parts.push_back(std::stoi(part)); } catch (...) {}
                part.clear();
            }
        } else {
            part += c;
        }
    }
    if (!part.empty()) {
        try { parts.push_back(std::stoi(part)); } catch (...) {}
    }
    
    if (parts.size() >= 3) {
        color.r = static_cast<Uint8>(std::clamp(parts[0], 0, 255));
        color.g = static_cast<Uint8>(std::clamp(parts[1], 0, 255));
        color.b = static_cast<Uint8>(std::clamp(parts[2], 0, 255));
        if (parts.size() >= 4) {
            color.a = static_cast<Uint8>(std::clamp(parts[3], 0, 255));
        }
        return color;
    }
    
    return std::nullopt;
}

ElementState LayoutImporter::parseElementState(const std::string& stateStr) {
    if (stateStr == "Hover") return ElementState::Hover;
    if (stateStr == "Pressed") return ElementState::Pressed;
    if (stateStr == "Disabled") return ElementState::Disabled;
    return ElementState::Normal;
}