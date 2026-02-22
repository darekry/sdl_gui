#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/tab_control.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/label.hpp"

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

        // Structure: Button0, Panel0, Button1, Panel1, Button2, Panel2
        // Second button is at index 2
        auto* secondButton = dynamic_cast<Button*>(tabs->getChildren()[2].get());
        REQUIRE(secondButton != nullptr);
        auto abs = secondButton->getAbsolutePosition();
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));

        REQUIRE_FALSE(firstPanel->isVisible());
        REQUIRE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
    }

    SECTION("Clicking on active tab does not change state") {
        // First button is at index 0
        auto* firstButton = dynamic_cast<Button*>(tabs->getChildren()[0].get());
        REQUIRE(firstButton != nullptr);
        auto abs = firstButton->getAbsolutePosition();
        
        // First tab is already active
        REQUIRE(firstPanel->isVisible());
        
        // Click on active tab
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        
        // Should still be visible
        REQUIRE(firstPanel->isVisible());
        REQUIRE_FALSE(secondPanel->isVisible());
    }
}

TEST_CASE("TabControl programmatic tab switching", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
    TabControl* tabs = tabControl.get();
    manager.addElement(std::move(tabControl));

    Panel* firstPanel = tabs->addTab("First");
    Panel* secondPanel = tabs->addTab("Second");
    Panel* thirdPanel = tabs->addTab("Third");

    SECTION("setActiveTab switches to different tab") {
        // Structure: Button0, Panel0, Button1, Panel1, Button2, Panel2
        auto& children = tabs->getChildren();
        REQUIRE(children.size() == 6); // 3 buttons + 3 panels
        
        // Buttons are at indices 0, 2, 4
        auto* firstButton = dynamic_cast<Button*>(children[0].get());
        auto* secondButton = dynamic_cast<Button*>(children[2].get());
        auto* thirdButton = dynamic_cast<Button*>(children[4].get());
        
        REQUIRE(firstButton != nullptr);
        REQUIRE(secondButton != nullptr);
        REQUIRE(thirdButton != nullptr);
        
        // Programmatically switch to second tab
        tabs->setActiveTab(secondButton);
        
        REQUIRE_FALSE(firstPanel->isVisible());
        REQUIRE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
    }

    SECTION("setActiveTab with same tab does nothing") {
        auto& children = tabs->getChildren();
        auto* firstButton = dynamic_cast<Button*>(children[0].get());
        REQUIRE(firstButton != nullptr);
        
        // First tab is already active
        REQUIRE(firstPanel->isVisible());
        
        // Set same tab again
        tabs->setActiveTab(firstButton);
        
        // Should still be the same
        REQUIRE(firstPanel->isVisible());
        REQUIRE_FALSE(secondPanel->isVisible());
    }

    SECTION("Multiple tab switches work correctly") {
        auto& children = tabs->getChildren();
        auto* firstButton = dynamic_cast<Button*>(children[0].get());
        auto* secondButton = dynamic_cast<Button*>(children[2].get());
        auto* thirdButton = dynamic_cast<Button*>(children[4].get());
        
        REQUIRE(firstButton != nullptr);
        REQUIRE(secondButton != nullptr);
        REQUIRE(thirdButton != nullptr);
        
        // Switch to second
        tabs->setActiveTab(secondButton);
        REQUIRE_FALSE(firstPanel->isVisible());
        REQUIRE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
        
        // Switch to third
        tabs->setActiveTab(thirdButton);
        REQUIRE_FALSE(firstPanel->isVisible());
        REQUIRE_FALSE(secondPanel->isVisible());
        REQUIRE(thirdPanel->isVisible());
        
        // Switch back to first
        tabs->setActiveTab(firstButton);
        REQUIRE(firstPanel->isVisible());
        REQUIRE_FALSE(secondPanel->isVisible());
        REQUIRE_FALSE(thirdPanel->isVisible());
    }
}

TEST_CASE("TabControl dimensions and positioning", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("TabControl has correct initial dimensions") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 30, 300, 200);
        TabControl* tabs = tabControl.get();
        
        REQUIRE(tabs->getX() == 20);
        REQUIRE(tabs->getY() == 30);
        REQUIRE(tabs->getWidth() == 300);
        REQUIRE(tabs->getHeight() == 200);
        
        manager.addElement(std::move(tabControl));
    }

    SECTION("TabControl with custom tab button height") {
        auto tabControl = std::make_unique<TabControl>(manager, 0, 0, 400, 300, 40);
        TabControl* tabs = tabControl.get();
        
        REQUIRE(tabs->getWidth() == 400);
        REQUIRE(tabs->getHeight() == 300);
        
        manager.addElement(std::move(tabControl));
    }

    SECTION("Tab panels are positioned below tab buttons") {
        auto tabControl = std::make_unique<TabControl>(manager, 0, 0, 300, 200, 30);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel = tabs->addTab("Test");
        
        // Panel should be positioned at y = tabButtonHeight
        REQUIRE(panel->getY() == 30);
        REQUIRE(panel->getHeight() == 200 - 30); // height - tabButtonHeight
    }

    SECTION("Multiple tabs are positioned horizontally") {
        auto tabControl = std::make_unique<TabControl>(manager, 0, 0, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        tabs->addTab("A");
        tabs->addTab("B");
        tabs->addTab("C");
        
        // Structure: Button0, Panel0, Button1, Panel1, Button2, Panel2
        auto& children = tabs->getChildren();
        auto* button0 = dynamic_cast<Button*>(children[0].get());
        auto* button1 = dynamic_cast<Button*>(children[2].get());
        auto* button2 = dynamic_cast<Button*>(children[4].get());
        
        REQUIRE(button0 != nullptr);
        REQUIRE(button1 != nullptr);
        REQUIRE(button2 != nullptr);
        
        // All buttons should be at y = 0
        REQUIRE(button0->getY() == 0);
        REQUIRE(button1->getY() == 0);
        REQUIRE(button2->getY() == 0);
        
        // Buttons should be positioned sequentially in x
        REQUIRE(button0->getX() == 0);
        // Button 1 should be after button 0 (with spacing)
        REQUIRE(button1->getX() >= button0->getWidth());
        // Button 2 should be after button 1 (with spacing)
        REQUIRE(button2->getX() >= button1->getX() + button1->getWidth());
    }
}

TEST_CASE("TabControl visibility and enabled state", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden TabControl hides all content") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        auto tabsPtr = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel = tabs->addTab("Test");
        REQUIRE(panel->isVisible()); // Panel is visible initially
        
        // Hide the entire TabControl
        tabsPtr->setVisible(false);
        REQUIRE_FALSE(tabsPtr->isVisible());
    }

    SECTION("Tab panel visibility changes with active tab") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel1 = tabs->addTab("Tab1");
        Panel* panel2 = tabs->addTab("Tab2");
        
        REQUIRE(panel1->isVisible());
        REQUIRE_FALSE(panel2->isVisible());
        
        // Structure: Button0, Panel0, Button1, Panel1
        auto& children = tabs->getChildren();
        auto* button2 = dynamic_cast<Button*>(children[2].get());
        REQUIRE(button2 != nullptr);
        
        tabs->setActiveTab(button2);
        
        REQUIRE_FALSE(panel1->isVisible());
        REQUIRE(panel2->isVisible());
    }
}

TEST_CASE("TabControl component type", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
    TabControl* tabs = tabControl.get();
    
    SECTION("getComponentType returns correct type name") {
        REQUIRE(std::string(tabs->getComponentType()) == "TabControl");
    }
    
    manager.addElement(std::move(tabControl));
}

TEST_CASE("TabControl with no tabs", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Empty TabControl can be created") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        
        // Should have no children initially (no tabs added)
        // Note: TabControl might have some internal children, so we just verify it doesn't crash
        REQUIRE(tabs->getWidth() == 300);
        REQUIRE(tabs->getHeight() == 200);
        
        manager.addElement(std::move(tabControl));
    }

    SECTION("Adding first tab makes it active") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* firstPanel = tabs->addTab("First");
        
        // First tab should be automatically active
        REQUIRE(firstPanel->isVisible());
    }
}

TEST_CASE("TabControl tab button callbacks", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Tab button click callback triggers tab switch") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel1 = tabs->addTab("Tab1");
        Panel* panel2 = tabs->addTab("Tab2");
        
        REQUIRE(panel1->isVisible());
        REQUIRE_FALSE(panel2->isVisible());
        
        // Structure: Button0, Panel0, Button1, Panel1
        auto& children = tabs->getChildren();
        auto* button2 = dynamic_cast<Button*>(children[2].get());
        REQUIRE(button2 != nullptr);
        
        // Get absolute position for click
        auto abs = button2->getAbsolutePosition();
        
        // Simulate click on second tab button
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, abs.x + 5, abs.y + 5));
        
        // Tab should have switched
        REQUIRE_FALSE(panel1->isVisible());
        REQUIRE(panel2->isVisible());
    }
}

TEST_CASE("TabControl children hierarchy", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Tab buttons and panels are children of TabControl") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        tabs->addTab("A");
        tabs->addTab("B");
        
        auto& children = tabs->getChildren();
        
        // Should have 4 children: 2 buttons + 2 panels (interleaved)
        REQUIRE(children.size() == 4);
        
        // Structure: Button0, Panel0, Button1, Panel1
        auto* button0 = dynamic_cast<Button*>(children[0].get());
        auto* panel0 = dynamic_cast<Panel*>(children[1].get());
        auto* button1 = dynamic_cast<Button*>(children[2].get());
        auto* panel1 = dynamic_cast<Panel*>(children[3].get());
        
        REQUIRE(button0 != nullptr);
        REQUIRE(panel0 != nullptr);
        REQUIRE(button1 != nullptr);
        REQUIRE(panel1 != nullptr);
    }

    SECTION("Content can be added to tab panels") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel = tabs->addTab("Content");
        
        // Add a label to the tab panel
        auto label = std::make_unique<Label>(manager, 10, 10, "Hello", 16);
        Label* labelPtr = label.get();
        panel->addChild(std::move(label));
        
        // Label should be a child of the panel
        auto& panelChildren = panel->getChildren();
        REQUIRE(panelChildren.size() == 1);
        REQUIRE(panelChildren[0].get() == labelPtr);
    }
}

TEST_CASE("TabControl tab button width", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Tab button uses specified width") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel = tabs->addTab("Test", 150); // width = 150
        
        // Structure: Button0, Panel0
        auto& children = tabs->getChildren();
        auto* button = dynamic_cast<Button*>(children[0].get());
        REQUIRE(button != nullptr);
        REQUIRE(button->getWidth() == 150);
    }

    SECTION("Tab button uses default width when not specified") {
        auto tabControl = std::make_unique<TabControl>(manager, 20, 20, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        Panel* panel = tabs->addTab("Test"); // default width = 100
        
        auto& children = tabs->getChildren();
        auto* button = dynamic_cast<Button*>(children[0].get());
        REQUIRE(button != nullptr);
        REQUIRE(button->getWidth() == 100);
    }
}

TEST_CASE("TabControl absolute position for events", "[tab_control]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Tab buttons have correct absolute position when TabControl is offset") {
        auto tabControl = std::make_unique<TabControl>(manager, 50, 60, 300, 200);
        TabControl* tabs = tabControl.get();
        manager.addElement(std::move(tabControl));
        
        tabs->addTab("Tab1");
        tabs->addTab("Tab2");
        
        // Structure: Button0, Panel0, Button1, Panel1
        auto& children = tabs->getChildren();
        auto* button0 = dynamic_cast<Button*>(children[0].get());
        auto* button1 = dynamic_cast<Button*>(children[2].get());
        
        REQUIRE(button0 != nullptr);
        REQUIRE(button1 != nullptr);
        
        // Absolute position should include TabControl offset
        auto abs0 = button0->getAbsolutePosition();
        auto abs1 = button1->getAbsolutePosition();
        
        REQUIRE(abs0.x == 50); // TabControl x + button local x (0)
        REQUIRE(abs0.y == 60); // TabControl y + button local y (0)
        
        // Second button should be offset by first button width + spacing
        REQUIRE(abs1.x >= 50 + button0->getWidth());
        REQUIRE(abs1.y == 60);
    }
}
