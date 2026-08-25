#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"
#include "../src/constants.hpp"

TEST_CASE("Label text handling", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor with text sets initial content") {
        Label label(manager, 10, 20, "Hello World", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Empty initial text produces zero dimensions") {
        Label label(manager, 0, 0, "", 16);
        REQUIRE(label.getWidth() == 0);
        REQUIRE(label.getHeight() == 0);
    }

    SECTION("setText updates text content and size") {
        auto label = std::make_unique<Label>(manager, 0, 0, "Initial", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int initialWidth = labelPtr->getWidth();
        REQUIRE(initialWidth > 0);

        labelPtr->setText("New Text");
        REQUIRE(labelPtr->getWidth() > 0);
        REQUIRE(labelPtr->getWidth() != initialWidth);
    }

    SECTION("setText with longer text increases width") {
        auto label = std::make_unique<Label>(manager, 0, 0, "Short", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        labelPtr->setText("This is a much longer text string");
        REQUIRE(labelPtr->getWidth() > originalWidth);
    }

    SECTION("setText with shorter text decreases width") {
        auto label = std::make_unique<Label>(manager, 0, 0, "This is a very long text string", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        labelPtr->setText("Short");
        REQUIRE(labelPtr->getWidth() < originalWidth);
    }

    SECTION("setText with same text produces no size change") {
        auto label = std::make_unique<Label>(manager, 0, 0, "Same Text", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        int originalWidth = labelPtr->getWidth();
        int originalHeight = labelPtr->getHeight();
        labelPtr->setText("Same Text");
        REQUIRE(labelPtr->getWidth() == originalWidth);
        REQUIRE(labelPtr->getHeight() == originalHeight);
    }

    SECTION("setText to empty string resets dimensions to zero") {
        Label label(manager, 0, 0, "Some Text", 16);
        REQUIRE(label.getWidth() > 0);

        label.setText("");
        REQUIRE(label.getWidth() == 0);
        REQUIRE(label.getHeight() == 0);
    }
}

TEST_CASE("Label font size", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Default font_size (-1) uses theme default") {
        Label label(manager, 0, 0, "Test", -1);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Larger font produces larger dimensions") {
        Label smallLabel(manager, 0, 0, "Test", 12);
        Label largeLabel(manager, 0, 0, "Test", 24);

        REQUIRE(largeLabel.getWidth() > smallLabel.getWidth());
        REQUIRE(largeLabel.getHeight() > smallLabel.getHeight());
    }

    SECTION("Different font sizes produce different dimensions") {
        Label label12(manager, 0, 0, "Text", 12);
        Label label16(manager, 0, 0, "Text", 16);
        Label label20(manager, 0, 0, "Text", 20);

        REQUIRE(label16.getWidth() > label12.getWidth());
        REQUIRE(label20.getWidth() > label16.getWidth());
        REQUIRE(label20.getHeight() > label16.getHeight());
        REQUIRE(label16.getHeight() > label12.getHeight());
    }
}

TEST_CASE("Label position", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor sets correct position") {
        Label label(manager, 100, 200, "Test", 16);
        REQUIRE(label.getX() == 100);
        REQUIRE(label.getY() == 200);
    }

    SECTION("setPosition updates position") {
        Label label(manager, 0, 0, "Test", 16);
        label.setPosition(50, 75);
        REQUIRE(label.getX() == 50);
        REQUIRE(label.getY() == 75);
    }

    SECTION("getAbsolutePosition returns correct position") {
        Label label(manager, 100, 200, "Test", 16);
        SDL_Point pos = label.getAbsolutePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }

    SECTION("getRelativePosition returns correct position") {
        Label label(manager, 100, 200, "Test", 16);
        SDL_Point pos = label.getRelativePosition();
        REQUIRE(pos.x == 100);
        REQUIRE(pos.y == 200);
    }
}

TEST_CASE("Label visibility and enabled state", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Label is visible by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.isVisible());
    }

    SECTION("setVisible toggles visibility") {
        Label label(manager, 0, 0, "Test", 16);

        label.setVisible(false);
        REQUIRE_FALSE(label.isVisible());

        label.setVisible(true);
        REQUIRE(label.isVisible());
    }

    SECTION("Label is enabled by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.isEnabled());
    }

    SECTION("setEnabled toggles enabled state") {
        Label label(manager, 0, 0, "Test", 16);

        label.setEnabled(false);
        REQUIRE_FALSE(label.isEnabled());

        label.setEnabled(true);
        REQUIRE(label.isEnabled());
    }
}

TEST_CASE("Label special characters", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Label handles unicode characters correctly") {
        Label label(manager, 0, 0, "Unicode: éèêë 中文", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label handles special characters") {
        Label label(manager, 0, 0, "Special: !@#$%^&*()_+-=[]{}|;':\",./<>?", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("Label handles whitespace") {
        Label label(manager, 0, 0, "   ", 16);
        REQUIRE(label.getWidth() >= 0);
        REQUIRE(label.getHeight() >= 0);
    }

    SECTION("Label handles newline character without crash") {
        Label label(manager, 0, 0, "Line1\nLine2", 16);
        REQUIRE(label.getWidth() >= 0);
        REQUIRE(label.getHeight() >= 0);
    }

    SECTION("Label handles mixed content") {
        Label label(manager, 0, 0, "Hello 世界! @#$ 123", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }
}

TEST_CASE("Label multi-line support", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Two lines are taller than a single line") {
        Label single(manager, 0, 0, "Line1", 16);
        Label multi(manager, 0, 0, "Line1\nLine2", 16);
        REQUIRE(multi.getHeight() > single.getHeight());
        REQUIRE(multi.getWidth() > 0);
    }

    SECTION("Height equals line count times font height") {
        auto font = manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        REQUIRE(font != nullptr);
        int lineHeight = TTF_GetFontHeight(font.get());

        Label twoLines(manager, 0, 0, "Line1\nLine2", 16);
        REQUIRE(twoLines.getHeight() == 2 * lineHeight);

        Label threeLines(manager, 0, 0, "Line1\nLine2\nLine3", 16);
        REQUIRE(threeLines.getHeight() == 3 * lineHeight);
    }

    SECTION("Width equals widest line") {
        Label wideRef(manager, 0, 0, "TheLongestLine", 16);
        Label multi(manager, 0, 0, "Short\nTheLongestLine\nTiny", 16);
        REQUIRE(multi.getWidth() == wideRef.getWidth());
    }

    SECTION("Empty lines count toward height") {
        auto font = manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        REQUIRE(font != nullptr);
        int lineHeight = TTF_GetFontHeight(font.get());

        Label label(manager, 0, 0, "AA\n\nAA", 16);
        REQUIRE(label.getHeight() == 3 * lineHeight);

        Label singleRef(manager, 0, 0, "AA", 16);
        REQUIRE(label.getWidth() == singleRef.getWidth());
    }

    SECTION("Trailing newline adds empty last line") {
        auto font = manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        REQUIRE(font != nullptr);
        int lineHeight = TTF_GetFontHeight(font.get());

        Label label(manager, 0, 0, "Text\n", 16);
        REQUIRE(label.getHeight() == 2 * lineHeight);
    }

    SECTION("Leading newline adds empty first line") {
        auto font = manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        REQUIRE(font != nullptr);
        int lineHeight = TTF_GetFontHeight(font.get());

        Label label(manager, 0, 0, "\nText", 16);
        REQUIRE(label.getHeight() == 2 * lineHeight);
    }

    SECTION("CRLF is treated as a single line break") {
        Label lf(manager, 0, 0, "Line1\nLine2", 16);
        Label crlf(manager, 0, 0, "Line1\r\nLine2", 16);
        REQUIRE(crlf.getHeight() == lf.getHeight());
        REQUIRE(crlf.getWidth() == lf.getWidth());
    }

    SECTION("setText switches between single-line and multi-line") {
        Label label(manager, 0, 0, "One", 16);
        int singleHeight = label.getHeight();
        int singleWidth = label.getWidth();

        label.setText("One\nTwo");
        REQUIRE(label.getHeight() > singleHeight);

        label.setText("One");
        REQUIRE(label.getHeight() == singleHeight);
        REQUIRE(label.getWidth() == singleWidth);
    }

    SECTION("Multi-line label renders without crash") {
        auto label = std::make_unique<Label>(manager, 10, 10, "First\nSecond\nThird", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));
        manager.render();
        REQUIRE(labelPtr->getHeight() > 0);
    }
}

TEST_CASE("Label component type", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns non-null") {
        Label label(manager, 0, 0, "Test", 16);
        const char* type = label.getComponentType();
        REQUIRE(type != nullptr);
    }
}

TEST_CASE("Label hierarchy", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Label can be added to manager") {
        auto label = std::make_unique<Label>(manager, 50, 50, "Managed Label", 16);
        Label* labelPtr = label.get();
        manager.addElement(std::move(label));

        REQUIRE(labelPtr->getX() == 50);
        REQUIRE(labelPtr->getY() == 50);
    }

    SECTION("Label parent is null by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getParent() == nullptr);
    }

    SECTION("Label has no children by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getChildren().empty());
    }

    SECTION("Label ID can be set and retrieved") {
        Label label(manager, 0, 0, "Test", 16);
        label.setID("testLabel123");
        REQUIRE(label.getID() == "testLabel123");
    }

    SECTION("Label ID is empty by default") {
        Label label(manager, 0, 0, "Test", 16);
        REQUIRE(label.getID().empty());
    }
}

TEST_CASE("Label size calculations", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Long text has non-zero dimensions") {
        Label label(manager, 0, 0, "This is a very long text that should have significant width", 16);
        REQUIRE(label.getWidth() > 0);
        REQUIRE(label.getHeight() > 0);
    }

    SECTION("getSize returns same values as getWidth/getHeight") {
        Label label(manager, 0, 0, "Test", 16);
        int width, height;
        label.getSize(width, height);
        REQUIRE(width == label.getWidth());
        REQUIRE(height == label.getHeight());
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
}