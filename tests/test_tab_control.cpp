#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui.hpp"
#include "../src/tab_control.hpp"
#include "../src/font_manager.hpp"
#include "../src/texture_manager.hpp"

TEST_CASE("TabControl Functionality", "[tab_control]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    FontManager fontManager;
    auto font = fontManager.loadFont("assets/ARIAL.TTF", 16);
    REQUIRE(font != nullptr);

    SECTION("Initialization") {
        TabControl tabControl(0, 0, 800, 600);
        REQUIRE(tabControl.getX() == 0);
        REQUIRE(tabControl.getY() == 0);
        REQUIRE(tabControl.getWidth() == 800);
        REQUIRE(tabControl.getHeight() == 600);
    }

    SECTION("Adding Tabs") {
        TabControl tabControl(0, 0, 800, 600);
        
        Panel* panel1 = tabControl.addTab("Tab 1");
        REQUIRE(panel1 != nullptr);
        REQUIRE(panel1->isEnabled() == true); // First tab should be active

        Panel* panel2 = tabControl.addTab("Tab 2");
        REQUIRE(panel2 != nullptr);
        REQUIRE(panel2->isEnabled() == false); // Second tab should be inactive
    }

    SECTION("Setting Active Tab") {
        TabControl tabControl(0, 0, 800, 600);
        
        Panel* panel1 = tabControl.addTab("Tab 1");
        Panel* panel2 = tabControl.addTab("Tab 2");

        // Manually find the button for the second tab to activate it
        // This is a simplification for the test. In a real scenario, we'd have better access.
        const auto& children = tabControl.getChildren();
        Button* tab2Button = nullptr;
        for(const auto& child : children) {
            Button* btn = dynamic_cast<Button*>(child.get());
            if(btn) {
                // A simple way to distinguish buttons for this test
                if(btn->getX() > 0) { 
                    tab2Button = btn;
                    break;
                }
            }
        }
        
        REQUIRE(tab2Button != nullptr);
        tabControl.setActiveTab(tab2Button);

        REQUIRE(panel1->isEnabled() == false);
        REQUIRE(panel2->isEnabled() == true);
    }

    SECTION("Event Handling - Switching Tabs on Click") {
        TabControl tabControl(0, 0, 800, 600);
        
        Panel* panel1 = tabControl.addTab("Tab 1");
        Panel* panel2 = tabControl.addTab("Tab 2");

        // Find the button for the second tab
        const auto& children = tabControl.getChildren();
        Button* tab2Button = nullptr;
        for(const auto& child : children) {
            Button* btn = dynamic_cast<Button*>(child.get());
            if(btn && btn->getX() > 0) { // Find second button based on position
                tab2Button = btn;
                break;
            }
        }
        REQUIRE(tab2Button != nullptr);

        // Simulate click on the second tab button
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, tab2Button->getX() + 10, tab2Button->getY() + 10);
        tabControl.handleEvent(event);

        REQUIRE(panel1->isEnabled() == false);
        REQUIRE(panel2->isEnabled() == true);
    }
}