#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "canvas.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 700;

int main(int, char**) {
    try {
        SDLApp app("Mini Paint", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Canvas
        auto canvas = std::make_unique<Canvas>(guiManager, 20, 80, 640, 480);
        auto canvasRef = guiManager.makeRef(canvas.get());
        guiManager.addElement(std::move(canvas));

        // Clear button
        auto clearBtn = std::make_unique<Button>(guiManager, 20, 20, 100, 40, "Clear");
        clearBtn->setOnClickCallback([canvasRef](GUIElement*) {
            if (canvasRef) canvasRef->clear();
        });
        guiManager.addElement(std::move(clearBtn));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);

            guiManager.update();
            guiManager.render();
            guiManager.cleanup();

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}