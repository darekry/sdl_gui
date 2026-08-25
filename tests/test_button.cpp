#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("Button mouse hover behavior", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
    Button* btn = button.get();
    manager.addElement(std::move(button));

    SECTION("Mouse enters button bounds -> state changes to Hover") {
        REQUIRE(btn->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Mouse leaves button bounds -> state changes to Normal") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseMotion(5, 5));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Mouse motion outside button does not affect state") {
        REQUIRE(btn->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(200, 200));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Mouse on button edge still triggers hover") {
        manager.processEvent(helper.createMouseMotion(10, 10));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseMotion(109, 49));
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Mouse just outside button edge does not trigger hover") {
        manager.processEvent(helper.createMouseMotion(9, 10));
        REQUIRE(btn->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(110, 50));
        REQUIRE(btn->getState() == ElementState::Normal);
    }
}

TEST_CASE("Button click behavior - complete cycle", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Click");
    Button* btn = button.get();
    int clickCount = 0;
    GUIElement* clickedElement = nullptr;
    button->setOnClickCallback([&](GUIElement* elem) {
        ++clickCount;
        clickedElement = elem;
    });
    manager.addElement(std::move(button));

    SECTION("Mouse down inside button -> state becomes Pressed") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(btn->getState() == ElementState::Pressed);
        REQUIRE(clickCount == 0);
    }

    SECTION("Mouse up inside button after press -> callback fires, state = Hover") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Click callback receives correct pointer") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickedElement == btn);
    }

    SECTION("Multiple clicks increment counter") {
        for (int i = 0; i < 3; ++i) {
            manager.processEvent(helper.createMouseMotion(20, 20));
            manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
            manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        }

        REQUIRE(clickCount == 3);
    }

    SECTION("Click without prior hover still works") {
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
    }
}

TEST_CASE("Button cancel behavior", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Cancel");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("Press inside, move outside, release outside -> no onClick callback") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseMotion(200, 200));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 200, 200));
        REQUIRE(clickCount == 0);
        // BUG FIX: State should be Normal after releasing outside, not Hover or Pressed
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Press inside, release outside without motion -> no callback, state Normal") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 200, 200));

        REQUIRE(clickCount == 0);
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Press outside, release inside -> no callback") {
        manager.processEvent(helper.createMouseMotion(200, 200));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 200, 200));
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 0);
        REQUIRE(btn->getState() == ElementState::Hover);
    }
}

TEST_CASE("Button disabled state", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Disabled");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("Disabled button ignores mouse motion (state unchanged)") {
        REQUIRE(btn->getState() == ElementState::Normal);
        btn->setEnabled(false);
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Disabled button ignores click") {
        btn->setEnabled(false);
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 0);
    }

    SECTION("setEnabled(true) allows interactions again") {
        btn->setEnabled(false);
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Normal);

        btn->setEnabled(true);
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
    }

    SECTION("isEnabled() returns correct value") {
        REQUIRE(btn->isEnabled() == true);
        btn->setEnabled(false);
        REQUIRE(btn->isEnabled() == false);
        btn->setEnabled(true);
        REQUIRE(btn->isEnabled() == true);
    }
}

TEST_CASE("Button visibility", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Hidden");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("Hidden button does not respond to hover") {
        btn->setVisible(false);
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Normal);
    }

    SECTION("Hidden button does not respond to click") {
        btn->setVisible(false);
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 0);
    }

    SECTION("Visible button responds to events") {
        btn->setVisible(true);

        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("isVisible() returns correct value") {
        REQUIRE(btn->isVisible() == true);
        btn->setVisible(false);
        REQUIRE(btn->isVisible() == false);
        btn->setVisible(true);
        REQUIRE(btn->isVisible() == true);
    }
}

TEST_CASE("Multiple buttons interaction", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Button1");
    Button* btn1 = button1.get();
    int clicks1 = 0;
    button1->setOnClickCallback([&](GUIElement*) { ++clicks1; });

    auto button2 = std::make_unique<Button>(manager, 120, 10, 100, 40, "Button2");
    Button* btn2 = button2.get();
    int clicks2 = 0;
    button2->setOnClickCallback([&](GUIElement*) { ++clicks2; });

    manager.addElement(std::move(button1));
    manager.addElement(std::move(button2));

    SECTION("Each button can be clicked independently") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clicks1 == 1);
        REQUIRE(clicks2 == 0);

        manager.processEvent(helper.createMouseMotion(130, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 130, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 130, 20));

        REQUIRE(clicks1 == 1);
        REQUIRE(clicks2 == 1);
    }

    SECTION("Hover state of one button does not affect others") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn1->getState() == ElementState::Hover);
        REQUIRE(btn2->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(130, 20));
        REQUIRE(btn1->getState() == ElementState::Normal);
        REQUIRE(btn2->getState() == ElementState::Hover);
    }

    SECTION("Pressing one button does not affect others") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(btn1->getState() == ElementState::Pressed);
        REQUIRE(btn2->getState() == ElementState::Normal);
    }
}

TEST_CASE("Button label text", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Button created with label text") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "My Label");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        REQUIRE(btn->getWidth() == 100);
        REQUIRE(btn->getHeight() == 40);
    }

    SECTION("Button created with empty label") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        REQUIRE(btn->getWidth() == 100);
        REQUIRE(btn->getHeight() == 40);
    }

    SECTION("Button created without label parameter (default)") {
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 40);
        Button* btn = button.get();
        manager.addElement(std::move(button));

        REQUIRE(btn->getWidth() == 100);
        REQUIRE(btn->getHeight() == 40);
    }

    SECTION("label re-centers when size changes after creation") {
        auto button = std::make_unique<Button>(manager, 0, 0, 10, 10, "ABC");
        Button* btn = button.get();
        manager.addElement(std::move(button));

        btn->setSize(200, 60);

        REQUIRE(btn->getChildren().size() == 1);
        GUIElement* label = btn->getChildren()[0].get();
        int labelWidth = 0, labelHeight = 0;
        label->getSize(labelWidth, labelHeight);
        REQUIRE(label->getX() == (200 - labelWidth) / 2);
        REQUIRE(label->getY() == (60 - labelHeight) / 2);
    }
}

TEST_CASE("Button callback types", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Callbacks");
    Button* btn = button.get();
    manager.addElement(std::move(button));

    SECTION("setOnClickCallback works correctly") {
        int clickCount = 0;
        btn->setOnClickCallback([&](GUIElement*) { ++clickCount; });

        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
    }

    SECTION("Callback can be changed at runtime") {
        int counter = 0;
        btn->setOnClickCallback([&](GUIElement*) { counter += 10; });

        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(counter == 10);

        btn->setOnClickCallback([&](GUIElement*) { counter += 100; });

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(counter == 110);
    }

    SECTION("setOnMouseOverCallback can be set (setter works)") {
        int hoverCount = 0;
        btn->setOnMouseOverCallback([&](GUIElement*) { ++hoverCount; });
        REQUIRE(btn->isEnabled() == true);
    }
}

TEST_CASE("Button right-click does not trigger onClick", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "RightClick");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("Right-click does not trigger onClick callback") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_RIGHT, 20, 20));

        REQUIRE(clickCount == 0);
        REQUIRE(btn->getState() == ElementState::Hover);
    }

    SECTION("Middle-click does not trigger onClick callback") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_MIDDLE, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_MIDDLE, 20, 20));

        REQUIRE(clickCount == 0);
        REQUIRE(btn->getState() == ElementState::Hover);
    }
}

TEST_CASE("Button rapid click sequence", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Rapid");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("Rapid double-click triggers two callbacks") {
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 2);
    }

    SECTION("Press twice without release between -> only one click") {
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
    }

    SECTION("State transitions through complete click cycle") {
        REQUIRE(btn->getState() == ElementState::Normal);

        manager.processEvent(helper.createMouseMotion(20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Hover);
        REQUIRE(clickCount == 1);
    }
}

TEST_CASE("Button state remains pressed during drag outside", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button = std::make_unique<Button>(manager, 10, 10, 100, 40, "Drag");
    Button* btn = button.get();
    int clickCount = 0;
    button->setOnClickCallback([&](GUIElement*) { ++clickCount; });
    manager.addElement(std::move(button));

    SECTION("State stays Pressed while dragging outside") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseMotion(200, 200));
        REQUIRE(btn->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseMotion(150, 50));
        REQUIRE(btn->getState() == ElementState::Pressed);
    }

    SECTION("Release inside after drag outside fires callback") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        manager.processEvent(helper.createMouseMotion(200, 200));
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 20, 20));

        REQUIRE(clickCount == 1);
        REQUIRE(btn->getState() == ElementState::Hover);
    }
}

TEST_CASE("Button mouse capture behavior", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto button1 = std::make_unique<Button>(manager, 10, 10, 100, 40, "Btn1");
    Button* btn1 = button1.get();
    int clicks1 = 0;
    button1->setOnClickCallback([&](GUIElement*) { ++clicks1; });

    auto button2 = std::make_unique<Button>(manager, 120, 10, 100, 40, "Btn2");
    Button* btn2 = button2.get();
    int clicks2 = 0;
    button2->setOnClickCallback([&](GUIElement*) { ++clicks2; });

    manager.addElement(std::move(button1));
    manager.addElement(std::move(button2));

    SECTION("Pressing btn1 and moving to btn2 area doesn't affect btn2") {
        manager.processEvent(helper.createMouseMotion(20, 20));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 20, 20));
        REQUIRE(btn1->getState() == ElementState::Pressed);

        manager.processEvent(helper.createMouseMotion(130, 20));
        REQUIRE(btn1->getState() == ElementState::Pressed);
        REQUIRE(btn2->getState() == ElementState::Normal);
    }
}