#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/radio_button.hpp"
#include "../src/radio_group.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"

TEST_CASE("RadioButton and RadioGroup Functionality", "[radio_button][radio_group]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    TextureManager textureManager(renderer);
    FontManager fontManager;
    RadioGroup group;

    SECTION("RadioButton Initialization") {
        RadioButton radio1(10, 20, 20, 20, "Option A");
        radio1.setGroup(&group);
        group.addRadioButton(&radio1);
        REQUIRE(radio1.getX() == 10);
        REQUIRE(radio1.getY() == 20);
        REQUIRE(radio1.getWidth() == 20);
        REQUIRE(radio1.getHeight() == 20);
        REQUIRE(radio1.isSelected() == false);
    }

    SECTION("RadioGroup - Single Selection") {
        RadioButton radio1(10, 10, 20, 20, "Option 1");
        radio1.setGroup(&group);
        group.addRadioButton(&radio1);
        RadioButton radio2(10, 40, 20, 20, "Option 2");
        radio2.setGroup(&group);
        group.addRadioButton(&radio2);
        RadioButton radio3(10, 70, 20, 20, "Option 3");
        radio3.setGroup(&group);
        group.addRadioButton(&radio3);

        REQUIRE(radio1.isSelected() == false);
        REQUIRE(radio2.isSelected() == false);
        REQUIRE(radio3.isSelected() == false);

        // Kliknięcie radio1
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15);
        radio1.handleEvent(event);
        REQUIRE(radio1.isSelected() == true);
        REQUIRE(radio2.isSelected() == false);
        REQUIRE(radio3.isSelected() == false);

        // Kliknięcie radio2
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 45);
        radio2.handleEvent(event);
        REQUIRE(radio1.isSelected() == false); // radio1 powinno się odznaczyć
        REQUIRE(radio2.isSelected() == true);
        REQUIRE(radio3.isSelected() == false);

        // Kliknięcie radio3
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 75);
        radio3.handleEvent(event);
        REQUIRE(radio1.isSelected() == false);
        REQUIRE(radio2.isSelected() == false); // radio2 powinno się odznaczyć
        REQUIRE(radio3.isSelected() == true);
    }

    SECTION("RadioButton - onToggle Callback") {
        RadioButton radio1(10, 10, 20, 20, "Option 1");
        radio1.setGroup(&group);
        group.addRadioButton(&radio1);
        bool toggled = false;
        bool newState = false;
        radio1.setOnChange([&](RadioButton* rb) { toggled = true; newState = rb->isSelected(); });

        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15);
        radio1.handleEvent(event);
        REQUIRE(toggled == true);
        REQUIRE(newState == true);

        toggled = false; // Reset flagę
        RadioButton radio2(10, 40, 20, 20, "Option 2");
        radio2.setGroup(&group);
        group.addRadioButton(&radio2);
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 45); // Nowe zdarzenie dla radio2
        radio2.handleEvent(event); // Kliknięcie radio2, radio1 powinno się odznaczyć
        REQUIRE(toggled == true); // Callback radio1 powinien być wywołany
        REQUIRE(newState == false); // Z nowym stanem false
    }

    SECTION("RadioGroup - Set Selected") {
        RadioButton radio1(10, 10, 20, 20, "Option 1");
        radio1.setGroup(&group);
        group.addRadioButton(&radio1);
        RadioButton radio2(10, 40, 20, 20, "Option 2");
        radio2.setGroup(&group);
        group.addRadioButton(&radio2);
 
        radio1.setSelected(true);
        REQUIRE(radio1.isSelected() == true);
        REQUIRE(radio2.isSelected() == false);
 
        radio2.setSelected(true);
        REQUIRE(radio1.isSelected() == false);
        REQUIRE(radio2.isSelected() == true);
 
        radio1.setSelected(false);
        radio2.setSelected(false);
        REQUIRE(radio1.isSelected() == false);
        REQUIRE(radio2.isSelected() == false);
    }

}