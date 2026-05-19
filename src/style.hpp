#pragma once

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_log.h>

import std.compat;

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
    std::optional<int> borderRadius;  // Promień zaokrąglenia rogów (0 = ostre)
    std::optional<int> fontSize;
    std::optional<std::string> fontName;

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

// Funkcja do logowania właściwości obiektu Style
inline void logStyle(const Style& style, const char* styleName) {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--- Logging Style: %s ---", styleName);
    if (style.backgroundColor.has_value()) {
        const auto& c = style.backgroundColor.value();
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BackgroundColor: (%d, %d, %d, %d)", c.r, c.g, c.b, c.a);
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BackgroundColor: nullopt");
    }
    if (style.textColor.has_value()) {
        const auto& c = style.textColor.value();
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  TextColor: (%d, %d, %d, %d)", c.r, c.g, c.b, c.a);
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  TextColor: nullopt");
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  Texture: %s", (style.texture.has_value() ? "set" : "nullopt"));
    if (style.borderColor.has_value()) {
        const auto& c = style.borderColor.value();
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderColor: (%d, %d, %d, %d)", c.r, c.g, c.b, c.a);
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderColor: nullopt");
    }
    if (style.borderWidth.has_value()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderWidth: %d", style.borderWidth.value());
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderWidth: nullopt");
    }
    if (style.borderRadius.has_value()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderRadius: %d", style.borderRadius.value());
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  BorderRadius: nullopt");
    }
    if (style.fontSize.has_value()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  FontSize: %d", style.fontSize.value());
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  FontSize: nullopt");
    }
    if (style.fontName.has_value()) {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  FontName: %s", style.fontName.value().c_str());
    } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "  FontName: nullopt");
    }
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "--------------------------------------");
}
