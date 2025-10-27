#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/tab_control.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"

TEST_CASE("TabControl manages tabs and panels", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
    TabControl* tabs = tabControl.get();
    manager.addElement(std::move(tabControl));

    Panel* firstPanel = tabs->addTab("First");
    Panel* secondPanel = tabs->addTab("Second");
    Panel* thirdPanel = tabs->addTab("Third");

    SECTION("First tab is active after creation") {
        REQUIRE(firstPanel->isVisible());
        REQUIRE_FALSE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
    }

    SECTION("Clicking on other tab switches visibility") {
        REQUIRE(firstPanel->isVisible());

        auto* secondButton = dynamic_cast<Button*>(tabs->getChildren()[2].get());
        REQUIRE(secondButton != nullptr);
        auto abs = secondButton->getAbsolutePosition();
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));

        REQUIRE_FALSE(firstPanel->isVisible());
        REQUIRE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
    }
}
