#ifndef STYLE_HPP
#define STYLE_HPP

#include "texture_manager.hpp"
#include <SDL2/SDL_pixels.h>
import std.compat;

// Definiuje stany, w jakich może znaleźć się element interfejsu.
enum class ElementState {
    Normal,
    Hover,
    Pressed,
    Disabled
};

// Struktura przechowująca atrybuty wizualne dla pojedynczego stanu elementu.
// Użycie std::optional pozwala na dziedziczenie niezdefiniowanych właściwości z motywu.
struct Style {
    std::optional<SDL_Color> backgroundColor;
    std::optional<SDL_Color> textColor;
    std::optional<SharedTexture> texture;
    std::optional<SDL_Color> borderColor;
    std::optional<int> borderWidth;
    std::optional<int> fontSize;
    std::optional<std::string> fontName;
};

#endif // STYLE_HPP