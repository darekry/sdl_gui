#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <memory>

#include "SDL_render.h"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "combobox.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

    if (!(IMG_Init(IMG_INIT_PNG))) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! IMG_Error: %s", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! TTF_Error: %s", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("ComboBox Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window could not be created! SDL_Error: %s", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    GUIManager guiManager(renderer);

    // Załaduj czcionkę, aby ComboBox mógł jej użyć
    if (!guiManager.getFontManager().loadFont("assets/fonts/font.ttf", 16)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font 'assets/fonts/font.ttf'");
        // Warto rozważyć przerwanie aplikacji, jeśli czcionka jest kluczowa
    }

    // Utwórz ComboBox
    auto comboBox = std::make_unique<ComboBox>(100, 100, 200, 30);
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
    while (!quit) {
        quit = guiManager.handleEvents();

        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);

        guiManager.render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}