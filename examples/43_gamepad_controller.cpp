/**
 * @file 43_gamepad_controller.cpp
 * @brief Controller/gamepad UI — D-pad grid navigation, button hints, gamepad events
 *
 * Demonstrates patterns for controller-driven interfaces:
 * - D-pad (or left stick) navigation across a grid of elements
 * - A/B/X/Y action buttons with visual hints
 * - Focus highlight on the active element
 * - Fallback: arrow keys + Enter/Esc on desktop
 * - Auto-detection and hot-plug of the gamepad
 *
 * Controls:
 *   Gamepad D-pad / Left stick → move across the grid
 *   Gamepad A (South) → select / activate
 *   Gamepad B (East)  → back / cancel
 *   Keyboard: arrow keys + Enter + Esc (fallback)
 */

#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// --- Grid of elements ---
const int GRID_COLS = 3;
const int GRID_ROWS = 2;
const int CARD_W = 180;
const int CARD_H = 160;
const int CARD_GAP = 20;
const int GRID_START_X = (SCREEN_WIDTH - (CARD_W * GRID_COLS + CARD_GAP * (GRID_COLS - 1))) / 2;
const int GRID_START_Y = 100;

// --- Card structure (widget + data) ---
struct Card {
    Panel* panel = nullptr;
    Label* nameLabel = nullptr;
    Label* descLabel = nullptr;
    std::string name;
    int row = 0;
    int col = 0;
};

// --- Button hint colors ---
struct ButtonHintColors {
    SDL_Color bg    = {220, 60, 60, 255};    // A — red (South)
    SDL_Color text  = {255, 255, 255, 255};
};
const ButtonHintColors kBtnColors[4] = {
    {{220, 60,  60,  255}},  // A — red (South)
    {{220, 150, 30,  255}},  // B — orange (East)
    {{50,  100, 220, 255}},  // X — blue (West)
    {{180, 130, 30,  255}},  // Y — gold (North)
};

// --- Set focus on a card ---
static void focusCard(GUIManager& mgr, Card* card, Card*& currentFocus,
                      Label*& focusLabel) {
    if (currentFocus && currentFocus->panel) {
        currentFocus->panel->setBorder(ElementState::Normal, {120, 120, 140, 255}, 2);
    }
    currentFocus = card;
    if (card && card->panel) {
        card->panel->setBorder(ElementState::Normal, {0, 120, 215, 255}, 4);
        mgr.setKeyboardFocus(card->panel);
        focusLabel->setText("Focus: " + card->name);
    }
}

// --- Find the neighboring card in the grid ---
static Card* findNeighbor(Card* current, int dr, int dc, std::vector<Card>& cards) {
    if (!current) return nullptr;
    int targetR = current->row + dr;
    int targetC = current->col + dc;
    // Wrap-around
    if (targetR < 0) targetR = GRID_ROWS - 1;
    if (targetR >= GRID_ROWS) targetR = 0;
    if (targetC < 0) targetC = GRID_COLS - 1;
    if (targetC >= GRID_COLS) targetC = 0;

    for (auto& c : cards) {
        if (c.row == targetR && c.col == targetC) return &c;
    }
    return nullptr;
}

int main(int, char**) {
    try {
        SDLApp app("Gamepad Controller UI", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());
        guiManager.setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

        // === Title ===
        auto title = std::make_unique<Label>(guiManager, SCREEN_WIDTH / 2 - 180, 20,
                                             "CHARACTER SELECT", 28);
        title->setTextColor(ElementState::Normal, {220, 220, 230, 255});
        guiManager.addElement(std::move(title));

        auto subtitle = std::make_unique<Label>(guiManager, SCREEN_WIDTH / 2 - 160, 58,
            "Use D-Pad to navigate  |  A=Select  |  B=Back", 14);
        subtitle->setTextColor(ElementState::Normal, {140, 140, 160, 255});
        guiManager.addElement(std::move(subtitle));

        // === Card grid (2×3) ===
        std::vector<Card> cards;
        const char* cardNames[] = {"WARRIOR", "MAGE", "ROGUE", "PALADIN", "RANGER", "NECRO"};
        const char* cardDescs[] = {"Melee tank", "Spellcaster", "Stealth DPS",
                                   "Holy knight", "Bow master", "Dark arts"};

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                int idx = r * GRID_COLS + c;
                int cx = GRID_START_X + c * (CARD_W + CARD_GAP);
                int cy = GRID_START_Y + r * (CARD_H + CARD_GAP);

                auto cardPanel = std::make_unique<Panel>(guiManager, cx, cy, CARD_W, CARD_H);
                cardPanel->setBackgroundColor(ElementState::Normal, {40, 44, 56, 255});
                cardPanel->setBorder(ElementState::Normal, {120, 120, 140, 255}, 2);
                cardPanel->setBorderRadius(ElementState::Normal, 10);
                cardPanel->setCanGetKeyboardFocus(true);

                auto nameLabel = std::make_unique<Label>(guiManager, 16, 20, cardNames[idx], 22);
                nameLabel->setTextColor(ElementState::Normal, {255, 255, 255, 255});

                auto descLabel = std::make_unique<Label>(guiManager, 16, 60, cardDescs[idx], 14);
                descLabel->setTextColor(ElementState::Normal, {160, 160, 180, 255});

                // Button info
                auto ctrlInfo = std::make_unique<Label>(guiManager, 16, 110,
                    "Press  \x41\x20 to select", 12);  // U+A0 (nbsp) for spacing
                ctrlInfo->setTextColor(ElementState::Normal, {120, 120, 140, 255});

                Panel* panelPtr = cardPanel.get();
                Label* namePtr = nameLabel.get();
                Label* descPtr = descLabel.get();

                cardPanel->addChild(std::move(nameLabel));
                cardPanel->addChild(std::move(descLabel));
                cardPanel->addChild(std::move(ctrlInfo));
                guiManager.addElement(std::move(cardPanel));

                cards.push_back({panelPtr, namePtr, descPtr, cardNames[idx], r, c});
            }
        }

        // === Button hint panel (bottom-right corner) ===
        auto hintPanel = std::make_unique<Panel>(guiManager, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 180,
                                                 180, 160);
        hintPanel->setBackgroundColor(ElementState::Normal, {30, 32, 40, 220});
        hintPanel->setBorderRadius(ElementState::Normal, 8);
        hintPanel->setBorder(ElementState::Normal, {80, 80, 100, 255}, 1);

        auto hintTitle = std::make_unique<Label>(guiManager, 12, 8, "CONTROLS", 14);
        hintTitle->setTextColor(ElementState::Normal, {200, 200, 220, 255});
        hintPanel->addChild(std::move(hintTitle));

        const char* hintLetters[] = {"A", "B", "X", "Y"};
        const char* hintActions[] = {"Select", "Back", "Info", "Settings"};
        for (int i = 0; i < 4; ++i) {
            int hy = 32 + i * 28;
            auto letter = std::make_unique<Label>(guiManager, 12, hy, hintLetters[i], 16);
            letter->setTextColor(ElementState::Normal, kBtnColors[i].bg);
            hintPanel->addChild(std::move(letter));

            auto action = std::make_unique<Label>(guiManager, 40, hy, hintActions[i], 14);
            action->setTextColor(ElementState::Normal, {180, 180, 200, 255});
            hintPanel->addChild(std::move(action));
        }
        guiManager.addElement(std::move(hintPanel));

        // === Status label (bottom-left corner) ===
        auto statusLabel = std::make_unique<Label>(guiManager, 20, SCREEN_HEIGHT - 40,
            "Status: Ready — Press A or Enter to select a character", 14);
        statusLabel->setTextColor(ElementState::Normal, {160, 200, 160, 255});
        Label* statusPtr = statusLabel.get();
        guiManager.addElement(std::move(statusLabel));

        // === Focus label ===
        auto focusLabel = std::make_unique<Label>(guiManager, 20, SCREEN_HEIGHT - 70,
            "Focus: none", 14);
        focusLabel->setTextColor(ElementState::Normal, {120, 180, 255, 255});
        Label* focusPtr = focusLabel.get();
        guiManager.addElement(std::move(focusLabel));

        // --- State ---
        Card* currentFocus = nullptr;
        SDL_Gamepad* gamepad = nullptr;
        bool quit = false;
        SDL_Event e;

        // Set initial focus
        focusCard(guiManager, &cards[0], currentFocus, focusPtr);

        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    continue;
                }

                // --- Gamepad hot-plug ---
                if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
                    gamepad = SDL_OpenGamepad(e.gdevice.which);
                    if (gamepad) {
                        statusPtr->setText("Status: Gamepad connected — use D-Pad to navigate");
                        statusPtr->setTextColor(ElementState::Normal, {160, 255, 160, 255});
                    }
                } else if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
                    if (gamepad && e.gdevice.which == SDL_GetGamepadID(gamepad)) {
                        SDL_CloseGamepad(gamepad);
                        gamepad = nullptr;
                        statusPtr->setText("Status: Gamepad disconnected — use Arrow keys");
                        statusPtr->setTextColor(ElementState::Normal, {255, 200, 160, 255});
                    }
                }

                // --- Gamepad navigation (D-pad + left stick) ---
                else if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
                    Card* neighbor = nullptr;
                    switch (e.gbutton.button) {
                        case SDL_GAMEPAD_BUTTON_DPAD_UP:    neighbor = findNeighbor(currentFocus, -1, 0, cards); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:  neighbor = findNeighbor(currentFocus,  1, 0, cards); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:  neighbor = findNeighbor(currentFocus,  0, -1, cards); break;
                        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: neighbor = findNeighbor(currentFocus,  0,  1, cards); break;
                        case SDL_GAMEPAD_BUTTON_SOUTH: // A — Select
                            if (currentFocus) {
                                statusPtr->setText("Selected: " + currentFocus->name + " (A button)");
                                statusPtr->setTextColor(ElementState::Normal, {255, 255, 160, 255});
                            }
                            break;
                        case SDL_GAMEPAD_BUTTON_EAST: // B — Back
                            statusPtr->setText("Status: Back pressed (B) — returning...");
                            statusPtr->setTextColor(ElementState::Normal, {255, 160, 160, 255});
                            break;
                        case SDL_GAMEPAD_BUTTON_WEST: // X — Info
                            if (currentFocus) {
                                statusPtr->setText("Info: " + currentFocus->name + " — " +
                                    (currentFocus->descLabel ? currentFocus->descLabel->getText() : ""));
                                statusPtr->setTextColor(ElementState::Normal, {160, 200, 255, 255});
                            }
                            break;
                        case SDL_GAMEPAD_BUTTON_NORTH: // Y — Settings
                            statusPtr->setText("Status: Settings menu (Y)");
                            statusPtr->setTextColor(ElementState::Normal, {200, 180, 255, 255});
                            break;
                        default: break;
                    }
                    if (neighbor) {
                        focusCard(guiManager, neighbor, currentFocus, focusPtr);
                    }
                }

                // --- Left analog stick as an alternative to the D-pad ---
                else if (e.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
                    // Deadzone: ignore small movements
                    const int16_t deadzone = 8000;
                    static bool axisNavLock = false; // Prevents repeated triggering

                    if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX || e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY) {
                        if (axisNavLock) {
                            // Wait until the stick returns to the deadzone
                            int16_t lx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
                            int16_t ly = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
                            if (std::abs(lx) < deadzone / 2 && std::abs(ly) < deadzone / 2) {
                                axisNavLock = false;
                            }
                            continue;
                        }

                        int16_t lx = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
                        int16_t ly = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);

                        Card* neighbor = nullptr;
                        if (std::abs(lx) > std::abs(ly)) {
                            if (lx > deadzone)       neighbor = findNeighbor(currentFocus, 0, 1, cards);
                            else if (lx < -deadzone) neighbor = findNeighbor(currentFocus, 0, -1, cards);
                        } else {
                            if (ly > deadzone)       neighbor = findNeighbor(currentFocus, 1, 0, cards);
                            else if (ly < -deadzone) neighbor = findNeighbor(currentFocus, -1, 0, cards);
                        }
                        if (neighbor) {
                            focusCard(guiManager, neighbor, currentFocus, focusPtr);
                            axisNavLock = true;
                        }
                    }
                }

                // --- Fallback: arrow keys + Enter/Esc ---
                else if (e.type == SDL_EVENT_KEY_DOWN) {
                    Card* neighbor = nullptr;
                    switch (e.key.key) {
                        case SDLK_UP:    neighbor = findNeighbor(currentFocus, -1, 0, cards); break;
                        case SDLK_DOWN:  neighbor = findNeighbor(currentFocus,  1, 0, cards); break;
                        case SDLK_LEFT:  neighbor = findNeighbor(currentFocus,  0, -1, cards); break;
                        case SDLK_RIGHT: neighbor = findNeighbor(currentFocus,  0,  1, cards); break;
                        case SDLK_RETURN:
                            if (currentFocus) {
                                statusPtr->setText("Selected: " + currentFocus->name + " (Enter)");
                                statusPtr->setTextColor(ElementState::Normal, {255, 255, 160, 255});
                            }
                            break;
                        case SDLK_ESCAPE:
                            statusPtr->setText("Status: Back (Esc)");
                            statusPtr->setTextColor(ElementState::Normal, {255, 160, 160, 255});
                            break;
                        case SDLK_I: // X = Info
                            if (currentFocus && currentFocus->descLabel) {
                                statusPtr->setText("Info: " + currentFocus->name + " — " +
                                    currentFocus->descLabel->getText());
                                statusPtr->setTextColor(ElementState::Normal, {160, 200, 255, 255});
                            }
                            break;
                        default: break;
                    }
                    if (neighbor) {
                        focusCard(guiManager, neighbor, currentFocus, focusPtr);
                    }
                }

                // Standard GUI processing
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 20, 22, 30, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }

        if (gamepad) {
            SDL_CloseGamepad(gamepad);
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
