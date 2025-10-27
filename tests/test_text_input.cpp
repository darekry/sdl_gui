#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_input.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextInput functionality", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Initialization") {
        TextInput ti(manager, 10, 20, 200, 30);
        REQUIRE(ti.getText().empty());
        REQUIRE_FALSE(ti.hasKeyboardFocus());
        REQUIRE_FALSE(ti.isLocked());
    }

    SECTION("setText and getText") {
        auto ti = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        bool changed = false;
        input->setOnTextChanged([&](TextInput*) { changed = true; });

        input->setText(std::string("Hello"));
        REQUIRE(input->getText() == "Hello");
        REQUIRE(changed);

        changed = false;
        input->setText(std::string("Hello"));
        REQUIRE_FALSE(changed);

        input->setText(std::string("World"));
        REQUIRE(input->getText() == "World");
        REQUIRE(changed);
    }

    SECTION("Clicking on input gives focus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        REQUIRE_FALSE(input->hasKeyboardFocus());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));

        REQUIRE(input->hasKeyboardFocus());
    }

    SECTION("Typing updates text when focused") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        bool textChanged = false;
        input->setOnTextChanged([&](TextInput*) { textChanged = true; });

        manager.processEvent(helper.createTextInputEvent("A"));
        REQUIRE(textChanged);
        REQUIRE(input->getText() == "A");

        textChanged = false;
        manager.processEvent(helper.createTextInputEvent("B"));
        REQUIRE(textChanged);
        REQUIRE(input->getText() == "AB");
    }

    SECTION("Backspace removes character") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setText(std::string("Test"));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(input->getText() == "Tes");

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE));
        REQUIRE(input->getText() == "Te");
    }

    SECTION("Enter key calls onEnterPressed and removes focus") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        bool enterPressed = false;
        input->setOnEnterPressed([&](TextInput*) { enterPressed = true; });

        input->setText(std::string("Done"));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(enterPressed);
        REQUIRE_FALSE(input->hasKeyboardFocus());
    }

    SECTION("Locked input ignores events") {
        auto ti = std::make_unique<TextInput>(manager, 50, 50, 200, 30);
        TextInput* input = ti.get();
        manager.addElement(std::move(ti));

        input->setLocked(true);
        REQUIRE(input->isLocked());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE_FALSE(input->hasKeyboardFocus());

        input->setLocked(false);
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 55, 55));
        REQUIRE(input->hasKeyboardFocus());
    }
}
