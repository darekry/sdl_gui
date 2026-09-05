#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/progress_bar.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ProgressBar - defaults", "[progress_bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("constructor sets default range and value") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        REQUIRE(bar.getValue() == 0.0f);
        REQUIRE(bar.getMin() == 0.0f);
        REQUIRE(bar.getMax() == 100.0f);
    }

    SECTION("default orientation is horizontal and text shown") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        REQUIRE(bar.getOrientation() == Orientation::Horizontal);
        REQUIRE(bar.getShowText());
    }

    SECTION("getComponentTypeIdId returns ProgressBar") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        REQUIRE(bar.getComponentTypeId() == ComponentType::ProgressBar);
    }
}

TEST_CASE("ProgressBar - setValue", "[progress_bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setValue stores value") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setValue(42.0f);
        REQUIRE(bar.getValue() == 42.0f);
    }

    SECTION("setValue clamps to min") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setValue(-10.0f);
        REQUIRE(bar.getValue() == 0.0f);
    }

    SECTION("setValue clamps to max") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setValue(150.0f);
        REQUIRE(bar.getValue() == 100.0f);
    }

    SECTION("setValue with custom range") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setRange(10.0f, 90.0f);
        bar.setValue(5.0f);
        REQUIRE(bar.getValue() == 10.0f);
        bar.setValue(95.0f);
        REQUIRE(bar.getValue() == 90.0f);
    }

    SECTION("getValuePtr points at the live value") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        float* p = bar.getValuePtr();
        REQUIRE(p != nullptr);
        *p = 55.0f;
        REQUIRE(bar.getValue() == 55.0f);
    }
}

TEST_CASE("ProgressBar - range manipulation", "[progress_bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setMin updates min") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setMin(20.0f);
        REQUIRE(bar.getMin() == 20.0f);
    }

    SECTION("setMax updates max") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setMax(80.0f);
        REQUIRE(bar.getMax() == 80.0f);
    }

    SECTION("setRange updates both") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setRange(5.0f, 95.0f);
        REQUIRE(bar.getMin() == 5.0f);
        REQUIRE(bar.getMax() == 95.0f);
    }

    SECTION("setMin clamps current value") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setValue(50.0f);
        bar.setMin(60.0f);
        REQUIRE(bar.getValue() == 60.0f);
    }

    SECTION("setMax clamps current value") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setValue(50.0f);
        bar.setMax(30.0f);
        REQUIRE(bar.getValue() == 30.0f);
    }

    SECTION("setMin above max adjusts max") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setMin(150.0f);
        REQUIRE(bar.getMin() == 150.0f);
        REQUIRE(bar.getMax() == 151.0f);
    }

    SECTION("setMax below min adjusts min") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setMax(-10.0f);
        REQUIRE(bar.getMax() == -10.0f);
        REQUIRE(bar.getMin() == -11.0f);
    }

    SECTION("setMin within range keeps max") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setMin(80.0f);
        REQUIRE(bar.getMin() == 80.0f);
        REQUIRE(bar.getMax() == 100.0f);
    }

    SECTION("setRange with max below min fixes range") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setRange(90.0f, 10.0f);
        REQUIRE(bar.getMax() == 91.0f);
    }
}

TEST_CASE("ProgressBar - configuration", "[progress_bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("orientation can be switched") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setOrientation(Orientation::Vertical);
        REQUIRE(bar.getOrientation() == Orientation::Vertical);
        bar.setOrientation(Orientation::Horizontal);
        REQUIRE(bar.getOrientation() == Orientation::Horizontal);
    }

    SECTION("showText can be toggled") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setShowText(false);
        REQUIRE_FALSE(bar.getShowText());
        bar.setShowText(true);
        REQUIRE(bar.getShowText());
    }

    SECTION("textFormat can be changed") {
        ProgressBar bar(manager, 0, 0, 200, 30);
        bar.setTextFormat("%.1f%%");
        // no crash + value still works
        bar.setValue(33.0f);
        REQUIRE(bar.getValue() == 33.0f);
    }
}

TEST_CASE("ProgressBar - render", "[progress_bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("render with value does not crash") {
        auto bar = std::make_unique<ProgressBar>(manager, 10, 10, 200, 30);
        bar->setValue(75.0f);
        manager.addElement(std::move(bar));
        manager.render();
    }

    SECTION("render vertical with text does not crash") {
        auto bar = std::make_unique<ProgressBar>(manager, 10, 10, 30, 200);
        bar->setValue(25.0f);
        bar->setOrientation(Orientation::Vertical);
        manager.addElement(std::move(bar));
        manager.render();
    }

    SECTION("render with showText off does not crash") {
        auto bar = std::make_unique<ProgressBar>(manager, 10, 10, 200, 30);
        bar->setShowText(false);
        manager.addElement(std::move(bar));
        manager.render();
    }

    SECTION("render at 0% and 100% does not crash") {
        auto bar = std::make_unique<ProgressBar>(manager, 10, 10, 200, 30);
        ProgressBar* barPtr = bar.get();
        bar->setValue(0.0f);
        manager.addElement(std::move(bar));
        manager.render();

        barPtr->setValue(100.0f);
        manager.render();
    }
}
