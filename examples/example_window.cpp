#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "sdl_app.hpp"
#include "SDL_log.h"
#include "label.hpp"

import std.compat;

// Helper function to create a 1x1 texture of a specific color
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
        windowPanel->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = guiManager.getTextureManager().addTexture("window_bg", createColorTexture(renderer, {.r=200, .g=200, .b=200, .a=255})), .borderColor = std::nullopt, .borderWidth = std::nullopt, .fontSize = std::nullopt, .fontName = std::nullopt});
        windowPanel->setDraggable(true);
        auto *windowPanel_p = windowPanel.get();
        // Title bar
        auto titleBar = std::make_unique<Panel>(guiManager, 0, 0, 300, 30);
        titleBar->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = guiManager.getTextureManager().addTexture("title_bg", createColorTexture(renderer, {100, 100, 150, 255})), .borderColor = std::nullopt, .borderWidth = std::nullopt, .fontSize = std::nullopt, .fontName = std::nullopt});
        auto titleLabel = std::make_unique<Label>(guiManager, 5, 5, "Window Title", 16);
        titleBar->addChild(std::move(titleLabel));

        // Close button
        auto closeButton = std::make_unique<Button>(guiManager, 270, 5, 25, 20,"X");
        closeButton->setOnClickCallback([windowPanel_p](GUIElement*){
            windowPanel_p->markForDeletion();
            std::cout << "Window marked for deletion.\n";
        });

        // Content panel
        auto contentPanel = std::make_unique<Panel>(guiManager, 5, 35, 290, 160);
        contentPanel->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = guiManager.getTextureManager().addTexture("content_bg", createColorTexture(renderer, {220, 220, 220, 255})), .borderColor = std::nullopt, .borderWidth = std::nullopt, .fontSize = std::nullopt, .fontName = std::nullopt});
        auto contentLabel = std::make_unique<Label>(guiManager, 5, 5, "This is the content area.", 14);
        contentPanel->addChild(std::move(contentLabel));

        // Add a checkbox to the content to show it moves with the window
        auto sampleCheckbox = std::make_unique<Checkbox>(guiManager, 10, 40, 20, 20);
        auto checkboxLabel = std::make_unique<Label>(guiManager, 35, 40, "Sample Checkbox", 14);
        contentPanel->addChild(std::move(sampleCheckbox));
        contentPanel->addChild(std::move(checkboxLabel));

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