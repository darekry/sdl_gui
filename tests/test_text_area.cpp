#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_area.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextArea Focus Behavior", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click inside gives keyboard focus") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);

        REQUIRE(handled);
    }

    SECTION("Click outside removes focus") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);

        event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 400, 400);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Text input only works when focused") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createTextInputEvent("X");
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());

        SDL_Event clickEvent = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(clickEvent);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("Y");
        handled = area->handleEvent(event);

        REQUIRE(handled);
        REQUIRE(area->getText() == "Y");
    }

    SECTION("Click on boundary is handled") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 50, 100);
        REQUIRE(area->handleEvent(event));

        event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 349, 100);
        REQUIRE(area->handleEvent(event));
    }
}

TEST_CASE("TextArea Multi-line Text", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setText with newlines creates multiple lines") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText("Line 1\nLine 2\nLine 3");
        REQUIRE(ta.getText() == "Line 1\nLine 2\nLine 3");
    }

    SECTION("getText returns full text including newlines") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setText("First\nSecond\nThird\nFourth");
        REQUIRE(ta.getText() == "First\nSecond\nThird\nFourth");
    }

    SECTION("Pressing Enter adds newline at cursor position") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

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
    }

    SECTION("Backspace at newline boundary removes newline") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

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

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "A\n");

        area->handleEvent(event);
        REQUIRE(area->getText() == "A");
    }

    SECTION("Multiple newlines handled correctly") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (int i = 0; i < 3; ++i) {
            event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
            area->handleEvent(event);
        }

        REQUIRE(area->getText() == "\n\n\n");
    }
}

TEST_CASE("TextArea Cursor Navigation", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Arrow left moves cursor within line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

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

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "ABXC");
    }

    SECTION("Arrow right moves cursor forward") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "AXB");
    }

    SECTION("Arrow left does nothing at beginning of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Arrow right does nothing at end of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Cursor navigation across lines with arrow keys") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

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

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nXB");
    }

    SECTION("Cursor position persists after text change via setText") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("H");
        area->handleEvent(event);
        event = helper.createTextInputEvent("e");
        area->handleEvent(event);
        event = helper.createTextInputEvent("l");
        area->handleEvent(event);
        event = helper.createTextInputEvent("l");
        area->handleEvent(event);
        event = helper.createTextInputEvent("o");
        area->handleEvent(event);

        area->setText("Hi");

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);

        REQUIRE(area->getText() == "H");
    }
}

TEST_CASE("TextArea Text Editing", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Typing inserts at cursor position") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("H");
        area->handleEvent(event);
        event = helper.createTextInputEvent("i");
        area->handleEvent(event);

        REQUIRE(area->getText() == "Hi");
    }

    SECTION("Backspace removes character before cursor") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

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

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "Tes");

        area->handleEvent(event);
        REQUIRE(area->getText() == "Te");
    }

    SECTION("Backspace does nothing when text is empty") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Insert text in middle of existing text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "ABC");
    }

    SECTION("setText variants work correctly") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);

        ta.setText(std::string_view("View text"));
        REQUIRE(ta.getText() == "View text");

        std::string movable = "Movable text";
        ta.setText(std::move(movable));
        REQUIRE(ta.getText() == "Movable text");

        ta.setText("C-string text");
        REQUIRE(ta.getText() == "C-string text");
    }
}

TEST_CASE("TextArea Word Wrap", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Word wrap is enabled by default") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getWordWrap() == true);
    }

    SECTION("setWordWrap(false) disables word wrap") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setWordWrap(false);
        REQUIRE_FALSE(ta.getWordWrap());
    }

    SECTION("setWordWrap(true) enables word wrap") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        ta.setWordWrap(false);
        ta.setWordWrap(true);
        REQUIRE(ta.getWordWrap());
    }

    SECTION("getWordWrap returns current setting") {
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

TEST_CASE("TextArea Scrolling", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Mouse wheel scrolls content when hovered") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 100, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Line 1\nLine 2\nLine 3\nLine 4\nLine 5\nLine 6\nLine 7\nLine 8");

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createMouseWheel(-1, 0);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);

        event = helper.createMouseWheel(1, 0);
        handled = area->handleEvent(event);
        REQUIRE(handled);
    }

    SECTION("Mouse wheel does not scroll when not focused") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 100, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Line 1\nLine 2\nLine 3");
        area->render(manager.getRenderer());

        SDL_Event event = helper.createMouseWheel(-1, 0);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Mouse wheel handles zero scroll") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 100, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setText("Line 1\nLine 2\nLine 3");

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createMouseWheel(0, 0);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);
    }
}

TEST_CASE("TextArea Disabled State", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled TextArea ignores click events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setEnabled(false);

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Disabled TextArea ignores text input") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setEnabled(false);

        SDL_Event event = helper.createTextInputEvent("X");
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Disabled TextArea ignores keyboard events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setEnabled(false);

        SDL_Event event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        REQUIRE_FALSE(area->handleEvent(event));
    }

    SECTION("Disabled TextArea ignores mouse wheel") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setEnabled(false);

        SDL_Event event = helper.createMouseWheel(-1, 0);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("setEnabled returns correct state") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.isEnabled());
        ta.setEnabled(false);
        REQUIRE_FALSE(ta.isEnabled());
        ta.setEnabled(true);
        REQUIRE(ta.isEnabled());
    }
}

TEST_CASE("TextArea Hidden State", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Hidden TextArea ignores click events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setVisible(false);

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Hidden TextArea ignores text input") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setVisible(false);

        SDL_Event event = helper.createTextInputEvent("X");
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Hidden TextArea ignores keyboard events") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->setVisible(false);

        SDL_Event event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        REQUIRE_FALSE(area->handleEvent(event));
    }

    SECTION("setVisible returns correct state") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.isVisible());
        ta.setVisible(false);
        REQUIRE_FALSE(ta.isVisible());
        ta.setVisible(true);
        REQUIRE(ta.isVisible());
    }
}

TEST_CASE("TextArea Component Type", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getComponentType returns TextArea") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(std::string(ta.getComponentType()) == "TextArea");
    }
}

TEST_CASE("TextArea Large Text", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Can handle large amounts of text") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);

        std::string largeText;
        for (int i = 0; i < 1000; ++i) {
            largeText += "Line " + std::to_string(i) + "\n";
        }
        largeText.pop_back();

        ta.setText(largeText);
        REQUIRE(ta.getText() == largeText);
    }

    SECTION("Long lines handled correctly") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);

        std::string longLine(500, 'X');
        ta.setText(longLine);

        REQUIRE(ta.getText() == longLine);
    }

    SECTION("Large text with many newlines") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (int i = 0; i < 50; ++i) {
            event = helper.createTextInputEvent("L");
            area->handleEvent(event);
            event = helper.createTextInputEvent("i");
            area->handleEvent(event);
            event = helper.createTextInputEvent("n");
            area->handleEvent(event);
            event = helper.createTextInputEvent("e");
            area->handleEvent(event);
            event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
            area->handleEvent(event);
        }

        REQUIRE_FALSE(area->getText().empty());
    }
}

TEST_CASE("TextArea OnTextChanged Callback", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Callback is invoked when text changes via input") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        int callbackCount = 0;
        TextArea* callbackSource = nullptr;

        area->setOnTextChanged([&](TextArea* source) {
            callbackCount++;
            callbackSource = source;
        });

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(callbackCount == 1);
        REQUIRE(callbackSource == area);
    }

    SECTION("Callback is invoked when text changes via setText") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);

        int callbackCount = 0;
        ta.setOnTextChanged([&](TextArea*) { callbackCount++; });

        ta.setText("New text");
        REQUIRE(callbackCount == 1);

        ta.setText("Another text");
        REQUIRE(callbackCount == 2);
    }

    SECTION("Callback is invoked on backspace") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        int callbackCount = 0;
        area->setOnTextChanged([&](TextArea*) { callbackCount++; });

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(callbackCount == 1);

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(callbackCount == 2);
    }

    SECTION("Callback is invoked on Enter key") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        int callbackCount = 0;
        area->setOnTextChanged([&](TextArea*) { callbackCount++; });

        SDL_Event event = helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        area->handleEvent(event);
        REQUIRE(callbackCount == 1);
    }
}

TEST_CASE("TextArea Initialization", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor sets position and size") {
        TextArea ta(manager, 10, 20, 300, 200, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getX() == 10);
        REQUIRE(ta.getY() == 20);
        REQUIRE(ta.getWidth() == 300);
        REQUIRE(ta.getHeight() == 200);
    }

    SECTION("Initial text is empty") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.getText().empty());
    }

    SECTION("Initial enabled state is true") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.isEnabled());
    }

    SECTION("Initial visible state is true") {
        TextArea ta(manager, 0, 0, 200, 100, "assets/fonts/font.ttf", 14);
        REQUIRE(ta.isVisible());
    }
}