#include "gui_manager.hpp"
#include "checkbox.hpp"
#include "sdl_app.hpp"
#include "label.hpp"
#include "panel.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Checkbox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto panel = std::make_unique<Panel>(guiManager, 100, 100, 200, 40);
        panel->setStyle(ElementState::Normal, {.backgroundColor = {{50, 50, 50, 255}}});

        auto checkbox = std::make_unique<Checkbox>(guiManager, 10, 10, 20, 20);
        checkbox->setOnChange([](Checkbox* , bool isChecked) {
            std::cout << "Checkbox state changed: " << (isChecked ? "Checked" : "Unchecked") << std::endl;
        });
        
        auto label = std::make_unique<Label>(guiManager, 40, 10, "Check me!", 14);

        panel->addChild(std::move(checkbox));
        panel->addChild(std::move(label));

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