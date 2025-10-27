#define CATCH_CONFIG_MAIN
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
}
