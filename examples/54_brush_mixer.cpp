/**
 * @file 54_brush_mixer.cpp
 * @brief Three sliders mix the Canvas pen color live while you paint.
 *
 * R/G/B sliders recolor the pen (and a swatch preview) on every change;
 * the Canvas itself handles drag-painting. A button clears the painting.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "canvas.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Brush Mixer - sliders mix paint color", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto canvas = std::make_unique<Canvas>(guiManager, 20, 60, 540, 500);
        auto canvasRef = guiManager.makeRef(canvas.get());
        guiManager.addElement(std::move(canvas));

        auto info = std::make_unique<Label>(guiManager, 20, 20,
            "Paint with the mouse. Sliders mix the pen color live.", 16);
        guiManager.addElement(std::move(info));

        auto swatch = std::make_unique<Panel>(guiManager, 580, 380, 200, 70);
        auto swatchRef = guiManager.makeRef(swatch.get());
        guiManager.addElement(std::move(swatch));

        auto readout = std::make_unique<Label>(guiManager, 580, 460, "", 16);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        auto sliderR = std::make_unique<Slider>(guiManager, 580, 80, 200, 32, 0, 255, 220, Orientation::Horizontal);
        auto sliderG = std::make_unique<Slider>(guiManager, 580, 180, 200, 32, 0, 255, 90, Orientation::Horizontal);
        auto sliderB = std::make_unique<Slider>(guiManager, 580, 280, 200, 32, 0, 255, 90, Orientation::Horizontal);
        auto refR = guiManager.makeRef(sliderR.get());
        auto refG = guiManager.makeRef(sliderG.get());
        auto refB = guiManager.makeRef(sliderB.get());

        auto chanLabel = [](const char* name, int y, GUIManager& m) {
            auto l = std::make_unique<Label>(m, 580, y, name, 16);
            m.addElement(std::move(l));
        };
        chanLabel("Red", 60, guiManager);
        chanLabel("Green", 160, guiManager);
        chanLabel("Blue", 260, guiManager);

        std::function<void()> refresh = [refR, refG, refB, canvasRef, swatchRef, readoutRef]() {
            if (!refR || !refG || !refB) return;
            SDL_Color c{static_cast<Uint8>(refR->getValue()),
                        static_cast<Uint8>(refG->getValue()),
                        static_cast<Uint8>(refB->getValue()), 255};
            if (canvasRef) canvasRef->setPenColor(c);
            if (swatchRef) swatchRef->setBackgroundColor(ElementState::Normal, c);
            if (readoutRef) {
                readoutRef->setText("#" + std::to_string(refR->getValue()) + "." +
                                    std::to_string(refG->getValue()) + "." +
                                    std::to_string(refB->getValue()));
            }
        };
        sliderR->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
        sliderG->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
        sliderB->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
        guiManager.addElement(std::move(sliderR));
        guiManager.addElement(std::move(sliderG));
        guiManager.addElement(std::move(sliderB));

        auto clearBtn = std::make_unique<Button>(guiManager, 580, 505, 200, 44, "Clear");
        clearBtn->setOnClickCallback([canvasRef](GUIElement*) {
            if (canvasRef) canvasRef->clear();
        });
        guiManager.addElement(std::move(clearBtn));
        refresh();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
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
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
