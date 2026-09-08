/**
 * @file 55_gravity_box.cpp
 * @brief Vertical slider = gravity, second slider = bounce damping.
 *
 * A ball falls inside a box with hand-rolled physics in the main loop.
 * Drop/Reset restarts the fall; a readout shows height and velocity.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "ui_helpers.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Gravity Box - sliders tune physics", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        constexpr int kX0 = 30, kY0 = 60, kX1 = 550, kY1 = 530;
        constexpr int kBall = 26;
        const float kFloor = static_cast<float>(kY1 - kBall);
        const float kBallX = (kX0 + kX1 - kBall) / 2.0f;

        auto box = std::make_unique<Panel>(guiManager, kX0, kY0, kX1 - kX0, kY1 - kY0);
        Style boxStyle;
        boxStyle.backgroundColor = {28, 30, 42, 255};
        boxStyle.borderColor = {98, 114, 164, 255};
        boxStyle.borderWidth = 2;
        boxStyle.borderRadius = 8;
        box->setStyle(ElementState::Normal, boxStyle);
        guiManager.addElement(std::move(box));

        auto [ball, ballRef] = addTracked(
            guiManager, std::make_unique<Panel>(guiManager, static_cast<int>(kBallX), kY0 + 10, kBall, kBall));
        ball->setBackgroundColor(ElementState::Normal, {140, 170, 240, 255});
        Style ballStyle;
        ballStyle.borderRadius = kBall / 2;
        ball->setStyle(ElementState::Normal, ballStyle);

        float gravity = 18.0f; // slider units -> px/s^2 via kScale
        float damping = 0.85f; // fraction of velocity kept per bounce
        float y = kY0 + 10.0f;
        float vy = 0.0f;
        constexpr float kScale = 30.0f;

        auto [status, statusRef] = addTracked(
            guiManager, std::make_unique<Label>(guiManager, kX0, 20, "", 16));

        auto gravSlider = std::make_unique<Slider>(guiManager, 590, 60, 36, 320,
                                                   0, 40, 18, Orientation::Vertical);
        gravSlider->setTooltip("Gravity");
        bindSliderValue(*gravSlider, gravity);
        guiManager.addElement(std::move(gravSlider));

        auto dampSlider = std::make_unique<Slider>(guiManager, 660, 60, 36, 320,
                                                   40, 99, 85, Orientation::Vertical);
        dampSlider->setTooltip("Bounce kept (%)");
        bindSliderValue(*dampSlider, damping, 0.01f);
        guiManager.addElement(std::move(dampSlider));

        addLabel(guiManager, 580, 390, "Gravity", 15);
        addLabel(guiManager, 650, 390, "Bounce", 15);

        addButton(guiManager, 580, 440, 150, 44, "Drop", [&y, &vy, kY0](GUIElement*) {
            y = kY0 + 10.0f;
            vy = 0.0f;
        });

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
            vy += gravity * kScale * dt;
            y += vy * dt;
            if (y >= kFloor) {
                y = kFloor;
                vy = -vy * damping;
                if (std::fabs(vy) < 25.0f) vy = 0.0f; // rest on the floor
            }
            if (ballRef) ballRef->setPosition(static_cast<int>(kBallX), static_cast<int>(y));
            if (statusRef) {
                statusRef->setText("h=" + std::to_string(static_cast<int>(kFloor - y)) +
                                   "px  v=" + std::to_string(static_cast<int>(vy)) + "px/s");
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
