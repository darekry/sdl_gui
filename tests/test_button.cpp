#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/gui.hpp"
#include "../src/button.hpp"

TEST_CASE("Button Functionality", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click inside triggers callback once") {
        int clicks = 0;
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 50, "Click");
        button->setOnClickCallback([&](GUIElement*) { ++clicks; });

        Button* btn = button.get();
        manager.addElement(std::move(button));

        // Ustal hover wewnątrz przycisku
        SDL_Event move_in = helper.createMouseMotion(20, 20);
        manager.processEvent(move_in);

        // Down wewnątrz
        SDL_Event down_in = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(down_in);

        // Up wewnątrz
        SDL_Event up_in = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(up_in);

        // Oczekujemy pojedynczego wywołania callbacku oraz stanu hover (po zwolnieniu)
        REQUIRE(clicks == 1);
        REQUIRE(btn->isHovered() == true);
    }

    SECTION("Click down in, release outside → no callback") {
        int clicks = 0;
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 50, "Click");
        button->setOnClickCallback([&](GUIElement*) { ++clicks; });

        Button* btn = button.get();
        manager.addElement(std::move(button));

        // Wejście kursora do przycisku
        SDL_Event move_in = helper.createMouseMotion(20, 20);
        manager.processEvent(move_in);

        // Down wewnątrz
        SDL_Event down_in = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(down_in);

        // Ruch i up na zewnątrz
        SDL_Event move_out = helper.createMouseMotion(1000, 1000);
        manager.processEvent(move_out);

        SDL_Event up_out = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(up_out);

        REQUIRE(clicks == 0);
        REQUIRE(btn->isHovered() == false);
    }

    SECTION("Hover in/out toggles state") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 50, "Hover");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        // Hover in
        SDL_Event move_in = helper.createMouseMotion(20, 20);
        manager.processEvent(move_in);
        REQUIRE(btn->isHovered() == true);

        // Hover out
        SDL_Event move_out = helper.createMouseMotion(1000, 1000);
        manager.processEvent(move_out);
        REQUIRE(btn->isHovered() == false);
    }

    SECTION("Drag mouse outside while pressed keeps Pressed state") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 50, "Drag Test");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        // Enter hover state
        SDL_Event move_in = helper.createMouseMotion(20, 20);
        manager.processEvent(move_in);
        REQUIRE(btn->isHovered() == true);

        // Press down inside
        SDL_Event down_in = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(down_in);
        REQUIRE(btn->getState() == ElementState::Pressed);

        // Move mouse outside while still pressed
        SDL_Event move_out = helper.createMouseMotion(1000, 1000);
        manager.processEvent(move_out);

        // BUG: Current implementation resets to Normal, but should stay Pressed
        // This test will initially FAIL, demonstrating the bug
        REQUIRE(btn->getState() == ElementState::Pressed); // Should stay Pressed
        REQUIRE(btn->isHovered() == false); // Should not be hovered

        // Release outside
        SDL_Event up_out = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(up_out);

        // After release, should go back to Normal (not Hover since mouse is outside)
        REQUIRE(btn->getState() == ElementState::Normal);
        REQUIRE(btn->isHovered() == false);
    }
}