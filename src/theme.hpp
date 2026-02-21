#pragma once

#include "style.hpp"

import std.compat;

// Klasa Theme przechowuje domyślne style dla wszystkich typów komponentów.
// Umożliwia globalną zmianę wyglądu aplikacji.
class Theme {
public:
    void setStyle(const std::string& type, Style style);
    Style getStyle(const std::string& type) const;

    void setDefaultStyle(Style style);
    const Style& getDefaultStyle() const;

    // Metoda fabryczna tworząca domyślny motyw w stylu "Windows 95/98".
    static Theme createDefaultTheme();

private:
    // Mapa: Typ komponentu -> Styl
    std::map<std::string, Style> m_styles;
    Style m_defaultStyle;
};
