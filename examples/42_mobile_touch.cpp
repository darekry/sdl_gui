/**
 * @file 42_mobile_touch.cpp
 * @brief Touch-optimized mobile UI — finger-sized widgets, virtual numpad, swipe
 *
 * Demonstrates patterns for touch interfaces (phone/tablet):
 * - Widgets with a minimum size of 44px (Apple HIG)
 * - Virtual numeric keypad (PIN / dial pad)
 * - Swipe gesture handling
 * - Large touch slider
 * - Finger-to-mouse event conversion for existing widgets
 * - On desktop, mouse control works as a fallback
 */

#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "slider.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// --- Phone screen dimensions (iPhone-like aspect) ---
const int PHONE_W = 360;
const int PHONE_H = 640;
const int PHONE_X = (SCREEN_WIDTH - PHONE_W) / 2;
const int PHONE_Y = (SCREEN_HEIGHT - PHONE_H) / 2;

// --- Minimum touch target size (Apple HIG: 44pt) ---
const int TOUCH_TARGET = 48;
const int NUMPAD_COLS = 3;
const int NUMPAD_ROWS = 4;
const int NUMPAD_PAD = 6;
const int NUMPAD_BTN = (TOUCH_TARGET * 3 + NUMPAD_PAD * 4) / NUMPAD_COLS;

// --- Swipe state ---
struct SwipeState {
    bool   active = false;
    float  start_x = 0.0f;
    float  start_y = 0.0f;
    float  current_x = 0.0f;
    float  current_y = 0.0f;
};

// --- Convert finger coordinates (0..1) to window pixels ---
static SDL_Point fingerToPixel(const SDL_TouchFingerEvent& f, int win_w, int win_h) {
    return {static_cast<int>(f.x * static_cast<float>(win_w)),
            static_cast<int>(f.y * static_cast<float>(win_h))};
}

// --- Translate a finger event into mouse events and send them to GUIManager ---
static void dispatchFingerAsMouse(GUIManager& mgr, const SDL_Event& finger_event,
                                  int win_w, int win_h) {
    const auto& f = finger_event.tfinger;
    SDL_Point px = fingerToPixel(f, win_w, win_h);

    SDL_Event mouse_ev{};
    mouse_ev.common.timestamp = finger_event.common.timestamp;

    if (finger_event.type == SDL_EVENT_FINGER_DOWN) {
        mouse_ev.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        mouse_ev.button.x = static_cast<float>(px.x);
        mouse_ev.button.y = static_cast<float>(px.y);
        mouse_ev.button.button = SDL_BUTTON_LEFT;
        mgr.processEvent(mouse_ev);
    } else if (finger_event.type == SDL_EVENT_FINGER_UP) {
        mouse_ev.type = SDL_EVENT_MOUSE_BUTTON_UP;
        mouse_ev.button.x = static_cast<float>(px.x);
        mouse_ev.button.y = static_cast<float>(px.y);
        mouse_ev.button.button = SDL_BUTTON_LEFT;
        mgr.processEvent(mouse_ev);
    } else if (finger_event.type == SDL_EVENT_FINGER_MOTION) {
        mouse_ev.type = SDL_EVENT_MOUSE_MOTION;
        mouse_ev.motion.x = static_cast<float>(px.x);
        mouse_ev.motion.y = static_cast<float>(px.y);
        mgr.processEvent(mouse_ev);
    }
}

int main(int, char**) {
    try {
        SDLApp app("Mobile Touch UI — Phone Simulator", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());
        // === Phone frame ===
        auto phone = std::make_unique<Panel>(guiManager, PHONE_X - 12, PHONE_Y - 12,
                                             PHONE_W + 24, PHONE_H + 24);
        phone->setBackgroundColor(ElementState::Normal, {30, 30, 35, 255});
        phone->setBorderRadius(ElementState::Normal, 24);
        guiManager.addElement(std::move(phone));

        // === Phone screen ===
        auto screen = std::make_unique<Panel>(guiManager, PHONE_X, PHONE_Y, PHONE_W, PHONE_H);
        screen->setBackgroundColor(ElementState::Normal, {245, 245, 250, 255});
        screen->setBorderRadius(ElementState::Normal, 12);
        screen->setClipChildren(true);
        Panel* screenPtr = screen.get();
        guiManager.addElement(std::move(screen));

        // === Status bar ===
        auto statusBar = std::make_unique<Panel>(guiManager, 0, 0, PHONE_W, 28);
        statusBar->setBackgroundColor(ElementState::Normal, {20, 20, 30, 255});
        auto carrierLabel = std::make_unique<Label>(guiManager, 12, 4, "Mobile UI Demo", 12);
        carrierLabel->setTextColor(ElementState::Normal, {200, 200, 210, 255});
        statusBar->addChild(std::move(carrierLabel));
        screenPtr->addChild(std::move(statusBar));

        // === Swipe detection area ===
        auto swipeArea = std::make_unique<Panel>(guiManager, 0, 28, PHONE_W, 80);
        swipeArea->setBackgroundColor(ElementState::Normal, {230, 235, 245, 255});
        swipeArea->setBorder(ElementState::Normal, {180, 185, 195, 255}, 1);

        auto swipeHint = std::make_unique<Label>(guiManager, 8, 8, "SWIPE AREA", 14);
        swipeHint->setTextColor(ElementState::Normal, {120, 120, 140, 255});
        auto swipeDirLabel = std::make_unique<Label>(guiManager, 8, 28, "← swipe here →", 16);
        swipeDirLabel->setTextColor(ElementState::Normal, {80, 80, 100, 255});
        Label* swipeDirPtr = swipeDirLabel.get();
        swipeArea->addChild(std::move(swipeHint));
        swipeArea->addChild(std::move(swipeDirLabel));
        Panel* swipeAreaPtr = swipeArea.get();

        // Swipe start → highlight in green
        swipeArea->setBackgroundColor(ElementState::Pressed, {180, 255, 180, 255});
        screenPtr->addChild(std::move(swipeArea));

        // === Number display field ===
        auto displayBg = std::make_unique<Panel>(guiManager, 20, 118, PHONE_W - 40, 56);
        displayBg->setBackgroundColor(ElementState::Normal, {255, 255, 255, 255});
        displayBg->setBorder(ElementState::Normal, {200, 200, 210, 255}, 1);
        displayBg->setBorderRadius(ElementState::Normal, 10);

        auto displayLabel = std::make_unique<Label>(guiManager, 16, 16, "", 28);
        displayLabel->setTextColor(ElementState::Normal, {40, 40, 50, 255});
        Label* displayPtr = displayLabel.get();
        displayBg->addChild(std::move(displayLabel));
        screenPtr->addChild(std::move(displayBg));

        // === 3×4 numeric keypad ===
        const char* keys[] = {"1","2","3","4","5","6","7","8","9","*","0","#"};
        const int numpadY = 190;
        const int btnSize = TOUCH_TARGET;
        const int totalGridW = btnSize * 3 + NUMPAD_PAD * 2;
        const int numpadStartX = (PHONE_W - totalGridW) / 2;

        for (int r = 0; r < NUMPAD_ROWS; ++r) {
            for (int c = 0; c < NUMPAD_COLS; ++c) {
                int idx = r * NUMPAD_COLS + c;
                int bx = numpadStartX + c * (btnSize + NUMPAD_PAD);
                int by = numpadY + r * (btnSize + NUMPAD_PAD);

                auto btn = std::make_unique<Button>(guiManager, bx, by, btnSize, btnSize, keys[idx]);
                btn->setBorderRadius(ElementState::Normal, btnSize / 2);  // Round
                btn->setBorder(ElementState::Normal, {180, 180, 200, 255}, 1);
                btn->setBackgroundColor(ElementState::Normal, {255, 255, 255, 255});
                btn->setBackgroundColor(ElementState::Hover, {220, 230, 255, 255});
                btn->setBackgroundColor(ElementState::Pressed, {170, 190, 240, 255});
                btn->setTextColor(ElementState::Normal, {40, 40, 50, 255});

                btn->setOnClickCallback([displayPtr, key = std::string(keys[idx])](GUIElement*) {
                    std::string current(displayPtr->getText());
                    if (current.size() < 10) {
                        displayPtr->setText(current + key);
                    }
                });
                screenPtr->addChild(std::move(btn));
            }
        }

        // === Clear button ===
        auto clearBtn = std::make_unique<Button>(guiManager, PHONE_W - btnSize - 20, numpadY,
                                                 btnSize, btnSize, "C");
        clearBtn->setBorderRadius(ElementState::Normal, btnSize / 2);
        clearBtn->setBackgroundColor(ElementState::Normal, {255, 220, 220, 255});
        clearBtn->setBackgroundColor(ElementState::Pressed, {255, 160, 160, 255});
        clearBtn->setTextColor(ElementState::Normal, {180, 40, 40, 255});
        clearBtn->setOnClickCallback([displayPtr](GUIElement*) {
            displayPtr->setText("");
        });
        screenPtr->addChild(std::move(clearBtn));

        // === Large touch slider (e.g. brightness/volume) ===
        auto sliderLabel = std::make_unique<Label>(guiManager, 20, PHONE_H - 90, "Brightness", 14);
        sliderLabel->setTextColor(ElementState::Normal, {80, 80, 100, 255});
        screenPtr->addChild(std::move(sliderLabel));

        auto slider = std::make_unique<Slider>(guiManager, 20, PHONE_H - 70,
                                               PHONE_W - 40, 36, 0, 100, 70, Orientation::Horizontal);
        slider->setBorderRadius(ElementState::Normal, 18); // Large, easy to hit
        Slider* sliderPtr = slider.get();

        auto sliderValLabel = std::make_unique<Label>(guiManager, PHONE_W - 50, PHONE_H - 90, "70", 14);
        sliderValLabel->setTextColor(ElementState::Normal, {80, 80, 100, 255});
        Label* sliderValPtr = sliderValLabel.get();

        slider->setOnChangeCallback([sliderValPtr](GUIElement* el) {
            auto* s = static_cast<Slider*>(el);
            sliderValPtr->setText(std::to_string(s->getValue()));
        });
        screenPtr->addChild(std::move(sliderValLabel));
        screenPtr->addChild(std::move(slider));

        // --- Swipe state ---
        SwipeState swipe{};

        // --- Main loop ---
        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    continue;
                }

                // Handle touch events (finger events)
                if (e.type == SDL_EVENT_FINGER_DOWN || e.type == SDL_EVENT_FINGER_UP ||
                    e.type == SDL_EVENT_FINGER_MOTION) {

                    SDL_Point px = fingerToPixel(e.tfinger, SCREEN_WIDTH, SCREEN_HEIGHT);

                    // --- Detect swipe inside the swipeArea ---
                    if (e.type == SDL_EVENT_FINGER_DOWN) {
                        SDL_Point local = screenPtr->toLocalCoords(px.x, px.y);
                        if (local.x >= 0 && local.x < PHONE_W && local.y >= 28 && local.y < 108) {
                            swipe.active = true;
                            swipe.start_x = e.tfinger.x;
                            swipe.start_y = e.tfinger.y;
                            swipe.current_x = e.tfinger.x;
                            swipe.current_y = e.tfinger.y;
                            swipeAreaPtr->setState(ElementState::Pressed);
                        }
                    } else if (e.type == SDL_EVENT_FINGER_MOTION && swipe.active) {
                        swipe.current_x = e.tfinger.x;
                        swipe.current_y = e.tfinger.y;
                    } else if (e.type == SDL_EVENT_FINGER_UP && swipe.active) {
                        float dx = swipe.current_x - swipe.start_x;
                        float dy = swipe.current_y - swipe.start_y;
                        const char* dir = "tap";
                        if (std::abs(dx) > std::abs(dy)) {
                            dir = dx > 0.03f ? "→ RIGHT" : (dx < -0.03f ? "← LEFT" : "tap");
                        } else {
                            dir = dy > 0.03f ? "↓ DOWN" : (dy < -0.03f ? "↑ UP" : "tap");
                        }
                        swipeDirPtr->setText(std::string("Swipe: ") + dir);
                        swipe.active = false;
                    }

                    // Translate finger → mouse for widgets
                    dispatchFingerAsMouse(guiManager, e, SCREEN_WIDTH, SCREEN_HEIGHT);
                }
                // Backspace clears the display
                else if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_BACKSPACE) {
                    displayPtr->setText(displayPtr->getText().substr(
                        0, displayPtr->getText().size() - 1));
                }
                else {
                    guiManager.processEvent(e);
                }
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 50, 52, 60, 255);
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
