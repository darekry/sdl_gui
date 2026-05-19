#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/checkbox.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Checkbox initial state", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Checkbox starts unchecked by default") {
        Checkbox cb(manager, 10, 20, 30, 40);
        REQUIRE_FALSE(cb.isChecked());
    }

    SECTION("isChecked returns false initially") {
        Checkbox cb(manager, 0, 0, 20, 20);
        REQUIRE(cb.isChecked() == false);
    }
}

TEST_CASE("Checkbox position and dimensions", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor parameters correctly initialize position and size") {
        Checkbox cb(manager, 10, 20, 100, 50);
        REQUIRE(cb.getX() == 10);
        REQUIRE(cb.getY() == 20);
        REQUIRE(cb.getWidth() == 100);
        REQUIRE(cb.getHeight() == 50);
    }

    SECTION("Zero position and dimensions") {
        Checkbox cb(manager, 0, 0, 1, 1);
        REQUIRE(cb.getX() == 0);
        REQUIRE(cb.getY() == 0);
        REQUIRE(cb.getWidth() == 1);
        REQUIRE(cb.getHeight() == 1);
    }
}

TEST_CASE("Checkbox component type", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns Checkbox") {
        Checkbox cb(manager, 0, 0, 20, 20);
        REQUIRE(std::string(cb.getComponentType()) == "Checkbox");
    }
}

TEST_CASE("Checkbox programmatic toggle", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setChecked(true) becomes checked and fires onChange callback") {
        Checkbox cb(manager, 0, 0, 20, 20);
        bool changed = false;
        bool lastState = false;
        cb.setOnChange([&](Checkbox*, bool checked) {
            changed = true;
            lastState = checked;
        });

        cb.setChecked(true);

        REQUIRE(cb.isChecked());
        REQUIRE(changed);
        REQUIRE(lastState == true);
    }

    SECTION("setChecked(false) becomes unchecked and fires onChange callback") {
        Checkbox cb(manager, 0, 0, 20, 20);
        cb.setChecked(true);
        REQUIRE(cb.isChecked());

        bool changed = false;
        bool lastState = true;
        cb.setOnChange([&](Checkbox*, bool checked) {
            changed = true;
            lastState = checked;
        });

        cb.setChecked(false);

        REQUIRE_FALSE(cb.isChecked());
        REQUIRE(changed);
        REQUIRE(lastState == false);
    }

    SECTION("setChecked(same value) does not fire callback") {
        Checkbox cb(manager, 0, 0, 20, 20);
        
        int callbackCount = 0;
        cb.setOnChange([&](Checkbox*, bool) { callbackCount++; });

        cb.setChecked(true);
        REQUIRE(callbackCount == 1);

        cb.setChecked(true);
        REQUIRE(callbackCount == 1);

        cb.setChecked(false);
        REQUIRE(callbackCount == 2);

        cb.setChecked(false);
        REQUIRE(callbackCount == 2);
    }

    SECTION("Programmatic setChecked works without callback set") {
        Checkbox cb(manager, 0, 0, 20, 20);
        
        REQUIRE_NOTHROW(cb.setChecked(true));
        REQUIRE(cb.isChecked());
        
        REQUIRE_NOTHROW(cb.setChecked(false));
        REQUIRE_FALSE(cb.isChecked());
    }
}

TEST_CASE("Checkbox click toggle behavior", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click toggles checked state: unchecked -> checked -> unchecked") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        REQUIRE_FALSE(cb->isChecked());

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(cb->isChecked());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE_FALSE(cb->isChecked());
    }

    SECTION("onChange callback fires with correct new state when checked") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        bool changed = false;
        bool lastState = false;
        cb->setOnChange([&](Checkbox*, bool checked) {
            changed = true;
            lastState = checked;
        });
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(changed);
        REQUIRE(lastState == true);
    }

    SECTION("onChange callback fires with correct new state when unchecked") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setChecked(true);
        REQUIRE(cb->isChecked());

        bool changed = false;
        bool lastState = true;
        cb->setOnChange([&](Checkbox*, bool checked) {
            changed = true;
            lastState = checked;
        });
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(changed);
        REQUIRE(lastState == false);
    }

    SECTION("Click outside checkbox does not toggle state") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        REQUIRE_FALSE(cb->isChecked());
    }

    SECTION("Press inside and release outside does not toggle") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 50, 50));

        REQUIRE_FALSE(cb->isChecked());
    }
}

TEST_CASE("Checkbox disabled state", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled checkbox ignores clicks - state doesn't change") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setEnabled(false);
        manager.addElement(std::move(checkbox));

        REQUIRE_FALSE(cb->isEnabled());
        REQUIRE_FALSE(cb->isChecked());

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE_FALSE(cb->isChecked());
    }

    SECTION("setEnabled(false) prevents toggling via click") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(cb->isChecked());

        cb->setEnabled(false);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(cb->isChecked());
    }

    SECTION("Programmatic setChecked still works on disabled checkbox") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setEnabled(false);
        
        bool changed = false;
        cb->setOnChange([&](Checkbox*, bool) { changed = true; });
        
        manager.addElement(std::move(checkbox));

        REQUIRE_FALSE(cb->isEnabled());

        cb->setChecked(true);
        REQUIRE(cb->isChecked());
        REQUIRE(changed);

        changed = false;
        cb->setChecked(false);
        REQUIRE_FALSE(cb->isChecked());
        REQUIRE(changed);
    }

    SECTION("Enabling disabled checkbox allows clicks again") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setEnabled(false);
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE_FALSE(cb->isChecked());

        cb->setEnabled(true);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(cb->isChecked());
    }
}

TEST_CASE("Checkbox hidden state", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden checkbox doesn't respond to events") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setVisible(false);
        manager.addElement(std::move(checkbox));

        REQUIRE_FALSE(cb->isVisible());
        REQUIRE_FALSE(cb->isChecked());

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE_FALSE(cb->isChecked());
    }

    SECTION("Hidden checkbox can still be set programmatically") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setVisible(false);
        
        bool changed = false;
        cb->setOnChange([&](Checkbox*, bool) { changed = true; });
        
        manager.addElement(std::move(checkbox));

        cb->setChecked(true);
        REQUIRE(cb->isChecked());
        REQUIRE(changed);
    }

    SECTION("Making hidden checkbox visible allows clicks") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        cb->setVisible(false);
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE_FALSE(cb->isChecked());

        cb->setVisible(true);

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(cb->isChecked());
    }
}

TEST_CASE("Multiple checkboxes", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Multiple checkboxes work independently") {
        auto cb1 = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        auto cb2 = std::make_unique<Checkbox>(manager, 40, 10, 20, 20);
        auto cb3 = std::make_unique<Checkbox>(manager, 70, 10, 20, 20);

        Checkbox* cb1_ptr = cb1.get();
        Checkbox* cb2_ptr = cb2.get();
        Checkbox* cb3_ptr = cb3.get();

        manager.addElement(std::move(cb1));
        manager.addElement(std::move(cb2));
        manager.addElement(std::move(cb3));

        REQUIRE_FALSE(cb1_ptr->isChecked());
        REQUIRE_FALSE(cb2_ptr->isChecked());
        REQUIRE_FALSE(cb3_ptr->isChecked());

        cb1_ptr->setChecked(true);
        REQUIRE(cb1_ptr->isChecked());
        REQUIRE_FALSE(cb2_ptr->isChecked());
        REQUIRE_FALSE(cb3_ptr->isChecked());

        cb2_ptr->setChecked(true);
        REQUIRE(cb1_ptr->isChecked());
        REQUIRE(cb2_ptr->isChecked());
        REQUIRE_FALSE(cb3_ptr->isChecked());

        cb3_ptr->setChecked(true);
        REQUIRE(cb1_ptr->isChecked());
        REQUIRE(cb2_ptr->isChecked());
        REQUIRE(cb3_ptr->isChecked());
    }

    SECTION("Each checkbox toggles independently via click") {
        auto cb1 = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        auto cb2 = std::make_unique<Checkbox>(manager, 40, 10, 20, 20);

        Checkbox* cb1_ptr = cb1.get();
        Checkbox* cb2_ptr = cb2.get();

        manager.addElement(std::move(cb1));
        manager.addElement(std::move(cb2));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(cb1_ptr->isChecked());
        REQUIRE_FALSE(cb2_ptr->isChecked());

        manager.processEvent(helper.createMouseMotion(45, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 45, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 45, 15));

        REQUIRE(cb1_ptr->isChecked());
        REQUIRE(cb2_ptr->isChecked());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE_FALSE(cb1_ptr->isChecked());
        REQUIRE(cb2_ptr->isChecked());
    }

    SECTION("Each checkbox has independent callback") {
        auto cb1 = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        auto cb2 = std::make_unique<Checkbox>(manager, 40, 10, 20, 20);

        Checkbox* cb1_ptr = cb1.get();
        Checkbox* cb2_ptr = cb2.get();

        int cb1Count = 0;
        int cb2Count = 0;

        cb1_ptr->setOnChange([&](Checkbox*, bool) { cb1Count++; });
        cb2_ptr->setOnChange([&](Checkbox*, bool) { cb2Count++; });

        manager.addElement(std::move(cb1));
        manager.addElement(std::move(cb2));

        cb1_ptr->setChecked(true);
        REQUIRE(cb1Count == 1);
        REQUIRE(cb2Count == 0);

        cb2_ptr->setChecked(true);
        REQUIRE(cb1Count == 1);
        REQUIRE(cb2Count == 1);

        cb1_ptr->setChecked(false);
        cb2_ptr->setChecked(false);
        REQUIRE(cb1Count == 2);
        REQUIRE(cb2Count == 2);
    }
}

TEST_CASE("Checkbox callback details", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Callback receives pointer to self") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        Checkbox* callbackSender = nullptr;

        cb->setOnChange([&](Checkbox* sender, bool) {
            callbackSender = sender;
        });
        manager.addElement(std::move(checkbox));

        cb->setChecked(true);
        REQUIRE(callbackSender == cb);
    }

    SECTION("Multiple setOnChange calls - last one wins") {
        Checkbox cb(manager, 0, 0, 20, 20);
        
        int callback1Count = 0;
        int callback2Count = 0;

        cb.setOnChange([&](Checkbox*, bool) { callback1Count++; });
        cb.setOnChange([&](Checkbox*, bool) { callback2Count++; });

        cb.setChecked(true);
        REQUIRE(callback1Count == 0);
        REQUIRE(callback2Count == 1);
    }

    SECTION("Empty callback does not crash on programmatic setChecked") {
        Checkbox cb(manager, 0, 0, 20, 20);
        
        REQUIRE_NOTHROW(cb.setChecked(true));
        REQUIRE(cb.isChecked());
    }
}

TEST_CASE("Checkbox right-click does not toggle", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Right-click does not toggle checkbox") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 15, 15));

        REQUIRE_FALSE(cb->isChecked());
    }

    SECTION("Middle-click does not toggle checkbox") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        manager.addElement(std::move(checkbox));

        manager.processEvent(helper.createMouseMotion(15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_MIDDLE, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_MIDDLE, 15, 15));

        REQUIRE_FALSE(cb->isChecked());
    }
}