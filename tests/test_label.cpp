#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

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
}
