#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/checkbox.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Checkbox functionality", "[checkbox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initialization") {
        Checkbox cb(manager, 10, 20, 20, 20);
        REQUIRE(cb.getX() == 10);
        REQUIRE(cb.getY() == 20);
        REQUIRE(cb.getWidth() == 20);
        REQUIRE(cb.getHeight() == 20);
        REQUIRE_FALSE(cb.isChecked());
    }

    SECTION("Toggle by clicking") {
        auto checkbox = std::make_unique<Checkbox>(manager, 10, 10, 20, 20);
        Checkbox* cb = checkbox.get();
        bool changed = false;
        bool lastState = false;
        cb->setOnChange([&](Checkbox*, bool checked) {
            changed = true;
            lastState = checked;
        });
        manager.addElement(std::move(checkbox));

        REQUIRE_FALSE(cb->isChecked());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(cb->isChecked());
        REQUIRE(changed);
        REQUIRE(lastState);

        changed = false;
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE_FALSE(cb->isChecked());
        REQUIRE(changed);
        REQUIRE_FALSE(lastState);
    }

    SECTION("Programmatic toggle") {
        Checkbox cb(manager, 0, 0, 20, 20);
        bool changed = false;
        cb.setOnChange([&](Checkbox*, bool) { changed = true; });

        cb.setChecked(true);
        REQUIRE(cb.isChecked());
        REQUIRE(changed);

        changed = false;
        cb.setChecked(true);
        REQUIRE(cb.isChecked());
        REQUIRE_FALSE(changed);

        cb.setChecked(false);
        REQUIRE_FALSE(cb.isChecked());
        REQUIRE(changed);
    }
}
