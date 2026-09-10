/**
 * @file 65_world_view.cpp
 * @brief WorldView camera: arrows/WASD pan a 400x300 view over an 800x600 world.
 *
 * Buttons live in WORLD coordinates (addWorldChild); the camera is a single
 * content shift with viewport clipping. The label shows cam + screenToWorld.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "world_view.hpp"
#include "button.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("WorldView - arrows/WASD to pan", 480, 400);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{480, 400});
        guiManager.setTheme(Theme::createDefaultTheme());

        WorldView* view = guiManager.create<WorldView>(40, 60, 400, 300);
        view->setWorldSize(800, 600);

        for (int gy = 0; gy < 6; ++gy) {
            for (int gx = 0; gx < 8; ++gx) {
                int wx = 20 + gx * 95;
                int wy = 20 + gy * 95;
                auto btn = std::make_unique<Button>(guiManager, wx, wy, 80, 40,
                    "w" + std::to_string(wx) + "," + std::to_string(wy));
                btn->setOnClickCallback([wx, wy](GUIElement*) {
                    std::cerr << "world click: " << wx << "," << wy << "\n";
                });
                view->addWorldChild(std::move(btn));
            }
        }

        auto info = std::make_unique<Label>(guiManager, 40, 10, "cam: 0,0", 16);
        auto infoRef = guiManager.makeRef(info.get());
        guiManager.addElement(std::move(info));

        auto refresh = [&]() {
            if (infoRef) {
                infoRef->setText("cam: " + std::to_string(view->getCamX()) +
                    "," + std::to_string(view->getCamY()));
            }
        };
        refresh();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
                    switch (e.key.key) {
                        case SDLK_LEFT: case SDLK_A: view->panBy(-32, 0); refresh(); break;
                        case SDLK_RIGHT: case SDLK_D: view->panBy(32, 0); refresh(); break;
                        case SDLK_UP: case SDLK_W: view->panBy(0, -32); refresh(); break;
                        case SDLK_DOWN: case SDLK_S: view->panBy(0, 32); refresh(); break;
                        default: break;
                    }
                }
                if (e.type == SDL_EVENT_MOUSE_MOTION) {
                    SDL_Point w = view->screenToWorld(
                        static_cast<int>(e.motion.x), static_cast<int>(e.motion.y));
                    if (infoRef) {
                        infoRef->setText("cam: " + std::to_string(view->getCamX()) +
                            "," + std::to_string(view->getCamY()) +
                            " mouse->world: " + std::to_string(w.x) + "," + std::to_string(w.y));
                    }
                }
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
