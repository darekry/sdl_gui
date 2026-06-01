#pragma once

#include "style.hpp"

import std.compat;

// Klasa Theme przechowuje domyślne style dla wszystkich typów komponentów i stanów.
// Umożliwia globalną zmianę wyglądu aplikacji runtime.
class Theme {
public:
    // === Per-type, per-state styles ===
    
    /** Set style for a specific component type and state */
    void setStyle(const std::string& type, ElementState state, Style style);
    
    /** Get style for a specific component type and state 
     *  Returns type-specific style merged with default style
     *  Falls back to Normal state if requested state not defined
     */
    Style getStyle(const std::string& type, ElementState state) const;
    
    // === Legacy API (backward compatible) ===
    
    /** Set style for Normal state of a component type */
    void setStyle(const std::string& type, Style style);
    
    /** Get style for Normal state of a component type */
    Style getStyle(const std::string& type) const;
    
    // === Default style ===
    
    void setDefaultStyle(Style style);
    const Style& getDefaultStyle() const;
    
    // === Factory ===
    
    /** Create default theme in Windows 95/98 style */
    static Theme createDefaultTheme();

private:
    // Mapa: Typ komponentu -> Stan -> Styl
    std::map<std::string, std::map<ElementState, Style>> m_typeStyles;
    Style m_defaultStyle;
};