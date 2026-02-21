#include "SDL_log.h"
#include "gui_manager.hpp"
#include "combobox.hpp"
#include "sdl_app.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("ComboBox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Utwórz ComboBox
        auto comboBox = std::make_unique<ComboBox>(guiManager, 100, 100, 200, 30);
        comboBox->addItem("Option 1");
        comboBox->addItem("Option 2");
        comboBox->addItem("Option 3");
        comboBox->addItem("A longer option 4");
        comboBox->setSelectedIndex(0);

        comboBox->on_selection_changed = [](int index, const std::string& item) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Selected item: %s at index: %d", item.c_str(), index);
        };

        guiManager.addElement(std::move(comboBox));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "An error occurred: %s", e.what());
        return 1;
    }

    return 0;
}