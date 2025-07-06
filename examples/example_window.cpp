#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/checkbox.hpp"
#include "SDL_log.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

// Helper function to create a 1x1 texture of a specific color
SDL_Texture* createColorTexture(SDL_Renderer* renderer, SDL_Color color) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
    if (!surface) return nullptr;
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Draggable Window Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
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

    // Main window panel
    auto windowPanel = std::make_unique<Panel>(guiManager, 100, 100, 300, 200);
    windowPanel->setTexture(std::shared_ptr<SDL_Texture>(createColorTexture(renderer, {200, 200, 200, 255}), SDL_DestroyTexture));
    Panel* windowPanelPtr = windowPanel.get();

    // Title bar
    auto titleBar = std::make_unique<Panel>(guiManager, 0, 0, 300, 30);
    titleBar->setTexture(std::shared_ptr<SDL_Texture>(createColorTexture(renderer, {100, 100, 150, 255}), SDL_DestroyTexture));
    titleBar->setLabel("Window Title", 16, {255, 255, 255, 255});
    titleBar->setDraggable(true);

    // Close button
    auto closeButton = std::make_unique<Button>(guiManager, 270, 5, 25, 20);
    closeButton->setLabel("X", 14, {255, 255, 255, 255});
    closeButton->setOnClickCallback([windowPanelPtr](GUIElement*){
        windowPanelPtr->setVisible(false);
        SDL_LogInfo(1 , "przycisk");
    });

    // Content panel
    auto contentPanel = std::make_unique<Panel>(guiManager, 5, 35, 290, 160);
    contentPanel->setTexture(std::shared_ptr<SDL_Texture>(createColorTexture(renderer, {220, 220, 220, 255}), SDL_DestroyTexture));
    contentPanel->setLabel("This is the content area.", 14, {0, 0, 0, 255});

    // Add a checkbox to the content to show it moves with the window
    auto sampleCheckbox = std::make_unique<Checkbox>(guiManager, 10, 40, 150, 20);
    sampleCheckbox->setLabel("Sample Checkbox", 14, {0, 0, 0, 255});

    contentPanel->addChild(std::move(sampleCheckbox));
    titleBar->addChild(std::move(closeButton));
        windowPanel->addChild(std::move(titleBar));
        windowPanel->addChild(std::move(contentPanel));
        guiManager.addElement(std::move(windowPanel));

    bool quit = false;
    while (!quit) {
        quit = guiManager.handleEvents();

        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderClear(renderer);

        guiManager.render(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}