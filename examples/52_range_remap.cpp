/**
 * @file 52_range_remap.cpp
 * @brief RangeSlider defines a live sub-range for a plain Slider.
 *
 * The RangeSlider picks [lo, hi]; the lower slider outputs 0..100 which is
 * remapped into that window. A ProgressBar visualizes the mapped result,
 * so dragging either control updates all three widgets at once.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "range_slider.hpp"
#include "progress_bar.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Range Remap - range slider drives a slider", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "Top: active window [lo,hi]. Middle: input 0..100. Bottom: mapped output", 16);
        guiManager.addElement(std::move(info));

        auto panel = std::make_unique<Panel>(guiManager, 60, 50, 680, 420);
        Style panelStyle;
        panelStyle.backgroundColor = {50, 52, 64, 255};
        panelStyle.borderColor = {98, 114, 164, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 10;
        panel->setStyle(ElementState::Normal, panelStyle);
        guiManager.addElement(std::move(panel));

        auto rangeLbl = std::make_unique<Label>(guiManager, 90, 70, "", 17);
        auto inputLbl = std::make_unique<Label>(guiManager, 90, 190, "", 17);
        auto mapLbl = std::make_unique<Label>(guiManager, 90, 310, "", 17);
        auto rangeLblRef = guiManager.makeRef(rangeLbl.get());
        auto inputLblRef = guiManager.makeRef(inputLbl.get());
        auto mapLblRef = guiManager.makeRef(mapLbl.get());
        guiManager.addElement(std::move(rangeLbl));
        guiManager.addElement(std::move(inputLbl));
        guiManager.addElement(std::move(mapLbl));

        auto range = std::make_unique<RangeSlider>(guiManager, 90, 100, 620, 44,
                                                   0, 100, 20, 80, Orientation::Horizontal);
        auto input = std::make_unique<Slider>(guiManager, 90, 220, 620, 44,
                                              0, 100, 50, Orientation::Horizontal);
        auto rangeRef = guiManager.makeRef(range.get());
        auto inputRef = guiManager.makeRef(input.get());

        auto output = std::make_unique<ProgressBar>(guiManager, 90, 340, 620, 36);
        output->setRange(0.0f, 100.0f);
        auto outputRef = guiManager.makeRef(output.get());
        guiManager.addElement(std::move(output));

        std::function<void()> refresh = [rangeRef, inputRef, outputRef,
                                         rangeLblRef, inputLblRef, mapLblRef]() {
            if (!rangeRef || !inputRef || !outputRef) return;
            int lo = rangeRef->getLowerValue();
            int hi = rangeRef->getUpperValue();
            int v = inputRef->getValue();
            float mapped = lo + (hi - lo) * (v / 100.0f);
            outputRef->setValue(mapped);
            if (rangeLblRef) rangeLblRef->setText("window: [" + std::to_string(lo) + ", " + std::to_string(hi) + "]");
            if (inputLblRef) inputLblRef->setText("input: " + std::to_string(v));
            if (mapLblRef) mapLblRef->setText("mapped: " + std::to_string(static_cast<int>(mapped)));
        };
        range->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
        input->setOnChangeCallback([refresh](GUIElement*) { refresh(); });
        guiManager.addElement(std::move(range));
        guiManager.addElement(std::move(input));
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
