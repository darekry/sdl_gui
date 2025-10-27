#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_area.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextArea basic functionality", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initialization") {
        TextArea ta(manager, 10, 20, 300, 200, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getX() == 10);
        REQUIRE(ta.getY() == 20);
        REQUIRE(ta.getWidth() == 300);
        REQUIRE(ta.getHeight() == 200);
        REQUIRE(ta.getText().empty());
    }

    SECTION("setText and getText") {
        auto ta = std::make_unique<TextArea>(manager, 10, 10, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Hello\nWorld");
        REQUIRE(area->getText() == "Hello\nWorld");

        area->setText("New text");
        REQUIRE(area->getText() == "New text");
    }

    SECTION("Multi-line text handling") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText("Line 1\nLine 2\nLine 3");
        REQUIRE(ta.getText() == "Line 1\nLine 2\nLine 3");
    }

    SECTION("Word wrap toggle") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getWordWrap() == true);
        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());
    }
}
