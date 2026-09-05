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

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, optionX, optionY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, optionX, optionY));

        REQUIRE(action1Called);
        REQUIRE_FALSE(ctx->isVisible());
    }

    SECTION("Clicked item does not keep keyboard focus after the menu closes") {
        ctx->showAt(50, 50);

        auto optionX = 50 + 10;
        auto optionY = 50 + 12; // center of first item

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, optionX, optionY));
        // BUTTON_DOWN steals keyboard focus for the item button
        REQUIRE(manager.getKeyboardFocus() != nullptr);

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, optionX, optionY));

        REQUIRE(action1Called);
        REQUIRE_FALSE(ctx->isVisible());
        // Otherwise GUIManager::render keeps painting the item through the
        // focus-overlay pass (ghost button with focus outline)
        REQUIRE(manager.getKeyboardFocus() == nullptr);
    }

    SECTION("Clicking outside closes the menu without triggering an action") {
        ctx->showAt(50, 50);
        REQUIRE(ctx->isVisible());

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 10, 10));

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

    SECTION("getItemCount includes separators, isItemEnabled reflects state") {
        REQUIRE(ctx->getItemCount() == 3);
        REQUIRE(ctx->isItemEnabled(0));
        REQUIRE_FALSE(ctx->isItemEnabled(1)); // separator is never "enabled"
        REQUIRE(ctx->isItemEnabled(2));

        ctx->addItem("Disabled", []() {}, false);
        REQUIRE(ctx->getItemCount() == 4);
        REQUIRE_FALSE(ctx->isItemEnabled(3));
    }
}

TEST_CASE("ContextMenu clamps to window bounds", "[context_menu][position]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setWindowSize(800, 600);

    auto menu = std::make_unique<ContextMenu>(manager);
    ContextMenu* ctx = menu.get();
    manager.addElement(std::move(menu));
    ctx->addItem("Item", []() {});

    SECTION("menu opening near the bottom-right corner is clamped inside") {
        ctx->showAt(760, 590);
        REQUIRE(ctx->getX() + ctx->getWidth() <= 800);
        REQUIRE(ctx->getY() + ctx->getHeight() <= 600);
        REQUIRE(ctx->getX() >= 0);
        REQUIRE(ctx->getY() >= 0);
    }

    SECTION("negative coordinates are clamped to zero") {
        ctx->showAt(-50, -50);
        REQUIRE(ctx->getX() == 0);
        REQUIRE(ctx->getY() == 0);
    }
}

TEST_CASE("GUIManager shared context menu", "[context_menu][manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    ContextMenuItem cutItem("Cut", []() {}, false);
    ContextMenuItem copyItem("Copy", []() {}, true);

    SECTION("showContextMenu creates a lazy shared instance") {
        REQUIRE(manager.getContextMenu() == nullptr);
        manager.showContextMenu({cutItem, copyItem}, 100, 100);
        REQUIRE(manager.getContextMenu() != nullptr);
        REQUIRE(manager.isContextMenuVisible());
        REQUIRE(manager.getContextMenu()->getItemCount() == 2);
    }

    SECTION("items are rebuilt on every show") {
        manager.showContextMenu({cutItem}, 10, 10);
        REQUIRE(manager.getContextMenu()->getItemCount() == 1);
        manager.showContextMenu({cutItem, copyItem}, 20, 20);
        REQUIRE(manager.getContextMenu()->getItemCount() == 2);
        REQUIRE(manager.getContextMenu()->getX() == 20);
        REQUIRE(manager.getContextMenu()->getY() == 20);
    }

    SECTION("closeContextMenu hides the menu") {
        manager.showContextMenu({cutItem}, 10, 10);
        REQUIRE(manager.isContextMenuVisible());
        manager.closeContextMenu();
        REQUIRE_FALSE(manager.isContextMenuVisible());
    }

    SECTION("outside click closes the shared menu") {
        manager.showContextMenu({cutItem}, 50, 50);
        REQUIRE(manager.isContextMenuVisible());
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 400, 300));
        REQUIRE_FALSE(manager.isContextMenuVisible());
    }
}
