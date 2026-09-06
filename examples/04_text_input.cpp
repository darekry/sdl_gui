#include "gui.hpp"
#include "gui_manager.hpp"
#include "text_input.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("TextInput Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});

        // Apply default theme for consistent styling
        guiManager.setTheme(Theme::createDefaultTheme());

        // Panel with background, border and rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 200, 100, 400, 160);
        Style panelStyle;
        panelStyle.backgroundColor = {50, 52, 64, 255};
        panelStyle.borderColor = {98, 114, 164, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 10;
        panel->setStyle(ElementState::Normal, panelStyle);

        // Title label above the input
        auto title = std::make_unique<Label>(guiManager, 20, 20, "Text Input Example", 22);
        Style titleStyle;
        titleStyle.textColor = {255, 255, 255, 255};
        title->setStyle(ElementState::Normal, titleStyle);
        panel->addChild(std::move(title));

        // Text input with tooltip and enter-press callback
        auto textInput = std::make_unique<TextInput>(guiManager, 20, 60, 360, 40);
        textInput->setTooltip("Type here and press Enter");
        textInput->setOnEnterPressed([](TextInput* ti) {
            LOG_INFO("TextInput", "Text submitted: {}", ti->getText());
        });
        panel->addChild(std::move(textInput));

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
