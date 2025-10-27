#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/radio_button.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("RadioButton standalone behaviour", "[radio_button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initial state is unselected") {
        RadioButton rb(manager, 0, 0, 20, 20);
        REQUIRE_FALSE(rb.isSelected());
    }

    SECTION("Clicking selects the radio button") {
        auto rb = std::make_unique<RadioButton>(manager, 10, 10, 20, 20);
        RadioButton* rbPtr = rb.get();
        manager.addElement(std::move(rb));

        bool changed = false;
        bool lastState = false;
        rbPtr->setOnChange([&](RadioButton*, bool selected) {
            changed = true;
            lastState = selected;
        });

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));

        REQUIRE(rbPtr->isSelected());
        REQUIRE(changed);
        REQUIRE(lastState);
    }

    SECTION("Programmatically setting selection invokes callback") {
        RadioButton rb(manager, 0, 0, 20, 20);
        bool changed = false;
        rb.setOnChange([&](RadioButton*, bool) { changed = true; });

        rb.setSelected(true);
        REQUIRE(rb.isSelected());
        REQUIRE(changed);

        changed = false;
        rb.setSelected(true);
        REQUIRE(rb.isSelected());
        REQUIRE_FALSE(changed);

        rb.setSelected(false);
        REQUIRE_FALSE(rb.isSelected());
        REQUIRE(changed);
    }
}
