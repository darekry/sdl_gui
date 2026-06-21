#include "gui_manager.hpp"
#include "combobox.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("ComboBox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        guiManager.setTheme(Theme::createDefaultTheme());

        // Panel container with border and rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 80, 60, 300, 180);
        panel->setBorder(ElementState::Normal, {100, 100, 100, 255}, 1);
        panel->setBorderRadius(ElementState::Normal, 8);

        // Title label above the ComboBox
        auto title = std::make_unique<Label>(guiManager, 20, 15, "Select an option:", 18);
        panel->addChild(std::move(title));

        // ComboBox inside the panel
        auto comboBox = std::make_unique<ComboBox>(guiManager, 20, 50, 220, 30);
        comboBox->addItem("Option 1");
        comboBox->addItem("Option 2");
        comboBox->addItem("Option 3");
        comboBox->addItem("A longer option 4");
        comboBox->setSelectedIndex(0);
        comboBox->setTooltip("Select an option from the list");

        comboBox->on_selection_changed = [](int index, const std::string& item) {
            LOG_INFO("ComboBox", "Selected: {} (index {})", item, index);
        };

        panel->addChild(std::move(comboBox));
        guiManager.addElement(std::move(panel));

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

            SDL_SetRenderDrawColor(renderer, 230, 230, 240, 255);
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
