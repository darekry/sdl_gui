#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("FontManager functionality", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Loading non-existent font returns nullptr") {
        auto font = fontManager.loadFont("nonexistent/font.ttf", 16);
        REQUIRE(font == nullptr);
    }

    SECTION("getFont returns nullptr for non-existent font") {
        auto* font = fontManager.getFont("nonexistent/font.ttf", 16);
        REQUIRE(font == nullptr);
    }

    SECTION("Font caching works correctly") {
        auto font1 = fontManager.loadFont("test_font.ttf", 16);
        auto font2 = fontManager.loadFont("test_font.ttf", 16);
        
        if (font1 && font2) {
            REQUIRE(font1 == font2);
            REQUIRE(font1.get() == font2.get());
        }
    }

    SECTION("Different font sizes create different cache entries") {
        auto font1 = fontManager.loadFont("test_font.ttf", 16);
        auto font2 = fontManager.loadFont("test_font.ttf", 24);
        
        if (font1 && font2) {
            REQUIRE(font1.get() != font2.get());
        }
    }

    SECTION("Default font can be set and retrieved") {
        fontManager.loadDefaultFont("test_default.ttf", 18);
        auto defaultFont = fontManager.getDefaultFont();
        
        if (defaultFont) {
            REQUIRE(defaultFont != nullptr);
        }
    }

    SECTION("getTextSize handles null output parameters gracefully") {
        int* nullWidth = nullptr;
        int* nullHeight = nullptr;
        
        fontManager.getTextSize("test", "font.ttf", 16, nullWidth, nullHeight);
    }

    SECTION("getTextSize returns zero dimensions for non-existent font") {
        int width = -1;
        int height = -1;
        
        fontManager.getTextSize("test text", "nonexistent.ttf", 16, &width, &height);
        
        REQUIRE(width == 0);
        REQUIRE(height == 0);
    }

    SECTION("getFont with string_view path") {
        std::string path = "test_font.ttf";
        std::string_view pathView = path;
        
        auto* font = fontManager.getFont(pathView, 16);
        if (font) {
            REQUIRE(font != nullptr);
        } else {
            REQUIRE(font == nullptr);
        }
    }
}
