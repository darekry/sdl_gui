#include "gui.hpp"
#include "gui_manager.hpp"
#include "checkbox.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

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
        auto panel = std::make_unique<Panel>(guiManager, 200, 80, 400, 250);
        Style panelStyle;
        panelStyle.backgroundColor = {50, 52, 64, 255};
        panelStyle.borderColor = {98, 114, 164, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 10;
        panel->setStyle(ElementState::Normal, panelStyle);
        // White text style shared by labels
        Style lightText;
        lightText.textColor = {255, 255, 255, 255};
        // Title label
        auto title = std::make_unique<Label>(guiManager, 20, 20, "Checkbox Example", 22);
        title->setStyle(ElementState::Normal, lightText);
        panel->addChild(std::move(title));

        // First checkbox with tooltip
        auto checkbox1 = std::make_unique<Checkbox>(guiManager, 20, 70, 24, 24);
        checkbox1->setTooltip("Toggle this option");
        checkbox1->setOnChange([](Checkbox*, bool isChecked) {
            LOG_INFO("Checkbox", "Checkbox 1 state: {}", isChecked ? "Checked" : "Unchecked");
        });
        panel->addChild(std::move(checkbox1));
        auto label1 = std::make_unique<Label>(guiManager, 55, 72, "Check me!", 16);
        label1->setStyle(ElementState::Normal, lightText);
        panel->addChild(std::move(label1));

        // Second checkbox with tooltip
        auto checkbox2 = std::make_unique<Checkbox>(guiManager, 20, 115, 24, 24);
        checkbox2->setTooltip("Toggle this option");
        checkbox2->setOnChange([](Checkbox*, bool isChecked) {
            LOG_INFO("Checkbox", "Checkbox 2 state: {}", isChecked ? "Checked" : "Unchecked");
        });
        panel->addChild(std::move(checkbox2));
        auto label2 = std::make_unique<Label>(guiManager, 55, 117, "Also check me", 16);
        label2->setStyle(ElementState::Normal, lightText);
        panel->addChild(std::move(label2));

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
