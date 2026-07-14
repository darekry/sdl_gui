#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/radio_group.hpp"
#include "../src/radio_button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/label.hpp"

TEST_CASE("RadioGroup - Container Behavior", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("RadioGroup can be created with position and size") {
        RadioGroup group(manager, 10, 20, 200, 100);
        REQUIRE(group.getX() == 10);
        REQUIRE(group.getY() == 20);
        REQUIRE(group.getWidth() == 200);
        REQUIRE(group.getHeight() == 100);
    }

    SECTION("RadioGroup inherits from Panel") {
        RadioGroup group(manager, 0, 0, 100, 50);
        Panel* panelPtr = dynamic_cast<Panel*>(&group);
        REQUIRE(panelPtr != nullptr);
    }

    SECTION("RadioGroup can contain RadioButton children") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        RadioButton* rbPtr = rb.get();
        groupPtr->addChild(std::move(rb));

        REQUIRE(groupPtr->getChildren().size() == 1);
        REQUIRE(dynamic_cast<RadioButton*>(groupPtr->getChildren()[0].get()) != nullptr);
    }

    SECTION("RadioGroup can contain multiple RadioButton children") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 5, 20, 20));
        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 30, 20, 20));
        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 55, 20, 20));

        REQUIRE(groupPtr->getChildren().size() == 3);
    }
}

TEST_CASE("RadioGroup - Selection Management", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getSelectedButton returns nullptr when nothing selected") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 5, 20, 20));
        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 30, 20, 20));

        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("getSelectedButton returns correct pointer when one is selected") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();

        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));

        rb1Ptr->setSelected(true);

        RadioButton* selected = groupPtr->getSelectedButton();
        REQUIRE(selected == rb1Ptr);
        REQUIRE(selected != rb2Ptr);
    }

    SECTION("Selecting one button deselects all others in the group") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
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

        rb2Ptr->setSelected(true);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
    }
}

TEST_CASE("RadioGroup - Mutual Exclusivity", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Select first - first is selected, others not") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
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
    }

    SECTION("Select second - second is selected, first deselected") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
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
        rb2Ptr->setSelected(true);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb2Ptr);
    }

    SECTION("Select third - third is selected, previous deselected") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
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
        rb2Ptr->setSelected(true);
        rb3Ptr->setSelected(true);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb3Ptr);
    }

    SECTION("Rapid selection changes maintain exclusivity") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 5, 55, 20, 20);
        auto rb4 = std::make_unique<RadioButton>(manager, 5, 80, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        RadioButton* rb3Ptr = rb3.get();
        RadioButton* rb4Ptr = rb4.get();

        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));
        groupPtr->addChild(std::move(rb3));
        groupPtr->addChild(std::move(rb4));

        rb1Ptr->setSelected(true);
        rb2Ptr->setSelected(true);
        rb1Ptr->setSelected(true);
        rb3Ptr->setSelected(true);
        rb4Ptr->setSelected(true);
        rb2Ptr->setSelected(true);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
        REQUIRE_FALSE(rb4Ptr->isSelected());
    }
}

TEST_CASE("RadioGroup - Programmatic Selection", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelected(true) on one button deselects others") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();

        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));

        rb1Ptr->setSelected(true);
        rb2Ptr->setSelected(true);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
    }

    SECTION("setSelected(false) does not affect other buttons") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();

        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::move(rb2));

        rb1Ptr->setSelected(true);
        rb1Ptr->setSelected(false);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("onButtonSelected method works correctly") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
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

        groupPtr->onButtonSelected(rb2Ptr);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("onButtonSelected with nullptr does not crash") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 5, 20, 20));

        REQUIRE_NOTHROW(groupPtr->onButtonSelected(nullptr));
    }
}

TEST_CASE("RadioGroup - Independent Groups", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Two RadioGroups have independent selections") {
        auto group1 = std::make_unique<RadioGroup>(manager, 0, 0, 100, 100);
        auto group2 = std::make_unique<RadioGroup>(manager, 110, 0, 100, 100);

        RadioGroup* group1Ptr = group1.get();
        RadioGroup* group2Ptr = group2.get();

        manager.addElement(std::move(group1));
        manager.addElement(std::move(group2));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb4 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        RadioButton* rb3Ptr = rb3.get();
        RadioButton* rb4Ptr = rb4.get();

        group1Ptr->addChild(std::move(rb1));
        group1Ptr->addChild(std::move(rb2));
        group2Ptr->addChild(std::move(rb3));
        group2Ptr->addChild(std::move(rb4));

        rb1Ptr->setSelected(true);
        rb3Ptr->setSelected(true);

        REQUIRE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE(rb3Ptr->isSelected());
        REQUIRE_FALSE(rb4Ptr->isSelected());
    }

    SECTION("Selecting in group A does not affect group B") {
        auto group1 = std::make_unique<RadioGroup>(manager, 0, 0, 100, 100);
        auto group2 = std::make_unique<RadioGroup>(manager, 110, 0, 100, 100);

        RadioGroup* group1Ptr = group1.get();
        RadioGroup* group2Ptr = group2.get();

        manager.addElement(std::move(group1));
        manager.addElement(std::move(group2));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb4 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        RadioButton* rb3Ptr = rb3.get();
        RadioButton* rb4Ptr = rb4.get();

        group1Ptr->addChild(std::move(rb1));
        group1Ptr->addChild(std::move(rb2));
        group2Ptr->addChild(std::move(rb3));
        group2Ptr->addChild(std::move(rb4));

        rb1Ptr->setSelected(true);
        rb3Ptr->setSelected(true);

        rb2Ptr->setSelected(true);

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
        REQUIRE(rb3Ptr->isSelected());
        REQUIRE_FALSE(rb4Ptr->isSelected());
        REQUIRE(group1Ptr->getSelectedButton() == rb2Ptr);
        REQUIRE(group2Ptr->getSelectedButton() == rb3Ptr);
    }

    SECTION("Each group tracks its own selected button") {
        auto group1 = std::make_unique<RadioGroup>(manager, 0, 0, 100, 100);
        auto group2 = std::make_unique<RadioGroup>(manager, 110, 0, 100, 100);

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

        REQUIRE(group1Ptr->getSelectedButton() == rb1Ptr);
        REQUIRE(group2Ptr->getSelectedButton() == rb2Ptr);
    }
}

TEST_CASE("RadioGroup - Non-RadioButton Children", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("RadioGroup can contain Panel children") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 5, 20, 20));
        groupPtr->addChild(std::make_unique<Panel>(manager, 30, 5, 50, 20));

        REQUIRE(groupPtr->getChildren().size() == 2);
    }

    SECTION("RadioGroup can contain Label children") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<RadioButton>(manager, 5, 5, 20, 20));
        groupPtr->addChild(std::make_unique<Label>(manager, 30, 5, "Label"));

        REQUIRE(groupPtr->getChildren().size() == 2);
    }

    SECTION("Non-RadioButton children are ignored by selection logic") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        RadioButton* rbPtr = rb.get();

        groupPtr->addChild(std::move(rb));
        groupPtr->addChild(std::make_unique<Panel>(manager, 30, 5, 50, 20));
        groupPtr->addChild(std::make_unique<Label>(manager, 30, 30, "Label"));

        rbPtr->setSelected(true);

        REQUIRE(rbPtr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rbPtr);
    }

    SECTION("getSelectedButton returns nullptr when only non-RadioButton children exist") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        groupPtr->addChild(std::make_unique<Panel>(manager, 5, 5, 50, 20));
        groupPtr->addChild(std::make_unique<Label>(manager, 5, 30, "Label"));

        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("Mixed children - selection works correctly with RadioButton and Panel") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        auto rb1 = std::make_unique<RadioButton>(manager, 5, 5, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 5, 30, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();

        groupPtr->addChild(std::move(rb1));
        groupPtr->addChild(std::make_unique<Panel>(manager, 30, 5, 50, 20));
        groupPtr->addChild(std::move(rb2));
        groupPtr->addChild(std::make_unique<Label>(manager, 30, 30, "Label"));

        rb1Ptr->setSelected(true);
        REQUIRE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());

        rb2Ptr->setSelected(true);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(rb2Ptr->isSelected());
    }
}

TEST_CASE("RadioGroup - Component Type", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns RadioGroup") {
        RadioGroup group(manager, 0, 0, 100, 50);
        REQUIRE(std::string(group.getComponentType()) == "RadioGroup");
    }

    SECTION("RadioGroup is a Panel subclass") {
        RadioGroup group(manager, 0, 0, 100, 50);
        Panel* panelPtr = dynamic_cast<Panel*>(&group);
        REQUIRE(panelPtr != nullptr);
    }
}

TEST_CASE("RadioGroup - Empty Group", "[radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("RadioGroup with no children works correctly") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        REQUIRE(groupPtr->getChildren().empty());
    }

    SECTION("getSelectedButton returns nullptr for empty group") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }

    SECTION("onButtonSelected on empty group does not crash") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();
        manager.addElement(std::move(group));

        REQUIRE_NOTHROW(groupPtr->onButtonSelected(nullptr));
    }
}