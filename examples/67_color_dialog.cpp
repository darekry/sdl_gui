/**
 * @file 67_color_dialog.cpp
 * @brief ColorDialog: RGB/HSV sliders with guard-synced HEX, previews and presets.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "composite/color_dialog.hpp"

#include "std.hpp"

static std::string toHex(SDL_Color c) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return std::string(buf, 7);
}

int main(int, char**) {
    try {
        SDLApp app("ColorDialog - pick a color", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        int ww = 0, wh = 0;
        app.getWindowSize(ww, wh);
        GUIManager guiManager(renderer, Viewport{ww, wh});
        guiManager.setTheme(Theme::createDefaultTheme());

        SDL_Color current{70, 130, 200, 255};

        auto swatch = std::make_unique<Panel>(guiManager, 300, 80, 200, 120);
        swatch->setBackgroundColor(ElementState::Normal, current);
        swatch->setBorder(ElementState::Normal, {98, 114, 164, 255}, 1);
        auto swatchRef = guiManager.makeRef(swatch.get());
        guiManager.addElement(std::move(swatch));

        auto info = std::make_unique<Label>(guiManager, 300, 210, toHex(current), 18);
        auto infoRef = guiManager.makeRef(info.get());
        guiManager.addElement(std::move(info));

        auto pick = std::make_unique<Button>(guiManager, 300, 250, 200, 40, "Pick color...");
        pick->setOnClickCallback([&guiManager, swatchRef, infoRef, &current](GUIElement*) {
            ColorDialog::create(guiManager, "Pick a color", current,
                [swatchRef, infoRef, &current](SDL_Color c) {
                    current = c;
                    if (swatchRef) swatchRef->setBackgroundColor(ElementState::Normal, c);
                    if (infoRef) infoRef->setText(toHex(c));
                });
        });
        guiManager.addElement(std::move(pick));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    break;
                }
                if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                    guiManager.handleResize(e.window.data1, e.window.data2);
                    continue;
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
