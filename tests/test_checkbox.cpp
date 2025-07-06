#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/checkbox.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Checkbox Functionality", "[checkbox]") {
    TestHelper helper;
    GUIManager guiManager(helper.getRenderer());

    SECTION("Initialization") {
        Checkbox checkbox(guiManager, 10, 20, 20, 20);
        REQUIRE(checkbox.getX() == 10);
        REQUIRE(checkbox.getY() == 20);
        REQUIRE(checkbox.getWidth() == 20);
        REQUIRE(checkbox.getHeight() == 20);
        REQUIRE(checkbox.isChecked() == false);

        Checkbox checkbox2(guiManager, 0, 0, 20, 20);
        checkbox2.setChecked(true);
        REQUIRE(checkbox2.isChecked() == true);
    }

    SECTION("Event Handling - Toggle") {
        Checkbox checkbox(guiManager, 10, 10, 20, 20);
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
        Checkbox checkbox(guiManager, 10, 10, 20, 20);
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