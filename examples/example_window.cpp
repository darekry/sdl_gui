#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/checkbox.hpp"
#include "helpers/sdl_app.hpp"
#include "SDL_log.h"
#include <iostream>
#include <memory>

// Helper function to create a 1x1 texture of a specific color
// Ta funkcja jest teraz używana tylko do stworzenia surowej tekstury,
// która następnie jest przekazywana do TextureManager.
SDL_Texture* createColorTexture(SDL_Renderer* renderer, SDL_Color color) {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 1, 1, 32, 0, 0, 0, 0);
    if (!surface) return nullptr;
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int main() {
    try {
        SDLApp app("Draggable Window Example", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Main window panel
        auto windowPanel = std::make_unique<Panel>(guiManager, 100, 100, 300, 200);
        windowPanel->setTexture(
            guiManager.getTextureManager().addTexture("window_bg", createColorTexture(renderer, {200, 200, 200, 255}))
        );
        Panel* windowPanelPtr = windowPanel.get();

        // Title bar
        auto titleBar = std::make_unique<Panel>(guiManager, 0, 0, 300, 30);
        titleBar->setTexture(
            guiManager.getTextureManager().addTexture("title_bg", createColorTexture(renderer, {100, 100, 150, 255}))
        );
        titleBar->setLabel("Window Title", 16, {255, 255, 255, 255});
        titleBar->setDraggable(true);

        // Close button
        auto closeButton = std::make_unique<Button>(guiManager, 270, 5, 25, 20);
        closeButton->setLabel("X", 14, {255, 255, 255, 255});
        closeButton->setOnClickCallback([windowPanelPtr](GUIElement*){
            windowPanelPtr->markForDeletion();
            std::cout << "Window marked for deletion.\n";
        });

        // Content panel
        auto contentPanel = std::make_unique<Panel>(guiManager, 5, 35, 290, 160);
        contentPanel->setTexture(
            guiManager.getTextureManager().addTexture("content_bg", createColorTexture(renderer, {220, 220, 220, 255}))
        );
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
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);

            guiManager.render();
            guiManager.cleanup();

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}