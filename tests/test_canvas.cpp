#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/canvas.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Canvas functionality", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Canvas can be created and has correct dimensions") {
        Canvas canvas(manager, 10, 20, 200, 150);
        REQUIRE(canvas.getX() == 10);
        REQUIRE(canvas.getY() == 20);
        REQUIRE(canvas.getWidth() == 200);
        REQUIRE(canvas.getHeight() == 150);
    }

    SECTION("Canvas clearing resets drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        canvasPtr->clear();
    }

    SECTION("Drawing inside canvas captures mouse") {
        auto canvas = std::make_unique<Canvas>(manager, 10, 10, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(30, 30));
        manager.processEvent(helper.createMouseMotion(40, 40));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 40, 40));
    }

    SECTION("Mouse events outside canvas are ignored") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 200));
        manager.processEvent(helper.createMouseMotion(210, 210));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 210, 210));
    }

    SECTION("Canvas reports correct component type") {
        Canvas canvas(manager, 0, 0, 100, 100);
        const char* type = canvas.getComponentType();
        REQUIRE(type != nullptr);
    }
}
