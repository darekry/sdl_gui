#include "gui_manager.hpp"
#include "radio_group.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("RadioButton Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        auto* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});

        // Apply the default dark theme instead of inline styles
        guiManager.setTheme(Theme::createDefaultTheme());

        // RadioGroup — a Panel that manages mutually exclusive RadioButtons
        auto radioGroup = std::make_unique<RadioGroup>(guiManager, 100, 100, 250, 200);
        radioGroup->setTooltip("Select one option");

        radioGroup->addOption("Option 1");
        radioGroup->addOption("Option 2", true);
        radioGroup->addOption("Option 3");

        // Log the selected option whenever it changes
        radioGroup->setOnSelectionChange([](int index, const std::string& text) {
            LOG_INFO("RadioButton", "Selected: {} (index {})", text, index);
        });

        guiManager.addElement(std::move(radioGroup));

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
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 44, 44, 44, 255);
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
