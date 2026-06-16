#include "gui_manager.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "label.hpp"
#include "sdl_app.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("RadioButton Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        auto* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // 1. Utwórz RadioGroup jako kontener Panel
        auto radioGroup = std::make_unique<RadioGroup>(guiManager, 100, 100, 250, 200);
        radioGroup->setStyle(ElementState::Normal, {.backgroundColor = {{40, 40, 40, 255}}, .textColor = std::nullopt, .texture = std::nullopt, .borderColor = std::nullopt, .borderWidth = std::nullopt, .borderRadius = std::nullopt, .fontSize = std::nullopt, .fontName = std::nullopt});
        radioGroup->setBorder(ElementState::Normal, {100, 100, 100, 255}, 2);
        
        radioGroup->addOption("Opcja 1 po polsku");
        radioGroup->addOption("Opcja 2", true);
        radioGroup->addOption("Opcja 3");

        auto infoLabel = std::make_unique<Label>(guiManager, 20, 150, "Wybierz jedną opcję:", 14);
        radioGroup->addChild(std::move(infoLabel));
        
        // Przekaż własność RadioGroup do GUIManager
        guiManager.addElement(std::move(radioGroup));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 44, 44, 44, 255);
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