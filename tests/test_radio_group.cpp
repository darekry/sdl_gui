#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/radio_group.hpp"
#include "../src/radio_button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"

TEST_CASE("RadioGroup functionality", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("RadioGroup can be created") {
        RadioGroup group(manager, 10, 10, 200, 100);
        REQUIRE(group.getX() == 10);
        REQUIRE(group.getY() == 10);
        REQUIRE(group.getWidth() == 200);
        REQUIRE(group.getHeight() == 100);
    }

    SECTION("getSelectedButton returns nullptr when no button is selected") {
        auto group = std::make_unique<RadioGroup>(manager, 10, 10, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        
        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));

        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("Selecting a radio button deselects others in the group") {
        auto group = std::make_unique<RadioGroup>(manager, 10, 10, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 5, 55, 20, 20);
        
        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        RadioButton* rb3Ptr = rb3.get();
        
        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));
        groupPtr->addChild(std::move(rb3));

        rb1Ptr->setSelected(true);
        REQUIRE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb1Ptr);

        rb2Ptr->setSelected(true);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb2Ptr);

        rb3Ptr->setSelected(true);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb3Ptr);
    }

    SECTION("RadioButtons within group deselect others when selected") {
        auto group = std::make_unique<RadioGroup>(manager, 10, 10, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        
        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        
        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));

        rb1Ptr->setSelected(true);
        REQUIRE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb1Ptr);

        rb2Ptr->setSelected(true);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb2Ptr);
    }

    SECTION("Multiple RadioGroups are independent") {
        auto group1 = std::make_unique<RadioGroup>(manager, 10, 10, 100, 100);
        auto group2 = std::make_unique<RadioGroup>(manager, 120, 10, 100, 100);
        
        RadioGroup* group1Ptr = group1.get();
        RadioGroup* group2Ptr = group2.get();
        
        manager.addElement(std::move(group1));
        manager.addElement(std::move(group2));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        
        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        
        group1Ptr->addChild(std::move(rb1));
        group2Ptr->addChild(std::move(rb2));

        rb1Ptr->setSelected(true);
        rb2Ptr->setSelected(true);
        
        REQUIRE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
    }

    SECTION("Non-radio-button children are ignored by group logic") {
        auto group = std::make_unique<RadioGroup>(manager, 10, 10, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto panel = std::make_unique<Panel>(manager, 30, 5, 50, 20);
        
        RadioButton* rbPtr = rb.get();
        
        groupPtr->addChild(std::move(rb));
        groupPtr->addChild(std::move(panel));

        rbPtr->setSelected(true);
        REQUIRE(rbPtr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rbPtr);
    }
}
