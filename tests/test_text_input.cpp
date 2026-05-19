#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_input.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextInput - Initial State", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("New TextInput has empty text") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE(ti.getText().empty());
    }

    SECTION("New TextInput has no keyboard focus") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE_FALSE(ti.hasKeyboardFocus());
    }

    SECTION("New TextInput is not locked") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE_FALSE(ti.isLocked());
    }

    SECTION("New TextInput is enabled and visible") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE(ti.isEnabled());
        REQUIRE(ti.isVisible());
    }

    SECTION("getComponentType returns TextInput") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE(std::string(ti.getComponentType()) == "TextInput");
    }
}

TEST_CASE("TextInput - Focus Behavior", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click inside text input gives it keyboard focus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        REQUIRE_FALSE(input->hasKeyboardFocus());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE(input->hasKeyboardFocus());
    }

    SECTION("Click outside removes focus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 300, 300));

        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("hasKeyboardFocus returns correct state") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        REQUIRE_FALSE(input->hasKeyboardFocus());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.setKeyboardFocus(nullptr);
        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("Focus gained through manager.setKeyboardFocus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.setKeyboardFocus(input);

        REQUIRE(input->hasKeyboardFocus());
        REQUIRE(manager.getKeyboardFocus() == input);
    }

    SECTION("Focus lost through manager.setKeyboardFocus(nullptr)") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.setKeyboardFocus(input);
        REQUIRE(input->hasKeyboardFocus());

        manager.setKeyboardFocus(nullptr);

        REQUIRE_FALSE(input->hasKeyboardFocus());
        REQUIRE(manager.getKeyboardFocus() == nullptr);
    }
}

TEST_CASE("TextInput - Text Editing", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Typing characters while focused appends to text") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        manager.processEvent(helper.createTextInputEvent("H"));
        manager.processEvent(helper.createTextInputEvent("i"));
        manager.processEvent(helper.createTextInputEvent("!"));

        REQUIRE(input->getText() == "Hi!");
    }

    SECTION("getText returns current text content") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setText(std::string("Test Content"));
        REQUIRE(input->getText() == "Test Content");
    }

    SECTION("setText changes text and fires onTextChanged callback") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        bool callbackFired = false;
        std::string newText;
        input->setOnTextChanged([&](TextInput* ti) {
            callbackFired = true;
            newText = ti->getText();
        });

        input->setText(std::string("Hello"));

        REQUIRE(callbackFired);
        REQUIRE(newText == "Hello");
    }

    SECTION("setText with same text does not fire callback") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setText(std::string("Same"));

        int callbackCount = 0;
        input->setOnTextChanged([&](TextInput*) { callbackCount++; });

        input->setText(std::string("Same"));

        REQUIRE(callbackCount == 0);
    }

    SECTION("setText with empty string clears text") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setText(std::string("Content"));
        REQUIRE(input->getText() == "Content");

        input->setText(std::string(""));
        REQUIRE(input->getText().empty());
    }

    SECTION("Backspace removes last character when cursor at end") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));
        manager.processEvent(helper.createTextInputEvent("C"));

        REQUIRE(input->getText() == "ABC");

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(input->getText() == "AB");

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(input->getText() == "A");
    }

    SECTION("Backspace does nothing when text is empty") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));

        REQUIRE(input->getText().empty());
    }

    SECTION("Backspace fires onTextChanged callback") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("X"));

        bool callbackFired = false;
        input->setOnTextChanged([&](TextInput*) { callbackFired = true; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));

        REQUIRE(callbackFired);
    }

    SECTION("Typing fires onTextChanged callback") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        int callbackCount = 0;
        input->setOnTextChanged([&](TextInput*) { callbackCount++; });

        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));

        REQUIRE(callbackCount == 2);
    }

    SECTION("Typing ignored when not focused") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText().empty());
    }
}

TEST_CASE("TextInput - Locked State", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setLocked true prevents focus gain on click") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setLocked(true);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("setLocked false allows focus gain on click") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setLocked(true);
        input->setLocked(false);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE(input->hasKeyboardFocus());
    }

    SECTION("isLocked returns correct state") {
        TextInput ti(manager, 10, 20, 200, 30);

        REQUIRE_FALSE(ti.isLocked());

        ti.setLocked(true);
        REQUIRE(ti.isLocked());

        ti.setLocked(false);
        REQUIRE_FALSE(ti.isLocked());
    }

    SECTION("Locked input ignores typing") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        input->setLocked(true);

        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText().empty());
    }

    SECTION("Locked input ignores backspace") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setText(std::string("Test"));
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        input->setLocked(true);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));

        REQUIRE(input->getText() == "Test");
    }

    SECTION("Locked input ignores arrow keys") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setText(std::string("Test"));
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        input->setLocked(true);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT));

        REQUIRE(input->getText() == "Test");
    }

    SECTION("setLocked true removes existing focus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        input->setLocked(true);

        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("Unlocked input allows editing after unlock") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setLocked(true);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE_FALSE(input->hasKeyboardFocus());

        input->setLocked(false);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.processEvent(helper.createTextInputEvent("X"));
        REQUIRE(input->getText() == "X");
    }
}

TEST_CASE("TextInput - Enter Key", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Pressing Enter fires onEnterPressed callback") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        bool enterPressed = false;
        input->setOnEnterPressed([&](TextInput*) { enterPressed = true; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(enterPressed);
    }

    SECTION("Enter callback receives TextInput pointer") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        TextInput* callbackSource = nullptr;
        input->setOnEnterPressed([&](TextInput* source) { callbackSource = source; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(callbackSource == input);
    }

    SECTION("Enter does not fire callback when not focused") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        bool enterPressed = false;
        input->setOnEnterPressed([&](TextInput*) { enterPressed = true; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE_FALSE(enterPressed);
    }

    SECTION("Enter callback can remove focus programmatically") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        input->setOnEnterPressed([&](TextInput*) { manager.setKeyboardFocus(nullptr); });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE_FALSE(input->hasKeyboardFocus());
    }
}

TEST_CASE("TextInput - Cursor Position", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Arrow left moves cursor left") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));
        manager.processEvent(helper.createTextInputEvent("C"));

        REQUIRE(input->getText() == "ABC");

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText() == "ABXC");
    }

    SECTION("Arrow right moves cursor right") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT));
        manager.processEvent(helper.createTextInputEvent("C"));

        REQUIRE(input->getText() == "ABC");
    }

    SECTION("Arrow left at beginning does nothing") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("X"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createTextInputEvent("Y"));

        REQUIRE(input->getText() == "YX");
    }

    SECTION("Arrow right at end does nothing") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("X"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT));
        manager.processEvent(helper.createTextInputEvent("Y"));

        REQUIRE(input->getText() == "XY");
    }

    SECTION("Backspace removes character before cursor") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));
        manager.processEvent(helper.createTextInputEvent("C"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));

        REQUIRE(input->getText() == "AC");
    }

    SECTION("Cursor position affects where characters are inserted") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("1"));
        manager.processEvent(helper.createTextInputEvent("2"));
        manager.processEvent(helper.createTextInputEvent("3"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText() == "1X23");
    }

    SECTION("setText adjusts cursor position if text becomes shorter") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("A"));
        manager.processEvent(helper.createTextInputEvent("B"));
        manager.processEvent(helper.createTextInputEvent("C"));
        manager.processEvent(helper.createTextInputEvent("D"));

        input->setText(std::string("XY"));

        manager.processEvent(helper.createTextInputEvent("Z"));
        REQUIRE(input->getText() == "XYZ");
    }
}

TEST_CASE("TextInput - Disabled State", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled TextInput ignores click") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setEnabled(false);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("Disabled TextInput ignores typing") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        input->setEnabled(false);

        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText().empty());
    }

    SECTION("Disabled TextInput ignores backspace") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setText(std::string("Test"));
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        input->setEnabled(false);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));

        REQUIRE(input->getText() == "Test");
    }

    SECTION("Disabled TextInput ignores arrow keys and maintains cursor") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("Test"));

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));

        input->setEnabled(false);
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        input->setEnabled(true);

        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText() == "TeXst");
    }

    SECTION("Re-enabling allows interaction") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setEnabled(false);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE_FALSE(input->hasKeyboardFocus());

        input->setEnabled(true);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.processEvent(helper.createTextInputEvent("X"));
        REQUIRE(input->getText() == "X");
    }
}

TEST_CASE("TextInput - Hidden State", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden TextInput still receives focus (visibility not checked in handleEvent)") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setVisible(false);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE(input->hasKeyboardFocus());
    }

    SECTION("Hidden TextInput can receive text input when focused") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        input->setVisible(false);
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(input->getText() == "X");
    }
}

TEST_CASE("TextInput - Multiple TextInput", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Multiple text inputs work independently") {
        auto ti1 = std::make_unique<TextInput>(manager, 10, 10, 100, 30);
        TextInput* input1 = ti1.get();
        manager.addElement(std::move(ti1));

        auto ti2 = std::make_unique<TextInput>(manager, 120, 10, 100, 30);
        TextInput* input2 = ti2.get();
        manager.addElement(std::move(ti2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        manager.processEvent(helper.createTextInputEvent("A"));

        REQUIRE(input1->getText() == "A");
        REQUIRE(input2->getText().empty());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 125, 15));
        manager.processEvent(helper.createTextInputEvent("B"));

        REQUIRE(input1->getText() == "A");
        REQUIRE(input2->getText() == "B");
    }

    SECTION("Focus on one removes focus from another") {
        auto ti1 = std::make_unique<TextInput>(manager, 10, 10, 100, 30);
        TextInput* input1 = ti1.get();
        manager.addElement(std::move(ti1));

        auto ti2 = std::make_unique<TextInput>(manager, 120, 10, 100, 30);
        TextInput* input2 = ti2.get();
        manager.addElement(std::move(ti2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));
        REQUIRE(input1->hasKeyboardFocus());
        REQUIRE_FALSE(input2->hasKeyboardFocus());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 125, 15));
        REQUIRE_FALSE(input1->hasKeyboardFocus());
        REQUIRE(input2->hasKeyboardFocus());
    }

    SECTION("Keyboard events go only to focused input") {
        auto ti1 = std::make_unique<TextInput>(manager, 10, 10, 100, 30);
        TextInput* input1 = ti1.get();
        manager.addElement(std::move(ti1));

        auto ti2 = std::make_unique<TextInput>(manager, 120, 10, 100, 30);
        TextInput* input2 = ti2.get();
        manager.addElement(std::move(ti2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 15));

        manager.processEvent(helper.createTextInputEvent("1"));
        manager.processEvent(helper.createTextInputEvent("2"));

        REQUIRE(input1->getText() == "12");
        REQUIRE(input2->getText().empty());
    }
}

TEST_CASE("TextInput - Unicode Text", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("TextInput handles UTF-8 characters") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        manager.processEvent(helper.createTextInputEvent("ą"));
        manager.processEvent(helper.createTextInputEvent("ę"));
        manager.processEvent(helper.createTextInputEvent("ź"));

        REQUIRE(input->getText() == "ąęź");
    }

    SECTION("TextInput handles emoji") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        manager.processEvent(helper.createTextInputEvent("😀"));

        REQUIRE(input->getText() == "😀");
    }
}

TEST_CASE("TextInput - setText Overloads", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setText with string_view") {
        TextInput ti(manager, 10, 20, 200, 30);
        std::string_view sv = "string_view test";
        ti.setText(sv);
        REQUIRE(ti.getText() == "string_view test");
    }

    SECTION("setText with string rvalue") {
        TextInput ti(manager, 10, 20, 200, 30);
        ti.setText(std::string("rvalue test"));
        REQUIRE(ti.getText() == "rvalue test");
    }
}