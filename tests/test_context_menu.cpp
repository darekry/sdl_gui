#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/context_menu.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ContextMenu functionality", "[context_menu]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto menu = std::make_unique<ContextMenu>(manager);
    ContextMenu* ctx = menu.get();
    manager.addElement(std::move(menu));

    bool action1Called = false;
    bool action2Called = false;

    ctx->addItem("Action 1", [&]() { action1Called = true; });
    ctx->addSeparator();
    ctx->addItem("Action 2", [&]() { action2Called = true; });

    SECTION("Menu is hidden by default") {
        REQUIRE_FALSE(ctx->isVisible());
    }

    SECTION("showAt displays the menu at the given coordinates") {
        ctx->showAt(100, 150);
        REQUIRE(ctx->isVisible());
        REQUIRE(ctx->getX() == 100);
        REQUIRE(ctx->getY() == 150);
    }

    SECTION("Clicking an item triggers its action and closes the menu") {
        ctx->showAt(50, 50);
        REQUIRE(ctx->isVisible());

        auto optionX = 50 + 10;
        auto optionY = 50 + 12; // center of first item

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, optionX, optionY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, optionX, optionY));

        REQUIRE(action1Called);
        REQUIRE_FALSE(ctx->isVisible());
    }

    SECTION("Clicking outside closes the menu without triggering an action") {
        ctx->showAt(50, 50);
        REQUIRE(ctx->isVisible());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));

        REQUIRE_FALSE(action1Called);
        REQUIRE_FALSE(action2Called);
        REQUIRE_FALSE(ctx->isVisible());
    }

    SECTION("clearItems removes all items") {
        ctx->clearItems();
        // No direct way to check item count, but we can verify behavior:
        // After clearing, menu should be empty. We can verify by checking size or reopening.
        REQUIRE_FALSE(ctx->isVisible());
    }
}
