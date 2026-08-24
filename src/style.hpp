#pragma once

#include <SDL3/SDL_pixels.h>
#include "logger.hpp"

#include "std.hpp"

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

// Typ fazowanego obramowania 3D w stylu Windows 95/98.
// Raised = wypukłość (przyciski), Sunken = wklęśnięcie (pola edycji).
enum class BevelType {
    Raised,
    Sunken
};

// Struktura przechowująca atrybuty wizualne dla pojedynczego stanu elementu.
// Użycie std::optional pozwala na dziedziczenie niezdefiniowanych właściwości z motywu.
struct Style {
    std::optional<SDL_Color> backgroundColor;
    std::optional<SDL_Color> textColor;
    std::optional<SharedTexture> texture;
    std::optional<SDL_Color> borderColor;
    std::optional<int> borderWidth;
    std::optional<int> borderRadius;  // Promień zaokrąglenia rogów (0 = ostre)
    std::optional<int> fontSize;
    std::optional<std::string> fontName;
    std::optional<SDL_Color> borderColorOuterTopLeft;
    std::optional<SDL_Color> borderColorOuterBottomRight;
    std::optional<SDL_Color> borderColorInnerTopLeft;
    std::optional<SDL_Color> borderColorInnerBottomRight;

    void mergeWith(const Style& base) {
        if (!backgroundColor.has_value() && base.backgroundColor.has_value()) {
            backgroundColor = base.backgroundColor;
        }
        if (!textColor.has_value() && base.textColor.has_value()) {
            textColor = base.textColor;
        }
        if (!texture.has_value() && base.texture.has_value()) {
            texture = base.texture;
        }
        if (!borderColor.has_value() && base.borderColor.has_value()) {
            borderColor = base.borderColor;
        }
        if (!borderColorOuterTopLeft.has_value() && base.borderColorOuterTopLeft.has_value()) {
            borderColorOuterTopLeft = base.borderColorOuterTopLeft;
        }
        if (!borderColorOuterBottomRight.has_value() && base.borderColorOuterBottomRight.has_value()) {
            borderColorOuterBottomRight = base.borderColorOuterBottomRight;
        }
        if (!borderColorInnerTopLeft.has_value() && base.borderColorInnerTopLeft.has_value()) {
            borderColorInnerTopLeft = base.borderColorInnerTopLeft;
        }
        if (!borderColorInnerBottomRight.has_value() && base.borderColorInnerBottomRight.has_value()) {
            borderColorInnerBottomRight = base.borderColorInnerBottomRight;
        }
        if (!borderWidth.has_value() && base.borderWidth.has_value()) {
            borderWidth = base.borderWidth;
        }
        if (!borderRadius.has_value() && base.borderRadius.has_value()) {
            borderRadius = base.borderRadius;
        }
        if (!fontSize.has_value() && base.fontSize.has_value()) {
            fontSize = base.fontSize;
        }
        if (!fontName.has_value() && base.fontName.has_value()) {
            fontName = base.fontName;
        }
    }

    bool operator==(const Style& other) const {
        if (backgroundColor.has_value() != other.backgroundColor.has_value()) { return false; }
        if (backgroundColor && !(*backgroundColor == *other.backgroundColor)) { return false; }

        if (textColor.has_value() != other.textColor.has_value()) { return false; }
        if (textColor && !(*textColor == *other.textColor)) { return false; }

        if (texture.has_value() != other.texture.has_value()) { return false; }
        if (texture && texture->get() != other.texture->get()) { return false; }

        if (borderColor.has_value() != other.borderColor.has_value()) { return false; }
        if (borderColor && !(*borderColor == *other.borderColor)) { return false; }

        if (borderColorOuterTopLeft.has_value() != other.borderColorOuterTopLeft.has_value()) { return false; }
        if (borderColorOuterTopLeft && !(*borderColorOuterTopLeft == *other.borderColorOuterTopLeft)) { return false; }

        if (borderColorOuterBottomRight.has_value() != other.borderColorOuterBottomRight.has_value()) { return false; }
        if (borderColorOuterBottomRight && !(*borderColorOuterBottomRight == *other.borderColorOuterBottomRight)) { return false; }

        if (borderColorInnerTopLeft.has_value() != other.borderColorInnerTopLeft.has_value()) { return false; }
        if (borderColorInnerTopLeft && !(*borderColorInnerTopLeft == *other.borderColorInnerTopLeft)) { return false; }

        if (borderColorInnerBottomRight.has_value() != other.borderColorInnerBottomRight.has_value()) { return false; }
        if (borderColorInnerBottomRight && !(*borderColorInnerBottomRight == *other.borderColorInnerBottomRight)) { return false; }

        if (borderWidth.has_value() != other.borderWidth.has_value()) { return false; }
        if (borderWidth && *borderWidth != *other.borderWidth) { return false; }

        if (borderRadius.has_value() != other.borderRadius.has_value()) { return false; }
        if (borderRadius && *borderRadius != *other.borderRadius) { return false; }

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

