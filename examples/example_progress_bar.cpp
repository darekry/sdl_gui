#include "gui.hpp"
#include "gui_manager.hpp"
#include "progress_bar.hpp"
#include "sdl_app.hpp"
#include "label.hpp"
#include "button.hpp"
#include "animation_manager.hpp"
#include "easing.hpp"

#include "std.hpp"

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
        hProgress->setValue(3.0f);
        guiManager.addElement(std::move(hProgress));
        
        // Vertical progress bar
        auto vLabel = std::make_unique<Label>(guiManager, 50, 120, "Vertical:", 16);
        guiManager.addElement(std::move(vLabel));
        
        auto vProgress = std::make_unique<ProgressBar>(guiManager, 180, 120, 30, 150);
        vProgress->setOrientation(Orientation::Vertical);
        vProgress->setValue(2.0f);
        guiManager.addElement(std::move(vProgress));
        
        // Animated loading progress bar
        auto aLabel = std::make_unique<Label>(guiManager, 50, 290, "Loading (5s):", 16);
        guiManager.addElement(std::move(aLabel));
        
        auto aProgress = std::make_unique<ProgressBar>(guiManager, 180, 290, 300, 30);
        aProgress->setRange(0, 100);
        aProgress->setValue(0);
        auto aProgressRef = guiManager.makeRef(aProgress.get());
        guiManager.addElement(std::move(aProgress));
        
        auto animManager = guiManager.getAnimationManager();
        auto startTime = std::make_shared<uint64_t>(0);
        auto dirtyAnimId = std::make_shared<uint32_t>(0);
        
        auto startBtn = std::make_unique<Button>(guiManager, 50, 340, 120, 30);
        startBtn->setTooltip("oo panie");
        auto startLabel = std::make_unique<Label>(guiManager, 0, 0, "Start Loading", 14);
        startLabel->setPosition(
            (startBtn->getWidth() - startLabel->getWidth()) / 2,
            (startBtn->getHeight() - startLabel->getHeight()) / 2);
        startBtn->addChild(std::move(startLabel));

        startBtn->setOnClickCallback([aProgressRef, animManager, startTime, dirtyAnimId](GUIElement*) {
            auto* pb = static_cast<ProgressBar*>(aProgressRef.get());
            if (!pb) return;

            pb->setValue(0);

            animManager->createAnimation(
                pb->getValuePtr(),
                0.0f,
                pb->getMax(),
                5000,
                Easing::easeInOutQuad,
                [aProgressRef]() {
                    if (auto* pb = static_cast<ProgressBar*>(aProgressRef.get())) {
                        pb->markDirty();
                    }
                    LOG_INFO("ProgressBar", "Loading complete!");
                }
            );

            if (*dirtyAnimId) {
                animManager->removeAnimation(*dirtyAnimId);
            }
            *dirtyAnimId = animManager->addAnimation(30, [aProgressRef]() {
                if (auto* pb = static_cast<ProgressBar*>(aProgressRef.get())) {
                    pb->markDirty();
                }
            });
        });
        guiManager.addElement(std::move(startBtn));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.update();

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
