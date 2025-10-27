#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Label basic properties", "[label]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Label can be created with text") {
        Label label(manager, 10, 20, "Test Label", 16);
        REQUIRE(label.getX() == 10);
        REQUIRE(label.getY() == 20);
    }
}
