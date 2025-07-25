#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/radio_button.hpp"
#include "../src/radio_group.hpp"
#include "../src/gui_manager.hpp"
#include "../src/label.hpp"
#include <memory>

TEST_CASE("RadioButton and RadioGroup Functionality", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager guiManager(helper.getRenderer());

    auto group = std::make_unique<RadioGroup>(guiManager, 50, 50, 200, 200);
    auto* group_ptr = group.get();

    auto radio1 = std::make_unique<RadioButton>(guiManager, 10, 10, "Radio 1");
    auto* radio1_ptr = radio1.get();
    group->addChild(std::move(radio1));

    auto radio2 = std::make_unique<RadioButton>(guiManager, 10, 40, "Radio 2");
    auto* radio2_ptr = radio2.get();
    group->addChild(std::move(radio2));
    
    guiManager.addElement(std::move(group));


    SECTION("RadioButton Initialization") {
        REQUIRE(radio1_ptr->getX() == 10);
        REQUIRE(radio1_ptr->getY() == 10);
        REQUIRE(radio1_ptr->isSelected() == false);
        REQUIRE(radio1_ptr->getLabel() != nullptr);
        REQUIRE(radio1_ptr->getParent() == group_ptr);
    }

    SECTION("RadioGroup - Single Selection via handleEvent") {
        REQUIRE(radio1_ptr->isSelected() == false);
        REQUIRE(radio2_ptr->isSelected() == false);

        // Symulacja kliknięcia na radio1
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 65, 65); // 50(group)+10(radio)+5(padding)
        guiManager.processEvent(event);

        REQUIRE(radio1_ptr->isSelected() == true);
        REQUIRE(radio2_ptr->isSelected() == false);
        REQUIRE(group_ptr->getSelectedButton() == radio1_ptr);
        
        // Symulacja kliknięcia na radio2
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 65, 95); // 50(group)+40(radio)+5(padding)
        guiManager.processEvent(event);
        
        REQUIRE(radio1_ptr->isSelected() == false);
        REQUIRE(radio2_ptr->isSelected() == true);
        REQUIRE(group_ptr->getSelectedButton() == radio2_ptr);
    }

    SECTION("RadioButton - OnChange Callback") {
        bool triggered = false;
        bool state = false;
        
        radio1_ptr->setOnChange([&](RadioButton* rb, bool selected){
            triggered = true;
            state = selected;
        });

        // Zaznaczanie programowe
        radio1_ptr->setSelected(true);
        REQUIRE(triggered == true);
        REQUIRE(state == true);

        // Reset
        triggered = false;
        state = false;

        // Odznaczanie przez grupę
        radio2_ptr->setSelected(true);
        REQUIRE(triggered == true); // Callback dla radio1_ptr powinien się wywołać
        REQUIRE(state == false);    // z nowym stanem
    }

    SECTION("RadioGroup - Direct Selection") {
        radio1_ptr->setSelected(true);
        REQUIRE(radio1_ptr->isSelected() == true);
        REQUIRE(radio2_ptr->isSelected() == false);

        radio2_ptr->setSelected(true);
        REQUIRE(radio1_ptr->isSelected() == false);
        REQUIRE(radio2_ptr->isSelected() == true);
        
        radio2_ptr->setSelected(false);
        REQUIRE(radio1_ptr->isSelected() == false);
        REQUIRE(radio2_ptr->isSelected() == false);
    }
}