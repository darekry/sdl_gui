#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/json_parser.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/progress_bar.hpp"

#include <string>

TEST_CASE("JsonParser - loadLayout", "[json_parser][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    JsonParser parser(manager);

    SECTION("loads a layout from JSON") {
        auto root = parser.loadLayout("tests/data/layout.json");
        REQUIRE(root != nullptr);
        REQUIRE(std::string(root->getComponentType()) == "Panel");
    }

    SECTION("applies id, position and size") {
        auto root = parser.loadLayout("tests/data/layout.json");
        REQUIRE(root != nullptr);
        REQUIRE(root->getID() == "main_panel");
        REQUIRE(root->getX() == 10);
        REQUIRE(root->getY() == 20);
        REQUIRE(root->getWidth() == 400);
        REQUIRE(root->getHeight() == 300);
    }

    SECTION("parses children") {
        auto root = parser.loadLayout("tests/data/layout.json");
        REQUIRE(root != nullptr);
        REQUIRE(root->getChildren().size() == 3);
    }

    SECTION("parses child properties") {
        auto root = parser.loadLayout("tests/data/layout.json");
        REQUIRE(root != nullptr);

        const auto& children = root->getChildren();

        auto* button = dynamic_cast<Button*>(children[0].get());
        REQUIRE(button != nullptr);
        REQUIRE(button->getID() == "ok_btn");
        REQUIRE(button->getX() == 50);
        REQUIRE(button->getY() == 250);
        REQUIRE(button->getWidth() == 100);
        REQUIRE(button->getHeight() == 40);
        REQUIRE_FALSE(button->isEnabled());

        auto* label = dynamic_cast<Label*>(children[1].get());
        REQUIRE(label != nullptr);
        REQUIRE(label->getID() == "title_label");
        REQUIRE(label->getText() == "Hello World");
        REQUIRE(label->getX() == 10);

        auto* bar = dynamic_cast<ProgressBar*>(children[2].get());
        REQUIRE(bar != nullptr);
        REQUIRE(bar->getValue() == 75.0f);
        REQUIRE(bar->getMin() == 0.0f);
        REQUIRE(bar->getMax() == 100.0f);
    }

    SECTION("missing file returns nullptr") {
        auto root = parser.loadLayout("tests/data/does_not_exist.json");
        REQUIRE(root == nullptr);
    }

    SECTION("invalid JSON returns nullptr") {
        auto root = parser.loadLayout("tests/data/bad.json");
        REQUIRE(root == nullptr);

        // unparseable content -> loadFile fails
        auto root2 = parser.loadLayout("tests/data/bad2.json");
        REQUIRE(root2 == nullptr);
    }

    SECTION("unknown element type returns nullptr") {
        auto root = parser.loadLayout("tests/data/unknown_type.json");
        REQUIRE(root == nullptr);
    }
}

TEST_CASE("JsonParser - types through layout", "[json_parser][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    JsonParser parser(manager);

    // Verifies int vs double handling, arrays (children) and string parsing
    // end-to-end through the same code path the parser uses for widgets.
    SECTION("float values are read via getFloat") {
        auto root = parser.loadLayout("tests/data/layout.json");
        REQUIRE(root != nullptr);
        // ProgressBar value 75 (int in JSON) must read as 75.0f
        auto* bar = dynamic_cast<ProgressBar*>(root->getChildren()[2].get());
        REQUIRE(bar != nullptr);
        REQUIRE(bar->getValue() == 75.0f);
    }
}
