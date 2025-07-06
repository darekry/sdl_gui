#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <memory>

#include "button.hpp"
#include "gui_manager.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG))) {
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

    SDL_Window* window = SDL_CreateWindow("Button Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    GUIManager guiManager(renderer);

    // --- Przycisk 1: Z tekstury z pliku ---
    SharedTexture buttonTextureFromFile = guiManager.getTextureManager().loadTexture("assets/button1.png");
    if (!buttonTextureFromFile) {
        std::cerr << "Failed to load button texture from file." << std::endl;
        // Można kontynuować bez tego przycisku lub zakończyć, w zależności od wymagań
    } else {
        auto buttonFromFile = std::make_unique<Button>(guiManager, 50, 50, 200, 80);
        buttonFromFile->setTexture(buttonTextureFromFile);
        buttonFromFile->setOnClickCallback([]([[maybe_unused]] GUIElement* elem) {
            std::cout << "Button 1 (from file) clicked!" << std::endl;
        });
        guiManager.addElement(std::move(buttonFromFile));
    }

    // --- Przycisk 2: Z tekstury z tekstu (używając setLabel) ---
    auto buttonFromText = std::make_unique<Button>(guiManager, 300, 50, 200, 80);
    buttonFromText->setLabel("Click Me!", 24, {0, 0, 0, 255}); // Czarny kolor
    buttonFromText->setOnClickCallback([]([[maybe_unused]] GUIElement* elem) {
        std::cout << "Button 2 (from text) clicked!" << std::endl;
    });
    guiManager.addElement(std::move(buttonFromText));

    // --- Przycisk 3: Z domyślną teksturą (placeholder) ---
    auto buttonDefault = std::make_unique<Button>(guiManager, 550, 50, 200, 80);
    buttonDefault->setOnClickCallback([]([[maybe_unused]] GUIElement* elem) {
        std::cout << "Button 3 (default) clicked!" << std::endl;
    });
    guiManager.addElement(std::move(buttonDefault));

    bool quit = false;
    while (!quit) {
        quit = guiManager.handleEvents();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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