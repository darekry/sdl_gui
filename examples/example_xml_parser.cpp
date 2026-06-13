#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "sgml_parser.hpp"

import std.compat;

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path_to_xml_layout>" << std::endl;
        return 1;
    }

    SDLApp app("XML Parser Example", 800, 600);
    GUIManager guiManager(app.getRenderer());
    SGMLParser parser(guiManager);

    std::string layout_file = argv[1];
    auto rootElement = parser.loadLayout(layout_file);

    if (!rootElement)
    {
        std::cerr << "Failed to load layout from: " << layout_file << std::endl;
        return 1;
    }

    guiManager.addElement(std::move(rootElement));

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            guiManager.processEvent(event);
        }
        guiManager.update();
        guiManager.cleanup();
        SDL_SetRenderDrawColor(app.getRenderer(), 255, 255, 255, 255);
        SDL_RenderClear(app.getRenderer());

        guiManager.render();

        SDL_RenderPresent(app.getRenderer());
    }

    return 0;
}