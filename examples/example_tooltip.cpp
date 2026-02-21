#include "button.hpp"
#include "checkbox.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "slider.hpp"
#include "label.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Tooltip Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Przycisk z podpowiedzią ---
        auto button = std::make_unique<Button>(guiManager, 50, 50, 200, 80);
        auto label = std::make_unique<Label>(guiManager, 0,0, "Najedź na mnie", 20);
        label->setPosition((button->getWidth() - label->getWidth()) / 2, (button->getHeight() - label->getHeight())/2);
        button->addChild(std::move(label));
        button->setTooltip("To jest podpowiedź dla przycisku.");
        button->setOnClickCallback(
            []([[maybe_unused]] GUIElement* elem) { std::cout << "Przycisk kliknięty!" << std::endl; });
        guiManager.addElement(std::move(button));

        // --- Checkbox z podpowiedzią ---
        auto checkbox = std::make_unique<Checkbox>(guiManager, 300, 50, 40, 40);
        checkbox->setTooltip("Zaznacz lub odznacz tę opcję.\nObsługuje\nwiele linii!");
        guiManager.addElement(std::move(checkbox));
        
        // --- Suwak z podpowiedzią ---
        auto slider = std::make_unique<Slider>(guiManager, 50, 200, 300, 30, 0, 100, 50, Orientation::Horizontal);
        slider->setTooltip("Użyj tego suwaka, aby zmienić wartość.");
        guiManager.addElement(std::move(slider));


        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            // Aktualizacja stanu i bezpieczne usuwanie elementów (ważne dla tooltipów!)
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Wystąpił błąd: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}