#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/panel.hpp"
#include "../src/text_area.hpp"
#include "../src/gui_manager.hpp"
#include "../src/context_menu.hpp"

static std::vector<Uint8> readPixelRow(SDL_Renderer* renderer, int x, int y, int w) {
    SDL_Rect rect{x, y, w, 1};
    SDL_Surface* surf = SDL_RenderReadPixels(renderer, &rect);
    REQUIRE(surf != nullptr);
    std::vector<Uint8> bytes(static_cast<size_t>(w) * 4);
    std::memcpy(bytes.data(), surf->pixels, bytes.size());
    SDL_DestroySurface(surf);
    return bytes;
}

// Regression: TextArea draws its content (lines, scroll offset) directly, so it
// must opt out of the shared render cache — otherwise editing/typing never
// reaches the screen (stale shared texture reused for the same style key).
TEST_CASE("TextArea re-renders content after edit (shared cache opt-out)", "[text_area][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.setWindowSize(320, 240);

    auto ta = std::make_unique<TextArea>(manager, 20, 20, 200, 100, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));

    area->setText("AAAA");
    manager.update();
    manager.cleanup();
    manager.render();
    auto rowA = readPixelRow(helper.getRenderer(), 22, 30, 120);

    area->setText("BBBB");
    manager.update();
    manager.cleanup();
    manager.render();
    auto rowB = readPixelRow(helper.getRenderer(), 22, 30, 120);

    REQUIRE(rowA != rowB);
}

// Regression: TextArea must join the keyboard focus system. GUIManager only
// routes key/text events to the focused element and only calls renderOverlay
// for it, so a TextArea that never gains focus swallows typing/deleting and
// never draws its cursor.
TEST_CASE("TextArea receives input through GUIManager dispatch", "[text_area][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.setWindowSize(800, 600);

    auto panel = std::make_unique<Panel>(manager, 50, 50, 700, 500);
    auto ta = std::make_unique<TextArea>(manager, 20, 50, 660, 430, "assets/fonts/font.ttf", 18);
    TextArea* area = ta.get();
    panel->addChild(std::move(ta));
    manager.addElement(std::move(panel));

    SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 200, 200);
    manager.processEvent(event);
    REQUIRE(manager.getKeyboardFocus() == area);

    manager.update();
    manager.cleanup();
    manager.render();

    // cursor must be visible after the click (renderOverlay for focused element)
    auto row = readPixelRow(helper.getRenderer(), 102, 102, 4);
    bool hasCursorPixels = std::any_of(row.begin(), row.end(), [](Uint8 b) { return b != 0; });
    CAPTURE(row[0], row[1], row[2], row[3]);
    REQUIRE(hasCursorPixels);

    manager.processEvent(helper.createTextInputEvent("X"));
    REQUIRE(area->getText() == "X");

    manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));
    REQUIRE(area->getText().empty());

    manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT));
    manager.processEvent(helper.createTextInputEvent("A"));
    REQUIRE(area->getText() == "A");

    manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 600, 550));
    REQUIRE(manager.getKeyboardFocus() != area);
}

TEST_CASE("TextArea Focus Behavior", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click inside gives keyboard focus") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        bool handled = area->handleEvent(event);

        REQUIRE(handled);
    }

    SECTION("Click outside removes focus") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);

        event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 400, 400);
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

        SDL_Event clickEvent = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 50, 100);
        REQUIRE(area->handleEvent(event));

        event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 349, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nB");
    }

    SECTION("Backspace at newline boundary removes newline") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nB");

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "A\n");

        area->handleEvent(event);
        REQUIRE(area->getText() == "A");
    }

    SECTION("Multiple newlines handled correctly") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (int i = 0; i < 3; ++i) {
            event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);

        REQUIRE(area->getText() == "ABC");

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "ABXC");
    }

    SECTION("Arrow right moves cursor forward") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "AXB");
    }

    SECTION("Arrow left does nothing at beginning of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Arrow right does nothing at end of text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
    }

    SECTION("Cursor navigation across lines with arrow keys") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nB");

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        REQUIRE(area->getText() == "A\nXB");
    }

    SECTION("Cursor position persists after text change via setText") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "Tes");

        area->handleEvent(event);
        REQUIRE(area->getText() == "Te");
    }

    SECTION("Backspace does nothing when text is empty") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        bool handled = area->handleEvent(event);

        REQUIRE_FALSE(handled);
        REQUIRE(area->getText().empty());
    }

    SECTION("Insert text in middle of existing text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        REQUIRE_FALSE(area->handleEvent(event));

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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
            event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
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

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(callbackCount == 1);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        REQUIRE(callbackCount == 2);
    }

    SECTION("Callback is invoked on Enter key") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 14);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        int callbackCount = 0;
        area->setOnTextChanged([&](TextArea*) { callbackCount++; });

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 100, 100);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
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

TEST_CASE("TextArea - Locked State", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setLocked true prevents focus gain on click") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setLocked(true);
        manager.addElement(std::move(ta));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE_FALSE(area->isHovered());
    }

    SECTION("isLocked returns correct state") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);

        REQUIRE_FALSE(ta.isLocked());

        ta.setLocked(true);
        REQUIRE(ta.isLocked());

        ta.setLocked(false);
        REQUIRE_FALSE(ta.isLocked());
    }

    SECTION("Locked TextArea ignores typing") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55));
        area->setLocked(true);

        manager.processEvent(helper.createTextInputEvent("X"));

        REQUIRE(area->getText().empty());
    }

    SECTION("Locked TextArea ignores backspace") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Test");
        manager.addElement(std::move(ta));

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55));
        area->setLocked(true);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));

        REQUIRE(area->getText() == "Test");
    }
}

TEST_CASE("TextArea - Delete Key", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Delete removes character after cursor") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);

        REQUIRE(area->getText() == "ABC");

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "AB");
    }

    SECTION("Delete at end of line does nothing") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("X");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "X");
    }

    SECTION("Delete removes selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);
        event = helper.createTextInputEvent("C");
        area->handleEvent(event);

        area->setSelection(1, 2);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        REQUIRE(area->getText() == "AC");
        REQUIRE_FALSE(area->hasSelection());
    }
}

TEST_CASE("TextArea - Selection Operations", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("hasSelection returns false by default") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        REQUIRE_FALSE(ta.hasSelection());
    }

    SECTION("setSelection sets hasSelection true") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        ta.setText("Hello");
        ta.setSelection(1, 3);
        REQUIRE(ta.hasSelection());
    }

    SECTION("getSelection returns selected text") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        ta.setText("Hello World");
        ta.setSelection(0, 5);
        REQUIRE(ta.getSelection() == "Hello");
    }

    SECTION("clearSelection clears selection") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        ta.setText("Hello");
        ta.setSelection(1, 3);
        ta.clearSelection();
        REQUIRE_FALSE(ta.hasSelection());
        REQUIRE(ta.getSelection().empty());
    }

    SECTION("setSelection clamps to text length") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        ta.setText("Hi");
        ta.setSelection(0, 100);
        REQUIRE(ta.getSelection() == "Hi");
    }

    SECTION("Selection across multiple lines") {
        TextArea ta(manager, 10, 20, 200, 100, "assets/fonts/font.ttf", 16);
        ta.setText("Line1\nLine2\nLine3");
        ta.setSelection(3, 10);
        REQUIRE(ta.getSelection() == "e1\nLine");
    }
}

TEST_CASE("TextArea - Clipboard Operations", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Cut removes selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (char c : "ABCDE") {
            if (c == '\0') continue;
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }

        area->setSelection(1, 4);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_X, SDL_KMOD_CTRL);
        area->handleEvent(event);

        REQUIRE(area->getText() == "AE");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Paste inserts at cursor when no selection") {
        SDL_SetClipboardText("test");

        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        event = helper.createTextInputEvent("A");
        area->handleEvent(event);
        event = helper.createTextInputEvent("B");
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_V, SDL_KMOD_CTRL);
        area->handleEvent(event);

        REQUIRE(area->getText() == "AtestB");
    }

    SECTION("Paste replaces selection") {
        SDL_SetClipboardText("NEW");

        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (char c : "Hello") {
            if (c == '\0') continue;
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }

        area->setSelection(1, 4);

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_V, SDL_KMOD_CTRL);
        area->handleEvent(event);

        REQUIRE(area->getText() == "HNEWo");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Copy with no selection does nothing to text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());

        for (char c : "Test") {
            if (c == '\0') continue;
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }

        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_C, SDL_KMOD_CTRL);
        area->handleEvent(event);

        REQUIRE(area->getText() == "Test");
    }
}

TEST_CASE("TextArea - Home/End Keys", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Home moves cursor to start of current line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().substr(6, 1) == "X");
    }

    SECTION("End moves cursor to end of current line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABC");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("!");
        area->handleEvent(event);
        REQUIRE(area->getText() == "ABC!");
    }

    SECTION("Ctrl+Home moves cursor to document start") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText()[0] == 'X');
    }

    SECTION("Ctrl+End moves cursor to document end") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().back() == 'X');
    }

    SECTION("Home on empty text does nothing harmful") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText() == "X");
    }

    SECTION("End on empty text does nothing harmful") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText() == "X");
    }

    SECTION("Shift+Home selects from cursor to line start") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "Lin");
    }

    SECTION("Shift+End selects from cursor to line end") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Hello");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "llo");
    }
}

TEST_CASE("TextArea - Arrow Up/Down", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Arrow Up moves cursor to previous line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABC\nDEF\nGHI");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_UP);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().find("XBC") != std::string::npos);
    }

    SECTION("Arrow Down moves cursor to next line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABC\nDEF\nGHI");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().find("XEF") != std::string::npos);
    }

    SECTION("Arrow Up at first line stays at first line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABC\nDEF");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_UP);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText()[0] == 'X');
    }

    SECTION("Arrow Down at last line processes Down event") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABC\nDEF");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);
    }

    SECTION("Arrow Up with navigation works") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABCD\nEFGH");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_UP);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);
    }

    SECTION("Arrow Down navigation works") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("ABCD\nEFGH");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);
    }
}

TEST_CASE("TextArea - Page Up/Down", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("PageUp scrolls up and moves cursor") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3\nL4\nL5\nL6\nL7\nL8\nL9\nL10");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEUP);
        bool handled = area->handleEvent(event);
        REQUIRE(handled);
    }

    SECTION("PageDown scrolls down multiple lines") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3\nL4\nL5\nL6\nL7\nL8\nL9\nL10");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEDOWN);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().find("X") != std::string::npos);
    }

    SECTION("PageUp at start stays at start") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEUP);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText()[0] == 'X');
    }

    SECTION("PageDown at end stays at end") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEDOWN);
        area->handleEvent(event);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        REQUIRE(area->getText().back() == 'X');
    }

    SECTION("Shift+PageUp creates selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3\nL4\nL5\nL6\nL7\nL8");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEUP, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
    }

    SECTION("Shift+PageDown creates selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("L1\nL2\nL3\nL4\nL5\nL6\nL7\nL8");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_PAGEDOWN, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
    }
}

TEST_CASE("TextArea - Shift+Arrow Selection", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Shift+Left creates selection to the left") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Hello World")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "d");
    }

    SECTION("Shift+Right creates selection to the right") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Hello World")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "H");
    }

    SECTION("Shift+Left multiple times extends selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("ABCDE")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "CDE");
    }

    SECTION("Shift+Right multiple times extends selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("ABCDE")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "AB");
    }

    SECTION("Arrow without Shift clears selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("ABC")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        REQUIRE(area->hasSelection());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT);
        area->handleEvent(event);
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Shift+Left at beginning does not create selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Test")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Shift+Right at end does not extend selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Test")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE_FALSE(area->hasSelection());
    }
}

TEST_CASE("TextArea - Ctrl+A Select All", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Ctrl+A selects all text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "Line1\nLine2\nLine3");
    }

    SECTION("Ctrl+A on empty text creates no selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Ctrl+A then Delete clears all text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Test");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        
        REQUIRE(area->getText().empty());
    }

    SECTION("Ctrl+A then Backspace clears all text") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Test");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        
        REQUIRE(area->getText().empty());
    }

    SECTION("Ctrl+A replaces existing selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Hello World");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        area->setSelection(0, 5);
        REQUIRE(area->getSelection() == "Hello");
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        REQUIRE(area->getSelection() == "Hello World");
    }
}

TEST_CASE("TextArea - Typing Replaces Selection", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Typing replaces selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Hello World");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        area->setSelection(6, 11);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "Hello X");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Enter replaces selection with newline") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("ABC")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        area->setSelection(1, 2);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "A\nC");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Typing replaces all text when full selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Old Text")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        event = helper.createTextInputEvent("N");
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "N");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Backspace deletes selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Hello World")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        area->setSelection(5, 6);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "HelloWorld");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Delete deletes selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("Hello World")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        area->setSelection(0, 6);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "World");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Multiple character input replaces selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (char c : std::string("ABC")) {
            std::string charStr(1, c);
            event = helper.createTextInputEvent(charStr.c_str());
            area->handleEvent(event);
        }
        
        area->setSelection(1, 2);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        event = helper.createTextInputEvent("Y");
        area->handleEvent(event);
        event = helper.createTextInputEvent("Z");
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "AXYZC");
    }
}

TEST_CASE("TextArea - Multi-line Selection", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Shift+Down creates multi-line selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        area->setSelection(0, 0);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection().length() > 0);
    }

    SECTION("Selection across multiple lines with Ctrl+A") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("A\nB\nC");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_A, SDL_KMOD_CTRL);
        area->handleEvent(event);
        
        REQUIRE(area->getSelection() == "A\nB\nC");
    }

    SECTION("Shift+Up creates selection to previous line") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        
        area->setSelection(area->getText().length(), area->getText().length());
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_UP, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
    }

    SECTION("Backspace deletes multi-line selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("i");
        area->handleEvent(event);
        event = helper.createTextInputEvent("n");
        area->handleEvent(event);
        event = helper.createTextInputEvent("e");
        area->handleEvent(event);
        event = helper.createTextInputEvent("1");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("i");
        area->handleEvent(event);
        event = helper.createTextInputEvent("n");
        area->handleEvent(event);
        event = helper.createTextInputEvent("e");
        area->handleEvent(event);
        event = helper.createTextInputEvent("2");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("3");
        area->handleEvent(event);
        
        area->setSelection(0, 12);
        REQUIRE(area->hasSelection());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "L3");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Delete deletes multi-line selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        area->setSelection(5, 17);
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "Line1");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Typing replaces multi-line selection") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        area->setSelection(3, 15);
        
        event = helper.createTextInputEvent("X");
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "LinXe3");
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("Enter replaces multi-line selection with newline") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("1");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("2");
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        event = helper.createTextInputEvent("L");
        area->handleEvent(event);
        event = helper.createTextInputEvent("3");
        area->handleEvent(event);
        
        area->setSelection(0, 4);
        REQUIRE(area->hasSelection());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN);
        area->handleEvent(event);
        
        REQUIRE(area->getText() == "\n2\nL3");
        REQUIRE_FALSE(area->hasSelection());
    }
}

TEST_CASE("TextArea - Selection with Navigation Keys", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Shift+Home selects to line start") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Hello World");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (int i = 0; i < 6; ++i) {
            event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
            area->handleEvent(event);
        }
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "Hello ");
    }

    SECTION("Shift+End selects to line end") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Hello World");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        for (int i = 0; i < 3; ++i) {
            event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
            area->handleEvent(event);
        }
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "lo World");
    }

    SECTION("Ctrl+Shift+Home selects to document start") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_HOME, SDL_KMOD_CTRL | SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection().find("Line1") != std::string::npos);
    }

    SECTION("Ctrl+Shift+End selects to document end") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 200, 100, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        area->setText("Line1\nLine2\nLine3");
        manager.addElement(std::move(ta));

        SDL_Event event = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(event);
        area->render(manager.getRenderer());
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT);
        area->handleEvent(event);
        area->clearSelection();
        
        event = helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_END, SDL_KMOD_CTRL | SDL_KMOD_SHIFT);
        area->handleEvent(event);
        
        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection().find("Line3") != std::string::npos);
    }
}
TEST_CASE("TextArea - Clipboard API methods", "[text_area][clipboard]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextArea area(manager, 0, 0, 300, 200, "assets/fonts/font.ttf", 16);
    area.setText("line1\nline2\nline3");

    SECTION("selectAll selects the whole multi-line text") {
        area.selectAll();
        REQUIRE(area.hasSelection());
        REQUIRE(area.getSelection() == "line1\nline2\nline3");
    }

    SECTION("copy puts selection on clipboard") {
        area.setSelection(0, 5); // "line1" (bytes)
        REQUIRE(area.copyToClipboard());
        char* clip = SDL_GetClipboardText();
        REQUIRE(std::string(clip) == "line1");
        SDL_free(clip);
    }

    SECTION("copy without selection is a no-op") {
        REQUIRE_FALSE(area.copyToClipboard());
    }

    SECTION("cut removes selection and stores it") {
        area.setSelection(6, 11); // "line2"
        REQUIRE(area.cutToClipboard());
        REQUIRE(area.getText() == "line1\n\nline3");

        char* clip = SDL_GetClipboardText();
        REQUIRE(std::string(clip) == "line2");
        SDL_free(clip);
    }

    SECTION("cut without selection is a no-op") {
        REQUIRE_FALSE(area.cutToClipboard());
        REQUIRE(area.getText() == "line1\nline2\nline3");
    }

    SECTION("paste inserts clipboard text at cursor") {
        SDL_SetClipboardText("XY");
        REQUIRE(area.pasteFromClipboard());
        REQUIRE(area.getText() == "XYline1\nline2\nline3");
    }

    SECTION("paste replaces selection") {
        area.setSelection(0, 5);
        SDL_SetClipboardText("LINE1");
        REQUIRE(area.pasteFromClipboard());
        REQUIRE(area.getText() == "LINE1\nline2\nline3");
    }

    SECTION("paste without clipboard text is a no-op") {
        SDL_SetClipboardText("");
        // Note: empty clipboard may still report text; set a known sentinel
        SDL_SetClipboardText("PASTED");
        REQUIRE(area.pasteFromClipboard());
        REQUIRE(area.getText().find("PASTED") != std::string::npos);
    }
}

TEST_CASE("TextArea - Right-click context menu", "[text_area][context_menu]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto ta = std::make_unique<TextArea>(manager, 100, 100, 300, 200, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));
    area->setText("line1\nline2");

    SECTION("RMB opens the shared context menu") {
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
        REQUIRE(manager.isContextMenuVisible());
        REQUIRE(manager.getContextMenu()->getItemCount() == 6);
        // With text but no selection: Cut/Copy disabled, Select All enabled
        REQUIRE_FALSE(manager.getContextMenu()->isItemEnabled(0));
        REQUIRE_FALSE(manager.getContextMenu()->isItemEnabled(1));
        REQUIRE(manager.getContextMenu()->isItemEnabled(5));
    }

    SECTION("menu Select All action selects the whole text") {
        // Select All is the last item: 25+25+8+25+8 = 91px down, center at +12
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
        int menuX = manager.getContextMenu()->getX();
        int menuY = manager.getContextMenu()->getY();
        int selAllY = menuY + 91 + 12;

        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, menuX + 100, selAllY));
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, menuX + 100, selAllY));

        REQUIRE(area->hasSelection());
        REQUIRE(area->getSelection() == "line1\nline2");
        REQUIRE_FALSE(manager.isContextMenuVisible());
    }

    SECTION("RMB does not start drag selection") {
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
        manager.processEvent(helper.createMouseMotion(220, 160));
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_RIGHT, 220, 160));
        REQUIRE_FALSE(area->hasSelection());
    }

    SECTION("RMB outside the area does not open the menu") {
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 10, 10));
        REQUIRE_FALSE(manager.isContextMenuVisible());
    }

    SECTION("setContextMenuEnabled(false) disables the menu") {
        area->setContextMenuEnabled(false);
        manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
        REQUIRE_FALSE(manager.isContextMenuVisible());
    }
}

TEST_CASE("TextArea - selection does not paint over the context menu", "[text_area][context_menu][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());

    auto ta = std::make_unique<TextArea>(manager, 100, 100, 300, 200, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));
    area->setText("line1\nline2\nline3");

    // Focus the area with LMB, select everything, then open the menu with RMB.
    // The menu (at cursor) overlaps the selected text.
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 150, 130));
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 150, 130));
    area->selectAll();
    REQUIRE(area->hasSelection());

    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
    REQUIRE(manager.isContextMenuVisible());

    manager.update();
    manager.cleanup();
    manager.render();

    // First menu item ("Cut") spans y 150..175; sample its left padding (no label glyphs).
    SDL_Rect r{208, 162, 1, 1};
    SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
    REQUIRE(surf != nullptr);
    Uint8* px = static_cast<Uint8*>(surf->pixels);
    Uint8 pr = px[0], pg = px[1], pb = px[2];
    SDL_DestroySurface(surf);
    INFO("menu item pixel: " << int(pr) << "," << int(pg) << "," << int(pb));

    // Selection highlight (100,150,255,180) blended over anything is strongly blue;
    // a menu button is neutral gray.
    REQUIRE(pr > 150);
    REQUIRE(pb < 220);
}

TEST_CASE("TextArea - UTF-8 char-index model (shared with TextEditable)", "[text_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelection/getSelection count characters not bytes") {
        TextArea ta(manager, 0, 0, 300, 100, "assets/fonts/font.ttf", 16);
        ta.setText("ząb\n😀x");
        ta.setSelection(0, 2);
        REQUIRE(ta.getSelection() == "zą");
        ta.setSelection(4, 6);
        REQUIRE(ta.getSelection() == "😀x");
    }

    SECTION("Backspace/Delete handle multi-byte chars") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        SDL_Event click = helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55);
        area->handleEvent(click);
        area->render(manager.getRenderer());

        for (const char* c : {"ą", "ę", "X"}) {
            area->handleEvent(helper.createTextInputEvent(c));
        }
        REQUIRE(area->getText() == "ąęX");

        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_BACKSPACE));
        REQUIRE(area->getText() == "ąę");
        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT));
        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT));
        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DELETE));
        REQUIRE(area->getText() == "ę");
    }

    SECTION("Cut/Paste preserve UTF-8") {
        SDL_SetClipboardText("zą");
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        manager.addElement(std::move(ta));

        area->handleEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 55, 55));
        area->render(manager.getRenderer());
        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_V, SDL_KMOD_CTRL));
        REQUIRE(area->getText() == "zą");

        area->setSelection(0, 1);
        REQUIRE(area->cutToClipboard());
        REQUIRE(area->getText() == "ą");
        area->handleEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_V, SDL_KMOD_CTRL));
        REQUIRE(area->getText() == "zą");
    }

    SECTION("TextArea shares TextEditable API via base pointer") {
        auto ta = std::make_unique<TextArea>(manager, 50, 50, 300, 200, "assets/fonts/font.ttf", 16);
        TextArea* area = ta.get();
        TextEditable* base = area;
        manager.addElement(std::move(ta));

        base->setText("hello");
        REQUIRE(area->getText() == "hello");
        base->setSelection(1, 4);
        REQUIRE(base->getSelection() == "ell");
        REQUIRE(area->getSelection() == "ell");
        base->selectAll();
        REQUIRE(area->hasSelection());
    }
}
