#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/json_parser.hpp"
#include "../src/sgml_parser.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/checkbox.hpp"
#include "../src/constants.hpp"
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
        REQUIRE(jroot->getComponentTypeId() == ComponentType::Panel);
        REQUIRE(sroot->getComponentTypeId() == ComponentType::Panel);
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

TEST_CASE("LayoutParser - bevel styles", "[layout_parser][bevel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("json: bevel shorthand fills Win95 palette") {
        JsonParser parser(manager);
        auto root = parser.loadLayout("tests/data/win95_bevel.json");
        REQUIRE(root != nullptr);

        const auto& desktopChildren = root->getChildren();
        REQUIRE(desktopChildren.size() == 3);

        auto* dialog = dynamic_cast<Panel*>(desktopChildren[2].get());
        REQUIRE(dialog != nullptr);
        REQUIRE(std::string(dialog->getID()) == "dialog");

        const Style dialogStyle = dialog->getComposedStyle(ElementState::Normal);
        REQUIRE(dialogStyle.borderColorOuterTopLeft == constants::kWin95Highlight);
        REQUIRE(dialogStyle.borderColorOuterBottomRight == constants::kWin95DarkShadow);
        REQUIRE(dialogStyle.borderColorInnerTopLeft == constants::kWin95Light);
        REQUIRE(dialogStyle.borderColorInnerBottomRight == constants::kWin95Shadow);

        const auto& dialogChildren = dialog->getChildren();
        REQUIRE(dialogChildren.size() == 11);

        auto* textInput = dynamic_cast<TextInput*>(dialogChildren[6].get());
        REQUIRE(textInput != nullptr);
        const Style inputPressed = textInput->getComposedStyle(ElementState::Pressed);
        REQUIRE(inputPressed.backgroundColor == SDL_Color{255, 255, 255, 255});
        REQUIRE(inputPressed.borderColorOuterTopLeft == constants::kWin95Shadow);
        REQUIRE(inputPressed.borderColorInnerTopLeft == constants::kWin95DarkShadow);

        auto* okButton = dynamic_cast<Button*>(dialogChildren[8].get());
        REQUIRE(okButton != nullptr);
        const Style okPressed = okButton->getComposedStyle(ElementState::Pressed);
        REQUIRE(okPressed.borderColorOuterTopLeft == constants::kWin95Shadow);
        REQUIRE(okPressed.borderColorOuterBottomRight == constants::kWin95Highlight);

        SECTION("button label re-centered after parser setSize") {
            REQUIRE(okButton->getChildren().size() == 1);
            auto* label = dynamic_cast<Label*>(okButton->getChildren()[0].get());
            REQUIRE(label != nullptr);
            int labelWidth = 0, labelHeight = 0;
            label->getSize(labelWidth, labelHeight);
            REQUIRE(label->getX() == (76 - labelWidth) / 2);
            REQUIRE(label->getY() == (28 - labelHeight) / 2);
        }

        SECTION("dialog centered by anchor applied during parse") {
            REQUIRE(dialog->getX() == (640 - 520) / 2);
            REQUIRE(dialog->getY() == (480 - 380) / 2);
        }
    }

    SECTION("xml: bevel shorthand with explicit color override") {
        SGMLParser parser(manager);
        auto root = parser.loadLayout("tests/data/win95_bevel.xml");
        REQUIRE(root != nullptr);

        const Style rootStyle = root->getComposedStyle(ElementState::Normal);
        REQUIRE(rootStyle.borderColorOuterTopLeft == constants::kWin95Highlight);
        REQUIRE(rootStyle.borderColorOuterBottomRight == constants::kWin95DarkShadow);

        const auto& children = root->getChildren();
        REQUIRE(children.size() == 1);
        const Style wellStyle = children[0]->getComposedStyle(ElementState::Normal);
        REQUIRE(wellStyle.borderColorOuterTopLeft == SDL_Color{17, 17, 17, 255});
        REQUIRE(wellStyle.borderColorOuterBottomRight == constants::kWin95Highlight);
        REQUIRE(wellStyle.borderColorInnerTopLeft == constants::kWin95DarkShadow);
        REQUIRE(wellStyle.borderColorInnerBottomRight == constants::kWin95Light);
    }
}
