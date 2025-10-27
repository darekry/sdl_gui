#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/style.hpp"

TEST_CASE("Button interactions follow the current event model", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Click");
    Button* btn = button.get();
    int clicks = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clicks; });
    manager.addElement(std::move(button));

    SECTION("Mouse hover updates the element state") {
        manager.processEvent(helper.createMouseMotion(5, 5));
        REQUIRE(btn->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseMotion(400, 400));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Releasing inside triggers the click callback once") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(clicks == 1);
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Releasing outside cancels the click callback") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseMotion(400, 400));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 400, 400));
        REQUIRE(clicks == 0);
        REQUIRE(btn->getState() != ElementState::Pressed);
    }
}
