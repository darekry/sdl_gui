#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/gui.hpp"
#include "../src/context_menu.hpp"

TEST_CASE("ContextMenu - integration with GUIManager", "[context_menu]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Show and hide toggles visibility") {
        int calls = 0;
        auto menu = std::make_unique<ContextMenu>(manager);
        menu->addItem("Item 1", [&]{ ++calls; });
        ContextMenu* menuPtr = menu.get();
        manager.addElement(std::move(menu));

        menuPtr->showAt(100, 100);
        REQUIRE(menuPtr->isVisible() == true);

        menuPtr->hide();
        REQUIRE(menuPtr->isVisible() == false);

        SDL_Event move_in = helper.createMouseMotion(110, 110);
        manager.processEvent(move_in);
        SDL_Event down = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(down);
        SDL_Event up = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(up);

        REQUIRE(calls == 0);
    }

    SECTION("Click outside closes the menu") {
        auto menu = std::make_unique<ContextMenu>(manager);
        menu->addItem("Item 1");
        ContextMenu* menuPtr = menu.get();
        manager.addElement(std::move(menu));

        menuPtr->showAt(100, 100);
        REQUIRE(menuPtr->isVisible() == true);

        SDL_Event down = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(down);
        SDL_Event up = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(up);

        REQUIRE(menuPtr->isVisible() == false);
    }

    SECTION("Selecting an item triggers its action once and closes") {
        int calls = 0;
        auto menu = std::make_unique<ContextMenu>(manager);
        menu->addItem("Action", [&]{ ++calls; });
        ContextMenu* menuPtr = menu.get();
        manager.addElement(std::move(menu));

        menuPtr->showAt(100, 100);
        REQUIRE(menuPtr->isVisible() == true);

        SDL_Event move_in = helper.createMouseMotion(110, 110);
        manager.processEvent(move_in);
        SDL_Event down = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(down);
        SDL_Event up = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(up);

        REQUIRE(calls == 1);
        REQUIRE(menuPtr->isVisible() == false);

        SDL_Event down2 = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(down2);
        SDL_Event up2 = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 110, 110);
        manager.processEvent(up2);

        REQUIRE(calls == 1);
    }
}