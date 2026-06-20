#pragma once

#include "../style.hpp"

#include "std.hpp"

struct EditorElement {
    std::string id;
    std::string type;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    std::string parentId;
    std::map<std::string, std::string> properties;
    std::map<ElementState, Style> styles;

    EditorElement() = default;
    
    EditorElement(std::string_view id_, std::string_view type_, int x_, int y_, int w_, int h_)
        : id(id_), type(type_), x(x_), y(y_), width(w_), height(h_) {}

    [[nodiscard]] std::string getProperty(const std::string& key, const std::string& defaultValue = "") const {
        auto it = properties.find(key);
        return (it != properties.end()) ? it->second : defaultValue;
    }

    void setProperty(const std::string& key, const std::string& value) {
        properties[key] = value;
    }

    [[nodiscard]] bool hasProperty(const std::string& key) const {
        return properties.find(key) != properties.end();
    }

    void removeProperty(const std::string& key) {
        properties.erase(key);
    }

    [[nodiscard]] bool hasStyle(ElementState state) const {
        return styles.find(state) != styles.end();
    }

    void setStyle(ElementState state, const Style& style) {
        styles[state] = style;
    }

    [[nodiscard]] std::string toXML() const {
        std::ostringstream oss;
        oss << "<" << type;
        oss << " id=\"" << id << "\"";
        oss << " x=\"" << x << "\"";
        oss << " y=\"" << y << "\"";
        oss << " width=\"" << width << "\"";
        oss << " height=\"" << height << "\"";
        
        for (const auto& [key, value] : properties) {
            oss << " " << key << "=\"" << value << "\"";
        }
        
        if (styles.empty()) {
            oss << "/>";
        } else {
            oss << ">\n";
            for (const auto& [state, style] : styles) {
                oss << styleToXML(state, style);
            }
            oss << "</" << type << ">";
        }
        
        return oss.str();
    }

    [[nodiscard]] std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"type\":\"" << type << "\"";
        oss << ",\"id\":\"" << id << "\"";
        oss << ",\"x\":" << x;
        oss << ",\"y\":" << y;
        oss << ",\"width\":" << width;
        oss << ",\"height\":" << height;
        
        for (const auto& [key, value] : properties) {
            oss << ",\"" << key << "\":";
            if (isNumericValue(value)) {
                oss << value;
            } else if (value == "true" || value == "false") {
                oss << value;
            } else {
                oss << "\"" << value << "\"";
            }
        }
        
        if (!styles.empty()) {
            oss << ",\"styles\":[";
            bool first = true;
            for (const auto& [state, style] : styles) {
                if (!first) oss << ",";
                first = false;
                oss << styleToJSON(state, style);
            }
            oss << "]";
        }
        
        oss << "}";
        return oss.str();
    }

private:
    [[nodiscard]] static bool isNumericValue(const std::string& value) {
        if (value.empty()) return false;
        try {
            std::size_t pos = 0;
            static_cast<void>(std::stoi(value, &pos));
            return pos == value.length();
        } catch (...) {
            return false;
        }
    }

    [[nodiscard]] static std::string elementStateToString(ElementState state) {
        switch (state) {
            case ElementState::Normal: return "Normal";
            case ElementState::Hover: return "Hover";
            case ElementState::Pressed: return "Pressed";
            case ElementState::Disabled: return "Disabled";
            default: return "Normal";
        }
    }

    [[nodiscard]] static std::string colorToString(const SDL_Color& c) {
        std::ostringstream oss;
        oss << static_cast<int>(c.r) << "," 
            << static_cast<int>(c.g) << "," 
            << static_cast<int>(c.b) << "," 
            << static_cast<int>(c.a);
        return oss.str();
    }

    [[nodiscard]] static std::string styleToXML(ElementState state, const Style& style) {
        std::ostringstream oss;
        oss << "<Style state=\"" << elementStateToString(state) << "\"";
        
        if (style.backgroundColor) oss << " backgroundColor=\"" << colorToString(*style.backgroundColor) << "\"";
        if (style.textColor) oss << " textColor=\"" << colorToString(*style.textColor) << "\"";
        if (style.borderColor) oss << " borderColor=\"" << colorToString(*style.borderColor) << "\"";
        if (style.borderWidth) oss << " borderWidth=\"" << *style.borderWidth << "\"";
        if (style.borderRadius) oss << " borderRadius=\"" << *style.borderRadius << "\"";
        if (style.fontSize) oss << " fontSize=\"" << *style.fontSize << "\"";
        if (style.fontName) oss << " fontName=\"" << *style.fontName << "\"";
        
        oss << "/>\n";
        return oss.str();
    }

    [[nodiscard]] static std::string styleToJSON(ElementState state, const Style& style) {
        std::ostringstream oss;
        oss << "{\"state\":\"" << elementStateToString(state) << "\"";
        
        if (style.backgroundColor) oss << ",\"backgroundColor\":\"" << colorToString(*style.backgroundColor) << "\"";
        if (style.textColor) oss << ",\"textColor\":\"" << colorToString(*style.textColor) << "\"";
        if (style.borderColor) oss << ",\"borderColor\":\"" << colorToString(*style.borderColor) << "\"";
        if (style.borderWidth) oss << ",\"borderWidth\":" << *style.borderWidth;
        if (style.borderRadius) oss << ",\"borderRadius\":" << *style.borderRadius;
        if (style.fontSize) oss << ",\"fontSize\":" << *style.fontSize;
        if (style.fontName) oss << ",\"fontName\":\"" << *style.fontName << "\"";
        
        oss << "}";
        return oss.str();
    }
};