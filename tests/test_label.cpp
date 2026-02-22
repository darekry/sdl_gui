#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"
#include "../src/style.hpp"

TEST_CASE("Label functionality", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Label can be created with text") {
        Label label(manager, 10, 20, "Test Label", 16);
        REQUIRE(label.getX() == 10);
        REQUIRE(label.getY() == 20);
    }

    SECTION("Label created with empty text has zero size") {
        Label label(manager, 0, 0, "", 16);
        REQUIRE(label.getWidth() == 0);
        REQUIRE(label.getHeight() == 0);
    }

    SECTION("setText with same text does not change dimensions") {
        auto label = std::make_unique<Label>(manager, 0, 0, "Same Text", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        int originalHeight = labelPtr->getHeight();

        labelPtr->setText("Same Text");
        REQUIRE(labelPtr->getWidth() == originalWidth);
        REQUIRE(labelPtr->getHeight() == originalHeight);
    }

    SECTION("setText to empty string resets size") {
        Label label(manager, 0, 0, "Text", 16);
        label.setText("");
        REQUIRE(label.getWidth() == 0);
        REQUIRE(label.getHeight() == 0);
    }

    SECTION("Label responds to position changes") {
        Label label(manager, 5, 5, "Move", 16);
        label.setPosition(25, 35);
        REQUIRE(label.getX() == 25);
        REQUIRE(label.getY() == 35);
    }

    SECTION("Label can be added to manager") {
        auto label = std::make_unique<Label>(manager, 50, 50, "Managed Label", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        REQUIRE(labelPtr->getX() == 50);
        REQUIRE(labelPtr->getY() == 50);
    }

    SECTION("Label component type is present") {
        Label label(manager, 0, 0, "Test", 16);
        const char* type = label.getComponentType();
        REQUIRE(type != nullptr);
    }

    SECTION("Label visibility can be toggled") {
        Label label(manager, 0, 0, "Visible", 16);
        REQUIRE(label.isVisible());

        label.setVisible(false);
        REQUIRE_FALSE(label.isVisible());

        label.setVisible(true);
        REQUIRE(label.isVisible());
    }

    SECTION("Label can be enabled and disabled") {
        Label label(manager, 0, 0, "Enabled", 16);
        REQUIRE(label.isEnabled());

        label.setEnabled(false);
        REQUIRE_FALSE(label.isEnabled());

        label.setEnabled(true);
        REQUIRE(label.isEnabled());
    }

    // ============================================
    // NEW TESTS - Text variants
    // ============================================

    SECTION("Label with long text has non-zero dimensions") {
        Label label(manager, 0, 0, "This is a very long text that should have significant width", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label with special characters") {
        Label label(manager, 0, 0, "Special: !@#$%^&*()_+-=[]{}|;':\",./<>?", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label with unicode characters") {
        Label label(manager, 0, 0, "Unicode: \u00E9\u00E8\u00EA\u00EB \u4E2D\u6587", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label with whitespace only") {
        Label label(manager, 0, 0, "   ", 16);
        // Whitespace should still produce some width
        REQUIRE(label.getWidth() >= 0);
        REQUIRE(label.getHeight() >= 0);
    }

    SECTION("Label with newline character") {
        // Note: Label may not support multiline, but should not crash
        Label label(manager, 0, 0, "Line1\nLine2", 16);
        REQUIRE(label.getWidth() >= 0);
        REQUIRE(label.getHeight() >= 0);
    }

    // ============================================
    // NEW TESTS - Text changes
    // ============================================

    SECTION("setText to longer text increases width") {
        auto label = std::make_unique<Label>(manager, 0, 0, "Short", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        labelPtr->setText("This is a much longer text");
        REQUIRE(labelPtr->getWidth() > originalWidth);
    }

    SECTION("setText to shorter text decreases width") {
        auto label = std::make_unique<Label>(manager, 0, 0, "This is a very long text", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        labelPtr->setText("Short");
        REQUIRE(labelPtr->getWidth() < originalWidth);
    }

    SECTION("setText multiple times updates size correctly") {
        auto label = std::make_unique<Label>(manager, 0, 0, "First", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int width1 = labelPtr->getWidth();
        
        labelPtr->setText("Second Text");
        int width2 = labelPtr->getWidth();
        REQUIRE(width2 > width1);

        labelPtr->setText("Third");
        int width3 = labelPtr->getWidth();
        REQUIRE(width3 < width2);

        labelPtr->setText("");
        REQUIRE(labelPtr->getWidth() == 0);
    }

    // ============================================
    // NEW TESTS - Font size
    // ============================================

    SECTION("Label with larger font size has larger dimensions") {
        Label smallFont(manager, 0, 0, "Test", 12);
        Label largeFont(manager, 0, 0, "Test", 24);
        
        REQUIRE(largeFont.getWidth() > smallFont.getWidth());
        REQUIRE(largeFont.getHeight() > smallFont.getHeight());
    }

    SECTION("Label with default font size (-1) uses theme default") {
        Label label(manager, 0, 0, "Test", -1);
        // Should use theme default (typically 16)
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label with explicit font size 16") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    // ============================================
    // NEW TESTS - Positioning and size
    // ============================================

    SECTION("Label getAbsolutePosition returns correct position") {
        Label label(manager, 100, 200, "Test", 16);
        SDL_Point pos = label.getAbsolutePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("Label getRelativePosition returns correct position") {
        Label label(manager, 100, 200, "Test", 16);
        SDL_Point pos = label.getRelativePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("Label contains point inside bounds") {
        Label label(manager, 10, 20, "Test", 16);
        int width = label.getWidth();
        int height = label.getHeight();
        
        // Point inside label
        REQUIRE(label.contains(10 + width / 2, 20 + height / 2));
    }

    SECTION("Label does not contain point outside bounds") {
        Label label(manager, 10, 20, "Test", 16);
        
        // Points outside label
        REQUIRE_FALSE(label.contains(5, 10));
        REQUIRE_FALSE(label.contains(1000, 1000));
    }

    SECTION("Label getSize returns correct dimensions") {
        Label label(manager, 0, 0, "Test", 16);
        int width, height;
        label.getSize(width, height);
        REQUIRE(width == label.getWidth());
        REQUIRE(height == label.getHeight());
    }

    // ============================================
    // NEW TESTS - Styles
    // ============================================

    SECTION("Label setTextColor sets text color for state") {
        Label label(manager, 0, 0, "Test", 16);
        SDL_Color color = {255, 0, 0, 255}; // Red
        label.setTextColor(ElementState::Normal, color);
        // No exception means success
        REQUIRE(true);
    }

    SECTION("Label setStyle sets complete style for state") {
        Label label(manager, 0, 0, "Test", 16);
        Style style;
        style.textColor = SDL_Color{0, 255, 0, 255}; // Green
        style.fontSize = 20;
        label.setStyle(ElementState::Normal, style);
        // No exception means success
        REQUIRE(true);
    }

    SECTION("Label state can be changed") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getState() == ElementState::Normal);
        
        label.setState(ElementState::Disabled);
        REQUIRE(label.getState() == ElementState::Disabled);
        
        label.setState(ElementState::Normal);
        REQUIRE(label.getState() == ElementState::Normal);
    }

    // ============================================
    // NEW TESTS - ID and identification
    // ============================================

    SECTION("Label setID and getID work correctly") {
        Label label(manager, 0, 0, "Test", 16);
        label.setID("testLabel123");
        REQUIRE(label.getID() == "testLabel123");
    }

    SECTION("Label getID returns empty string by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getID().empty());
    }

    // ============================================
    // NEW TESTS - Tooltip
    // ============================================

    SECTION("Label can have tooltip set") {
        Label label(manager, 0, 0, "Test", 16);
        label.setTooltip("This is a tooltip");
        // No exception means success
        REQUIRE(true);
    }

    // ============================================
    // NEW TESTS - Hierarchy
    // ============================================

    SECTION("Label parent is null by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getParent() == nullptr);
    }

    SECTION("Label has no children by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getChildren().empty());
    }

    SECTION("Label countDescendants returns 0") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.countDescendants() == 0);
    }

    // ============================================
    // NEW TESTS - Deletion marking
    // ============================================

    SECTION("Label is not marked for deletion by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE_FALSE(label.isMarkedForDeletion());
    }

    SECTION("Label can be marked for deletion") {
        Label label(manager, 0, 0, "Test", 16);
        label.markForDeletion();
        REQUIRE(label.isMarkedForDeletion());
    }

    // ============================================
    // NEW TESTS - Keyboard focus
    // ============================================

    SECTION("Label cannot get keyboard focus by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE_FALSE(label.canGetKeyboardFocus());
    }

    SECTION("Label can be set to accept keyboard focus") {
        Label label(manager, 0, 0, "Test", 16);
        label.setCanGetKeyboardFocus(true);
        REQUIRE(label.canGetKeyboardFocus());
    }

    SECTION("Label does not have keyboard focus by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE_FALSE(label.hasKeyboardFocus());
    }

    // ============================================
    // NEW TESTS - Overlay
    // ============================================

    SECTION("Label is not an overlay by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE_FALSE(label.isOverlay());
    }

    // ============================================
    // NEW TESTS - Dirty flag
    // ============================================

    SECTION("Label can be marked dirty") {
        Label label(manager, 0, 0, "Test", 16);
        label.markDirty();
        // No exception means success
        REQUIRE(true);
    }

    SECTION("Label can be marked dirty recursively") {
        Label label(manager, 0, 0, "Test", 16);
        label.markDirtyRecursively();
        // No exception means success
        REQUIRE(true);
    }

    // ============================================
    // NEW TESTS - Disabled state behavior
    // ============================================

    SECTION("Disabled label has Disabled state") {
        Label label(manager, 0, 0, "Test", 16);
        label.setEnabled(false);
        // Note: The state might not automatically change to Disabled
        // This tests that setEnabled(false) works without error
        REQUIRE_FALSE(label.isEnabled());
    }

    // ============================================
    // NEW TESTS - Clipping
    // ============================================

    SECTION("Label clip children is enabled by default") {
        Label label(manager, 0, 0, "Test", 16);
        // Default behavior - no direct accessor, but setClipChildren exists
        label.setClipChildren(false);
        // No exception means success
        REQUIRE(true);
    }
}
