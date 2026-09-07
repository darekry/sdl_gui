/**
 * @file 53_smooth_follow.cpp
 * @brief Slider sets a target, a marker eases toward it every frame.
 *
 * Demonstrates the "smooth follow" pattern without AnimationManager:
 * current += (target - current) * min(1, dt * speed). A second slider
 * controls follow speed, a ProgressBar mirrors the eased value.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "progress_bar.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Smooth Follow - eased slider follower", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        constexpr int kTrackX = 110, kTrackW = 580, kMarkerW = 16;

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "Top slider = target. Marker + bar ease toward it (speed slider = responsiveness)", 16);
        guiManager.addElement(std::move(info));

        auto track = std::make_unique<Panel>(guiManager, kTrackX, 220, kTrackW, 20);
        Style trackStyle;
        trackStyle.backgroundColor = {28, 30, 42, 255};
        trackStyle.borderWidth = 1;
        trackStyle.borderRadius = 10;
        track->setStyle(ElementState::Normal, trackStyle);
        guiManager.addElement(std::move(track));

        auto marker = std::make_unique<Panel>(guiManager, kTrackX, 212, kMarkerW, 36);
        marker->setBackgroundColor(ElementState::Normal, {220, 160, 80, 255});
        Style markerStyle;
        markerStyle.borderRadius = 8;
        marker->setStyle(ElementState::Normal, markerStyle);
        auto markerRef = guiManager.makeRef(marker.get());
        guiManager.addElement(std::move(marker));

        auto bar = std::make_unique<ProgressBar>(guiManager, kTrackX, 300, kTrackW, 30);
        bar->setRange(0.0f, 100.0f);
        auto barRef = guiManager.makeRef(bar.get());
        guiManager.addElement(std::move(bar));

        auto readout = std::make_unique<Label>(guiManager, kTrackX, 350, "", 18);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        float target = 30.0f;
        float current = 30.0f;
        float speed = 4.0f;

        auto targetSlider = std::make_unique<Slider>(guiManager, kTrackX, 120, kTrackW, 36,
                                                     0, 100, 30, Orientation::Horizontal);
        targetSlider->setTooltip("Target value");
        targetSlider->setOnChangeCallback([&target](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s) target = static_cast<float>(s->getValue());
        });
        guiManager.addElement(std::move(targetSlider));

        auto speedSlider = std::make_unique<Slider>(guiManager, kTrackX, 420, kTrackW, 36,
                                                    1, 12, 4, Orientation::Horizontal);
        speedSlider->setTooltip("Follow speed");
        speedSlider->setOnChangeCallback([&speed](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s) speed = static_cast<float>(s->getValue());
        });
        guiManager.addElement(std::move(speedSlider));

        Uint64 last = SDL_GetTicks();
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            float dt = static_cast<float>(frameStart - last) / 1000.0f;
            last = frameStart;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            float diff = target - current;
            current += diff * std::min(1.0f, dt * speed);
            if (std::fabs(target - current) < 0.02f) current = target;
            if (markerRef) {
                int mx = kTrackX + static_cast<int>(current / 100.0f * (kTrackW - kMarkerW));
                markerRef->setPosition(mx, markerRef->getY());
            }
            if (barRef) barRef->setValue(current);
            if (readoutRef) {
                readoutRef->setText("target=" + std::to_string(static_cast<int>(target)) +
                                    "  eased=" + std::to_string(static_cast<int>(current)));
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
