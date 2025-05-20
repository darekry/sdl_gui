#include "SDL2/SDL.h"
#include <SDL2/SDL_events.h>
#include "gui_manager.hpp" // Dodano include dla menedżera GUI
#include "gui.hpp" // Dodano include dla Panel i Button
#include "slider.hpp" // Dodano include dla Slider
#include "texture_manager.hpp" // Dodano include dla TextureManager

// Funkcja zwrotna dla suwaka
void onSliderValueChanged(GUIElement* element) {
    // Rzutowanie wskaźnika na Slider* i pobranie wartości
    Slider* slider = dynamic_cast<Slider*>(element);
    if (slider) {
    } else {
    }
}

int main(int argc, char const * argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    auto* window = SDL_CreateWindow("gui test", -1, -1, 800, 600, 0);
    auto* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    GUIManager gui_manager; // Utworzono instancję menedżera GUI bez argumentów
    TextureManager texture_manager(renderer); // Utworzono instancję menedżera tekstur

    // Utworzenie suwaka poziomego
    Slider* horizontalSlider = new Slider(50, 50, 300, 30, 0, 100, 50, Orientation::Horizontal);
    horizontalSlider->setOnChangeCallback(onSliderValueChanged);

    // Utworzenie suwaka pionowego
    Slider* verticalSlider = new Slider(400, 50, 30, 300, 0, 100, 50, Orientation::Vertical);
    verticalSlider->setOnChangeCallback(onSliderValueChanged);

    // Dodanie suwaków do menedżera GUI
    gui_manager.addElement(horizontalSlider);
    gui_manager.addElement(verticalSlider);

    uint should_close = false;
    while (!should_close)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 127);
        SDL_RenderClear(renderer);

        gui_manager.render(renderer); // Wywołanie renderowania GUI z przekazaniem renderera

        SDL_RenderPresent(renderer);

        // Obsługa zdarzeń przez menedżera GUI
        should_close = gui_manager.handleEvents();
    }


    SDL_Quit();
    return 0;
}
