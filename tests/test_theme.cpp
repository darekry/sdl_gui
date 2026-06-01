#include "../lib/catch_amalgamated.hpp"

#include "../src/theme.hpp"
#include "../src/style.hpp"

TEST_CASE("Theme functionality", "[theme]") {
    SECTION("Default theme can be created") {
        Theme theme = Theme::createDefaultTheme();
        Style defaultStyle = theme.getDefaultStyle();
        REQUIRE(defaultStyle.backgroundColor.has_value());
        if (defaultStyle.backgroundColor) {
            REQUIRE(defaultStyle.backgroundColor->r >= 0);
            REQUIRE(defaultStyle.backgroundColor->g >= 0);
            REQUIRE(defaultStyle.backgroundColor->b >= 0);
        }
    }

    SECTION("Custom styles can be set and retrieved") {
        Theme theme;
        Style customStyle;
        customStyle.backgroundColor = SDL_Color{255, 0, 0, 255};
        customStyle.textColor = SDL_Color{0, 255, 0, 255};
        customStyle.borderColor = SDL_Color{0, 0, 255, 255};

        theme.setStyle("Button", customStyle);
        Style retrieved = theme.getStyle("Button");

        REQUIRE(retrieved.backgroundColor.has_value());
        REQUIRE(retrieved.backgroundColor->r == 255);
        REQUIRE(retrieved.backgroundColor->g == 0);
        REQUIRE(retrieved.backgroundColor->b == 0);
        REQUIRE(retrieved.textColor.has_value());
        REQUIRE(retrieved.textColor->r == 0);
        REQUIRE(retrieved.textColor->g == 255);
        REQUIRE(retrieved.textColor->b == 0);
    }

    SECTION("getStyle returns default style for unknown types") {
        Theme theme;
        Style defaultStyle;
        defaultStyle.backgroundColor = SDL_Color{128, 128, 128, 255};
        theme.setDefaultStyle(defaultStyle);

        Style retrieved = theme.getStyle("UnknownComponent");
        REQUIRE(retrieved.backgroundColor.has_value());
        REQUIRE(retrieved.backgroundColor->r == 128);
        REQUIRE(retrieved.backgroundColor->g == 128);
        REQUIRE(retrieved.backgroundColor->b == 128);
    }

    SECTION("Default style can be set and retrieved") {
        Theme theme;
        Style defaultStyle;
        defaultStyle.backgroundColor = SDL_Color{200, 200, 200, 255};
        defaultStyle.textColor = SDL_Color{50, 50, 50, 255};

        theme.setDefaultStyle(defaultStyle);
        const Style& retrieved = theme.getDefaultStyle();

        REQUIRE(retrieved.backgroundColor.has_value());
        REQUIRE(retrieved.backgroundColor->r == 200);
        REQUIRE(retrieved.backgroundColor->g == 200);
        REQUIRE(retrieved.backgroundColor->b == 200);
        REQUIRE(retrieved.textColor.has_value());
        REQUIRE(retrieved.textColor->r == 50);
        REQUIRE(retrieved.textColor->g == 50);
        REQUIRE(retrieved.textColor->b == 50);
    }

    SECTION("Multiple component styles can be stored") {
        Theme theme;
        
        Style buttonStyle;
        buttonStyle.backgroundColor = SDL_Color{100, 100, 100, 255};
        theme.setStyle("Button", buttonStyle);

        Style labelStyle;
        labelStyle.backgroundColor = SDL_Color{200, 200, 200, 255};
        theme.setStyle("Label", labelStyle);

        Style checkboxStyle;
        checkboxStyle.backgroundColor = SDL_Color{150, 150, 150, 255};
        theme.setStyle("Checkbox", checkboxStyle);

        REQUIRE(theme.getStyle("Button").backgroundColor.has_value());
        REQUIRE(theme.getStyle("Button").backgroundColor->r == 100);
        REQUIRE(theme.getStyle("Label").backgroundColor.has_value());
        REQUIRE(theme.getStyle("Label").backgroundColor->r == 200);
        REQUIRE(theme.getStyle("Checkbox").backgroundColor.has_value());
        REQUIRE(theme.getStyle("Checkbox").backgroundColor->r == 150);
    }

    // === New per-state API tests ===

    SECTION("Per-state styles can be set and retrieved") {
        Theme theme;
        
        Style normalStyle;
        normalStyle.backgroundColor = SDL_Color{200, 200, 200, 255};
        theme.setStyle("Button", ElementState::Normal, normalStyle);
        
        Style hoverStyle;
        hoverStyle.backgroundColor = SDL_Color{220, 220, 220, 255};
        theme.setStyle("Button", ElementState::Hover, hoverStyle);
        
        Style pressedStyle;
        pressedStyle.backgroundColor = SDL_Color{150, 150, 150, 255};
        theme.setStyle("Button", ElementState::Pressed, pressedStyle);

        Style normalRetrieved = theme.getStyle("Button", ElementState::Normal);
        REQUIRE(normalRetrieved.backgroundColor.has_value());
        REQUIRE(normalRetrieved.backgroundColor->r == 200);

        Style hoverRetrieved = theme.getStyle("Button", ElementState::Hover);
        REQUIRE(hoverRetrieved.backgroundColor.has_value());
        REQUIRE(hoverRetrieved.backgroundColor->r == 220);

        Style pressedRetrieved = theme.getStyle("Button", ElementState::Pressed);
        REQUIRE(pressedRetrieved.backgroundColor.has_value());
        REQUIRE(pressedRetrieved.backgroundColor->r == 150);
    }

    SECTION("getStyle with state falls back to Normal state") {
        Theme theme;
        
        Style normalStyle;
        normalStyle.backgroundColor = SDL_Color{180, 180, 180, 255};
        theme.setStyle("Button", ElementState::Normal, normalStyle);
        
        // No Hover style defined - should fallback to Normal
        Style hoverRetrieved = theme.getStyle("Button", ElementState::Hover);
        REQUIRE(hoverRetrieved.backgroundColor.has_value());
        REQUIRE(hoverRetrieved.backgroundColor->r == 180);
    }

    SECTION("getStyle with state falls back to default style") {
        Theme theme;
        Style defaultStyle;
        defaultStyle.backgroundColor = SDL_Color{212, 208, 200, 255};
        theme.setDefaultStyle(defaultStyle);
        
        // No Button style defined at all - should return default
        Style retrieved = theme.getStyle("Button", ElementState::Normal);
        REQUIRE(retrieved.backgroundColor.has_value());
        REQUIRE(retrieved.backgroundColor->r == 212);
    }

    SECTION("Legacy getStyle(type) returns Normal state") {
        Theme theme;
        
        Style normalStyle;
        normalStyle.backgroundColor = SDL_Color{100, 100, 100, 255};
        theme.setStyle("Button", ElementState::Normal, normalStyle);
        
        Style hoverStyle;
        hoverStyle.backgroundColor = SDL_Color{150, 150, 150, 255};
        theme.setStyle("Button", ElementState::Hover, hoverStyle);
        
        // Legacy API should return Normal state
        Style legacyRetrieved = theme.getStyle("Button");
        REQUIRE(legacyRetrieved.backgroundColor.has_value());
        REQUIRE(legacyRetrieved.backgroundColor->r == 100);
    }

    SECTION("Default theme has per-state Button styles") {
        Theme theme = Theme::createDefaultTheme();
        
        Style normalBtn = theme.getStyle("Button", ElementState::Normal);
        REQUIRE(normalBtn.borderRadius.has_value());
        REQUIRE(*normalBtn.borderRadius == 4);
        
        Style hoverBtn = theme.getStyle("Button", ElementState::Hover);
        REQUIRE(hoverBtn.backgroundColor.has_value());
        // Hover should have different/lighter background
        REQUIRE(hoverBtn.backgroundColor->r > normalBtn.backgroundColor->r);
        
        Style pressedBtn = theme.getStyle("Button", ElementState::Pressed);
        REQUIRE(pressedBtn.backgroundColor.has_value());
        // Pressed should have darker background
        REQUIRE(pressedBtn.backgroundColor->r < normalBtn.backgroundColor->r);
    }
}
