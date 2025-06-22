#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/slider.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"

TEST_CASE("Slider Functionality", "[slider]") {
    TestHelper helper;
    SDL_Renderer* renderer = helper.getRenderer();
    TextureManager textureManager(renderer);
    FontManager fontManager;

    SECTION("Initialization") {
        Slider slider(10, 20, 200, 20, 0, 100, 50, Orientation::Horizontal);
        REQUIRE(slider.getX() == 10);
        REQUIRE(slider.getY() == 20);
        REQUIRE(slider.getWidth() == 200);
        REQUIRE(slider.getHeight() == 20);
        REQUIRE(slider.getValue() == 50);

        Slider slider2(0, 0, 100, 10, -10, 10, 0, Orientation::Horizontal);
        REQUIRE(slider2.getValue() == 0);
    }

    SECTION("Event Handling - Dragging") {
        Slider slider(10, 10, 200, 20, 0, 100, 50, Orientation::Horizontal);
        int changedValue = -1;
        slider.setOnChangeCallback([&](GUIElement*){ changedValue = slider.getValue(); });

        // Symulacja kliknięcia i przeciągnięcia w prawo
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 20); // Kliknięcie na początku suwaka
        slider.handleEvent(event);

        event = helper.create_mouse_event(SDL_MOUSEMOTION, 0, 110, 20); // Przeciągnięcie do środka (10 + 100)
        slider.handleEvent(event);
        // Wartość powinna być około 50 (połowa zakresu 0-100)
        REQUIRE(slider.getValue() > 40);
        REQUIRE(slider.getValue() < 60);
        REQUIRE(changedValue == slider.getValue());

        event = helper.create_mouse_event(SDL_MOUSEMOTION, 0, 210, 20); // Przeciągnięcie do końca (10 + 200)
        slider.handleEvent(event);
        REQUIRE(slider.getValue() == 100); // Powinno być 100
        REQUIRE(changedValue == 100);

        event = helper.create_mouse_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 210, 20);
        slider.handleEvent(event);

        // Symulacja kliknięcia i przeciągnięcia w lewo
        changedValue = -1;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 209, 20); // Kliknięcie na końcu suwaka (wewnątrz)
        slider.handleEvent(event);

        event = helper.create_mouse_event(SDL_MOUSEMOTION, 0, 10, 20); // Przeciągnięcie do początku
        slider.handleEvent(event);
        REQUIRE(slider.getValue() == 0); // Powinno być 0
        REQUIRE(changedValue == 0);

        event = helper.create_mouse_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 10, 20);
        slider.handleEvent(event);
    }

    SECTION("State - Value Clamping") {
        Slider slider(10, 10, 200, 20, 0, 100, 50, Orientation::Horizontal);
        // Testy setValue nie są możliwe, ponieważ metoda setValue nie istnieje.
        // Wartości min/max są ustawiane w konstruktorze i nie ma publicznej metody do ich zmiany.
        // Możemy jedynie testować, czy początkowa wartość jest poprawna i czy callback działa.
    }

    SECTION("State - Disabled Slider") {
        Slider slider(10, 10, 200, 20, 0, 100, 50, Orientation::Horizontal);
        int changedValue = -1;
        slider.setOnChangeCallback([&](GUIElement*){ changedValue = slider.getValue(); });


        slider.setEnabled(false);
        // Symulacja kliknięcia i przeciągnięcia, gdy suwak jest wyłączony
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 10, 20);
        slider.handleEvent(event);
        REQUIRE(changedValue == -1); // Wartość nie powinna się zmienić
    }
}