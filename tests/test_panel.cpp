#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Panel Functionality", "[panel]") {
    TestHelper helper;
    GUIManager guiManager(helper.getRenderer());

    SECTION("Initialization") {
        Panel panel(guiManager, 10, 20, 200, 150);
        REQUIRE(panel.getX() == 10);
        REQUIRE(panel.getY() == 20);
        REQUIRE(panel.getWidth() == 200);
        REQUIRE(panel.getHeight() == 150);
    }

    SECTION("Event Handling - Child Interaction") {
        Panel panel(guiManager, 50, 50, 200, 150);
        bool buttonClicked = false;
        auto button = std::make_unique<Button>(guiManager, 60, 60, 80, 40);
        button->setOnClickCallback([&](GUIElement*){ buttonClicked = true; });
        panel.addChild(std::move(button));

        // Symulacja kliknięcia wewnątrz przycisku (który jest dzieckiem panelu)
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 120, 120);
        panel.handleEvent(event);
        REQUIRE(buttonClicked == true);

        // Symulacja kliknięcia poza przyciskiem, ale wewnątrz panelu
        buttonClicked = false;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 150);
        panel.handleEvent(event);
        REQUIRE(buttonClicked == false); // Przycisk nie powinien być kliknięty

        // Symulacja kliknięcia poza panelem
        buttonClicked = false;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 5, 5);
        panel.handleEvent(event);
        REQUIRE(buttonClicked == false);
    }

    SECTION("State - Add and Remove Children") {
        Panel panel(guiManager, 0, 0, 100, 100);
        REQUIRE(panel.getChildren().empty());
 
        auto child1 = std::make_unique<Button>(guiManager, 0, 0, 10, 10);
        auto child2 = std::make_unique<Button>(guiManager, 0, 0, 10, 10);
        
        panel.addChild(std::move(child1));
        REQUIRE(panel.getChildren().size() == 1);

        panel.addChild(std::move(child2));
        REQUIRE(panel.getChildren().size() == 2);

        // Sprawdzenie, czy dzieci są poprawnie przechowywane (przez wskaźniki)
        // Nie możemy bezpośrednio sprawdzić unikalnych wskaźników po przeniesieniu,
        // ale możemy sprawdzić rozmiar listy dzieci.
    }

}