#include "label.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char **)
{
    try
    {
        SDLApp app("Hello World!", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer * renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto label = std::make_unique<Label>(guiManager, 150, 270, "Hello, World!", 48);
        guiManager.addElement(std::move(label));

        bool quit = false;
        SDL_Event e;
        while (!quit)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }
    }
    catch (const std::runtime_error & e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
