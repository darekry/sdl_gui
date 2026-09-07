#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/ui_helpers.hpp"
#include "../src/slider.hpp"
#include "../src/range_slider.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ui_helpers linkLabel", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("prefix/suffix variant refreshes immediately and on change") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 30, 0, 100, 42,
                                               Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "stale", 14);
        Slider* s = slider.get();
        Label* l = label.get();
        manager.addElement(std::move(slider));
        manager.addElement(std::move(label));

        linkLabel(*s, *l, "v=", "px");
        REQUIRE(l->getText() == "v=42px"); // immediate refresh, no event needed
        s->setValue(7);
        REQUIRE(l->getText() == "v=7px");
    }

    SECTION("formatter variant") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 30, 0, 120, 90,
                                               Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "", 14);
        Slider* s = slider.get();
        Label* l = label.get();
        manager.addElement(std::move(slider));
        manager.addElement(std::move(label));

        linkLabel(*s, *l, [](int v) { return std::to_string(v / 60) + "m"; });
        REQUIRE(l->getText() == "1m");
        s->setValue(120);
        REQUIRE(l->getText() == "2m");
    }

    SECTION("linkRangeLabel default and formatter variants") {
        auto range = std::make_unique<RangeSlider>(manager, 0, 0, 200, 30, 0, 100, 20, 80,
                                                   Orientation::Horizontal);
        auto range2 = std::make_unique<RangeSlider>(manager, 0, 100, 200, 30, 0, 100, 20, 80,
                                                    Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "", 14);
        auto label2 = std::make_unique<Label>(manager, 0, 140, "", 14);
        RangeSlider* r = range.get();
        RangeSlider* r2 = range2.get();
        Label* l = label.get();
        Label* l2 = label2.get();
        manager.addElement(std::move(range));
        manager.addElement(std::move(range2));
        manager.addElement(std::move(label));
        manager.addElement(std::move(label2));

        linkRangeLabel(*r, *l);
        REQUIRE(l->getText() == "[20, 80]");
        linkRangeLabel(*r2, *l2, [](int lo, int hi) {
            return std::to_string(lo) + ".." + std::to_string(hi);
        });
        r->setLowerValue(10);
        r2->setLowerValue(10);
        REQUIRE(l->getText() == "[10, 80]");
        REQUIRE(l2->getText() == "10..80");
    }
}
