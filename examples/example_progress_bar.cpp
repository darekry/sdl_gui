#include "gui.hpp"
#include "gui_manager.hpp"
#include "progress_bar.hpp"
#include "sdl_app.hpp"
#include "label.hpp"
#include "button.hpp"

import std.compat;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 400;

int main(int, char**) {
    try {
        SDLApp app("ProgressBar Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto titleLabel = std::make_unique<Label>(guiManager, 0, 20, "ProgressBar Examples", 24);
        titleLabel->setPosition((SCREEN_WIDTH - titleLabel->getWidth()) / 2, 20);
        guiManager.addElement(std::move(titleLabel));

        // Horizontal progress bar
        auto hLabel = std::make_unique<Label>(guiManager, 50, 70, "Horizontal:", 16);
        guiManager.addElement(std::move(hLabel));

        auto hProgress = std::make_unique<ProgressBar>(guiManager, 180, 70, 300, 30);
        hProgress->setValue(65.0f);
        guiManager.addElement(std::move(hProgress));

        // Vertical progress bar
        auto vLabel = std::make_unique<Label>(guiManager, 50, 120, "Vertical:", 16);
        guiManager.addElement(std::move(vLabel));

        auto vProgress = std::make_unique<ProgressBar>(guiManager, 180, 120, 30, 150);
        vProgress->setOrientation(Orientation::Vertical);
        vProgress->setValue(40.0f);
        guiManager.addElement(std::move(vProgress));

        // Animated progress bar with buttons
        auto aLabel = std::make_unique<Label>(guiManager, 50, 290, "Animated:", 16);
        guiManager.addElement(std::move(aLabel));

        auto aProgress = std::make_unique<ProgressBar>(guiManager, 180, 290, 300, 30);
        aProgress->setRange(0, 60);
        aProgress->setValue(0);
        aProgress->setTextFormat("%.0f / %.0f");
        auto aProgressRef = guiManager.makeRef(aProgress.get());
        guiManager.addElement(std::move(aProgress));

        // Timer-based animation for demo
        auto startBtn = std::make_unique<Button>(guiManager, 50, 340, 100, 30);
        auto startLabel = std::make_unique<Label>(guiManager, 0, 0, "Start", 14);
        startLabel->setPosition(
            (startBtn->getWidth() - startLabel->getWidth()) / 2,
            (startBtn->getHeight() - startLabel->getHeight()) / 2);
        startBtn->addChild(std::move(startLabel));
        startBtn->setOnClickCallback([aProgressRef](GUIElement*) {
            auto* pb = static_cast<ProgressBar*>(aProgressRef.get());
            if (pb) {
                pb->setValue(0);
            }
        });
        guiManager.addElement(std::move(startBtn));

        auto incBtn = std::make_unique<Button>(guiManager, 160, 340, 100, 30);
        auto incLabel = std::make_unique<Label>(guiManager, 0, 0, "+10", 14);
        incLabel->setPosition(
            (incBtn->getWidth() - incLabel->getWidth()) / 2,
            (incBtn->getHeight() - incLabel->getHeight()) / 2);
        incBtn->addChild(std::move(incLabel));
        incBtn->setOnClickCallback([aProgressRef](GUIElement*) {
            auto* pb = static_cast<ProgressBar*>(aProgressRef.get());
            if (pb) {
                pb->setValue(pb->getValue() + 10);
            }
        });
        guiManager.addElement(std::move(incBtn));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 212, 208, 200, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            guiManager.cleanup();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
