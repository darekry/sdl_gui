#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/combobox.hpp"
#include "../src/font_manager.hpp"
#include "../src/texture_manager.hpp"
#include "../src/gui.hpp"
#include <unistd.h> // For access()

TEST_CASE("ComboBox Functionality", "[combobox]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    FontManager fontManager;
    TextureManager textureManager(renderer);

    // Inicjalizacja TTF, jeśli jeszcze nie została zainicjowana
    if (TTF_Init() == -1) {
        FAIL("Failed to initialize TTF: " << TTF_GetError());
    }
    // Utworzenie katalogu na zasoby, jeśli nie istnieje
    system("mkdir -p assets/fonts");
    // Utworzenie prostej czcionki do testów, jeśli nie istnieje
    if (access("assets/fonts/font.ttf", F_OK) != 0) {
        // Plik nie istnieje, więc go tworzymy.
        // To jest uproszczenie dla testów, normalnie plik powinien istnieć.
        FILE* fp = fopen("assets/fonts/font.ttf", "w");
        if (fp) {
            fclose(fp);
        }
    }


    SECTION("Initialization") {
        ComboBox comboBox(renderer, 10, 20, 200, 30, fontManager, textureManager);
        REQUIRE(comboBox.getX() == 10);
        REQUIRE(comboBox.getY() == 20);
        REQUIRE(comboBox.getWidth() == 200);
        REQUIRE(comboBox.getHeight() == 30);
        REQUIRE(comboBox.getSelectedIndex() == -1);
        REQUIRE(comboBox.getSelectedItem() == "");
    }

    SECTION("Adding items and selection") {
        ComboBox comboBox(renderer, 10, 20, 200, 30, fontManager, textureManager);
        comboBox.addItem("Option 1");
        comboBox.addItem("Option 2");
        comboBox.addItem("Option 3");

        REQUIRE(comboBox.getSelectedIndex() == 0);
        REQUIRE(comboBox.getSelectedItem() == "Option 1");

        comboBox.setSelectedIndex(2);
        REQUIRE(comboBox.getSelectedIndex() == 2);
        REQUIRE(comboBox.getSelectedItem() == "Option 3");

        // Test invalid index
        comboBox.setSelectedIndex(99);
        REQUIRE(comboBox.getSelectedIndex() == 2); // Should not change
        REQUIRE(comboBox.getSelectedItem() == "Option 3");
    }

    SECTION("Event Handling - Toggle Dropdown") {
        ComboBox comboBox(renderer, 10, 20, 200, 30, fontManager, textureManager);
        comboBox.addItem("Option 1");

        // Początkowo zwinięty
        REQUIRE_FALSE(comboBox.isExpanded());

        // Kliknięcie na główny przycisk, aby rozwinąć
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 25);
        comboBox.handleEvent(event);
        event.type = SDL_MOUSEBUTTONUP;
        comboBox.handleEvent(event);
        comboBox.handleEvent(event);
        
        // Powinien być rozwinięty
        REQUIRE(comboBox.isExpanded());
    }
    SECTION("Event Handling - Item Selection") {
        ComboBox comboBox(renderer, 10, 20, 200, 30, fontManager, textureManager);
        comboBox.addItem("Option 1");
        comboBox.addItem("Option 2");

        int selected_idx = -1;
        std::string selected_str = "";
        comboBox.on_selection_changed = [&](int index, const std::string& item) {
            selected_idx = index;
            selected_str = item;
        };

        // 1. Rozwiń listę
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 25);
        comboBox.handleEvent(event);
        event.type = SDL_MOUSEBUTTONUP;
        comboBox.handleEvent(event);

        // 2. Kliknij na drugi element (Option 2)
        // Pozycja Y = y + height + item_height * item_index
        // Y = 20 + 30 + 30 * 1 = 80
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 85);
        comboBox.handleEvent(event);
        event.type = SDL_MOUSEBUTTONUP;
        comboBox.handleEvent(event);

        REQUIRE(comboBox.getSelectedIndex() == 1);
        REQUIRE(comboBox.getSelectedItem() == "Option 2");
        REQUIRE(selected_idx == 1);
        REQUIRE(selected_str == "Option 2");
    }
    
    SECTION("Event Handling - Close when clicking outside") {
        ComboBox comboBox(renderer, 10, 20, 200, 30, fontManager, textureManager);
        comboBox.addItem("Option 1");

        // 1. Rozwiń listę
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 25);
        comboBox.handleEvent(event);
        event.type = SDL_MOUSEBUTTONUP;
        comboBox.handleEvent(event);

        // 2. Kliknij poza komponentem
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 500, 500);
        comboBox.handleEvent(event);
        
        // Powinien się zwinąć.
        REQUIRE_FALSE(comboBox.isExpanded());

        // Sprawdzamy, czy po kliknięciu na element listy (co nie powinno być możliwe, bo jest zwinięty)
        // stan się nie zmienia.
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 15, 55);
        comboBox.handleEvent(event);
        REQUIRE(comboBox.getSelectedIndex() == 0); // Nadal powinien być wybrany pierwszy element
    }

    // Zamknięcie TTF
    TTF_Quit();
}