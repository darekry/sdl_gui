/**
 * @file 49_arc_sliders.cpp
 * @brief Horizontal sliders rotated by 90 degrees inside an ArcContainer.
 *
 * Three horizontal sliders are placed on an arc with rotateChild=true,
 * so each one is turned (the middle one by exactly 90 deg) and still
 * draggable. Together they mix the RGB color of the center preview panel.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "arc_container.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "ui_helpers.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Arc Sliders - RGB mixer", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        addLabel(guiManager, 10, 10,
                 "Sliders rotated on an arc: R (-50 deg), G (0 deg = vertical), B (+50 deg)", 16);

        auto [preview, previewRef] = addTracked(
            guiManager, std::make_unique<Panel>(guiManager, 330, 230, 140, 140));
        Style previewStyle;
        previewStyle.borderWidth = 2;
        previewStyle.borderRadius = 12;
        preview->setStyle(ElementState::Normal, previewStyle);

        auto [readout, readoutRef] = addTracked(
            guiManager, std::make_unique<Label>(guiManager, 315, 385, "", 18));

        auto arc = std::make_unique<ArcContainer>(guiManager, 400, 300, 250, -60, 60);

        // Shared refresh: read all three sliders, repaint preview + readout.
        auto sliderR = std::make_unique<Slider>(guiManager, 0, 0, 190, 40, 0, 255, 200, Orientation::Horizontal);
        auto sliderG = std::make_unique<Slider>(guiManager, 0, 0, 190, 40, 0, 255, 120, Orientation::Horizontal);
        auto sliderB = std::make_unique<Slider>(guiManager, 0, 0, 190, 40, 0, 255, 80, Orientation::Horizontal);
        auto refR = guiManager.makeRef(sliderR.get());
        auto refG = guiManager.makeRef(sliderG.get());
        auto refB = guiManager.makeRef(sliderB.get());

        std::function<void()> refresh = [refR, refG, refB, previewRef, readoutRef]() {
            if (!refR || !refG || !refB || !previewRef || !readoutRef) return;
            int r = refR->getValue();
            int g = refG->getValue();
            int b = refB->getValue();
            previewRef->setBackgroundColor(ElementState::Normal,
                {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), 255});
            readoutRef->setText("R " + std::to_string(r) + "  G " + std::to_string(g) +
                                "  B " + std::to_string(b));
        };
        onAnyChange({sliderR.get(), sliderG.get(), sliderB.get()}, refresh);

        arc->addChildAtAngle(std::move(sliderR), -50.0f, true);
        arc->addChildAtAngle(std::move(sliderG), 0.0f, true);
        arc->addChildAtAngle(std::move(sliderB), 50.0f, true);
        guiManager.addElement(std::move(arc));

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
