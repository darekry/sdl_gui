#include "gui_manager.hpp"
#include "button.hpp"
#include "canvas.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 700;

int main(int, char**) {
    try {
        SDLApp app("Mini Paint", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        guiManager.setTheme(Theme::createDefaultTheme());

        // Toolbar panel at the top for paint controls
        auto toolbar = std::make_unique<Panel>(guiManager, 10, 10, 880, 50);

        // Drawing canvas
        auto canvas = std::make_unique<Canvas>(guiManager, 20, 80, 640, 480);
        auto canvasRef = guiManager.makeRef(canvas.get());
        canvas->setTooltip("Draw with mouse");

        // Clear button — resets the canvas to white
        auto clearBtn = std::make_unique<Button>(guiManager, 10, 5, 100, 40, "Clear");
        clearBtn->setTooltip("Clear the canvas");
        clearBtn->setOnClickCallback([canvasRef](GUIElement*) {
            if (canvasRef) canvasRef->clear();
        });

        // Red button — switches pen color to red
        auto redBtn = std::make_unique<Button>(guiManager, 120, 5, 80, 40, "Red");
        redBtn->setTooltip("Change pen color to red");
        redBtn->setOnClickCallback([canvasRef](GUIElement*) {
            if (canvasRef) canvasRef->setPenColor({255, 0, 0, 255});
        });

        toolbar->addChild(std::move(clearBtn));
        toolbar->addChild(std::move(redBtn));
        guiManager.addElement(std::move(toolbar));
        guiManager.addElement(std::move(canvas));

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

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
