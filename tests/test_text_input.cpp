#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/gui.hpp"
#include "../src/text_input.hpp"

TEST_CASE("TextInput Functionality", "[text_input]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Focus and basic text input") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));

        int changedCount = 0;
        ptr->setOnTextChanged([&](TextInput*) { ++changedCount; });

        SDL_Event e = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);
        e = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);

        e = helper.createTextInputEvent("abc");
        manager.processEvent(e);

        REQUIRE(ptr->getText() == "abc");
        REQUIRE(changedCount >= 1);
    }

    SECTION("Backspace removes last character") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));

        SDL_Event e = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);
        e = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);

        ptr->setText(std::string_view{"ab"});

        e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        manager.processEvent(e);

        REQUIRE(ptr->getText() == "a");
    }

    SECTION("Arrow keys do not change text content") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));

        SDL_Event e = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);
        e = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);

        ptr->setText(std::string{"xyz"});

        e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT);
        manager.processEvent(e);
        e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT);
        manager.processEvent(e);

        REQUIRE(ptr->getText() == "xyz");
    }

    SECTION("Enter triggers OnEnterPressed") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));

        int enterCount = 0;
        ptr->setOnEnterPressed([&](TextInput*) { ++enterCount; });

        SDL_Event e = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);
        e = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);

        e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN);
        manager.processEvent(e);

        REQUIRE(enterCount == 1);
    }

    SECTION("Locked input ignores focus and text") {
        auto input = std::make_unique<TextInput>(manager, 10, 10, 200, 30);
        TextInput* ptr = input.get();
        manager.addElement(std::move(input));

        REQUIRE(ptr->getText().empty());

        ptr->setLocked(true);
        REQUIRE(ptr->isLocked() == true);

        SDL_Event e = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);
        e = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(e);

        e = helper.createTextInputEvent("q");
        manager.processEvent(e);

        REQUIRE(ptr->getText().empty());

        e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_BACKSPACE);
        manager.processEvent(e);
        REQUIRE(ptr->getText().empty());
    }
}