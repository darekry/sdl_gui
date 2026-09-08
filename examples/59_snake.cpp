/**
 * @file 59_snake.cpp
 * @brief Classic Snake on a grid of Panels, steered with arrows/WASD.
 *
 * The board is 20x14 Panels repainted only on logic steps (not every
 * frame). A slider sets steps-per-second, a button restarts. Shows how
 * to mix raw SDL keyboard input with guiManager.processEvent().
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "ui_helpers.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

namespace {

constexpr int kCols = 20, kRows = 14, kCell = 26;
constexpr int kOX = 30, kOY = 120;

const SDL_Color kBg{28, 30, 42, 255};
const SDL_Color kSnake{110, 200, 110, 255};
const SDL_Color kHead{170, 235, 170, 255};
const SDL_Color kFood{230, 90, 90, 255};

} // namespace

int main(int, char**) {
    try {
        SDLApp app("Snake - arrows/WASD, slider = speed", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto board = std::make_unique<Panel>(guiManager, kOX, kOY, kCols * kCell, kRows * kCell);
        Style boardStyle;
        boardStyle.backgroundColor = kBg;
        boardStyle.borderColor = {98, 114, 164, 255};
        boardStyle.borderWidth = 2;
        board->setStyle(ElementState::Normal, boardStyle);

        // Raw pointers: cells live exactly as long as the board, never deleted.
        auto cell = gridPanels(*board, kCols, kRows, kCell, kCell);
        for (int y = 0; y < kRows; ++y)
            for (int x = 0; x < kCols; ++x)
                cell[y][x]->setBackgroundColor(ElementState::Normal, kBg);
        guiManager.addElement(std::move(board));

        auto scoreLbl = std::make_unique<Label>(guiManager, kOX, 30, "score: 0", 20);
        auto statusLbl = std::make_unique<Label>(guiManager, kOX, 60, "arrows/WASD to steer", 16);
        auto scoreRef = guiManager.makeRef(scoreLbl.get());
        auto statusRef = guiManager.makeRef(statusLbl.get());
        guiManager.addElement(std::move(scoreLbl));
        guiManager.addElement(std::move(statusLbl));

        using Seg = std::pair<int, int>;
        std::deque<Seg> snake{{5, 7}, {4, 7}, {3, 7}};
        Seg dir{1, 0}, pending{1, 0};
        Seg food{12, 7};
        int score = 0;
        bool alive = true;
        int stepsPerSec = 8;

        auto repaint = [&]() {
            for (int y = 0; y < kRows; ++y)
                for (int x = 0; x < kCols; ++x)
                    cell[y][x]->setBackgroundColor(ElementState::Normal, kBg);
            for (const auto& s : snake)
                cell[s.second][s.first]->setBackgroundColor(ElementState::Normal, kSnake);
            cell[snake.front().second][snake.front().first]->setBackgroundColor(ElementState::Normal, kHead);
            cell[food.second][food.first]->setBackgroundColor(ElementState::Normal, kFood);
            if (scoreRef) scoreRef->setText("score: " + std::to_string(score));
        };

        auto reset = [&]() {
            snake = {{5, 7}, {4, 7}, {3, 7}};
            dir = pending = {1, 0};
            food = {12, 7};
            score = 0;
            alive = true;
            if (statusRef) statusRef->setText("arrows/WASD to steer");
            repaint();
        };

        auto speedSlider = std::make_unique<Slider>(guiManager, 580, 120, 180, 34,
                                                    3, 15, 8, Orientation::Horizontal);
        speedSlider->setTooltip("Steps per second");
        speedSlider->setOnChangeCallback([&stepsPerSec](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s) stepsPerSec = s->getValue();
        });
        guiManager.addElement(std::move(speedSlider));

        auto info = std::make_unique<Label>(guiManager, 580, 165, "speed (steps/s)", 15);
        guiManager.addElement(std::move(info));

        auto restartBtn = std::make_unique<Button>(guiManager, 580, 210, 180, 44, "Restart");
        restartBtn->setOnClickCallback([&reset](GUIElement*) { reset(); });
        guiManager.addElement(std::move(restartBtn));
        repaint();

        auto setDir = [&](int dx, int dy) {
            // No 180-degree turns.
            if (dx != -dir.first || dy != -dir.second) pending = {dx, dy};
        };

        Uint64 lastStep = SDL_GetTicks();
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                } else if (e.type == SDL_EVENT_KEY_DOWN) {
                    switch (e.key.key) {
                        case SDLK_UP: case SDLK_W: setDir(0, -1); break;
                        case SDLK_DOWN: case SDLK_S: setDir(0, 1); break;
                        case SDLK_LEFT: case SDLK_A: setDir(-1, 0); break;
                        case SDLK_RIGHT: case SDLK_D: setDir(1, 0); break;
                        default: break;
                    }
                }
                guiManager.processEvent(e);
            }
            Uint64 interval = 1000 / static_cast<Uint64>(stepsPerSec);
            if (alive && frameStart - lastStep >= interval) {
                lastStep = frameStart;
                dir = pending;
                Seg head{snake.front().first + dir.first, snake.front().second + dir.second};
                bool hitWall = head.first < 0 || head.first >= kCols || head.second < 0 || head.second >= kRows;
                bool hitSelf = false;
                for (const auto& s : snake)
                    if (s == head) { hitSelf = true; break; }
                if (hitWall || hitSelf) {
                    alive = false;
                    if (statusRef) statusRef->setText("GAME OVER - press Restart");
                } else {
                    snake.push_front(head);
                    if (head == food) {
                        score += 10;
                        do {
                            food = {rand() % kCols, rand() % kRows};
                            bool onSnake = false;
                            for (const auto& s : snake)
                                if (s == food) { onSnake = true; break; }
                            if (!onSnake) break;
                        } while (true);
                    } else {
                        snake.pop_back();
                    }
                }
                repaint();
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
