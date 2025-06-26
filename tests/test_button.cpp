#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"

TEST_CASE("Button Functionality", "[button]") {
    TestHelper helper;
    // Managers are now provided by the TestHelper
    // SDL_Renderer* renderer = helper.getRenderer();
    // TextureManager textureManager(renderer);
    // FontManager fontManager;

    SECTION("Initialization") {
        Button button(10, 20, 100, 50);
        REQUIRE(button.getX() == 10);
        REQUIRE(button.getY() == 20);
        REQUIRE(button.getWidth() == 100);
        REQUIRE(button.getHeight() == 50);
    }

    SECTION("Event Handling - Click") {
        bool clicked = false;
        Button button(10, 10, 100, 50);
        button.setOnClickCallback([&](GUIElement*){ clicked = true; });

        // Symulacja kliknięcia wewnątrz przycisku
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        button.handleEvent(event);
        
        REQUIRE(clicked == true);

        // Symulacja kliknięcia na zewnątrz
        clicked = false;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 200);
        button.handleEvent(event);
        REQUIRE(clicked == false);
    }

    SECTION("State - Disabled") {
    }
}