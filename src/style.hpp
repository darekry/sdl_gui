#ifndef STYLE_HPP
#define STYLE_HPP

#include <SDL2/SDL_pixels.h>
#include <memory>
#include <optional>
#include <string>
#include <iostream>

struct SDL_Texture;
using SharedTexture = std::shared_ptr<SDL_Texture>;

// Operator porównania dla SDL_Color
inline bool operator==(const SDL_Color& a, const SDL_Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

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

    bool operator==(const Style& other) const {
        if (backgroundColor.has_value() != other.backgroundColor.has_value()) { return false; }
        if (backgroundColor && !(*backgroundColor == *other.backgroundColor)) { return false; }

        if (textColor.has_value() != other.textColor.has_value()) { return false; }
        if (textColor && !(*textColor == *other.textColor)) { return false; }

        if (texture.has_value() != other.texture.has_value()) { return false; }
        if (texture && texture->get() != other.texture->get()) { return false; }

        if (borderColor.has_value() != other.borderColor.has_value()) { return false; }
        if (borderColor && !(*borderColor == *other.borderColor)) { return false; }

        if (borderWidth.has_value() != other.borderWidth.has_value()) { return false; }
        if (borderWidth && *borderWidth != *other.borderWidth) { return false; }

        if (fontSize.has_value() != other.fontSize.has_value()) { return false; }
        if (fontSize && *fontSize != *other.fontSize) { return false; }

        if (fontName.has_value() != other.fontName.has_value()) { return false; }
        if (fontName && *fontName != *other.fontName) { return false; }

        return true;
    }

    bool operator!=(const Style& other) const {
        return !(*this == other);
    }
};

// Funkcja do logowania właściwości obiektu Style
inline void logStyle(const Style& style, const char* styleName) {
    std::cout << "--- Logging Style: " << styleName << " ---\n";
    if (style.backgroundColor.has_value()) {
        const auto& c = style.backgroundColor.value();
        std::cout << "  BackgroundColor: (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a << ")\n";
    } else {
        std::cout << "  BackgroundColor: nullopt\n";
    }
    if (style.textColor.has_value()) {
        const auto& c = style.textColor.value();
        std::cout << "  TextColor: (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a << ")\n";
    } else {
        std::cout << "  TextColor: nullopt\n";
    }
    std::cout << "  Texture: " << (style.texture.has_value() ? "set" : "nullopt") << "\n";
    if (style.borderColor.has_value()) {
        const auto& c = style.borderColor.value();
        std::cout << "  BorderColor: (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a << ")\n";
    } else {
        std::cout << "  BorderColor: nullopt\n";
    }
    if (style.borderWidth.has_value()) {
        std::cout << "  BorderWidth: " << style.borderWidth.value() << "\n";
    } else {
        std::cout << "  BorderWidth: nullopt\n";
    }
    if (style.fontSize.has_value()) {
        std::cout << "  FontSize: " << style.fontSize.value() << "\n";
    } else {
        std::cout << "  FontSize: nullopt\n";
    }
    if (style.fontName.has_value()) {
        std::cout << "  FontName: " << style.fontName.value() << "\n";
    } else {
        std::cout << "  FontName: nullopt\n";
    }
    std::cout << "--------------------------------------\n";
}

#endif // STYLE_HPP