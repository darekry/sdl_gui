/**
 * @file 44_tv_remote.cpp
 * @brief TV Remote / 10-foot UI — large tiles, directional navigation, high contrast
 *
 * Demonstrates patterns for remote-controlled interfaces (Smart TV, media center):
 * - Large elements (min. 48px) readable from 3m distance
 * - Navigation with directional keys only (arrow keys = remote)
 * - Clear focus highlight: thick outline, background color change
 * - Focus wrapping at grid edges (wrap-around)
 * - High contrast, large fonts (20–32pt)
 * - No mouse interaction — keyboard only
 * - Enter (OK), Esc (Back), Backspace (Back) handling
 *
 * Controls (remote simulation):
 *   Arrow keys      → move across tiles
 *   Enter / Space   → select / activate (OK)
 *   Esc / Backspace → go back (Back)
 */

#include "panel.hpp"
#include "label.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "theme_presets.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// --- Tile grid (2 columns x 3 rows) ---
const int TILE_COLS = 2;
const int TILE_ROWS = 3;
const int TILE_W = 280;
const int TILE_H = 120;
const int TILE_GAP = 16;
const int TILE_START_X = (SCREEN_WIDTH - (TILE_W * TILE_COLS + TILE_GAP * (TILE_COLS - 1))) / 2;
const int TILE_START_Y = 110;

// --- TV colors (dark background, bright accents — Smart TV style) ---
const SDL_Color kTVBg       = {18, 20, 28, 255};
const SDL_Color kTVSurface  = {32, 36, 48, 255};
const SDL_Color kTVText     = {230, 230, 240, 255};
const SDL_Color kTVTextDim  = {140, 145, 160, 255};
const SDL_Color kTVAccent   = {80, 160, 255, 255};
const SDL_Color kTVSuccess  = {100, 220, 140, 255};

struct Tile {
    Panel* panel = nullptr;
    Label* nameLabel = nullptr;
    Label* iconLabel = nullptr;
    std::string name;
    int row = 0;
    int col = 0;
};

// --- Set focus on a tile (high contrast) ---
static void focusTile(Tile* tile, Tile*& currentFocus, Label*& statusLabel,
                      Label*& focusLabel) {
    // Reset the previous one
    if (currentFocus && currentFocus->panel) {
        currentFocus->panel->setBackgroundColor(ElementState::Normal, kTVSurface);
        currentFocus->panel->setBorder(ElementState::Normal, {50, 54, 68, 255}, 2);
    }
    currentFocus = tile;
    if (tile && tile->panel) {
        tile->panel->setBackgroundColor(ElementState::Normal, {40, 60, 120, 255});
        tile->panel->setBorder(ElementState::Normal, kTVAccent, 4);
        statusLabel->setText(tile->name);
        focusLabel->setText("Selected: " + tile->name + "  [Enter = Open, Esc = Back]");
    }
}

// --- Find the grid neighbor with wrapping ---
static Tile* findNeighborTile(Tile* current, int dr, int dc, std::vector<Tile>& tiles) {
    if (!current) return nullptr;
    int tr = (current->row + dr + TILE_ROWS) % TILE_ROWS;
    int tc = (current->col + dc + TILE_COLS) % TILE_COLS;
    for (auto& t : tiles) {
        if (t.row == tr && t.col == tc) return &t;
    }
    return nullptr;
}

int main(int, char**) {
    try {
        SDLApp app("TV Remote UI — Media Center", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());
        // === Title bar (top) ===
        auto topBar = std::make_unique<Panel>(guiManager, 0, 0, SCREEN_WIDTH, 60);
        topBar->setBackgroundColor(ElementState::Normal, {12, 14, 22, 255});
        topBar->setBorder(ElementState::Normal, {30, 34, 50, 255}, 1);

        auto logoLabel = std::make_unique<Label>(guiManager, 28, 10, "SMART TV HOME", 28);
        logoLabel->setTextColor(ElementState::Normal, kTVAccent);
        topBar->addChild(std::move(logoLabel));

        auto clockLabel = std::make_unique<Label>(guiManager, SCREEN_WIDTH - 140, 18, "22:45", 18);
        clockLabel->setTextColor(ElementState::Normal, kTVTextDim);
        topBar->addChild(std::move(clockLabel));
        guiManager.addElement(std::move(topBar));

        // === Tile grid ===
        std::vector<Tile> tiles;
        struct TileDef {
            const char* name;
            const char* icon;
        };
        const TileDef tileDefs[] = {
            {"Movies",      "[M]"},
            {"TV Shows",    "[T]"},
            {"Music",       "[U]"},
            {"Live TV",     "[L]"},
            {"Settings",    "[S]"},
            {"Search",      "[?]"},
        };

        for (int r = 0; r < TILE_ROWS; ++r) {
            for (int c = 0; c < TILE_COLS; ++c) {
                int idx = r * TILE_COLS + c;
                int tx = TILE_START_X + c * (TILE_W + TILE_GAP);
                int ty = TILE_START_Y + r * (TILE_H + TILE_GAP);

                auto tile = std::make_unique<Panel>(guiManager, tx, ty, TILE_W, TILE_H);
                tile->setBackgroundColor(ElementState::Normal, kTVSurface);
                tile->setBorder(ElementState::Normal, {50, 54, 68, 255}, 2);
                tile->setBorderRadius(ElementState::Normal, 12);
                tile->setCanGetKeyboardFocus(true);

                // Icon (placeholder emoji + symbol)
                auto icon = std::make_unique<Label>(guiManager, 24, 22, tileDefs[idx].icon, 28);
                icon->setTextColor(ElementState::Normal, kTVText);
                Label* iconPtr = icon.get();
                tile->addChild(std::move(icon));

                // Name
                auto nameLabel = std::make_unique<Label>(guiManager, 80, 26, tileDefs[idx].name, 26);
                nameLabel->setTextColor(ElementState::Normal, kTVText);
                Label* namePtr = nameLabel.get();

                // Description (below the name)
                auto descLabel = std::make_unique<Label>(guiManager, 80, 62,
                    "Press OK to open", 14);
                descLabel->setTextColor(ElementState::Normal, kTVTextDim);

                tile->addChild(std::move(nameLabel));
                tile->addChild(std::move(descLabel));

                Panel* panelPtr = tile.get();
                guiManager.addElement(std::move(tile));
                tiles.push_back({panelPtr, namePtr, iconPtr, tileDefs[idx].name, r, c});
            }
        }

        // === Bottom status bar ===
        auto bottomBar = std::make_unique<Panel>(guiManager, 0, SCREEN_HEIGHT - 70,
                                                  SCREEN_WIDTH, 70);
        bottomBar->setBackgroundColor(ElementState::Normal, {12, 14, 22, 255});
        bottomBar->setBorder(ElementState::Normal, {30, 34, 50, 255}, 1);

        auto statusTitle = std::make_unique<Label>(guiManager, 28, 10, "Navigation:", 14);
        statusTitle->setTextColor(ElementState::Normal, kTVTextDim);
        bottomBar->addChild(std::move(statusTitle));

        auto statusLabel = std::make_unique<Label>(guiManager, 28, 32, "Movies", 20);
        statusLabel->setTextColor(ElementState::Normal, kTVAccent);
        Label* statusPtr = statusLabel.get();
        bottomBar->addChild(std::move(statusLabel));

        auto hintLabel = std::make_unique<Label>(guiManager, 28, 54,
            "ARROWS = Navigate  |  ENTER = OK  |  ESC = Back", 12);
        hintLabel->setTextColor(ElementState::Normal, {90, 95, 110, 255});
        bottomBar->addChild(std::move(hintLabel));

        auto focusInfo = std::make_unique<Label>(guiManager, SCREEN_WIDTH - 380, 20,
            "Selected: Movies  [Enter = Open, Esc = Back]", 13);
        focusInfo->setTextColor(ElementState::Normal, kTVTextDim);
        Label* focusPtr = focusInfo.get();
        bottomBar->addChild(std::move(focusInfo));

        guiManager.addElement(std::move(bottomBar));

        // --- State ---
        Tile* currentFocus = nullptr;

        // Set initial focus on the first tile
        focusTile(&tiles[0], currentFocus, statusPtr, focusPtr);

        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    continue;
                }

                // --- Directional navigation (remote simulation) ---
                if (e.type == SDL_EVENT_KEY_DOWN) {
                    Tile* neighbor = nullptr;

                    switch (e.key.key) {
                        case SDLK_UP:    neighbor = findNeighborTile(currentFocus, -1, 0, tiles); break;
                        case SDLK_DOWN:  neighbor = findNeighborTile(currentFocus,  1, 0, tiles); break;
                        case SDLK_LEFT:  neighbor = findNeighborTile(currentFocus,  0, -1, tiles); break;
                        case SDLK_RIGHT: neighbor = findNeighborTile(currentFocus,  0,  1, tiles); break;

                        case SDLK_RETURN:
                        case SDLK_SPACE:
                            // OK / Select — simulate pressing OK on the remote
                            if (currentFocus) {
                                statusPtr->setText("OPENED: " + currentFocus->name);
                                statusPtr->setTextColor(ElementState::Normal, kTVSuccess);
                                focusPtr->setText("Opened " + currentFocus->name +
                                    " — press Esc to go back");
                                focusPtr->setTextColor(ElementState::Normal, kTVSuccess);
                            }
                            break;

                        case SDLK_ESCAPE:
                        case SDLK_BACKSPACE:
                            // Back — simulate the Back button on the remote
                            statusPtr->setText("Home");
                            statusPtr->setTextColor(ElementState::Normal, kTVAccent);
                            focusPtr->setText("Returned to home screen");
                            focusPtr->setTextColor(ElementState::Normal, kTVTextDim);
                            break;

                        case SDLK_M: // Shortcut: Movies
                            focusTile(&tiles[0], currentFocus, statusPtr, focusPtr); break;
                        case SDLK_T: // Shortcut: TV Shows
                            focusTile(&tiles[1], currentFocus, statusPtr, focusPtr); break;
                        case SDLK_S: // Shortcut: Settings
                            focusTile(&tiles[4], currentFocus, statusPtr, focusPtr); break;

                        default: break;
                    }

                    if (neighbor) {
                        focusTile(neighbor, currentFocus, statusPtr, focusPtr);
                    }
                }

                // Standard GUI processing (TAB, focus, etc.)
                // Ignore mouse events in TV mode
                if (e.type != SDL_EVENT_MOUSE_MOTION &&
                    e.type != SDL_EVENT_MOUSE_BUTTON_DOWN &&
                    e.type != SDL_EVENT_MOUSE_BUTTON_UP) {
                    guiManager.processEvent(e);
                }
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, kTVBg.r, kTVBg.g, kTVBg.b, kTVBg.a);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
