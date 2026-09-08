#include "gui.hpp"
#include "gui_manager.hpp"
#include "range_slider.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "ui_helpers.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("RangeSlider Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto panel = std::make_unique<Panel>(guiManager, 120, 80, 560, 210);
        styleCard(*panel);

        const SDL_Color lightText{255, 255, 255, 255};

        addLabel(*panel, 20, 20, "RangeSlider Example", 22, lightText);

        Label* valueLabel = addLabel(*panel, 200, 120, "20 - 80", 18, lightText);

        auto rangeSlider = std::make_unique<RangeSlider>(guiManager, 20, 70, 520, 30, 0, 100, 20, 80, Orientation::Horizontal);
        rangeSlider->setTooltip("Drag thumbs to set range");
        linkRangeLabel(*rangeSlider, *valueLabel, [](int lo, int hi) {
            return std::to_string(lo) + " - " + std::to_string(hi);
        });
        panel->addChild(std::move(rangeSlider));
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
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
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
