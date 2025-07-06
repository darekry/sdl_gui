#include "gui.hpp"
#include "gui_manager.hpp"
#include "checkbox.hpp"
#include "helpers/sdl_app.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Checkbox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Utwórz checkbox
        auto checkbox = std::make_unique<Checkbox>(guiManager, 100, 100, 30, 30);
        checkbox->setLabel("Check me!", 24, {255, 255, 255, 255});
        checkbox->setOnChange([]([[maybe_unused]]Checkbox* cb, bool isChecked) {
            std::cout << "Checkbox state changed: " << (isChecked ? "Checked" : "Unchecked") << std::endl;
        });

        guiManager.addElement(std::move(checkbox));

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