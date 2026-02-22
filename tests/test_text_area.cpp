#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_area.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextArea basic functionality", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initialization") {
        TextArea ta(manager, 10, 20, 300, 200, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getX() == 10);
        REQUIRE(ta.getY() == 20);
        REQUIRE(ta.getWidth() == 300);
        REQUIRE(ta.getHeight() == 200);
        REQUIRE(ta.getText().empty());
    }

    SECTION("setText and getText") {
        auto ta = std::make_unique<TextArea>(manager, 10, 10, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Hello\nWorld");
        REQUIRE(area->getText() == "Hello\nWorld");

        area->setText("New text");
        REQUIRE(area->getText() == "New text");
    }

    SECTION("Multi-line text handling") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText("Line 1\nLine 2\nLine 3");
        REQUIRE(ta.getText() == "Line 1\nLine 2\nLine 3");
    }

    SECTION("Word wrap toggle") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getWordWrap() == true);
        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());
    }

    SECTION("getComponentType returns correct type name") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(std::string(ta.getComponentType()) == "TextArea");
    }
}

TEST_CASE("TextArea mouse interaction", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click inside TextArea returns handled") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Click inside the TextArea
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);

        REQUIRE(handled);
    }

    SECTION("Click outside TextArea returns not handled") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Click outside the TextArea
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 400, 400);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Click on TextArea boundary is handled") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Click exactly on the left edge
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 100);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);

        // Click exactly on the right edge
        event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 349, 100);
        handled = area->handleEvent(event);
        REQUIRE(handled);
    }
}

TEST_CASE("TextArea text editing", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Typing adds characters when focused") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate the TextArea first
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);

        // Initial render to initialize internal state (m_lines etc.)
        area->render(manager.getRenderer());

        // Type some text
        event = helper.createTextInputEvent("H");
        area->handleEvent(event);
        REQUIRE(area->getText() == "H");

        event = helper.createTextInputEvent("e");
        area->handleEvent(event);
        REQUIRE(area->getText() == "He");

        event = helper.createTextInputEvent("l");
        area->handleEvent(event);
        event = helper.createTextInputEvent("l");
        area->handleEvent(event);
        event = helper.createTextInputEvent("o");
        area->handleEvent(event);
        REQUIRE(area->getText() == "Hello");
    }

    SECTION("Backspace removes last character") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("T");
        area->handleEvent(event);
        event = helper.createTextInputEvent("e");
        area->handleEvent(event);
        event = helper.createTextInputEvent("s");
        area->handleEvent(event);
        event = helper.createTextInputEvent("t");
        area->handleEvent(event);
        REQUIRE(area->getText() == "Test");

        // Press backspace
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "Tes");

        // Press backspace again
        area->handleEvent(event);
        REQUIRE(area->getText() == "Te");
    }

    SECTION("Backspace does nothing when text is empty") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        // Press backspace on empty text
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Enter key adds new line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("1");
        area->handleEvent(event);
        REQUIRE(area->getText() == "L1");

        // Press Enter
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        area->handleEvent(event);
        REQUIRE(area->getText() == "L1\n");

        // Type on new line
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("2");
        area->handleEvent(event);
        REQUIRE(area->getText() == "L1\nL2");
    }

    SECTION("Arrow keys move cursor left and right") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);
        REQUIRE(area->getText() == "ABC");

        // Move cursor left
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);

        // Insert character in the middle (between B and C)
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText() == "ABXC");

        // Move cursor left again
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        // Now cursor should be between A and B
        event = helper.createTextInputEvent("Y");
        area->handleEvent(event);
        REQUIRE(area->getText() == "AYBXC");
    }

    SECTION("Arrow right does nothing at end of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        // Try to move right (should do nothing since cursor is at end)
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT);
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
    }

    SECTION("Arrow left does nothing at beginning of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        // Move left to beginning
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        // Try to move left again (should do nothing)
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
    }
}

TEST_CASE("TextArea scrolling", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Mouse wheel scrolls content when hovered") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 100, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Set multi-line text
        area->setText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8");

        // Activate the TextArea
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);

        // Initial render to initialize internal state
        area->render(manager.getRenderer());

        // Scroll down (wheel.y = -1 means scroll down content)
        event.type = SDL_MOUSEWHEEL;
        event.wheel.y = -1;
        event.wheel.x = 0;
        bool handled = area->handleEvent(event);
        REQUIRE(handled);

        // Scroll up
        event.wheel.y = 1;
        handled = area->handleEvent(event);
        REQUIRE(handled);
    }

    SECTION("Mouse wheel does not scroll when not hovered") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 100, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Line 1\nLine 2\nLine 3");
        area->render(manager.getRenderer());

        // Don't activate the TextArea - just try to scroll
        SDL_Event event;
        event.type = SDL_MOUSEWHEEL;
        event.wheel.y = -1;
        event.wheel.x = 0;
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
    }
}

TEST_CASE("TextArea disabled state", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled TextArea ignores events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setEnabled(false);
        REQUIRE_FALSE(area->isEnabled());

        // Try to click
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);

        // Try to type
        event = helper.createTextInputEvent("X");
        handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }
}

TEST_CASE("TextArea visibility", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden TextArea ignores events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setVisible(false);
        REQUIRE_FALSE(area->isVisible());

        // Try to click
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
    }
}

TEST_CASE("TextArea setText variants", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setText with string_view") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText(std::string_view("View text"));
        REQUIRE(ta.getText() == "View text");
    }

    SECTION("setText with rvalue string") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        std::string movable = "Movable text";
        ta.setText(std::move(movable));
        REQUIRE(ta.getText() == "Movable text");
    }

    SECTION("setText with const char*") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText("C-string text");
        REQUIRE(ta.getText() == "C-string text");
    }

    SECTION("setText truncates cursor position if needed") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and type
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("o");
        area->handleEvent(event);
        event = helper.createTextInputEvent("n");
        area->handleEvent(event);
        event = helper.createTextInputEvent("g");
        area->handleEvent(event);
        REQUIRE(area->getText() == "Long");

        // Set shorter text - cursor should be clamped
        area->setText("Hi");
        REQUIRE(area->getText() == "Hi");

        // Backspace should work correctly
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "H");
    }
}

TEST_CASE("TextArea multi-line editing", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Cursor navigation across lines") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and create multi-line text
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nB");

        // Move cursor left to go from after B to before B
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        // Insert X between newline and B
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText() == "A\nXB");
    }

    SECTION("Backspace removes newline character") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Activate and create multi-line text
        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nB");

        // Backspace to remove B
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "A\n");

        // Backspace again to remove newline
        area->handleEvent(event);
        REQUIRE(area->getText() == "A");
    }
}

TEST_CASE("TextArea word wrap", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Word wrap enabled by default") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getWordWrap());
    }

    SECTION("Word wrap can be disabled") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());
    }

    SECTION("Word wrap can be toggled multiple times") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getWordWrap());

        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());

        ta.setWordWrap(true);
        REQUIRE(ta.getWordWrap());

        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());
    }
}

TEST_CASE("TextArea text input without focus", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Text input ignored when not focused") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Don't activate - just try to type
        SDL_Event event = helper.createTextInputEvent("X");
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Key events ignored when not focused") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        // Don't activate - just try key events
        SDL_Event event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        bool handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        handled = area->handleEvent(event);
        REQUIRE_FALSE(handled);
    }
}
