#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

#include "gui_manager.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int /*argc*/, char* /*args*/[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL2 GUI RadioButton Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Init();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Init();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    GUIManager guiManager(renderer);

    // Załaduj czcionkę
    SharedFont font = guiManager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) {
        std::cerr << "Failed to load font." << std::endl;
        return 1;
    }

    auto radioGroup = std::make_shared<RadioGroup>();

    auto createRadioButton = [&](int x, int y, const std::string& label) {
        auto rb = std::make_unique<RadioButton>(guiManager, x, y, 20, 20);
        rb->setLabel(label, 24, {255, 255, 255, 255});
        radioGroup->addRadioButton(rb.get()); // Add button to the group
        return rb;
    };

    guiManager.addElement(createRadioButton(100, 100, "Option 1"));

    auto rb2 = createRadioButton(100, 150, "Option 2");
    rb2->setSelected(true); // Pre-select the second button
    guiManager.addElement(std::move(rb2));

    guiManager.addElement(createRadioButton(100, 200, "Option 3"));

    bool quit = false;

    while (!quit) {
        if (guiManager.handleEvents()) {
            quit = true;
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
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