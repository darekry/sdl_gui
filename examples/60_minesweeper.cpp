/**
 * @file 60_minesweeper.cpp
 * @brief Minesweeper on a 9x9 grid of Buttons with Label faces.
 *
 * Left click reveals (with flood fill on empty cells), right click flags
 * via setOnRightClickCallback(). Each cell is a Button plus a child Label
 * (Button has no setText, so the face is a separate widget).
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

namespace {

constexpr int kN = 9, kMines = 10, kCell = 40;
constexpr int kOX = 30, kOY = 90;

struct Cell {
    Button* btn = nullptr;
    Label* face = nullptr;
    bool mine = false;
    bool revealed = false;
    bool flagged = false;
    int adj = 0;
};

SDL_Color numberColor(int n) {
    switch (n) {
        case 1: return {100, 150, 250, 255};
        case 2: return {110, 200, 110, 255};
        case 3: return {230, 90, 90, 255};
        default: return {200, 170, 90, 255};
    }
}

} // namespace

int main(int, char**) {
    try {
        SDLApp app("Minesweeper - click to reveal, right-click to flag", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto status = std::make_unique<Label>(guiManager, kOX, 25, "", 20);
        auto statusRef = guiManager.makeRef(status.get());
        guiManager.addElement(std::move(status));

        auto field = std::make_unique<Panel>(guiManager, kOX, kOY, kN * kCell, kN * kCell);
        Style fieldStyle;
        fieldStyle.backgroundColor = {28, 30, 42, 255};
        fieldStyle.borderColor = {98, 114, 164, 255};
        fieldStyle.borderWidth = 2;
        field->setStyle(ElementState::Normal, fieldStyle);

        std::vector<std::vector<Cell>> grid(kN, std::vector<Cell>(kN));
        bool gameOver = false;
        int revealedCount = 0;

        std::function<void()> updateStatus;
        std::function<void(int, int)> reveal;

        updateStatus = [&]() {
            if (!statusRef) return;
            if (gameOver) return; // terminal message already set
            int flags = 0;
            for (const auto& row : grid)
                for (const auto& c : row)
                    if (c.flagged) ++flags;
            statusRef->setText("mines: " + std::to_string(kMines - flags) +
                               "   revealed: " + std::to_string(revealedCount) +
                               "/" + std::to_string(kN * kN - kMines));
        };

        reveal = [&](int x, int y) {
            if (x < 0 || x >= kN || y < 0 || y >= kN) return;
            Cell& c = grid[y][x];
            if (c.revealed || c.flagged || gameOver) return;
            c.revealed = true;
            ++revealedCount;
            c.btn->setEnabled(false);
            if (c.mine) {
                gameOver = true;
                c.face->setText("*");
                for (auto& row : grid)
                    for (auto& m : row)
                        if (m.mine && !m.revealed) {
                            m.revealed = true;
                            m.face->setText("*");
                            m.btn->setEnabled(false);
                        }
                if (statusRef) statusRef->setText("BOOM! Press New Game");
                return;
            }
            if (c.adj > 0) {
                c.face->setText(std::to_string(c.adj));
                Style s;
                s.textColor = numberColor(c.adj);
                c.face->setStyle(ElementState::Normal, s);
            } else {
                c.face->setText("");
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx)
                        if (dx || dy) reveal(x + dx, y + dy);
            }
            if (revealedCount == kN * kN - kMines) {
                gameOver = true;
                if (statusRef) statusRef->setText("YOU WIN! Press New Game");
                return;
            }
            updateStatus();
        };

        for (int y = 0; y < kN; ++y) {
            for (int x = 0; x < kN; ++x) {
                auto btn = std::make_unique<Button>(guiManager, x * kCell + 1, y * kCell + 1,
                                                    kCell - 2, kCell - 2, "");
                auto face = std::make_unique<Label>(guiManager, 12, 6, "", 20);
                grid[y][x].face = face.get();
                btn->addChild(std::move(face));
                btn->setOnClickCallback([x, y, &reveal](GUIElement*) { reveal(x, y); });
                btn->setOnRightClickCallback([x, y, &grid, &gameOver, &updateStatus](GUIElement*, int, int) {
                    if (gameOver) return;
                    Cell& c = grid[y][x];
                    if (c.revealed) return;
                    c.flagged = !c.flagged;
                    c.face->setText(c.flagged ? "F" : "");
                    if (c.flagged) {
                        Style s;
                        s.textColor = {230, 90, 90, 255};
                        c.face->setStyle(ElementState::Normal, s);
                    }
                    updateStatus();
                });
                grid[y][x].btn = btn.get();
                field->addChild(std::move(btn));
            }
        }
        guiManager.addElement(std::move(field));

        auto plant = [&]() {
            for (auto& row : grid)
                for (auto& c : row) {
                    c.mine = c.revealed = c.flagged = false;
                    c.adj = 0;
                    c.face->setText("");
                    c.btn->setEnabled(true);
                }
            int placed = 0;
            while (placed < kMines) {
                int x = rand() % kN, y = rand() % kN;
                if (!grid[y][x].mine) {
                    grid[y][x].mine = true;
                    ++placed;
                }
            }
            for (int y = 0; y < kN; ++y)
                for (int x = 0; x < kN; ++x) {
                    int n = 0;
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx) {
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && nx < kN && ny >= 0 && ny < kN && grid[ny][nx].mine) ++n;
                        }
                    grid[y][x].adj = n;
                }
            gameOver = false;
            revealedCount = 0;
            updateStatus();
        };

        auto newBtn = std::make_unique<Button>(guiManager, 450, 120, 200, 44, "New Game");
        newBtn->setOnClickCallback([&plant](GUIElement*) { plant(); });
        guiManager.addElement(std::move(newBtn));

        auto help = std::make_unique<Label>(guiManager, 450, 180,
            "Left click: reveal\nRight click: flag (F)\nFind all safe cells!", 16);
        guiManager.addElement(std::move(help));
        plant();

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
