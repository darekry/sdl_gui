#include "gui_manager.hpp"
#include "text_area.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("TextArea Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());

        // Panel with background, border and rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 50, 50, 700, 500);
        Style panelStyle;
        panelStyle.backgroundColor = {40, 42, 54, 255};
        panelStyle.borderColor = {98, 114, 164, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 10;
        panel->setStyle(ElementState::Normal, panelStyle);

        // Title label
        auto title = std::make_unique<Label>(guiManager, 20, 15, "TextArea Example", 22);
        Style titleStyle;
        titleStyle.textColor = {255, 255, 255, 255};
        title->setStyle(ElementState::Normal, titleStyle);
        panel->addChild(std::move(title));

        // Multi-line text area with default content and tooltip
        auto textArea = std::make_unique<TextArea>(guiManager, 20, 50, 660, 430, "assets/fonts/font.ttf", 18);
        textArea->setText("This is a multi-line text editing area.\n"
                          "You can type, edit, select, and scroll through text.\n"
                          "Word wrap is enabled by default.\n\n"
                          "Try typing here to see how text input works.");
        textArea->setTooltip("Multi-line text editing area");
        panel->addChild(std::move(textArea));
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
