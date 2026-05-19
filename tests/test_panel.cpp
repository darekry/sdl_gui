#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Panel constructors initialize position and size correctly", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor with x, y, width, height parameters") {
        Panel panel(manager, 10, 20, 200, 150);
        REQUIRE(panel.getX() == 10);
        REQUIRE(panel.getY() == 20);
        REQUIRE(panel.getWidth() == 200);
        REQUIRE(panel.getHeight() == 150);
    }

    SECTION("Constructor with SDL_Rect parameter") {
        SDL_Rect rect{50, 75, 300, 250};
        Panel panel(manager, rect);
        REQUIRE(panel.getX() == 50);
        REQUIRE(panel.getY() == 75);
        REQUIRE(panel.getWidth() == 300);
        REQUIRE(panel.getHeight() == 250);
    }
}

TEST_CASE("Panel getComponentType returns correct type name", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    Panel panel(manager, 0, 0, 100, 100);
    REQUIRE(std::string(panel.getComponentType()) == "Panel");
}

TEST_CASE("Panel as container: addChild and getChildren", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    SECTION("Panel can contain children") {
        auto button = std::make_unique<Button>(manager, 5, 5, 80, 30, "Button");
        Button* buttonPtr = button.get();
        panelPtr->addChild(std::move(button));

        auto label = std::make_unique<Label>(manager, 5, 40, "Label");
        Label* labelPtr = label.get();
        panelPtr->addChild(std::move(label));

        const auto& children = panelPtr->getChildren();
        REQUIRE(children.size() == 2);
        REQUIRE(children[0].get() == buttonPtr);
        REQUIRE(children[1].get() == labelPtr);
    }

    SECTION("Children are positioned relative to panel") {
        auto button = std::make_unique<Button>(manager, 15, 25, 80, 30, "");
        Button* buttonPtr = button.get();
        panelPtr->addChild(std::move(button));

        REQUIRE(buttonPtr->getX() == 15);
        REQUIRE(buttonPtr->getY() == 25);
        REQUIRE(buttonPtr->getParent() == panelPtr);

        auto absPos = buttonPtr->getAbsolutePosition();
        REQUIRE(absPos.x == 10 + 15);
        REQUIRE(absPos.y == 10 + 25);
    }
}

TEST_CASE("Panel event forwarding to children", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    SECTION("Click on child button triggers callback") {
        bool clicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Child");
        button->setOnClickCallback([&](GUIElement*) { clicked = true; });
        panelPtr->addChild(std::move(button));

        int buttonCenterX = 50 + 10 + 40;
        int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        REQUIRE(clicked);
    }

    SECTION("Click on panel area (not on child) handled by panel") {
        bool buttonClicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Child");
        button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
        panelPtr->addChild(std::move(button));

        int panelAreaX = 50 + 150;
        int panelAreaY = 50 + 100;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, panelAreaX, panelAreaY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, panelAreaX, panelAreaY));
        REQUIRE_FALSE(buttonClicked);
    }

    SECTION("Events outside panel not forwarded to children") {
        bool buttonClicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Child");
        button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
        panelPtr->addChild(std::move(button));

        int outsideX = 300;
        int outsideY = 300;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, outsideX, outsideY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, outsideX, outsideY));
        REQUIRE_FALSE(buttonClicked);
    }
}

TEST_CASE("Draggable panel behavior", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setDraggable(true) enables dragging") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 120, 80);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110));
        manager.processEvent(helper.createMouseMotion(160, 170));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 160, 170));

        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 160);
    }

    SECTION("Non-draggable panel does not move on drag attempt") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 120, 80);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 110, 110));
        manager.processEvent(helper.createMouseMotion(160, 170));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 160, 170));

        REQUIRE(panelPtr->getX() == 100);
        REQUIRE(panelPtr->getY() == 100);
    }

    SECTION("Mouse down on draggable panel starts drag") {
        auto panel = std::make_unique<Panel>(manager, 50, 50, 100, 60);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 75, 75));

        manager.processEvent(helper.createMouseMotion(125, 95));

        REQUIRE(panelPtr->getX() == 100);
        REQUIRE(panelPtr->getY() == 70);
    }

    SECTION("Mouse motion while dragging moves panel") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 150, 100);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 150, 130));
        REQUIRE(panelPtr->getX() == 100);
        REQUIRE(panelPtr->getY() == 100);

        manager.processEvent(helper.createMouseMotion(200, 160));
        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 130);

        manager.processEvent(helper.createMouseMotion(250, 200));
        REQUIRE(panelPtr->getX() == 200);
        REQUIRE(panelPtr->getY() == 170);
    }

    SECTION("Mouse up ends drag") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 150, 100);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 150, 130));
        manager.processEvent(helper.createMouseMotion(200, 160));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 200, 160));

        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 130);

        manager.processEvent(helper.createMouseMotion(300, 300));
        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 130);
    }

    SECTION("Panel position updates correctly during drag") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 100);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 50));

        manager.processEvent(helper.createMouseMotion(70, 80));
        REQUIRE(panelPtr->getX() == 20);
        REQUIRE(panelPtr->getY() == 30);

        manager.processEvent(helper.createMouseMotion(100, 150));
        REQUIRE(panelPtr->getX() == 50);
        REQUIRE(panelPtr->getY() == 100);
    }

    SECTION("Children move with panel during drag") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 300, 200);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        auto button = std::make_unique<Button>(manager, 20, 30, 80, 40, "");
        Button* buttonPtr = button.get();
        panelPtr->addChild(std::move(button));

        auto absBefore = buttonPtr->getAbsolutePosition();
        REQUIRE(absBefore.x == 120);
        REQUIRE(absBefore.y == 130);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 250, 150));
        manager.processEvent(helper.createMouseMotion(300, 200));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 300, 200));

        REQUIRE(buttonPtr->getX() == 20);
        REQUIRE(buttonPtr->getY() == 30);

        auto absAfter = buttonPtr->getAbsolutePosition();
        REQUIRE(absAfter.x == 170);
        REQUIRE(absAfter.y == 180);
    }

    SECTION("Drag offset maintains relative click position") {
        auto panel = std::make_unique<Panel>(manager, 100, 100, 200, 100);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 250, 150));
        manager.processEvent(helper.createMouseMotion(300, 200));

        REQUIRE(panelPtr->getX() == 150);
        REQUIRE(panelPtr->getY() == 150);
    }

    SECTION("Click on child button does not start drag") {
        auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
        Panel* panelPtr = panel.get();
        panelPtr->setDraggable(true);
        manager.addElement(std::move(panel));

        bool buttonClicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Button");
        button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
        panelPtr->addChild(std::move(button));

        int buttonCenterX = 50 + 10 + 40;
        int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));

        REQUIRE(buttonClicked);
        REQUIRE(panelPtr->getX() == 50);
        REQUIRE(panelPtr->getY() == 50);
    }
}

TEST_CASE("Panel visibility affects children", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    bool buttonClicked = false;
    auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Button");
    button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
    panelPtr->addChild(std::move(button));

    SECTION("Hidden panel hides children from events") {
        panelPtr->setVisible(false);
        REQUIRE_FALSE(panelPtr->isVisible());

        int buttonCenterX = 50 + 10 + 40;
        int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));

        REQUIRE_FALSE(buttonClicked);
    }

    SECTION("Visible panel allows children to receive events") {
        panelPtr->setVisible(true);
        REQUIRE(panelPtr->isVisible());

        int buttonCenterX = 50 + 10 + 40;
        int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));

        REQUIRE(buttonClicked);
    }
}

TEST_CASE("Panel enabled state affects event handling", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    bool buttonClicked = false;
    auto button = std::make_unique<Button>(manager, 10, 10, 80, 40, "Button");
    button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
    panelPtr->addChild(std::move(button));

    SECTION("Enabled panel allows children to respond to events") {
        panelPtr->setEnabled(true);
        REQUIRE(panelPtr->isEnabled());

        int buttonCenterX = 50 + 10 + 40;
        int buttonCenterY = 50 + 10 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));

        REQUIRE(buttonClicked);
    }
}

TEST_CASE("Panel clip children functionality", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 50, 50, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    SECTION("setClipChildren can be called") {
        panelPtr->setClipChildren(true);
        panelPtr->setClipChildren(false);
    }
}

TEST_CASE("Panel contains method works correctly", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 100, 100, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    SECTION("Points inside panel") {
        REQUIRE(panelPtr->contains(100, 100));
        REQUIRE(panelPtr->contains(150, 150));
        REQUIRE(panelPtr->contains(299, 249));
    }

    SECTION("Points outside panel") {
        REQUIRE_FALSE(panelPtr->contains(99, 100));
        REQUIRE_FALSE(panelPtr->contains(100, 99));
        REQUIRE_FALSE(panelPtr->contains(300, 150));
        REQUIRE_FALSE(panelPtr->contains(150, 250));
    }
}

TEST_CASE("Panel with nested children", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto outerPanel = std::make_unique<Panel>(manager, 10, 10, 300, 250);
    Panel* outerPtr = outerPanel.get();
    manager.addElement(std::move(outerPanel));

    auto innerPanel = std::make_unique<Panel>(manager, 20, 20, 200, 180);
    Panel* innerPtr = innerPanel.get();
    outerPtr->addChild(std::move(innerPanel));

    bool buttonClicked = false;
    auto button = std::make_unique<Button>(manager, 30, 30, 80, 40, "");
    button->setOnClickCallback([&](GUIElement*) { buttonClicked = true; });
    innerPtr->addChild(std::move(button));

    SECTION("Deeply nested button responds to click") {
        int buttonCenterX = 10 + 20 + 30 + 40;
        int buttonCenterY = 10 + 20 + 30 + 20;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, buttonCenterX, buttonCenterY));

        REQUIRE(buttonClicked);
    }

    SECTION("Nested panel parent chain is correct") {
        REQUIRE(innerPtr->getParent() == outerPtr);
        REQUIRE(outerPtr->getParent() == nullptr);
    }

    SECTION("Nested panel absolute position is correct") {
        auto absPos = innerPtr->getAbsolutePosition();
        REQUIRE(absPos.x == 10 + 20);
        REQUIRE(absPos.y == 10 + 20);
    }
}

TEST_CASE("Panel countDescendants", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Empty panel has no descendants") {
        auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));

        REQUIRE(panelPtr->countDescendants() == 0);
    }

    SECTION("Panel with children counts correctly - Button creates internal Label") {
        auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));

        panelPtr->addChild(std::make_unique<Button>(manager, 0, 0, 50, 30, "Btn"));
        panelPtr->addChild(std::make_unique<Label>(manager, 0, 40, "Label"));

        REQUIRE(panelPtr->countDescendants() == 3);
    }

    SECTION("Panel with nested children counts recursively") {
        auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
        Panel* panelPtr = panel.get();
        manager.addElement(std::move(panel));

        auto innerPanel = std::make_unique<Panel>(manager, 0, 0, 100, 100);
        Panel* innerPtr = innerPanel.get();
        innerPtr->addChild(std::make_unique<Button>(manager, 0, 0, 50, 30, "NestedBtn"));
        panelPtr->addChild(std::move(innerPanel));
        panelPtr->addChild(std::make_unique<Label>(manager, 0, 0, "Label"));

        REQUIRE(panelPtr->countDescendants() == 4);
    }
}

TEST_CASE("Panel clearChildren removes all children", "[panel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 150);
    Panel* panelPtr = panel.get();
    manager.addElement(std::move(panel));

    panelPtr->addChild(std::make_unique<Button>(manager, 0, 0, 50, 30, "Btn"));
    panelPtr->addChild(std::make_unique<Label>(manager, 0, 40, "Label"));

    REQUIRE(panelPtr->getChildren().size() == 2);

    panelPtr->clearChildren();

    REQUIRE(panelPtr->getChildren().empty());
    REQUIRE(panelPtr->countDescendants() == 0);
}