#pragma once

#include "style.hpp"
#include "texture_manager.hpp" // StringHash

#include "std.hpp"

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
    static constexpr size_t stateIdx(ElementState s) { return static_cast<size_t>(s); }
    std::unordered_map<std::string, std::array<std::optional<Style>, 4>, StringHash, std::equal_to<>> m_typeStyles;
    Style m_defaultStyle;
};