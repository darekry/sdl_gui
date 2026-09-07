/**
 * @file 61_breakout.cpp
 * @brief Breakout: mouse steers the paddle, ball physics runs per frame.
 *
 * Bricks are Panels hidden via setVisible(false) on hit (no deletion, so
 * raw pointers stay valid). A slider sets ball speed, arrows nudge the
 * paddle as a keyboard alternative.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

namespace {

constexpr int kColsB = 9, kRowsB = 4;
constexpr int kBrickW = 80, kBrickH = 24, kGap = 6;
constexpr int kBX0 = 30, kBY0 = 90;

struct Brick {
    Panel* panel = nullptr;
    bool alive = true;
};

} // namespace

int main(int, char**) {
    try {
        SDLApp app("Breakout - mouse steers, slider = ball speed", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        std::vector<Brick> bricks;
        const SDL_Color rowColors[kRowsB] = {{220, 100, 100, 255}, {220, 160, 80, 255},
                                             {120, 200, 120, 255}, {100, 150, 250, 255}};
        for (int r = 0; r < kRowsB; ++r) {
            for (int c = 0; c < kColsB; ++c) {
                auto b = std::make_unique<Panel>(guiManager, kBX0 + c * (kBrickW + kGap),
                                                 kBY0 + r * (kBrickH + kGap), kBrickW, kBrickH);
                b->setBackgroundColor(ElementState::Normal, rowColors[r]);
                bricks.push_back({b.get(), true});
                guiManager.addElement(std::move(b));
            }
        }

        constexpr int kPaddleW = 110, kPaddleH = 16, kPaddleY = 540;
        auto paddle = std::make_unique<Panel>(guiManager, 345, kPaddleY, kPaddleW, kPaddleH);
        paddle->setBackgroundColor(ElementState::Normal, {240, 240, 240, 255});
        auto paddleRef = guiManager.makeRef(paddle.get());
        guiManager.addElement(std::move(paddle));

        constexpr int kBallS = 14;
        auto ball = std::make_unique<Panel>(guiManager, 393, 300, kBallS, kBallS);
        ball->setBackgroundColor(ElementState::Normal, {250, 220, 120, 255});
        Style ballStyle;
        ballStyle.borderRadius = kBallS / 2;
        ball->setStyle(ElementState::Normal, ballStyle);
        auto ballRef = guiManager.makeRef(ball.get());
        guiManager.addElement(std::move(ball));

        auto scoreLbl = std::make_unique<Label>(guiManager, kBX0, 30, "score: 0   lives: 3", 20);
        auto statusLbl = std::make_unique<Label>(guiManager, kBX0, 55, "move mouse to steer", 15);
        auto scoreRef = guiManager.makeRef(scoreLbl.get());
        auto statusRef = guiManager.makeRef(statusLbl.get());
        guiManager.addElement(std::move(scoreLbl));
        guiManager.addElement(std::move(statusLbl));

        float ballSpeed = 380.0f;
        float bx = 393.0f, by = 300.0f, vx = 200.0f, vy = -280.0f;
        int score = 0, lives = 3;
        bool playing = true;

        auto resetBall = [&]() {
            bx = 393.0f;
            by = 300.0f;
            vx = ballSpeed * 0.5f;
            vy = -ballSpeed * 0.7f;
        };
        auto refreshHud = [&]() {
            if (scoreRef) scoreRef->setText("score: " + std::to_string(score) +
                                            "   lives: " + std::to_string(lives));
        };

        auto speedSlider = std::make_unique<Slider>(guiManager, 600, 300, 160, 34,
                                                    200, 700, 380, Orientation::Horizontal);
        speedSlider->setTooltip("Ball speed (px/s)");
        speedSlider->setOnChangeCallback([&](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            float len = std::sqrt(vx * vx + vy * vy);
            ballSpeed = static_cast<float>(s->getValue());
            if (len > 1.0f) {
                vx = vx / len * ballSpeed;
                vy = vy / len * ballSpeed;
            }
        });
        guiManager.addElement(std::move(speedSlider));

        auto restartBtn = std::make_unique<Button>(guiManager, 600, 360, 160, 44, "Restart");
        restartBtn->setOnClickCallback([&](GUIElement*) {
            for (auto& b : bricks) {
                b.alive = true;
                b.panel->setVisible(true);
            }
            score = 0;
            lives = 3;
            playing = true;
            resetBall();
            refreshHud();
            if (statusRef) statusRef->setText("move mouse to steer");
        });
        guiManager.addElement(std::move(restartBtn));
        refreshHud();

        auto clampPaddle = [&](int x) {
            if (x < 10) x = 10;
            if (x > 790 - kPaddleW) x = 790 - kPaddleW;
            return x;
        };

        Uint64 last = SDL_GetTicks();
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            float dt = static_cast<float>(frameStart - last) / 1000.0f;
            last = frameStart;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
                    if (paddleRef) paddleRef->setPosition(clampPaddle(e.motion.x - kPaddleW / 2), kPaddleY);
                } else if (e.type == SDL_EVENT_KEY_DOWN && paddleRef) {
                    int px = paddleRef->getX();
                    if (e.key.key == SDLK_LEFT || e.key.key == SDLK_A) px -= 24;
                    if (e.key.key == SDLK_RIGHT || e.key.key == SDLK_D) px += 24;
                    paddleRef->setPosition(clampPaddle(px), kPaddleY);
                }
                guiManager.processEvent(e);
            }
            if (playing && paddleRef && ballRef) {
                bx += vx * dt;
                by += vy * dt;
                if (bx <= 10) { bx = 10; vx = std::fabs(vx); }
                if (bx + kBallS >= 790) { bx = 790 - kBallS; vx = -std::fabs(vx); }
                if (by <= 10) { by = 10; vy = std::fabs(vy); }
                int px = paddleRef->getX();
                if (vy > 0 && by + kBallS >= kPaddleY && by + kBallS <= kPaddleY + kPaddleH + 8 &&
                    bx + kBallS >= px && bx <= px + kPaddleW) {
                    float off = (bx + kBallS / 2.0f - (px + kPaddleW / 2.0f)) / (kPaddleW / 2.0f);
                    float len = std::sqrt(vx * vx + vy * vy);
                    vx = off * len * 0.8f;
                    vy = -std::sqrt(std::fmax(1.0f, len * len - vx * vx));
                    by = kPaddleY - kBallS;
                }
                for (auto& b : bricks) {
                    if (!b.alive) continue;
                    int rx = b.panel->getX(), ry = b.panel->getY();
                    if (bx + kBallS >= rx && bx <= rx + kBrickW && by + kBallS >= ry && by <= ry + kBrickH) {
                        b.alive = false;
                        b.panel->setVisible(false);
                        vy = -vy;
                        score += 10;
                        refreshHud();
                        break;
                    }
                }
                if (by > 600) {
                    if (--lives <= 0) {
                        playing = false;
                        if (statusRef) statusRef->setText("GAME OVER - press Restart");
                        refreshHud();
                    } else {
                        resetBall();
                        refreshHud();
                    }
                }
                bool cleared = true;
                for (const auto& b : bricks)
                    if (b.alive) { cleared = false; break; }
                if (cleared && playing) {
                    playing = false;
                    if (statusRef) statusRef->setText("YOU WIN! Press Restart");
                }
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
