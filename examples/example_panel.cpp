#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "panel.hpp"



const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Panel Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Utwórz panel
        auto panel = std::make_unique<Panel>(guiManager, 100, 100, 600, 400);
        Style panel_style;
        panel_style.borderColor = {255, 0, 0, 128};
        panel_style.borderWidth = 5;
        panel->setStyle(ElementState::Normal, panel_style);

        guiManager.addElement(std::move(panel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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