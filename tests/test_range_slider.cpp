#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/range_slider.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("RangeSlider - Value Initialization", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor sets lower and upper values correctly") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        REQUIRE(slider.getLowerValue() == 20);
        REQUIRE(slider.getUpperValue() == 80);
    }

    SECTION("Constructor clamps lower value to min") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, -10, 80, Orientation::Horizontal);
        REQUIRE(slider.getLowerValue() == 0);
    }

    SECTION("Constructor clamps upper value to max") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 150, Orientation::Horizontal);
        REQUIRE(slider.getUpperValue() == 100);
    }

    SECTION("Constructor swaps values if lower > upper") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 80, 20, Orientation::Horizontal);
        REQUIRE(slider.getLowerValue() == 20);
        REQUIRE(slider.getUpperValue() == 80);
    }
}

TEST_CASE("RangeSlider - setLowerValue / setUpperValue", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setLowerValue updates lower value") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setLowerValue(30);
        REQUIRE(ptr->getLowerValue() == 30);
    }

    SECTION("setUpperValue updates upper value") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setUpperValue(90);
        REQUIRE(ptr->getUpperValue() == 90);
    }

    SECTION("setLowerValue clamps to min") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setLowerValue(-10);
        REQUIRE(ptr->getLowerValue() == 0);
    }

    SECTION("setUpperValue clamps to max") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setUpperValue(150);
        REQUIRE(ptr->getUpperValue() == 100);
    }

    SECTION("setLowerValue cannot exceed upper value") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setLowerValue(90);
        REQUIRE(ptr->getLowerValue() <= ptr->getUpperValue());
        REQUIRE(ptr->getLowerValue() == 80);
    }

    SECTION("setUpperValue cannot go below lower value") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        ptr->setUpperValue(10);
        REQUIRE(ptr->getUpperValue() >= ptr->getLowerValue());
        REQUIRE(ptr->getUpperValue() == 20);
    }

    SECTION("setLowerValue fires onChange callback") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        bool fired = false;
        ptr->setOnChangeCallback([&](GUIElement*) { fired = true; });
        ptr->setLowerValue(30);
        REQUIRE(fired);
    }

    SECTION("setUpperValue fires onChange callback") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        bool fired = false;
        ptr->setOnChangeCallback([&](GUIElement*) { fired = true; });
        ptr->setUpperValue(90);
        REQUIRE(fired);
    }

    SECTION("setLowerValue with same value does not fire callback") {
        auto slider = std::make_unique<RangeSlider>(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int count = 0;
        ptr->setOnChangeCallback([&](GUIElement*) { ++count; });
        ptr->setLowerValue(20);
        REQUIRE(count == 0);
    }
}

TEST_CASE("RangeSlider - Dragging", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Dragging lower thumb changes lower value") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int lowerThumbX = 50 + 40 + 2;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, lowerThumbX, trackY));
        REQUIRE(ptr->getLowerValue() != 80);
    }

    SECTION("Dragging upper thumb changes upper value") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int upperThumbX = 50 + 160 + 2;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, upperThumbX, trackY));
        REQUIRE(ptr->getUpperValue() != 20);
    }

    SECTION("Clicking track closer to lower thumb activates it") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int clickX = 50 + 30;
        int trackY = 50 + 20;

        int oldLower = ptr->getLowerValue();
        int oldUpper = ptr->getUpperValue();
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, clickX, trackY));
        REQUIRE(ptr->getLowerValue() != oldLower);
        REQUIRE(ptr->getUpperValue() == oldUpper);
    }

    SECTION("Dragging continues on mouse motion") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int lowerThumbX = 50 + 40 + 2;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, lowerThumbX, trackY));
        manager.processEvent(helper.createMouseMotion(lowerThumbX + 30, trackY));
        REQUIRE(ptr->getLowerValue() > 20);
    }

    SECTION("Mouse up stops dragging") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        int thumbX = 50 + 40 + 2;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, thumbX, trackY));
        int afterDown = ptr->getLowerValue();
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, thumbX, trackY));
        manager.processEvent(helper.createMouseMotion(thumbX + 30, trackY));
        REQUIRE(ptr->getLowerValue() == afterDown);
    }
}

TEST_CASE("RangeSlider - Mouse Wheel", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Mouse wheel up when hovered increases lower value by default") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(ptr->getLowerValue() == 21);
    }

    SECTION("Mouse wheel down when hovered decreases lower value by default") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(-1));
        REQUIRE(ptr->getLowerValue() == 19);
    }

    SECTION("Mouse wheel ignored when not hovered") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(ptr->getLowerValue() == 20);
    }

    SECTION("setWheelStep changes increment amount") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        ptr->setWheelStep(5);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(ptr->getLowerValue() == 25);
    }
}

TEST_CASE("RangeSlider - Range", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getMin / getMax return correct values") {
        RangeSlider slider(manager, 0, 0, 200, 40, 10, 90, 20, 80, Orientation::Horizontal);
        REQUIRE(slider.getMin() == 10);
        REQUIRE(slider.getMax() == 90);
    }

    SECTION("setMin clamps lower value if below new min") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        slider.setMin(30);
        REQUIRE(slider.getLowerValue() == 30);
    }

    SECTION("setMax clamps upper value if above new max") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        slider.setMax(60);
        REQUIRE(slider.getUpperValue() == 60);
    }

    SECTION("setRange updates both min and max") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        slider.setRange(30, 70);
        REQUIRE(slider.getMin() == 30);
        REQUIRE(slider.getMax() == 70);
    }
}

TEST_CASE("RangeSlider - Orientation", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Horizontal slider works correctly") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        REQUIRE(slider.getWidth() == 200);
        REQUIRE(slider.getHeight() == 40);
    }

    SECTION("Vertical slider works correctly") {
        RangeSlider slider(manager, 0, 0, 40, 200, 0, 100, 20, 80, Orientation::Vertical);
        REQUIRE(slider.getWidth() == 40);
        REQUIRE(slider.getHeight() == 200);
    }
}

TEST_CASE("RangeSlider - Disabled State", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled slider ignores mouse wheel") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        ptr->setEnabled(false);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(ptr->getLowerValue() == 20);
    }

    SECTION("Disabled slider ignores track click") {
        auto slider = std::make_unique<RangeSlider>(manager, 50, 50, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        RangeSlider* ptr = slider.get();
        ptr->setEnabled(false);
        manager.addElement(std::move(slider));

        int thumbX = 50 + 40 + 2;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, thumbX, trackY));
        REQUIRE(ptr->getLowerValue() == 20);
    }
}

TEST_CASE("RangeSlider - Component Type", "[rangeslider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns RangeSlider") {
        RangeSlider slider(manager, 0, 0, 200, 40, 0, 100, 20, 80, Orientation::Horizontal);
        REQUIRE(std::string(slider.getComponentType()) == "RangeSlider");
    }
}
