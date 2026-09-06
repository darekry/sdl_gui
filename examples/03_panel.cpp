#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
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
        SDLApp app("Panel Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());

        const int panel_w = 400;
        const int panel_h = 260;
        auto panel = std::make_unique<Panel>(
            guiManager,
            (SCREEN_WIDTH - panel_w) / 2,
            (SCREEN_HEIGHT - panel_h) / 2,
            panel_w, panel_h);
        Style panel_style;
        panel_style.backgroundColor = {224, 255, 255, 255};
        panel_style.borderColor = {72, 209, 204, 255};
        panel_style.borderWidth = 3;
        panel_style.borderRadius = 8;
        panel->setStyle(ElementState::Normal, panel_style);
        panel->setTooltip("Panels can contain other widgets");

        auto title = std::make_unique<Label>(
            guiManager, 20, 20, "Panel with Children", 28);
        Style title_style;
        title_style.textColor = {0, 105, 92, 255};
        title->setStyle(ElementState::Normal, title_style);
        panel->addChild(std::move(title));

        auto button = std::make_unique<Button>(
            guiManager, 100, 100, 200, 50, "Click Me");
        button->setTooltip("Click this button");
        button->setOnClickCallback(
            [](GUIElement*) {
                LOG_INFO("PanelExample", "Button clicked inside the panel!");
            });

        panel->addChild(std::move(button));
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

            SDL_SetRenderDrawColor(renderer, 250, 250, 250, 255);
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
