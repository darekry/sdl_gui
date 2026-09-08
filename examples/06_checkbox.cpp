#include "gui.hpp"
#include "gui_manager.hpp"
#include "checkbox.hpp"
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
        SDLApp app("Checkbox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());
        // Panel with border and rounded corners
        Panel* panel = addDarkPanel(guiManager, 200, 80, 400, 250);
        // White text style shared by labels
        const SDL_Color lightText{255, 255, 255, 255};
        // Title label
        addLabel(*panel, 20, 20, "Checkbox Example", 22, lightText);

        // First checkbox with tooltip
        LabeledCheckbox cb1 = addLabeledCheckbox(*panel, 20, 70, "Check me!", 24, 16, lightText);
        cb1.box->setTooltip("Toggle this option");
        cb1.box->setOnChange([](Checkbox*, bool isChecked) {
            LOG_INFO("Checkbox", "Checkbox 1 state: {}", isChecked ? "Checked" : "Unchecked");
        });

        // Second checkbox with tooltip
        LabeledCheckbox cb2 = addLabeledCheckbox(*panel, 20, 115, "Also check me", 24, 16, lightText);
        cb2.box->setTooltip("Toggle this option");
        cb2.box->setOnChange([](Checkbox*, bool isChecked) {
            LOG_INFO("Checkbox", "Checkbox 2 state: {}", isChecked ? "Checked" : "Unchecked");
        });

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
