#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/radio_button.hpp"
#include "../src/radio_group.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("RadioButton initial state", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("starts unselected") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE_FALSE(rb.isSelected());
    }

    SECTION("position and dimensions from constructor") {
        RadioButton rb(manager, 10, 20, 100, 50);
        REQUIRE(rb.getX() == 10);
        REQUIRE(rb.getY() == 20);
        REQUIRE(rb.getWidth() == 100);
        REQUIRE(rb.getHeight() == 50);
    }

    SECTION("component type is RadioButton") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE(std::string(rb.getComponentType()) == "RadioButton");
    }
}

TEST_CASE("RadioButton selection behavior standalone", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelected(true) selects and fires callback") {
        RadioButton rb(manager, 0, 0, 20, 20);
        bool callbackFired = false;
        bool callbackState = false;
        RadioButton* callbackSender = nullptr;

        rb.setOnChange([&](RadioButton* sender, bool selected) {
            callbackFired = true;
            callbackState = selected;
            callbackSender = sender;
        });

        rb.setSelected(true);

        REQUIRE(rb.isSelected());
        REQUIRE(callbackFired);
        REQUIRE(callbackState == true);
        REQUIRE(callbackSender == &rb);
    }

    SECTION("setSelected(false) deselects and fires callback") {
        RadioButton rb(manager, 0, 0, 20, 20);
        rb.setSelected(true);
        REQUIRE(rb.isSelected());

        bool callbackFired = false;
        bool callbackState = true;

        rb.setOnChange([&](RadioButton*, bool selected) {
            callbackFired = true;
            callbackState = selected;
        });

        rb.setSelected(false);

        REQUIRE_FALSE(rb.isSelected());
        REQUIRE(callbackFired);
        REQUIRE(callbackState == false);
    }

    SECTION("setSelected(same value) does not fire callback") {
        RadioButton rb(manager, 0, 0, 20, 20);
        int callbackCount = 0;

        rb.setOnChange([&](RadioButton*, bool) { callbackCount++; });

        rb.setSelected(true);
        REQUIRE(callbackCount == 1);

        rb.setSelected(true);
        REQUIRE(callbackCount == 1);

        rb.setSelected(false);
        REQUIRE(callbackCount == 2);

        rb.setSelected(false);
        REQUIRE(callbackCount == 2);
    }

    SECTION("callback receives pointer to RadioButton and boolean state") {
        RadioButton rb(manager, 0, 0, 20, 20);
        RadioButton* receivedSender = nullptr;
        bool receivedState = false;

        rb.setOnChange([&](RadioButton* sender, bool state) {
            receivedSender = sender;
            receivedState = state;
        });

        rb.setSelected(true);

        REQUIRE(receivedSender == &rb);
        REQUIRE(receivedState == true);
    }
}

TEST_CASE("RadioButton disabled state", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("disabled RadioButton is not enabled") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        manager.addElement(std::move(rb));

        REQUIRE(rbPtr->isEnabled());

        rbPtr->setEnabled(false);

        REQUIRE_FALSE(rbPtr->isEnabled());
    }

    SECTION("disabled RadioButton ignores click events") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        rbPtr->setEnabled(false);
        int callbackCount = 0;
        rbPtr->setOnChange([&](RadioButton*, bool) { callbackCount++; });
        manager.addElement(std::move(rb));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE_FALSE(rbPtr->isSelected());
        REQUIRE(callbackCount == 0);
    }

    SECTION("setSelected works programmatically on disabled button") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        rbPtr->setEnabled(false);
        bool callbackFired = false;
        rbPtr->setOnChange([&](RadioButton*, bool) { callbackFired = true; });
        manager.addElement(std::move(rb));

        rbPtr->setSelected(true);

        REQUIRE(rbPtr->isSelected());
        REQUIRE(callbackFired);

        callbackFired = false;
        rbPtr->setSelected(false);

        REQUIRE_FALSE(rbPtr->isSelected());
        REQUIRE(callbackFired);
    }
}

TEST_CASE("RadioButton hidden state", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("hidden RadioButton does not respond to events") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        rbPtr->setVisible(false);
        int callbackCount = 0;
        rbPtr->setOnChange([&](RadioButton*, bool) { callbackCount++; });
        manager.addElement(std::move(rb));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE_FALSE(rbPtr->isSelected());
        REQUIRE(callbackCount == 0);
    }

    SECTION("setSelected works programmatically on hidden button") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        rbPtr->setVisible(false);
        manager.addElement(std::move(rb));

        rbPtr->setSelected(true);

        REQUIRE(rbPtr->isSelected());
    }
}

TEST_CASE("RadioButton click behavior in RadioGroup", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("click on unselected button selects it") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        int callbackCount = 0;
        rbPtr->setOnChange([&](RadioButton*, bool) { callbackCount++; });
        manager.addElement(std::move(rb));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(rbPtr->isSelected());
        REQUIRE(callbackCount == 1);
    }

    SECTION("click on already selected button stays selected") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        rbPtr->setSelected(true);
        int callbackCount = 0;
        rbPtr->setOnChange([&](RadioButton*, bool) { callbackCount++; });
        manager.addElement(std::move(rb));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(rbPtr->isSelected());
        REQUIRE(callbackCount == 0);
    }
}

TEST_CASE("RadioButton mutual exclusivity in RadioGroup", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("selecting button A deselects buttons B and C") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 40, 10, 20, 20);
        auto rb3 = std::make_unique<RadioButton>(manager, 70, 10, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();
        RadioButton* rb3Ptr = rb3.get();

        group->addChild(std::move(rb1));
        group->addChild(std::move(rb2));
        group->addChild(std::move(rb3));

        manager.addElement(std::move(group));

        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE_FALSE(rb2Ptr->isSelected());
        REQUIRE_FALSE(rb3Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == nullptr);

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

    SECTION("callbacks fire for both deselected and selected buttons") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        auto rb2 = std::make_unique<RadioButton>(manager, 40, 10, 20, 20);

        RadioButton* rb1Ptr = rb1.get();
        RadioButton* rb2Ptr = rb2.get();

        int rb1CallbackCount = 0;
        int rb2CallbackCount = 0;
        bool rb1LastState = false;
        bool rb2LastState = false;

        rb1Ptr->setOnChange([&](RadioButton*, bool selected) {
            rb1CallbackCount++;
            rb1LastState = selected;
        });
        rb2Ptr->setOnChange([&](RadioButton*, bool selected) {
            rb2CallbackCount++;
            rb2LastState = selected;
        });

        group->addChild(std::move(rb1));
        group->addChild(std::move(rb2));
        manager.addElement(std::move(group));

        rb1Ptr->setSelected(true);
        REQUIRE(rb1CallbackCount == 1);
        REQUIRE(rb1LastState == true);
        REQUIRE(rb2CallbackCount == 0);

        rb2Ptr->setSelected(true);
        REQUIRE(rb1CallbackCount == 2);
        REQUIRE(rb1LastState == false);
        REQUIRE(rb2CallbackCount == 1);
        REQUIRE(rb2LastState == true);
    }

    SECTION("deselecting via setSelected(false) clears group selection") {
        auto group = std::make_unique<RadioGroup>(manager, 0, 0, 200, 100);
        RadioGroup* groupPtr = group.get();

        auto rb1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rb1Ptr = rb1.get();
        group->addChild(std::move(rb1));
        manager.addElement(std::move(group));

        rb1Ptr->setSelected(true);
        REQUIRE(rb1Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == rb1Ptr);

        rb1Ptr->setSelected(false);
        REQUIRE_FALSE(rb1Ptr->isSelected());
        REQUIRE(groupPtr->getSelectedButton() == nullptr);
    }
}

TEST_CASE("RadioButton multiple groups independence", "[radio_button][radio_group]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("buttons in different groups are independent") {
        auto groupA = std::make_unique<RadioGroup>(manager, 0, 0, 100, 100);
        auto groupB = std::make_unique<RadioGroup>(manager, 100, 0, 100, 100);

        auto rbA1 = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        auto rbA2 = std::make_unique<RadioButton>(manager, 40, 10, 20, 20);
        auto rbB1 = std::make_unique<RadioButton>(manager, 110, 10, 20, 20);
        auto rbB2 = std::make_unique<RadioButton>(manager, 140, 10, 20, 20);

        RadioButton* rbA1Ptr = rbA1.get();
        RadioButton* rbA2Ptr = rbA2.get();
        RadioButton* rbB1Ptr = rbB1.get();
        RadioButton* rbB2Ptr = rbB2.get();

        groupA->addChild(std::move(rbA1));
        groupA->addChild(std::move(rbA2));
        groupB->addChild(std::move(rbB1));
        groupB->addChild(std::move(rbB2));

        RadioGroup* groupAPtr = groupA.get();
        RadioGroup* groupBPtr = groupB.get();

        manager.addElement(std::move(groupA));
        manager.addElement(std::move(groupB));

        rbA1Ptr->setSelected(true);
        REQUIRE(rbA1Ptr->isSelected());
        REQUIRE_FALSE(rbA2Ptr->isSelected());
        REQUIRE_FALSE(rbB1Ptr->isSelected());
        REQUIRE_FALSE(rbB2Ptr->isSelected());
        REQUIRE(groupAPtr->getSelectedButton() == rbA1Ptr);
        REQUIRE(groupBPtr->getSelectedButton() == nullptr);

        rbB1Ptr->setSelected(true);
        REQUIRE(rbA1Ptr->isSelected());
        REQUIRE_FALSE(rbA2Ptr->isSelected());
        REQUIRE(rbB1Ptr->isSelected());
        REQUIRE_FALSE(rbB2Ptr->isSelected());
        REQUIRE(groupAPtr->getSelectedButton() == rbA1Ptr);
        REQUIRE(groupBPtr->getSelectedButton() == rbB1Ptr);

        rbA2Ptr->setSelected(true);
        REQUIRE_FALSE(rbA1Ptr->isSelected());
        REQUIRE(rbA2Ptr->isSelected());
        REQUIRE(rbB1Ptr->isSelected());
        REQUIRE_FALSE(rbB2Ptr->isSelected());

        rbB2Ptr->setSelected(true);
        REQUIRE_FALSE(rbA1Ptr->isSelected());
        REQUIRE(rbA2Ptr->isSelected());
        REQUIRE_FALSE(rbB1Ptr->isSelected());
        REQUIRE(rbB2Ptr->isSelected());
    }
}

TEST_CASE("RadioButton empty callback", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("no callback set does not crash on selection") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        manager.addElement(std::move(rb));

        REQUIRE_NOTHROW(rbPtr->setSelected(true));
        REQUIRE(rbPtr->isSelected());

        REQUIRE_NOTHROW(rbPtr->setSelected(false));
        REQUIRE_FALSE(rbPtr->isSelected());
    }

    SECTION("replacing callback uses last one") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();

        int firstCount = 0;
        int secondCount = 0;

        rbPtr->setOnChange([&](RadioButton*, bool) { firstCount++; });
        rbPtr->setOnChange([&](RadioButton*, bool) { secondCount++; });

        manager.addElement(std::move(rb));

        rbPtr->setSelected(true);

        REQUIRE(firstCount == 0);
        REQUIRE(secondCount == 1);
    }
}