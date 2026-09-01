#include "button.hpp"
#include "checkbox.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "slider.hpp"
#include "label.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Tooltip Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Button with tooltip ---
        auto button = std::make_unique<Button>(guiManager, 50, 50, 200, 80);
        auto label = std::make_unique<Label>(guiManager, 0,0, "Najedź na mnie", 20);
        label->setPosition((button->getWidth() - label->getWidth()) / 2, (button->getHeight() - label->getHeight())/2);
        button->addChild(std::move(label));
        button->setTooltip("To jest podpowiedź dla przycisku.");
        button->setOnClickCallback(
            []([[maybe_unused]] GUIElement* elem) { LOG_INFO("Tooltip", "Przycisk kliknięty!"); });
        guiManager.addElement(std::move(button));

        // --- Checkbox with tooltip ---
        auto checkbox = std::make_unique<Checkbox>(guiManager, 300, 50, 40, 40);
        checkbox->setTooltip("Zaznacz lub odznacz tę opcję.\nObsługuje\nwiele linii!");
        guiManager.addElement(std::move(checkbox));
        
        // --- Slider with tooltip ---
        auto slider = std::make_unique<Slider>(guiManager, 50, 200, 300, 30, 0, 100, 50, Orientation::Horizontal);
        slider->setTooltip("Użyj tego suwaka, aby zmienić wartość.");
        guiManager.addElement(std::move(slider));


        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            // Update timers (required for tooltips!)
            guiManager.update();

            // Update state and safely remove elements (important for tooltips!)
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