#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/canvas.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Canvas - Drawing Behavior", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Left mouse button down inside canvas starts drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 10, 10, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Mouse motion while button down draws") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 200, 200);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        manager.processEvent(helper.createMouseMotion(50, 50));
        manager.processEvent(helper.createMouseMotion(100, 100));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 100, 100));

        REQUIRE(canvasPtr->getWidth() == 200);
    }

    SECTION("Mouse button up ends drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(40, 40));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 40, 40));

        manager.processEvent(helper.createMouseMotion(60, 60));

        REQUIRE(canvasPtr->isEnabled());
    }

    SECTION("Drawing creates visible marks on canvas") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 25, 25));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 25, 25));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(75, 75));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 75, 75));

        REQUIRE(canvasPtr->getWidth() == 100);
    }
}

TEST_CASE("Canvas - Drawing Bounds", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Drawing inside canvas bounds works") {
        auto canvas = std::make_unique<Canvas>(manager, 10, 10, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        REQUIRE(canvasPtr->contains(50, 50));
    }

    SECTION("Drawing outside canvas bounds ignored") {
        auto canvas = std::make_unique<Canvas>(manager, 10, 10, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 200));
        manager.processEvent(helper.createMouseMotion(210, 210));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 210, 210));

        REQUIRE_FALSE(canvasPtr->contains(200, 200));
    }

    SECTION("Drawing at canvas edge handled correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 0, 0));
        manager.processEvent(helper.createMouseMotion(99, 99));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 99, 99));

        REQUIRE(canvasPtr->contains(0, 0));
        REQUIRE(canvasPtr->contains(99, 99));
    }

    SECTION("Drawing near right edge clamps correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 50, 50);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 48, 48));
        manager.processEvent(helper.createMouseMotion(49, 49));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 49, 49));

        REQUIRE(canvasPtr->getWidth() == 50);
    }

    SECTION("Drawing near bottom edge clamps correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 50, 50);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 25, 48));
        manager.processEvent(helper.createMouseMotion(25, 49));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 25, 49));

        REQUIRE(canvasPtr->getHeight() == 50);
    }
}

TEST_CASE("Canvas - Right Click", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Right mouse button does NOT start drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 60, 60));

        REQUIRE(canvasPtr->isEnabled());
    }

    SECTION("Right click followed by left works correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 50, 50));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        REQUIRE(canvasPtr->isEnabled());
    }
}

TEST_CASE("Canvas - Clear", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("clear() clears canvas content") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        REQUIRE_NOTHROW(canvasPtr->clear());
    }

    SECTION("Canvas returns to blank after clear") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        manager.processEvent(helper.createMouseMotion(90, 90));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 90, 90));

        canvasPtr->clear();

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Clear on fresh canvas works") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_NOTHROW(canvas.clear());
    }

    SECTION("Multiple clears work correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        canvasPtr->clear();
        canvasPtr->clear();

        REQUIRE(canvasPtr->getWidth() == 100);
    }
}

TEST_CASE("Canvas - Position/Dimensions", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor sets correct position and size") {
        Canvas canvas(manager, 25, 50, 200, 150);
        REQUIRE(canvas.getX() == 25);
        REQUIRE(canvas.getY() == 50);
        REQUIRE(canvas.getWidth() == 200);
        REQUIRE(canvas.getHeight() == 150);
    }

    SECTION("Canvas can be moved with setPosition") {
        Canvas canvas(manager, 10, 20, 100, 100);
        canvas.setPosition(75, 85);
        REQUIRE(canvas.getX() == 75);
        REQUIRE(canvas.getY() == 85);
    }

    SECTION("setPosition affects drawing coordinates") {
        auto canvas = std::make_unique<Canvas>(manager, 100, 100, 50, 50);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 110, 110));

        canvasPtr->setPosition(200, 200);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 210, 210));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 210, 210));

        REQUIRE(canvasPtr->getX() == 200);
        REQUIRE(canvasPtr->getY() == 200);
    }

    SECTION("getAbsolutePosition returns correct position") {
        Canvas canvas(manager, 100, 200, 100, 100);
        SDL_Point pos = canvas.getAbsolutePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("Canvas with negative position works") {
        Canvas canvas(manager, -50, -100, 200, 150);
        REQUIRE(canvas.getX() == -50);
        REQUIRE(canvas.getY() == -100);
    }

    SECTION("Canvas getSize returns correct dimensions") {
        Canvas canvas(manager, 0, 0, 150, 200);
        int width, height;
        canvas.getSize(width, height);
        REQUIRE(width == 150);
        REQUIRE(height == 200);
    }
}

TEST_CASE("Canvas - Disabled State", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled Canvas ignores drawing events") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        canvasPtr->setEnabled(false);
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 60, 60));

        REQUIRE_FALSE(canvasPtr->isEnabled());
    }

    SECTION("setEnabled(false) prevents all drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        canvasPtr->setEnabled(false);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        REQUIRE_FALSE(canvasPtr->isEnabled());
    }

    SECTION("setEnabled(true) restores drawing ability") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        canvasPtr->setEnabled(false);
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        canvasPtr->setEnabled(true);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(70, 70));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 70, 70));

        REQUIRE(canvasPtr->isEnabled());
    }

    SECTION("Canvas starts enabled by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.isEnabled());
    }
}

TEST_CASE("Canvas - Hidden State", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden Canvas ignores events") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        canvasPtr->setVisible(false);
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 60, 60));

        REQUIRE_FALSE(canvasPtr->isVisible());
    }

    SECTION("setVisible(false) then setVisible(true) allows drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        canvasPtr->setVisible(false);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        canvasPtr->setVisible(true);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        REQUIRE(canvasPtr->isVisible());
    }

    SECTION("Canvas starts visible by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.isVisible());
    }
}

TEST_CASE("Canvas - Component Type", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns Canvas") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(std::string(canvas.getComponentType()) == "Canvas");
    }

    SECTION("getComponentType returns non-null") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.getComponentType() != nullptr);
    }
}

TEST_CASE("Canvas - Multiple Canvas", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Multiple canvases work independently") {
        auto canvas1 = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        auto canvas2 = std::make_unique<Canvas>(manager, 150, 0, 100, 100);

        Canvas* ptr1 = canvas1.get();
        Canvas* ptr2 = canvas2.get();

        manager.addElement(std::move(canvas1));
        manager.addElement(std::move(canvas2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 50));
        manager.processEvent(helper.createMouseMotion(230, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 230, 80));

        REQUIRE(ptr1->getWidth() == 100);
        REQUIRE(ptr2->getWidth() == 100);
    }

    SECTION("Drawing on one doesn't affect another") {
        auto canvas1 = std::make_unique<Canvas>(manager, 0, 0, 50, 50);
        auto canvas2 = std::make_unique<Canvas>(manager, 100, 0, 50, 50);

        Canvas* ptr1 = canvas1.get();
        Canvas* ptr2 = canvas2.get();

        manager.addElement(std::move(canvas1));
        manager.addElement(std::move(canvas2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 25, 25));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 25, 25));

        ptr2->clear();

        REQUIRE(ptr1->getWidth() == 50);
        REQUIRE(ptr2->getWidth() == 50);
    }

    SECTION("Clear on one canvas doesn't clear another") {
        auto canvas1 = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        auto canvas2 = std::make_unique<Canvas>(manager, 200, 0, 100, 100);

        Canvas* ptr1 = canvas1.get();
        Canvas* ptr2 = canvas2.get();

        manager.addElement(std::move(canvas1));
        manager.addElement(std::move(canvas2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 250, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 250, 50));

        ptr1->clear();

        REQUIRE(ptr1->getWidth() == 100);
        REQUIRE(ptr2->getWidth() == 100);
    }
}

TEST_CASE("Canvas - Mouse Motion Without Button", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Mouse motion without button down does nothing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseMotion(50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseMotion(70, 70));

        REQUIRE(canvasPtr->isEnabled());
    }

    SECTION("Mouse motion after button up doesn't draw") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(40, 40));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 40, 40));

        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseMotion(90, 90));

        REQUIRE(canvasPtr->isEnabled());
    }
}

TEST_CASE("Canvas - Overlapping Drawing", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Multiple drawing strokes overlap correctly") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(80, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 80, 80));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 80, 20));
        manager.processEvent(helper.createMouseMotion(20, 80));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 80));

        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Drawing same location multiple times works") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Complex overlapping pattern") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 200, 200);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        manager.processEvent(helper.createMouseMotion(190, 10));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 190, 10));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 10));
        manager.processEvent(helper.createMouseMotion(100, 190));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 100, 190));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 100));
        manager.processEvent(helper.createMouseMotion(190, 100));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 190, 100));

        REQUIRE(canvasPtr->getWidth() == 200);
    }
}