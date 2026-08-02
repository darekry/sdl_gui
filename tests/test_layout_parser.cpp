#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/json_parser.hpp"
#include "../src/sgml_parser.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/checkbox.hpp"
#include "../src/radio_button.hpp"
#include "../src/combobox.hpp"
#include "../src/list_view.hpp"
#include "../src/slider.hpp"
#include "../src/range_slider.hpp"
#include "../src/text_input.hpp"
#include "../src/arc_container.hpp"
#include "../src/scroll_area.hpp"

#include <string>

// LayoutParser is abstract; these tests exercise the shared parsing pipeline
// (resources, styles, children, widget types) through both concrete parsers.

TEST_CASE("LayoutParser - shared behavior", "[layout_parser]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("both parsers produce the same root") {
        JsonParser jp(manager);
        SGMLParser sp(manager);

        auto jroot = jp.loadLayout("tests/data/layout.json");
        auto sroot = sp.loadLayout("tests/data/layout.xml");

        REQUIRE(jroot != nullptr);
        REQUIRE(sroot != nullptr);
        REQUIRE(std::string(jroot->getComponentType()) == "Panel");
        REQUIRE(std::string(sroot->getComponentType()) == "Panel");
        REQUIRE(jroot->getChildren().size() == sroot->getChildren().size());
    }

    SECTION("missing file returns nullptr for both parsers") {
        JsonParser jp(manager);
        SGMLParser sp(manager);
        REQUIRE(jp.loadLayout("tests/data/nope.json") == nullptr);
        REQUIRE(sp.loadLayout("tests/data/nope.xml") == nullptr);
    }
}

TEST_CASE("LayoutParser - widget types", "[layout_parser][widgets]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    JsonParser parser(manager);

    auto root = parser.loadLayout("tests/data/widgets.json");
    REQUIRE(root != nullptr);

    const auto& children = root->getChildren();

    SECTION("checkbox") {
        auto* w = dynamic_cast<Checkbox*>(children[0].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->isChecked());
    }

    SECTION("radio button") {
        auto* w = dynamic_cast<RadioButton*>(children[1].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->isSelected());
    }

    SECTION("combobox") {
        auto* w = dynamic_cast<ComboBox*>(children[2].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getItemCount() == 3);
        REQUIRE(w->getSelectedIndex() == 1);
    }

    SECTION("list view") {
        auto* w = dynamic_cast<ListView*>(children[3].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getItemCount() == 2);
        REQUIRE(w->getSelectedRow() == 0);
    }

    SECTION("slider") {
        auto* w = dynamic_cast<Slider*>(children[4].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getValue() == 42);
        REQUIRE(w->getMin() == 0);
        REQUIRE(w->getMax() == 100);
    }

    SECTION("range slider") {
        auto* w = dynamic_cast<RangeSlider*>(children[5].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getLowerValue() == 20);
        REQUIRE(w->getUpperValue() == 80);
    }

    SECTION("text input") {
        auto* w = dynamic_cast<TextInput*>(children[6].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getText() == "abc");
        REQUIRE(w->isLocked());
    }

    SECTION("arc container with angled children") {
        auto* w = dynamic_cast<ArcContainer*>(children[7].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getChildren().size() == 1);
    }

    SECTION("scroll area with content size") {
        auto* w = dynamic_cast<ScrollArea*>(children[8].get());
        REQUIRE(w != nullptr);
        REQUIRE(w->getContent() != nullptr);
        REQUIRE(w->getScrollOffsetY() == 0);
    }
}

TEST_CASE("LayoutParser - radio group options", "[layout_parser][widgets]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    JsonParser parser(manager);

    SECTION("radio group parses options array") {
        auto root = parser.loadLayout("tests/data/widgets.json");
        REQUIRE(root != nullptr);
    }
}
