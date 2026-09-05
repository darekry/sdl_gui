#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/cursor.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Cursor - Construction", "[cursor]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Cursor can be created") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor != nullptr);
    }

    SECTION("Initial state is Normal") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor->getState() == CursorState::Normal);
    }

    SECTION("Cursor is an overlay element") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor->isOverlay() == true);
    }

    SECTION("Initial scale is 1.0") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor->getScale() == 1.0f);
    }

    SECTION("Initial offset is (0, 0)") {
        auto cursor = std::make_unique<Cursor>(manager);
        int offsetX, offsetY;
        cursor->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == 0);
        REQUIRE(offsetY == 0);
    }

    SECTION("getComponentTypeIdId returns Cursor") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor->getComponentTypeId() == ComponentType::Cursor);
    }
}

TEST_CASE("Cursor - State Management", "[cursor][state]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setState changes state") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setState(CursorState::Hover);
        REQUIRE(cursorPtr->getState() == CursorState::Hover);

        cursorPtr->setState(CursorState::Pressed);
        REQUIRE(cursorPtr->getState() == CursorState::Pressed);

        cursorPtr->setState(CursorState::Disabled);
        REQUIRE(cursorPtr->getState() == CursorState::Disabled);

        cursorPtr->setState(CursorState::Normal);
        REQUIRE(cursorPtr->getState() == CursorState::Normal);
    }

    SECTION("setState to all available states") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setState(CursorState::Normal);
        REQUIRE(cursorPtr->getState() == CursorState::Normal);

        cursorPtr->setState(CursorState::Hover);
        REQUIRE(cursorPtr->getState() == CursorState::Hover);

        cursorPtr->setState(CursorState::Pressed);
        REQUIRE(cursorPtr->getState() == CursorState::Pressed);

        cursorPtr->setState(CursorState::Disabled);
        REQUIRE(cursorPtr->getState() == CursorState::Disabled);

        cursorPtr->setState(CursorState::Busy);
        REQUIRE(cursorPtr->getState() == CursorState::Busy);

        cursorPtr->setState(CursorState::Text);
        REQUIRE(cursorPtr->getState() == CursorState::Text);

        cursorPtr->setState(CursorState::Custom1);
        REQUIRE(cursorPtr->getState() == CursorState::Custom1);

        cursorPtr->setState(CursorState::Custom2);
        REQUIRE(cursorPtr->getState() == CursorState::Custom2);

        cursorPtr->setState(CursorState::Custom3);
        REQUIRE(cursorPtr->getState() == CursorState::Custom3);
    }

    SECTION("getState returns current state after setState") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE(cursorPtr->getState() == CursorState::Normal);

        cursorPtr->setState(CursorState::Custom1);
        REQUIRE(cursorPtr->getState() == CursorState::Custom1);

        // State persists
        REQUIRE(cursorPtr->getState() == CursorState::Custom1);
    }
}

TEST_CASE("Cursor - Offset Management", "[cursor][offset]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setOffset changes offset values") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setOffset(10, 20);
        int offsetX, offsetY;
        cursorPtr->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == 10);
        REQUIRE(offsetY == 20);
    }

    SECTION("setOffset can be set multiple times") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setOffset(5, 5);
        int offsetX, offsetY;
        cursorPtr->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == 5);
        REQUIRE(offsetY == 5);

        cursorPtr->setOffset(15, 25);
        cursorPtr->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == 15);
        REQUIRE(offsetY == 25);
    }

    SECTION("setOffset with zero values") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setOffset(10, 20);
        cursorPtr->setOffset(0, 0);
        int offsetX, offsetY;
        cursorPtr->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == 0);
        REQUIRE(offsetY == 0);
    }

    SECTION("setOffset with negative values") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setOffset(-5, -10);
        int offsetX, offsetY;
        cursorPtr->getOffset(offsetX, offsetY);
        REQUIRE(offsetX == -5);
        REQUIRE(offsetY == -10);
    }
}

TEST_CASE("Cursor - Scale Management", "[cursor][scale]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setScale changes scale value") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE(cursorPtr->getScale() == 1.0f);

        cursorPtr->setScale(2.0f);
        REQUIRE(cursorPtr->getScale() == 2.0f);

        cursorPtr->setScale(0.5f);
        REQUIRE(cursorPtr->getScale() == 0.5f);
    }

    SECTION("setScale can be set multiple times") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setScale(1.5f);
        REQUIRE(cursorPtr->getScale() == 1.5f);

        cursorPtr->setScale(3.0f);
        REQUIRE(cursorPtr->getScale() == 3.0f);

        cursorPtr->setScale(0.25f);
        REQUIRE(cursorPtr->getScale() == 0.25f);
    }

    SECTION("setScale with zero clamps to minimum 0.1") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setScale(0.0f);
        // Library intentionally clamps scale to minimum 0.1
        REQUIRE(cursorPtr->getScale() == 0.1f);
    }

    SECTION("setScale preserves last set value") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setScale(2.5f);
        // Check multiple times - should remain the same
        REQUIRE(cursorPtr->getScale() == 2.5f);
        REQUIRE(cursorPtr->getScale() == 2.5f);
    }
}

TEST_CASE("Cursor - Visibility", "[cursor][visibility]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Cursor is visible by default") {
        auto cursor = std::make_unique<Cursor>(manager);
        REQUIRE(cursor->isVisible() == true);
    }

    SECTION("setVisible(false) hides cursor") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setVisible(false);
        REQUIRE(cursorPtr->isVisible() == false);
    }

    SECTION("setVisible(true) shows cursor") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        cursorPtr->setVisible(false);
        manager.addElement(std::move(cursor));

        cursorPtr->setVisible(true);
        REQUIRE(cursorPtr->isVisible() == true);
    }

    SECTION("Visibility can be toggled multiple times") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setVisible(false);
        REQUIRE(cursorPtr->isVisible() == false);

        cursorPtr->setVisible(true);
        REQUIRE(cursorPtr->isVisible() == true);

        cursorPtr->setVisible(false);
        REQUIRE(cursorPtr->isVisible() == false);

        cursorPtr->setVisible(true);
        REQUIRE(cursorPtr->isVisible() == true);
    }
}

TEST_CASE("Cursor - OnStateChanged Callback", "[cursor][callback]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setOnStateChanged callback fires on state change") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        bool callbackFired = false;
        CursorState callbackState = CursorState::Normal;

        cursorPtr->setOnStateChanged([&](CursorState state) {
            callbackFired = true;
            callbackState = state;
        });

        cursorPtr->setState(CursorState::Hover);

        REQUIRE(callbackFired == true);
        REQUIRE(callbackState == CursorState::Hover);
    }

    SECTION("Callback receives correct new state") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        CursorState lastState = CursorState::Normal;
        int callbackCount = 0;

        cursorPtr->setOnStateChanged([&](CursorState state) {
            lastState = state;
            callbackCount++;
        });

        cursorPtr->setState(CursorState::Pressed);
        REQUIRE(lastState == CursorState::Pressed);
        REQUIRE(callbackCount == 1);

        cursorPtr->setState(CursorState::Disabled);
        REQUIRE(lastState == CursorState::Disabled);
        REQUIRE(callbackCount == 2);
    }

    SECTION("Callback fires for all state changes") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        int callbackCount = 0;
        cursorPtr->setOnStateChanged([&](CursorState) { callbackCount++; });

        cursorPtr->setState(CursorState::Hover);
        REQUIRE(callbackCount == 1);

        cursorPtr->setState(CursorState::Pressed);
        REQUIRE(callbackCount == 2);

        cursorPtr->setState(CursorState::Busy);
        REQUIRE(callbackCount == 3);

        cursorPtr->setState(CursorState::Text);
        REQUIRE(callbackCount == 4);

        cursorPtr->setState(CursorState::Custom1);
        REQUIRE(callbackCount == 5);
    }

    SECTION("Callback can be replaced") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        int firstCallbackCount = 0;
        int secondCallbackCount = 0;

        cursorPtr->setOnStateChanged([&](CursorState) { firstCallbackCount++; });
        cursorPtr->setState(CursorState::Hover);
        REQUIRE(firstCallbackCount == 1);
        REQUIRE(secondCallbackCount == 0);

        cursorPtr->setOnStateChanged([&](CursorState) { secondCallbackCount++; });
        cursorPtr->setState(CursorState::Pressed);
        REQUIRE(firstCallbackCount == 1);
        REQUIRE(secondCallbackCount == 1);
    }

    SECTION("No callback set - setState still works") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->setState(CursorState::Hover));
        REQUIRE(cursorPtr->getState() == CursorState::Hover);
    }
}

TEST_CASE("Cursor - Cursor Texture Setup", "[cursor][texture]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setCursorTexture does not crash without texture file") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        // This will fail to load texture but should not crash
        REQUIRE_NOTHROW(cursorPtr->setCursorTexture(CursorState::Normal, "nonexistent.png"));
    }

    SECTION("setAnimatedCursor does not crash without texture file") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->setAnimatedCursor(CursorState::Normal, "nonexistent.png", 4, 1, 12.0f));
    }

    SECTION("setCursorTexture with hotspot values") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->setCursorTexture(CursorState::Hover, "cursor_hover.png", 5, 10));
        REQUIRE_NOTHROW(cursorPtr->setCursorTexture(CursorState::Pressed, "cursor_pressed.png", 0, 0));
    }

    SECTION("setAnimatedCursor with all parameters") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->setAnimatedCursor(
            CursorState::Busy, "busy_cursor.png", 8, 2, 24.0f, 8, 8));
    }
}

TEST_CASE("Cursor - Event Handling", "[cursor][events]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("handleEvent returns false for mouse motion") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        SDL_Event event = helper.createMouseMotion(100, 100);
        bool handled = cursorPtr->handleEvent(event);

        // Cursor doesn't consume events - it's an overlay
        REQUIRE(handled == false);
    }

    SECTION("handleEvent returns false for mouse button") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = cursorPtr->handleEvent(event);

        REQUIRE(handled == false);
    }
}

TEST_CASE("Cursor - Render Overlay", "[cursor][render]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("renderOverlay can be called without texture") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));
    }

    SECTION("renderOverlay works with different states") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        manager.addElement(std::move(cursor));

        cursorPtr->setState(CursorState::Hover);
        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));

        cursorPtr->setState(CursorState::Pressed);
        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));

        cursorPtr->setState(CursorState::Disabled);
        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));
    }

    SECTION("renderOverlay respects visibility") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        cursorPtr->setVisible(false);
        manager.addElement(std::move(cursor));

        // Should not crash when invisible
        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));
    }

    SECTION("renderOverlay works with scale and offset") {
        auto cursor = std::make_unique<Cursor>(manager);
        Cursor* cursorPtr = cursor.get();
        cursorPtr->setScale(2.0f);
        cursorPtr->setOffset(10, 10);
        manager.addElement(std::move(cursor));

        REQUIRE_NOTHROW(cursorPtr->renderOverlay(helper.getRenderer()));
    }
}

TEST_CASE("CursorState - Enum Values", "[cursor][enum]") {
    SECTION("All CursorState values are distinct") {
        REQUIRE(CursorState::Normal != CursorState::Hover);
        REQUIRE(CursorState::Hover != CursorState::Pressed);
        REQUIRE(CursorState::Pressed != CursorState::Disabled);
        REQUIRE(CursorState::Disabled != CursorState::Busy);
        REQUIRE(CursorState::Busy != CursorState::Text);
        REQUIRE(CursorState::Text != CursorState::Custom1);
        REQUIRE(CursorState::Custom1 != CursorState::Custom2);
        REQUIRE(CursorState::Custom2 != CursorState::Custom3);
    }

    SECTION("CursorState can be used in comparisons") {
        CursorState state = CursorState::Hover;
        REQUIRE(state == CursorState::Hover);
        REQUIRE(state != CursorState::Normal);
        REQUIRE(state != CursorState::Pressed);
    }
}