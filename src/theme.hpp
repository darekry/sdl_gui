#ifndef THEME_HPP
#define THEME_HPP

#include "style.hpp"
import std.compat;
// Klasa Theme przechowuje domyślne style dla wszystkich typów komponentów.
// Umożliwia globalną zmianę wyglądu aplikacji.
class Theme {
public:
    // Ustawia domyślny styl dla danego typu komponentu i stanu.
    void setStyle(const std::string& componentType, ElementState state, Style style);

    // Pobiera domyślny styl dla danego typu komponentu i stanu.
    // Jeśli dla danego komponentu nie ma zdefiniowanego stylu, zwraca styl domyślny.
    const Style& getStyle(const std::string& componentType, ElementState state) const;

    // Metoda fabryczna tworząca domyślny motyw w stylu "Windows 95/98".
    static Theme createDefaultTheme();

    // Pobiera domyślny styl bazowy.
    const Style& getDefaultStyle() const;

private:
    // Mapa: Typ komponentu -> Mapa: Stan -> Styl
    std::map<std::string, std::map<ElementState, Style>> styles;
    
    // Styl używany, gdy dla danego komponentu brakuje specyficznego stylu.
    Style defaultStyle;
};

#endif // THEME_HPP