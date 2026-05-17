#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/slider.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Slider functionality", "[slider]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Slider initializes with the given value") {
        Slider slider(manager, 0, 0, 200, 40, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getValue() == 50);
    }

    SECTION("setValue clamps value to valid range and triggers callback") {
        auto slider = std::make_unique<Slider>(manager, 10, 10, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        bool changed = false;
        sliderPtr->setOnChangeCallback([&](GUIElement*) { changed = true; });

        sliderPtr->setValue(75);
        REQUIRE(sliderPtr->getValue() == 75);
        REQUIRE(changed);

        changed = false;
        sliderPtr->setValue(150);
        REQUIRE(sliderPtr->getValue() == 100);
        REQUIRE(changed);

        changed = false;
        sliderPtr->setValue(-10);
        REQUIRE(sliderPtr->getValue() == 0);
        REQUIRE(changed);

        changed = false;
        sliderPtr->setValue(0);
        REQUIRE(sliderPtr->getValue() == 0);
        REQUIRE_FALSE(changed);
    }

    SECTION("Clicking increment button increases value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        auto* incBtn = sliderPtr->getIncrementButton();
        REQUIRE(incBtn != nullptr);

        const int x = incBtn->getX() + sliderPtr->getX() + 5;
        const int y = incBtn->getY() + sliderPtr->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, x, y));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, x, y));

        REQUIRE(sliderPtr->getValue() == 51);
    }

    SECTION("Clicking decrement button decreases value") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        auto* decBtn = sliderPtr->getDecrementButton();
        REQUIRE(decBtn != nullptr);

        const int x = decBtn->getX() + sliderPtr->getX() + 5;
        const int y = decBtn->getY() + sliderPtr->getY() + 5;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, x, y));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, x, y));

        REQUIRE(sliderPtr->getValue() == 49);
    }

    SECTION("Mouse wheel increases value when slider is hovered") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        // Symuluj hover nad sliderem
        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render(); // Render aby zaktualizować stan hover

        // Kółko w górę powinno zwiększyć wartość
        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 51);
    }

    SECTION("Mouse wheel decreases value when slider is hovered") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        // Symuluj hover nad sliderem
        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        // Kółko w dół powinno zmniejszyć wartość
        manager.processEvent(helper.createMouseWheel(-1));
        REQUIRE(sliderPtr->getValue() == 49);
    }

    SECTION("Mouse wheel with custom step") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        sliderPtr->setWheelStep(5);
        manager.addElement(std::move(slider));

        // Symuluj hover
        manager.processEvent(helper.createMouseMotion(150, 70));
        manager.render();

        // Kółko w górę z krokiem 5
        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 55);
    }

    SECTION("Mouse wheel ignored when slider is not hovered") {
        auto slider = std::make_unique<Slider>(manager, 50, 50, 200, 40, 0, 100, 50, Orientation::Horizontal);
        Slider* sliderPtr = slider.get();
        manager.addElement(std::move(slider));

        // Bez hover - kółko nie powinno zmienić wartości
        manager.processEvent(helper.createMouseWheel(1));
        REQUIRE(sliderPtr->getValue() == 50);
    }
}
