#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "test_helper.cpp"
#include "../src/checkbox.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"

TEST_CASE("Checkbox Functionality", "[checkbox]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    TextureManager textureManager(renderer);
    FontManager fontManager;

    SECTION("Initialization") {
        Checkbox checkbox(10, 20, 20, 20, "Option 1");
        REQUIRE(checkbox.getX() == 10);
        REQUIRE(checkbox.getY() == 20);
        REQUIRE(checkbox.getWidth() == 20);
        REQUIRE(checkbox.getHeight() == 20);
        REQUIRE(checkbox.isChecked() == false);

        Checkbox checkbox2(0, 0, 20, 20, "Option 2");
        checkbox2.setChecked(true);
        REQUIRE(checkbox2.isChecked() == true);
    }

    SECTION("Event Handling - Toggle") {
        Checkbox checkbox(10, 10, 20, 20, "Toggle Me");
        bool toggled = false;
        bool newState = false;
        checkbox.setOnChange([&](Checkbox*, bool checked) { toggled = true; newState = checked; });

        REQUIRE(checkbox.isChecked() == false);

        // Kliknięcie - powinno zaznaczyć
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15);
        checkbox.handleEvent(event);
        REQUIRE(checkbox.isChecked() == true);
        REQUIRE(toggled == true);
        REQUIRE(newState == true);

        toggled = false; // Reset flagę

        // Kliknięcie ponownie - powinno odznaczyć
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15);
        checkbox.handleEvent(event);
        REQUIRE(checkbox.isChecked() == false);
        REQUIRE(toggled == true);
        REQUIRE(newState == false);

        // Kliknięcie poza checkboxem - nie powinno zmienić stanu
        toggled = false;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        checkbox.handleEvent(event);
        REQUIRE(checkbox.isChecked() == false);
        REQUIRE(toggled == false);
    }

    SECTION("State - Set Checked") {
        Checkbox checkbox(10, 10, 20, 20, "Set Checked");
        bool toggled = false;
        checkbox.setOnChange([&](Checkbox*, bool) { toggled = true; });

        checkbox.setChecked(true);
        REQUIRE(checkbox.isChecked() == true);
        REQUIRE(toggled == true); // Callback powinien być wywołany

        toggled = false;
        checkbox.setChecked(true); // Ustawienie na ten sam stan
        REQUIRE(checkbox.isChecked() == true);
        REQUIRE(toggled == false); // Callback nie powinien być wywołany

        checkbox.setChecked(false);
        REQUIRE(checkbox.isChecked() == false);
        REQUIRE(toggled == true); // Callback powinien być wywołany
    }
    }