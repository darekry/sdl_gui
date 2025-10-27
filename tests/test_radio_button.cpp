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
