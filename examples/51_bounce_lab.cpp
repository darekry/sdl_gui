/**
 * @file 51_bounce_lab.cpp
 * @brief Sliders steer a manual per-frame animation: speed and ball size.
 *
 * A ball bounces inside a box using hand-rolled integration in the main
 * loop (dt from SDL_GetTicks). One slider sets velocity in px/s, another
 * resizes the ball live, a button pauses/resumes.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "ui_helpers.hpp"
#include "button.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Bounce Lab - sliders drive animation", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        constexpr int kX0 = 20, kY0 = 60, kX1 = 660, kY1 = 480;

        auto box = std::make_unique<Panel>(guiManager, kX0, kY0, kX1 - kX0, kY1 - kY0);
        Style boxStyle;
        boxStyle.backgroundColor = {28, 30, 42, 255};
        boxStyle.borderColor = {98, 114, 164, 255};
        boxStyle.borderWidth = 2;
        boxStyle.borderRadius = 8;
        box->setStyle(ElementState::Normal, boxStyle);
        guiManager.addElement(std::move(box));

        auto [ball, ballRef] = addTracked(
            guiManager, std::make_unique<Panel>(guiManager, 100, 150, 24, 24));
        ball->setBackgroundColor(ElementState::Normal, {120, 200, 120, 255});
        Style ballStyle;
        ballStyle.borderRadius = 12;
        ball->setStyle(ElementState::Normal, ballStyle);

        float bx = 100.0f, by = 150.0f;
        float vx = 160.0f, vy = 120.0f;
        float speed = 220.0f; // px/s, scaled to keep direction
        int ballSize = 24;
        bool paused = false;

        auto [status, statusRef] = addTracked(
            guiManager, std::make_unique<Label>(guiManager, 20, 20, "", 16));

        auto refreshStatus = [&]() {
            if (!statusRef) return;
            statusRef->setText(std::string(paused ? "PAUSED  " : "running  ") +
                               "speed=" + std::to_string(static_cast<int>(speed)) +
                               "px/s  size=" + std::to_string(ballSize));
        };

        auto speedSlider = std::make_unique<Slider>(guiManager, 690, 60, 36, 300,
                                                    0, 600, 220, Orientation::Vertical);
        speedSlider->setTooltip("Ball speed (px/s)");
        speedSlider->setOnChangeCallback([&speed, &vx, &vy, refreshStatus](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            float len = std::sqrt(vx * vx + vy * vy);
            speed = static_cast<float>(s->getValue());
            if (len > 0.001f) {
                vx = vx / len * speed;
                vy = vy / len * speed;
            } else {
                vx = speed;
            }
            refreshStatus();
        });
        guiManager.addElement(std::move(speedSlider));

        auto sizeSlider = std::make_unique<Slider>(guiManager, 690, 380, 36, 100,
                                                   8, 48, 24, Orientation::Vertical);
        sizeSlider->setTooltip("Ball size");
        sizeSlider->setOnChangeCallback([&ballSize, ballRef, refreshStatus](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            ballSize = s->getValue();
            if (ballRef) {
                ballRef->setSize(ballSize, ballSize);
                Style round;
                round.borderRadius = ballSize / 2;
                ballRef->setStyle(ElementState::Normal, round);
            }
            refreshStatus();
        });
        guiManager.addElement(std::move(sizeSlider));

        auto pauseBtn = std::make_unique<Button>(guiManager, 20, 500, 140, 40, "Pause");
        pauseBtn->setOnClickCallback([&paused, refreshStatus](GUIElement*) {
            paused = !paused;
            refreshStatus();
        });
        guiManager.addElement(std::move(pauseBtn));
        refreshStatus();

        // Main loop: fixed integration step scaled by real dt.
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
            if (!paused && ballRef) {
                bx += vx * dt;
                by += vy * dt;
                if (bx <= kX0) { bx = kX0; vx = std::fabs(vx); }
                if (bx + ballSize >= kX1) { bx = kX1 - ballSize; vx = -std::fabs(vx); }
                if (by <= kY0) { by = kY0; vy = std::fabs(vy); }
                if (by + ballSize >= kY1) { by = kY1 - ballSize; vy = -std::fabs(vy); }
                ballRef->setPosition(static_cast<int>(bx), static_cast<int>(by));
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
