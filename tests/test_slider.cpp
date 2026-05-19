#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/slider.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Slider - Value Initialization", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor sets initial value correctly") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getValue() == 50);
    }

    SECTION("Constructor clamps initial value to min") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, -10, Orientation::Horizontal);
        REQUIRE(slider.getValue() == 0);
    }

    SECTION("Constructor clamps initial value to max") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 150, Orientation::Horizontal);
        REQUIRE(slider.getValue() == 100);
    }

    SECTION("getValue returns current value") {
        Slider slider(manager, 0, 0, 200, 40, 10, 90, 45, Orientation::Horizontal);
        REQUIRE(slider.getValue() == 45);
    }
}

TEST_CASE("Slider - setValue Behavior", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setValue updates value") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        sliderPtr->setValue(75);
        REQUIRE(sliderPtr->getValue() == 75);
    }

    SECTION("setValue clamps to min range") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        sliderPtr->setValue(-50);
        REQUIRE(sliderPtr->getValue() == 0);
    }

    SECTION("setValue clamps to max range") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        sliderPtr->setValue(200);
        REQUIRE(sliderPtr->getValue() == 100);
    }

    SECTION("setValue fires onChange callback when value changes") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        bool callbackFired = false;
        sliderPtr->setOnChangeCallback([&](GUIElement*) { callbackFired = true; });

        sliderPtr->setValue(75);
        REQUIRE(callbackFired);
    }

    SECTION("setValue with same value doesn't fire callback") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int callbackCount = 0;
        sliderPtr->setOnChangeCallback([&](GUIElement*) { ++callbackCount; });

        sliderPtr->setValue(50);
        REQUIRE(callbackCount == 0);
    }

    SECTION("setValue callback provides slider pointer") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        GUIElement* callbackSource = nullptr;
        sliderPtr->setOnChangeCallback([&](GUIElement* src) { callbackSource = src; });

        sliderPtr->setValue(60);
        REQUIRE(callbackSource == sliderPtr);
    }
}

TEST_CASE("Slider - Increment/Decrement Buttons", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Clicking increment button increases value by 1") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        Button* incBtn = sliderPtr->getIncrementButton();
        REQUIRE(incBtn != nullptr);

        int btnX = sliderPtr->getX() + incBtn->getX() + 5;
        int btnY = sliderPtr->getY() + incBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 51);
    }

    SECTION("Clicking decrement button decreases value by 1") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        Button* decBtn = sliderPtr->getDecrementButton();
        REQUIRE(decBtn != nullptr);

        int btnX = sliderPtr->getX() + decBtn->getX() + 5;
        int btnY = sliderPtr->getY() + decBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 49);
    }

    SECTION("Increment button respects max value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 100, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        Button* incBtn = sliderPtr->getIncrementButton();
        int btnX = sliderPtr->getX() + incBtn->getX() + 5;
        int btnY = sliderPtr->getY() + incBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 100);
    }

    SECTION("Decrement button respects min value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 0, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        Button* decBtn = sliderPtr->getDecrementButton();
        int btnX = sliderPtr->getX() + decBtn->getX() + 5;
        int btnY = sliderPtr->getY() + decBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 0);
    }

    SECTION("getIncrementButton returns valid pointer") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getIncrementButton() != nullptr);
    }

    SECTION("getDecrementButton returns valid pointer") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getDecrementButton() != nullptr);
    }
}

TEST_CASE("Slider - Mouse Wheel", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Mouse wheel up when hovered increases value by wheelStep") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 51);
    }

    SECTION("Mouse wheel down when hovered decreases value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(-1));
        REQUIRE(sliderPtr->getValue() == 49);
    }

    SECTION("Mouse wheel ignored when not hovered") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 50);
    }

    SECTION("setWheelStep changes increment amount") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setWheelStep(5);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 55);
    }

    SECTION("getWheelStep returns current wheel step") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getWheelStep() == 1);

        slider.setWheelStep(10);
        REQUIRE(slider.getWheelStep() == 10);
    }

    SECTION("setWheelStep ignores zero or negative values") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setWheelStep(0);
        REQUIRE(slider.getWheelStep() == 1);

        slider.setWheelStep(-5);
        REQUIRE(slider.getWheelStep() == 1);
    }

    SECTION("Wheel step respects max value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 98, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setWheelStep(5);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 100);
    }

    SECTION("Wheel step respects min value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 2, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setWheelStep(5);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(-1));
        REQUIRE(sliderPtr->getValue() == 0);
    }
}

TEST_CASE("Slider - Dragging Track", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click on track changes value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int trackX = 50 + 40 + 10;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackX, trackY));
        REQUIRE(sliderPtr->getValue() != 50);
    }

    SECTION("Dragging to left edge sets min value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int trackStartX = 50 + 40;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackStartX, trackY));
        manager.processEvent(helper.createMouseMotion(trackStartX, trackY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, trackStartX, trackY));

        REQUIRE(sliderPtr->getValue() == 0);
    }

    SECTION("Dragging to far right gives max value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int trackEndX = 50 + 40 + 119;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackEndX, trackY));
        REQUIRE(sliderPtr->getValue() >= 99);
    }

    SECTION("Dragging updates value continuously") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int trackStartX = 50 + 40;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackStartX, trackY));
        REQUIRE(sliderPtr->getValue() == 0);

        manager.processEvent(helper.createMouseMotion(trackStartX + 60, trackY));
        REQUIRE(sliderPtr->getValue() > 0);
    }
}

TEST_CASE("Slider - Orientation", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Horizontal slider works correctly") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getWidth() == 200);
        REQUIRE(slider.getHeight() == 40);
        REQUIRE(slider.getValue() == 50);
    }

    SECTION("Vertical slider works correctly") {
        Slider slider(manager, 0, 0, 40, 200, 0, 100, 50, Orientation::Vertical);
        REQUIRE(slider.getWidth() == 40);
        REQUIRE(slider.getHeight() == 200);
        REQUIRE(slider.getValue() == 50);
    }

    SECTION("Vertical slider mouse wheel increases value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 40, 200, 0, 100, 50, Orientation::Vertical);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(70, 150));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 51);
    }

    SECTION("Vertical slider dragging changes value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 40, 200, 0, 100, 50, Orientation::Vertical);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        int trackStartY = 50 + 40;
        int trackX = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackX, trackStartY));
        REQUIRE(sliderPtr->getValue() == 0);
    }
}

TEST_CASE("Slider - Disabled State", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled slider ignores increment button") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setEnabled(false);
        manager.addElement(std::move(slider));

        Button* incBtn = sliderPtr->getIncrementButton();
        int btnX = sliderPtr->getX() + incBtn->getX() + 5;
        int btnY = sliderPtr->getY() + incBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 50);
    }

    SECTION("Disabled slider ignores decrement button") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setEnabled(false);
        manager.addElement(std::move(slider));

        Button* decBtn = sliderPtr->getDecrementButton();
        int btnX = sliderPtr->getX() + decBtn->getX() + 5;
        int btnY = sliderPtr->getY() + decBtn->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, btnX, btnY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, btnX, btnY));

        REQUIRE(sliderPtr->getValue() == 50);
    }

    SECTION("Disabled slider ignores mouse wheel") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setEnabled(false);
        manager.addElement(std::move(slider));

        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 50);
    }

    SECTION("Disabled slider ignores track drag") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setEnabled(false);
        manager.addElement(std::move(slider));

        int trackX = 50 + 40;
        int trackY = 50 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, trackX, trackY));
        REQUIRE(sliderPtr->getValue() == 50);
    }
}

TEST_CASE("Slider - Range", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getMin returns correct value") {
        Slider slider(manager, 0, 0, 200, 40, 10, 90, 50, Orientation::Horizontal);
        REQUIRE(slider.getMin() == 10);
    }

    SECTION("getMax returns correct value") {
        Slider slider(manager, 0, 0, 200, 40, 10, 90, 50, Orientation::Horizontal);
        REQUIRE(slider.getMax() == 90);
    }

    SECTION("setMin updates min value") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setMin(20);
        REQUIRE(slider.getMin() == 20);
    }

    SECTION("setMax updates max value") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setMax(80);
        REQUIRE(slider.getMax() == 80);
    }

    SECTION("setRange updates both min and max") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setRange(20, 80);
        REQUIRE(slider.getMin() == 20);
        REQUIRE(slider.getMax() == 80);
    }

    SECTION("setMin clamps current value if below new min") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setMin(60);
        REQUIRE(slider.getValue() == 60);
    }

    SECTION("setMax clamps current value if above new max") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setMax(30);
        REQUIRE(slider.getValue() == 30);
    }

    SECTION("setRange clamps current value to new range") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        slider.setRange(70, 90);
        REQUIRE(slider.getValue() == 70);
    }
}

TEST_CASE("Slider - Component Type", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns Slider") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(std::string(slider.getComponentType()) == "Slider");
    }
}