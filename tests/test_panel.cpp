#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Panel behaviour", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initialization exposes basic geometry") {
        Panel panel(manager, 10, 20, 200, 150);
        REQUIRE(panel.getX() == 10);
        REQUIRE(panel.getY() == 20);
        REQUIRE(panel.getWidth() == 200);
        REQUIRE(panel.getHeight() == 150);
    }

    SECTION("Panel forwards events to its children") {
        auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));

        bool clicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Child");
        button->setOnClickCallback([&](GUIElement*) { clicked = true; });
        panelPtr->addChild(std::move(button));

        const int buttonCenterX = 50 + 10 + 40;
        const int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        REQUIRE(clicked);

        clicked = false;
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 80, 180));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 180));
        REQUIRE_FALSE(clicked);
    }

    SECTION("Draggable panel follows the mouse while dragging") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 120, 80);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110));
        manager.processEvent(helper.createMouseMotion(160, 170));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 160, 170));

        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 160);
    }
}
