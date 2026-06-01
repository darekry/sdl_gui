#pragma once

#include "style.hpp"

import std.compat;

struct ThemeTypeCompare {
    using is_transparent = void;
    
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return lhs < rhs;
    }
    
    bool operator()(std::string_view lhs, const std::string& rhs) const {
        return lhs < rhs;
    }
    
    bool operator()(const std::string& lhs, std::string_view rhs) const {
        return lhs < rhs;
    }
};

class Theme {
public:
    void setStyle(std::string_view type, ElementState state, Style style);
    
    Style getStyle(std::string_view type, ElementState state) const;
    
    void setStyle(std::string_view type, Style style);
    
    Style getStyle(std::string_view type) const;
    
    void setDefaultStyle(Style style);
    const Style& getDefaultStyle() const;
    
    static Theme createDefaultTheme();

private:
    std::map<std::string, std::map<ElementState, Style>, ThemeTypeCompare> m_typeStyles;
    Style m_defaultStyle;
};