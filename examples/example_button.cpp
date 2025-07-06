#include "button.hpp"
#include "gui_manager.hpp"
#include "helpers/sdl_app.hpp"

import std.compat;



const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Button Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Przycisk 1: Z tekstury z pliku ---
        SharedTexture buttonTextureFromFile = guiManager.getTextureManager().loadTexture("assets/button1.png");
        if (!buttonTextureFromFile) {
            std::cerr << "Failed to load button texture from file." << std::endl;
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