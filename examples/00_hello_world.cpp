#include "panel.hpp"
#include "label.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**)
{
    try
    {
        SDLApp app("Hello World!", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());

        // Root panel centered on screen with rounded corners and visible border
        const int panel_w = 400;
        const int panel_h = 200;
        auto panel = std::make_unique<Panel>(
            guiManager,
            (SCREEN_WIDTH - panel_w) / 2,
            (SCREEN_HEIGHT - panel_h) / 2,
            panel_w, panel_h);

        Style panel_style;
        panel_style.backgroundColor = {240, 248, 255, 255};   // Alice blue
        panel_style.borderColor = {100, 149, 237, 255};        // Cornflower blue
        panel_style.borderWidth = 2;
        panel_style.borderRadius = 12;
        panel->setStyle(ElementState::Normal, panel_style);
        panel->setTooltip("This is a Panel with a Label inside");

        // Label centered inside the panel
        auto label = std::make_unique<Label>(guiManager, 0, 0, "Hello, World!", 48);
        int lw, lh;
        label->getSize(lw, lh);
        label->setPosition((panel_w - lw) / 2, (panel_h - lh) / 2);

        Style label_style;
        label_style.textColor = {25, 25, 112, 255};  // Midnight blue
        label->setStyle(ElementState::Normal, label_style);

        panel->addChild(std::move(label));
        guiManager.addElement(std::move(panel));

        bool quit = false;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
