/**
 * @file 58_equalizer.cpp
 * @brief Five vertical sliders form a mixer; bars dance around them.
 *
 * Each channel pairs a vertical Slider with a vertical ProgressBar.
 * Per frame the bar shows slider value plus a small sine wobble, like a
 * VU meter idling - sliders stay the single source of truth.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "progress_bar.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Equalizer - sliders with dancing meters", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "Drag the thin sliders - the wide bars are live VU meters", 16);
        guiManager.addElement(std::move(info));

        constexpr int kChannels = 5;
        const int kInit[kChannels] = {70, 45, 85, 55, 60};

        std::vector<ElementRef<Slider>> sliders;
        std::vector<ElementRef<ProgressBar>> bars;
        sliders.reserve(kChannels);
        bars.reserve(kChannels);

        for (int i = 0; i < kChannels; ++i) {
            int x = 90 + i * 125;
            auto slider = std::make_unique<Slider>(guiManager, x, 80, 40, 340,
                                                   0, 100, kInit[i], Orientation::Vertical);
            slider->setTooltip("Channel " + std::to_string(i + 1));
            sliders.push_back(guiManager.makeRef(slider.get()));
            guiManager.addElement(std::move(slider));

            auto bar = std::make_unique<ProgressBar>(guiManager, x + 50, 80, 40, 340);
            bar->setOrientation(Orientation::Vertical);
            bar->setRange(0.0f, 100.0f);
            bar->setValue(static_cast<float>(kInit[i]));
            bars.push_back(guiManager.makeRef(bar.get()));
            guiManager.addElement(std::move(bar));

            auto cap = std::make_unique<Label>(guiManager, x + 12, 435, "CH" + std::to_string(i + 1), 16);
            guiManager.addElement(std::move(cap));
        }

        auto readout = std::make_unique<Label>(guiManager, 90, 480, "", 17);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            float t = static_cast<float>(frameStart) / 1000.0f;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            int sum = 0;
            for (size_t i = 0; i < sliders.size(); ++i) {
                if (!sliders[i] || !bars[i]) continue;
                float v = static_cast<float>(sliders[i]->getValue());
                sum += static_cast<int>(v);
                float fi = static_cast<float>(i);
                float wobble = std::sin(t * 3.0f + fi * 1.3f) * 5.0f * (0.2f + v / 100.0f);
                bars[i]->setValue(std::fmin(100.0f, std::fmax(0.0f, v + wobble)));
            }
            if (readoutRef) {
                readoutRef->setText("mix level: " + std::to_string(sum / kChannels) + " / 100");
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
