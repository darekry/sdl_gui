#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "theme.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    try {
        SDLApp app("AnimatedImage Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());
        // Styled panel to contain all widgets
        auto panel = std::make_unique<Panel>(guiManager, 120, 60, 560, 440);
        panel->setBackgroundColor(ElementState::Normal, {40, 42, 54, 255});
        panel->setBorder(ElementState::Normal, {98, 114, 164, 255}, 2);
        panel->setBorderRadius(ElementState::Normal, 10);

        // Title label
        auto title = std::make_unique<Label>(guiManager, 20, 20, "Animated Sprite Sheet", 24);
        title->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        panel->addChild(std::move(title));

        // AnimatedImage loads a sprite sheet with 9 frames in a single row
        auto anim = std::make_unique<AnimatedImage>(guiManager, 152, 60, 256, 128);
        auto animRef = guiManager.makeRef(anim.get());
        anim->setSpriteSheet("assets/anim.png", 9, 1);
        anim->setFPS(12.0f);
        anim->setLoop(true);
        anim->setTooltip("Animated sprite sheet");
        anim->play();

        // Frame counter label, updated on every frame change
        auto frameLbl = std::make_unique<Label>(guiManager, 20, 210, "Frame: 1 / 9", 16);
        Label* framePtr = frameLbl.get();
        frameLbl->setTextColor(ElementState::Normal, {180, 180, 200, 255});
        anim->setOnFrameChanged([framePtr](int frame) {
            framePtr->setText("Frame: " + std::to_string(frame + 1) + " / 9");
        });
        panel->addChild(std::move(frameLbl));

        // Play/Pause toggle button
        auto toggleBtn = std::make_unique<Button>(guiManager, 180, 270, 200, 44, "Toggle Play/Pause");
        toggleBtn->setTooltip("Pause or resume the animation");
        toggleBtn->setOnClickCallback([animRef](GUIElement*) {
            if (animRef->isPlaying()) {
                animRef->pause();
                LOG_INFO("AnimatedImage", "Animation paused");
            } else {
                animRef->play();
                LOG_INFO("AnimatedImage", "Animation resumed");
            }
        });
        panel->addChild(std::move(toggleBtn));
        panel->addChild(std::move(anim));
        guiManager.addElement(std::move(panel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);
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
