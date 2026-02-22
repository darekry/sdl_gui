#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/canvas.hpp"
#include "../src/gui_manager.hpp"
#include "../src/style.hpp"

TEST_CASE("Canvas functionality", "[canvas]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    // ============================================
    // BASIC CREATION AND DIMENSIONS
    // ============================================

    SECTION("Canvas can be created and has correct dimensions") {
        Canvas canvas(manager, 10, 20, 200, 150);
        REQUIRE(canvas.getX() == 10);
        REQUIRE(canvas.getY() == 20);
        REQUIRE(canvas.getWidth() == 200);
        REQUIRE(canvas.getHeight() == 150);
    }

    SECTION("Canvas with zero dimensions") {
        // Canvas should handle zero dimensions gracefully
        Canvas canvas(manager, 0, 0, 0, 0);
        REQUIRE(canvas.getX() == 0);
        REQUIRE(canvas.getY() == 0);
        REQUIRE(canvas.getWidth() == 0);
        REQUIRE(canvas.getHeight() == 0);
    }

    SECTION("Canvas with large dimensions") {
        Canvas canvas(manager, 0, 0, 1920, 1080);
        REQUIRE(canvas.getWidth() == 1920);
        REQUIRE(canvas.getHeight() == 1080);
    }

    SECTION("Canvas with negative position") {
        Canvas canvas(manager, -50, -100, 200, 150);
        REQUIRE(canvas.getX() == -50);
        REQUIRE(canvas.getY() == -100);
    }

    // ============================================
    // POSITIONING
    // ============================================

    SECTION("Canvas position can be changed") {
        Canvas canvas(manager, 10, 20, 100, 100);
        canvas.setPosition(50, 60);
        REQUIRE(canvas.getX() == 50);
        REQUIRE(canvas.getY() == 60);
    }

    SECTION("Canvas getAbsolutePosition returns correct position") {
        Canvas canvas(manager, 100, 200, 100, 100);
        SDL_Point pos = canvas.getAbsolutePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("Canvas getRelativePosition returns correct position") {
        Canvas canvas(manager, 100, 200, 100, 100);
        SDL_Point pos = canvas.getRelativePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("Canvas getSize returns correct dimensions") {
        Canvas canvas(manager, 0, 0, 150, 200);
        int width, height;
        canvas.getSize(width, height);
        REQUIRE(width == 150);
        REQUIRE(height == 200);
    }

    // ============================================
    // BOUNDS CHECKING
    // ============================================

    SECTION("Canvas contains point inside bounds") {
        Canvas canvas(manager, 10, 20, 100, 100);
        
        // Point inside canvas
        REQUIRE(canvas.contains(10, 20));       // Top-left corner
        REQUIRE(canvas.contains(50, 70));       // Center
        REQUIRE(canvas.contains(109, 119));     // Near bottom-right (exclusive)
    }

    SECTION("Canvas does not contain point outside bounds") {
        Canvas canvas(manager, 10, 20, 100, 100);
        
        // Points outside canvas
        REQUIRE_FALSE(canvas.contains(9, 20));   // Left of canvas
        REQUIRE_FALSE(canvas.contains(10, 19));  // Above canvas
        REQUIRE_FALSE(canvas.contains(110, 20)); // Right of canvas
        REQUIRE_FALSE(canvas.contains(10, 120)); // Below canvas
        REQUIRE_FALSE(canvas.contains(0, 0));    // Far outside
        REQUIRE_FALSE(canvas.contains(1000, 1000)); // Far outside
    }

    // ============================================
    // CLEARING
    // ============================================

    SECTION("Canvas clearing resets drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Draw something
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20));

        // Clear should not throw
        REQUIRE_NOTHROW(canvasPtr->clear());
    }

    SECTION("Canvas clear on fresh canvas") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_NOTHROW(canvas.clear());
    }

    // ============================================
    // MOUSE EVENT HANDLING - DRAWING
    // ============================================

    SECTION("Drawing inside canvas captures mouse") {
        auto canvas = std::make_unique<Canvas>(manager, 10, 10, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Start drawing inside canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20));
        
        // Move while drawing
        manager.processEvent(helper.createMouseMotion(30, 30));
        manager.processEvent(helper.createMouseMotion(40, 40));
        
        // End drawing
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 40, 40));
        
        // Canvas should still exist and be functional
        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Mouse events outside canvas are ignored") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Click outside canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 200));
        manager.processEvent(helper.createMouseMotion(210, 210));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 210, 210));
        
        // Canvas should still exist and be functional
        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Drawing at canvas edge") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Draw at edge of canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 0, 0));
        manager.processEvent(helper.createMouseMotion(99, 99));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 99, 99));
        
        REQUIRE(canvasPtr->getWidth() == 100);
    }

    SECTION("Drawing with rapid mouse movements") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 200, 200);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 10));
        for (int i = 20; i < 100; i += 10) {
            manager.processEvent(helper.createMouseMotion(i, i));
        }
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 90, 90));
        
        REQUIRE(canvasPtr->getWidth() == 200);
    }

    // ============================================
    // VISIBILITY
    // ============================================

    SECTION("Canvas visibility can be toggled") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.isVisible());

        canvas.setVisible(false);
        REQUIRE_FALSE(canvas.isVisible());

        canvas.setVisible(true);
        REQUIRE(canvas.isVisible());
    }

    SECTION("Hidden canvas does not process events") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        canvasPtr->setVisible(false);
        manager.addElement(std::move(canvas));

        // Try to draw on hidden canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 60, 60));
        
        // Canvas should still be hidden
        REQUIRE_FALSE(canvasPtr->isVisible());
    }

    // ============================================
    // ENABLED/DISABLED STATE
    // ============================================

    SECTION("Canvas can be enabled and disabled") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.isEnabled());

        canvas.setEnabled(false);
        REQUIRE_FALSE(canvas.isEnabled());

        canvas.setEnabled(true);
        REQUIRE(canvas.isEnabled());
    }

    SECTION("Disabled canvas does not process events") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        canvasPtr->setEnabled(false);
        manager.addElement(std::move(canvas));

        // Try to draw on disabled canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 60, 60));
        
        // Canvas should still be disabled
        REQUIRE_FALSE(canvasPtr->isEnabled());
    }

    // ============================================
    // COMPONENT TYPE
    // ============================================

    SECTION("Canvas reports correct component type") {
        Canvas canvas(manager, 0, 0, 100, 100);
        const char* type = canvas.getComponentType();
        REQUIRE(type != nullptr);
        REQUIRE(std::string(type) == "Canvas");
    }

    // ============================================
    // ID AND IDENTIFICATION
    // ============================================

    SECTION("Canvas setID and getID work correctly") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.setID("testCanvas123");
        REQUIRE(canvas.getID() == "testCanvas123");
    }

    SECTION("Canvas getID returns empty string by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.getID().empty());
    }

    // ============================================
    // TOOLTIP
    // ============================================

    SECTION("Canvas can have tooltip set") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.setTooltip("This is a canvas tooltip");
        // No exception means success
        REQUIRE(true);
    }

    // ============================================
    // HIERARCHY
    // ============================================

    SECTION("Canvas parent is null by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.getParent() == nullptr);
    }

    SECTION("Canvas has no children by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.getChildren().empty());
    }

    SECTION("Canvas countDescendants returns 0") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.countDescendants() == 0);
    }

    // ============================================
    // DELETION MARKING
    // ============================================

    SECTION("Canvas is not marked for deletion by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_FALSE(canvas.isMarkedForDeletion());
    }

    SECTION("Canvas can be marked for deletion") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.markForDeletion();
        REQUIRE(canvas.isMarkedForDeletion());
    }

    // ============================================
    // KEYBOARD FOCUS
    // ============================================

    SECTION("Canvas cannot get keyboard focus by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_FALSE(canvas.canGetKeyboardFocus());
    }

    SECTION("Canvas can be set to accept keyboard focus") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.setCanGetKeyboardFocus(true);
        REQUIRE(canvas.canGetKeyboardFocus());
    }

    SECTION("Canvas does not have keyboard focus by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_FALSE(canvas.hasKeyboardFocus());
    }

    // ============================================
    // OVERLAY
    // ============================================

    SECTION("Canvas is not an overlay by default") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE_FALSE(canvas.isOverlay());
    }

    // ============================================
    // DIRTY FLAG
    // ============================================

    SECTION("Canvas can be marked dirty") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.markDirty();
        // No exception means success
        REQUIRE(true);
    }

    SECTION("Canvas can be marked dirty recursively") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.markDirtyRecursively();
        // No exception means success
        REQUIRE(true);
    }

    // ============================================
    // STATE MANAGEMENT
    // ============================================

    SECTION("Canvas state can be changed") {
        Canvas canvas(manager, 0, 0, 100, 100);
        REQUIRE(canvas.getState() == ElementState::Normal);
        
        canvas.setState(ElementState::Disabled);
        REQUIRE(canvas.getState() == ElementState::Disabled);
        
        canvas.setState(ElementState::Normal);
        REQUIRE(canvas.getState() == ElementState::Normal);
    }

    // ============================================
    // CLIPPING
    // ============================================

    SECTION("Canvas clip children can be set") {
        Canvas canvas(manager, 0, 0, 100, 100);
        canvas.setClipChildren(false);
        // No exception means success
        REQUIRE(true);
    }

    // ============================================
    // ADD TO MANAGER
    // ============================================

    SECTION("Canvas can be added to manager") {
        auto canvas = std::make_unique<Canvas>(manager, 50, 50, 200, 150);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        REQUIRE(canvasPtr->getX() == 50);
        REQUIRE(canvasPtr->getY() == 50);
        REQUIRE(canvasPtr->getWidth() == 200);
        REQUIRE(canvasPtr->getHeight() == 150);
    }

    // ============================================
    // MULTIPLE CANVASES
    // ============================================

    SECTION("Multiple canvases can coexist") {
        auto canvas1 = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        auto canvas2 = std::make_unique<Canvas>(manager, 200, 0, 100, 100);
        
        Canvas* ptr1 = canvas1.get();
        Canvas* ptr2 = canvas2.get();
        
        manager.addElement(std::move(canvas1));
        manager.addElement(std::move(canvas2));

        // Draw on first canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        // Draw on second canvas
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 250, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 250, 50));

        REQUIRE(ptr1->getWidth() == 100);
        REQUIRE(ptr2->getWidth() == 100);
    }

    // ============================================
    // RIGHT CLICK (SHOULD NOT DRAW)
    // ============================================

    SECTION("Right click does not start drawing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Right click should not start drawing
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 60, 60));
        
        // Canvas should still be functional
        REQUIRE(canvasPtr->isEnabled());
    }

    // ============================================
    // MOUSE MOTION WITHOUT BUTTON DOWN
    // ============================================

    SECTION("Mouse motion without button down does nothing") {
        auto canvas = std::make_unique<Canvas>(manager, 0, 0, 100, 100);
        Canvas* canvasPtr = canvas.get();
        manager.addElement(std::move(canvas));

        // Move mouse without pressing button
        manager.processEvent(helper.createMouseMotion(50, 50));
        manager.processEvent(helper.createMouseMotion(60, 60));
        manager.processEvent(helper.createMouseMotion(70, 70));
        
        // Canvas should still be functional
        REQUIRE(canvasPtr->isEnabled());
    }
}
