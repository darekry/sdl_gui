#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/sgml_parser.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/progress_bar.hpp"

#include <string>

TEST_CASE("SGMLParser - loadLayout", "[sgml_parser][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    SGMLParser parser(manager);

    SECTION("loads a layout from XML") {
        auto root = parser.loadLayout("tests/data/layout.xml");
        REQUIRE(root != nullptr);
        REQUIRE(root->getComponentTypeId() == ComponentType::Panel);
    }

    SECTION("applies id, position and size") {
        auto root = parser.loadLayout("tests/data/layout.xml");
        REQUIRE(root != nullptr);
        REQUIRE(root->getID() == "main_panel");
        REQUIRE(root->getX() == 10);
        REQUIRE(root->getY() == 20);
        REQUIRE(root->getWidth() == 400);
        REQUIRE(root->getHeight() == 300);
    }

    SECTION("parses children and their attributes") {
        auto root = parser.loadLayout("tests/data/layout.xml");
        REQUIRE(root != nullptr);
        REQUIRE(root->getChildren().size() == 3);

        const auto& children = root->getChildren();

        auto* button = dynamic_cast<Button*>(children[0].get());
        REQUIRE(button != nullptr);
        REQUIRE(button->getID() == "ok_btn");
        REQUIRE_FALSE(button->isEnabled());
        REQUIRE(button->getX() == 50);
        REQUIRE(button->getWidth() == 100);

        auto* label = dynamic_cast<Label*>(children[1].get());
        REQUIRE(label != nullptr);
        REQUIRE(label->getText() == "Hello World");

        auto* bar = dynamic_cast<ProgressBar*>(children[2].get());
        REQUIRE(bar != nullptr);
        REQUIRE(bar->getValue() == 75.0f);
    }

    SECTION("missing file returns nullptr") {
        auto root = parser.loadLayout("tests/data/does_not_exist.xml");
        REQUIRE(root == nullptr);
    }

    SECTION("invalid XML returns nullptr") {
        auto root = parser.loadLayout("tests/data/bad.xml");
        REQUIRE(root == nullptr);
    }
}
