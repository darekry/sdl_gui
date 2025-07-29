#include "gui.hpp"
#include "gui_manager.hpp"
#include "slider.hpp"
#include "helpers/sdl_app.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Slider Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Tworzenie etykiety do wyświetlania wartości
        auto valueLabel = std::make_unique<Label>(guiManager, 320, 100, "50", 16);
        auto* valueLabelPtr = valueLabel.get();
        guiManager.addElement(std::move(valueLabel));

        // Tworzenie suwaka i ustawienie callbacku
        auto slider = std::make_unique<Slider>(guiManager, 100, 100, 200, 20, 0, 100, 50, Orientation::Horizontal);
        slider->setOnChangeCallback([valueLabelPtr](GUIElement* element) {
            Slider* slider_ptr = static_cast<Slider*>(element);
            if (slider_ptr && valueLabelPtr) {
                valueLabelPtr->setText(std::to_string(slider_ptr->getValue()));
            }
        });
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

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}