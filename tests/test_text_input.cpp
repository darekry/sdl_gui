#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "test_helper.cpp"
#include "../src/text_input.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"

TEST_CASE("TextInput Functionality", "[text_input]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    TextureManager textureManager(renderer);
    FontManager fontManager;

    SECTION("Initialization") {
        TextInput textInput(10, 20, 200, 30);
        REQUIRE(textInput.getX() == 10);
        REQUIRE(textInput.getY() == 20);
        REQUIRE(textInput.getWidth() == 200);
        REQUIRE(textInput.getHeight() == 30);
        textInput.setText("Hello");
        REQUIRE(textInput.getText() == "Hello");
    }

    SECTION("Event Handling - Focus and Text Input") {
        TextInput textInput(10, 10, 200, 30);
        std::string changedText = "";
        textInput.setOnTextChanged([&](TextInput* input) { changedText = input->getText(); });

        // 1. Kliknięcie w pole tekstowe - powinno uzyskać fokus
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        textInput.handleEvent(event);

        // 2. Wprowadzanie tekstu
        event = helper.create_text_input_event("Test");
        textInput.handleEvent(event);
        REQUIRE(textInput.getText() == "Test");
        REQUIRE(changedText == "Test");

        event = helper.create_text_input_event("ing");
        textInput.handleEvent(event);
        REQUIRE(textInput.getText() == "Testing");
        REQUIRE(changedText == "Testing");

        // 3. Backspace
        event = helper.create_key_event(SDL_KEYDOWN, SDLK_BACKSPACE);
        textInput.handleEvent(event);
        REQUIRE(textInput.getText() == "Testin");
        REQUIRE(changedText == "Testin");

        // 4. Enter (powinien wywołać onTextSubmit i stracić fokus)
        bool submitted = false;
        textInput.setOnEnterPressed([&](TextInput*){ submitted = true; });
        event = helper.create_key_event(SDL_KEYDOWN, SDLK_RETURN);
        textInput.handleEvent(event);
        REQUIRE(submitted == true);
 
        // 5. Kliknięcie poza polem tekstowym - powinno stracić fokus
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 300, 300);
        textInput.handleEvent(event);
    }

    SECTION("State - Disabled TextInput") {
        TextInput textInput(10, 10, 200, 30);
        textInput.setText("Initial");
        std::string changedText = "";
        textInput.setOnTextChanged([&](TextInput* input) { changedText = input->getText(); });
        textInput.setLocked(true);
 
        REQUIRE(textInput.isLocked() == true);
 
        // Próba uzyskania fokusu
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        textInput.handleEvent(event);
 
        // Próba wprowadzania tekstu
        event = helper.create_text_input_event("New Text");
        textInput.handleEvent(event);
        REQUIRE(textInput.getText() == "Initial"); // Tekst nie powinien się zmienić
        REQUIRE(changedText == "");
    }
}