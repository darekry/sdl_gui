#include "gui.hpp"
#include "gui_manager.hpp"
#include "slider.hpp"
#include "helpers/sdl_app.hpp"
import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Slider Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Utwórz suwak
        auto slider = std::make_unique<Slider>(guiManager, 100, 100, 200, 20, 0, 100, 50, Orientation::Horizontal);

        // Dostęp do przycisków przez getChildren() jest niebezpieczny, jeśli zmieni się kolejność.
        // Lepszym podejściem byłoby dodanie dedykowanych metod w klasie Slider.
        // Na potrzeby tego przykładu, zakładamy, że kolejność jest stała.
        if (slider->getChildren().size() >= 2) {
            if(auto dec_btn = dynamic_cast<Button*>(slider->getChildren()[0].get())) {
                dec_btn->setLabel("<", 24, {0,0,0,255});
            }
            if(auto inc_btn = dynamic_cast<Button*>(slider->getChildren()[1].get())) {
                inc_btn->setLabel(">", 24, {0,0,0,255});
            }
        }

        slider->setOnChangeCallback([](GUIElement* element) {
            Slider* slider_ptr = static_cast<Slider*>(element);
            if (slider_ptr) {
                std::cout << "Slider value changed: " << slider_ptr->getValue() << std::endl;
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