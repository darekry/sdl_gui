/**
 * @file 62_memory.cpp
 * @brief Memory pairs on a 4x4 grid of Buttons with hidden Label faces.
 *
 * Pure mouse game (no keyboard): flip two cards, matches stay open and
 * lock, mismatches flip back after 700 ms via a main-loop deadline.
 * Restart reshuffles the symbols.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "ui_helpers.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

namespace {

constexpr int kSide = 4, kCellM = 100, kGapM = 10;
constexpr int kOXM = 190, kOYM = 90;

struct Card {
    Button* btn = nullptr;
    Label* face = nullptr;
    int symbol = 0;
    bool open = false;
    bool done = false;
};

} // namespace

int main(int, char**) {
    try {
        SDLApp app("Memory - find all 8 pairs", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto status = std::make_unique<Label>(guiManager, kOXM, 30, "moves: 0   pairs: 0/8", 20);
        auto statusRef = guiManager.makeRef(status.get());
        guiManager.addElement(std::move(status));

        std::vector<Card> cards;
        cards.reserve(kSide * kSide);
        int moves = 0, pairs = 0;
        int first = -1; // index of the first open card, -1 = none
        bool flipping = false; // mismatch pair shown, waiting for flip-back
        int dealId = 0; // bumped per deal; stale after() callbacks bail out

        std::function<void()> refresh;
        refresh = [&]() {
            if (statusRef) statusRef->setText("moves: " + std::to_string(moves) +
                                              "   pairs: " + std::to_string(pairs) + "/8" +
                                              (pairs == 8 ? "   YOU WIN!" : ""));
        };

        auto setOpen = [&](int i, bool open) {
            cards[i].open = open;
            cards[i].face->setVisible(open);
            if (open) cards[i].btn->setBackgroundColor(ElementState::Normal, {70, 80, 110, 255});
        };

        for (int y = 0; y < kSide; ++y) {
            for (int x = 0; x < kSide; ++x) {
                int i = y * kSide + x;
                auto btn = std::make_unique<Button>(guiManager, kOXM + x * (kCellM + kGapM),
                                                    kOYM + y * (kCellM + kGapM), kCellM, kCellM, "");
                auto face = std::make_unique<Label>(guiManager, 38, 30, "?", 32);
                face->setVisible(false);
                Card c;
                c.btn = btn.get();
                c.face = face.get();
                btn->addChild(std::move(face));
                btn->setOnClickCallback([i, &cards, &first, &moves, &pairs, &flipping,
                                         &dealId, &setOpen, &refresh, &guiManager](GUIElement*) {
                    if (flipping) return; // waiting for flip-back
                    Card& c = cards[i];
                    if (c.open || c.done) return;
                    setOpen(i, true);
                    if (first < 0) {
                        first = i;
                    } else {
                        ++moves;
                        if (cards[first].symbol == c.symbol) {
                            cards[first].done = c.done = true;
                            cards[first].btn->setEnabled(false);
                            c.btn->setEnabled(false);
                            cards[first].btn->setBackgroundColor(ElementState::Normal, {90, 170, 110, 255});
                            c.btn->setBackgroundColor(ElementState::Normal, {90, 170, 110, 255});
                            ++pairs;
                            first = -1;
                            refresh();
                        } else {
                            int a = first, b = i, id = dealId;
                            first = -1;
                            flipping = true;
                            refresh();
                            // Delayed flip-back via GUIManager::after() (fired from update()).
                            guiManager.after(700, [&cards, a, b, id, &flipping, &dealId]() {
                                if (id != dealId) return; // restarted meanwhile
                                cards[a].open = cards[b].open = false;
                                cards[a].face->setVisible(false);
                                cards[b].face->setVisible(false);
                                cards[a].btn->setBackgroundColor(ElementState::Normal, {50, 52, 64, 255});
                                cards[b].btn->setBackgroundColor(ElementState::Normal, {50, 52, 64, 255});
                                flipping = false;
                            });
                        }
                    }
                });
                cards.push_back(c);
                guiManager.addElement(std::move(btn));
            }
        }

        auto deal = [&]() {
            std::vector<int> syms = shuffledPairs(8);
            for (size_t i = 0; i < cards.size(); ++i) {
                cards[i].symbol = syms[i];
                cards[i].open = cards[i].done = false;
                cards[i].face->setText(std::string(1, static_cast<char>('A' + syms[i])));
                cards[i].face->setVisible(false);
                cards[i].btn->setEnabled(true);
                cards[i].btn->setBackgroundColor(ElementState::Normal, {50, 52, 64, 255});
            }
            moves = pairs = 0;
            first = -1;
            flipping = false;
            ++dealId; // invalidates any pending flip-back
            refresh();
        };

        auto restartBtn = std::make_unique<Button>(guiManager, 620, 90, 150, 44, "Restart");
        restartBtn->setOnClickCallback([&deal](GUIElement*) { deal(); });
        guiManager.addElement(std::move(restartBtn));
        deal();

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
