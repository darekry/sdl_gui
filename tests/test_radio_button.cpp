#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/gui.hpp"
#include "../src/radio_button.hpp"
#include "../src/radio_group.hpp"

TEST_CASE("RadioButton and RadioGroup - selection behavior", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto group = std::make_unique<RadioGroup>(manager, 50, 50, 200, 200);
    RadioGroup* pg = group.get();

    auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
    RadioButton* p1 = rb1.get();
    group->addChild(std::move(rb1));

    auto rb2 = std::make_unique<RadioButton>(manager, 10, 40, 20, 20);
    RadioButton* p2 = rb2.get();
    group->addChild(std::move(rb2));

    manager.addElement(std::move(group));

    SECTION("Single selection by click") {
        REQUIRE(p1->isSelected() == false);
        REQUIRE(p2->isSelected() == false);

        // Click inside first radio: group(50,50) + rb1(10,10)
        SDL_Event e = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 65, 65);
        manager.processEvent(e);
        e = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 65, 65);
        manager.processEvent(e);

        REQUIRE(p1->isSelected() == true);
        REQUIRE(p2->isSelected() == false);
        REQUIRE(pg->getSelectedButton() == p1);

        // Click inside second radio: group(50,50) + rb2(10,40)
        e = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 65, 95);
        manager.processEvent(e);
        e = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 65, 95);
        manager.processEvent(e);

        REQUIRE(p1->isSelected() == false);
        REQUIRE(p2->isSelected() == true);
        REQUIRE(pg->getSelectedButton() == p2);
    }

    SECTION("Click outside does not change selection") {
        REQUIRE(p1->isSelected() == false);
        REQUIRE(p2->isSelected() == false);
        REQUIRE(pg->getSelectedButton() == nullptr);

        SDL_Event e = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(e);
        e = helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 1000, 1000);
        manager.processEvent(e);

        REQUIRE(p1->isSelected() == false);
        REQUIRE(p2->isSelected() == false);
        REQUIRE(pg->getSelectedButton() == nullptr);
    }

    SECTION("Programmatic selection updates exclusivity") {
        p1->setSelected(true);
        REQUIRE(p1->isSelected() == true);
        REQUIRE(p2->isSelected() == false);
        REQUIRE(pg->getSelectedButton() == p1);

        p2->setSelected(true);
        REQUIRE(p1->isSelected() == false);
        REQUIRE(p2->isSelected() == true);
        REQUIRE(pg->getSelectedButton() == p2);
    }
}