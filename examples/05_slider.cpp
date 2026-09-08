#include "gui.hpp"
#include "gui_manager.hpp"
#include "slider.hpp"
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
        SDLApp app("Slider Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());
        // Panel with border and rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 120, 80, 560, 210);
        styleCard(*panel);
        // White text style shared by labels
        const SDL_Color lightText{255, 255, 255, 255};
        // Title label
        addLabel(*panel, 20, 20, "Slider Example", 22, lightText);

        // Label showing current slider value
        Label* valueLabel = addLabel(*panel, 260, 120, "50", 18, lightText);

        // Slider with tooltip and change callback
        auto slider = std::make_unique<Slider>(guiManager, 20, 70, 520, 30, 0, 100, 50, Orientation::Horizontal);
        slider->setTooltip("Drag to change value");
        linkLabel(*slider, *valueLabel);
        panel->addChild(std::move(slider));
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
