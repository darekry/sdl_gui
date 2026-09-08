#pragma once
// ui_helpers.hpp — free UI glue: data binding, generators, small builders.
//
// Extension point for helpers that don't belong to any single widget:
// add a declaration here + definition in ui_helpers.cpp (templates stay in
// this header). Keep each helper dependency-free (operates on public widget
// API) and liveness-safe (capture ElementRef, never raw widget pointers, in
// stored callbacks; returned raw pointers are owned by the parent/manager).
#include "slider.hpp"
#include "range_slider.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "gui_manager.hpp"

// Mirrors a Slider value into a Label, refreshed immediately and on every
// change. Replaces the hand-written setOnChangeCallback + to_string glue.
// NOTE: one binding per widget — linking the same slider again replaces the
// previous binding (same semantics as setOnChangeCallback).
void linkLabel(Slider& slider, Label& label,
               std::string prefix = {}, std::string suffix = {});

// Same, with a custom formatter (e.g. [](int v){ return std::to_string(v / 60) + " min"; }).
void linkLabel(Slider& slider, Label& label, std::function<std::string(int)> format);

// Mirrors a RangeSlider [lo, hi] window into a Label (default "[lo, hi]").
void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::string prefix = "[", std::string mid = ", ", std::string suffix = "]");

// Same, with a custom formatter receiving (lower, upper).
void linkRangeLabel(RangeSlider& slider, Label& label,
                    std::function<std::string(int, int)> format);

// ---- grid generators (2D layouts, e.g. board games) ----

// Fills a parent with a cols×rows grid of Panels (Snake board, lights-out,
// …). Cells are positioned at (ox + x*(cellW+gap), oy + y*(cellH+gap))
// relative to the parent; returned pointers are owned by the parent.
std::vector<std::vector<Panel*>> gridPanels(GUIElement& parent, int cols, int rows,
                                            int cellW, int cellH, int gap = 0,
                                            int ox = 0, int oy = 0);
std::vector<std::vector<Panel*>> gridPanels(GUIManager& manager, int cols, int rows,
                                            int cellW, int cellH, int gap = 0,
                                            int ox = 0, int oy = 0);

// A clickable cell: Button + centered Label face. Button has no setText(),
// so the face carries the caption (Minesweeper numbers/flags, Memory
// symbols). Faces start empty; single-char captions stay ~centered.
struct FaceCell {
    Button* button = nullptr;
    Label* face = nullptr;
};
std::vector<std::vector<FaceCell>> faceGrid(GUIElement& parent, int cols, int rows,
                                            int cellW, int cellH, int gap = 0,
                                            int ox = 0, int oy = 0, int faceSize = -1);
std::vector<std::vector<FaceCell>> faceGrid(GUIManager& manager, int cols, int rows,
                                            int cellW, int cellH, int gap = 0,
                                            int ox = 0, int oy = 0, int faceSize = -1);

// ---- strip builder (1D rows/columns: mixer channels, control rows) ----

// Places n widgets via factory(manager, x, y, w, h, index); positions step
// by (pitchX, pitchY) per item. Returns owned-by-parent raw pointers.
template <typename T, typename Factory>
std::vector<T*> makeStrip(GUIElement& parent, int n, int x, int y, int w, int h,
                          int pitchX, int pitchY, Factory&& factory) {
    GUIManager& m = parent.getManager();
    std::vector<T*> out;
    out.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        auto widget = factory(m, x + i * pitchX, y + i * pitchY, w, h, i);
        T* raw = widget.get();
        parent.addChild(std::move(widget));
        out.push_back(raw);
    }
    return out;
}

template <typename T, typename Factory>
std::vector<T*> makeStrip(GUIManager& manager, int n, int x, int y, int w, int h,
                          int pitchX, int pitchY, Factory&& factory) {
    std::vector<T*> out;
    out.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        auto widget = factory(manager, x + i * pitchX, y + i * pitchY, w, h, i);
        T* raw = widget.get();
        manager.addElement(std::move(widget));
        out.push_back(raw);
    }
    return out;
}

// ---- deck generator ----

// Shuffled pairs deck {0,0,1,1,…,pairs-1,pairs-1} for Memory-style games.
std::vector<int> shuffledPairs(int pairs);

// ---- tracked ownership (makeRef-before-add fusion) ----

// Adds a widget and returns {raw pointer, live ElementRef} in one step.
// Kills the classic crash: makeRef() AFTER std::move() grabs a dead pointer.
// Usage: auto [ball, ballRef] = addTracked(guiManager, std::make_unique<Panel>(...));
template <typename T>
std::pair<T*, ElementRef<T>> addTracked(GUIManager& manager, std::unique_ptr<T> widget) {
    T* raw = widget.get();
    ElementRef<T> ref = manager.makeRef(raw);
    manager.addElement(std::move(widget));
    return {raw, ref};
}

template <typename T>
std::pair<T*, ElementRef<T>> addTracked(GUIElement& parent, std::unique_ptr<T> widget) {
    T* raw = widget.get();
    ElementRef<T> ref = parent.getManager().makeRef(raw);
    parent.addChild(std::move(widget));
    return {raw, ref};
}

// ---- card style (dark rounded panel look) ----

// Applies the canonical dark-panel look (bg/border/width/radius). Defaults
// reproduce the blob repeated across the demo apps; pass explicit colors for
// variants (playfield, sidebar, …).
void styleCard(Panel& panel, SDL_Color bg = {50, 52, 64, 255},
               SDL_Color border = {98, 114, 164, 255}, int borderWidth = 2,
               int radius = 10);

// Card factory: creates a styled Panel as child (or top-level) in one call.
Panel* addDarkPanel(GUIElement& parent, int x, int y, int w, int h);
Panel* addDarkPanel(GUIManager& manager, int x, int y, int w, int h);

// ---- one-liner builders (create → configure → attach) ----

// Button with optional click handler, attached in one call. Returned pointer
// is owned by the parent/manager.
Button* addButton(GUIElement& parent, int x, int y, int w, int h,
                  std::string text, Button::OnClickCallback onClick = {});
Button* addButton(GUIManager& manager, int x, int y, int w, int h,
                  std::string text, Button::OnClickCallback onClick = {});

// Label attached in one call. textColor is applied only when given (otherwise
// the theme default shows) — pass white explicitly on dark backgrounds.
Label* addLabel(GUIElement& parent, int x, int y, std::string text,
                int fontSize = 16, std::optional<SDL_Color> textColor = std::nullopt);
Label* addLabel(GUIManager& manager, int x, int y, std::string text,
                int fontSize = 16, std::optional<SDL_Color> textColor = std::nullopt);

// ---- labeled checkbox (box + caption) ----

struct LabeledCheckbox {
    Checkbox* box = nullptr;
    Label* label = nullptr;
};

// Checkbox at (x, y) with a text label to its right (offset by boxSize + 8).
// textColor is applied to the caption only when given (dark panels need
// explicit white). Both pointers are owned by the parent/manager.
LabeledCheckbox addLabeledCheckbox(GUIElement& parent, int x, int y, std::string text,
                                   int boxSize = 24, int fontSize = 16,
                                   std::optional<SDL_Color> textColor = std::nullopt);
LabeledCheckbox addLabeledCheckbox(GUIManager& manager, int x, int y, std::string text,
                                   int boxSize = 24, int fontSize = 16,
                                   std::optional<SDL_Color> textColor = std::nullopt);

// ---- 2D grid with per-cell factory (numpads, card hands, tile maps) ----

// Generalizes gridPanels/faceGrid/makeStrip to heterogeneous cells:
// factory(manager, col, row, x, y, w, h) builds each cell. Positions step by
// (cellW + gapX, cellH + gapY) from (x0, y0). Returns owned-by-parent pointers.
template <typename T, typename Factory>
std::vector<std::vector<T*>> makeGrid(GUIElement& parent, int cols, int rows,
                                      int x0, int y0, int cellW, int cellH,
                                      int gapX, int gapY, Factory&& factory) {
    GUIManager& m = parent.getManager();
    std::vector<std::vector<T*>> grid(static_cast<size_t>(rows),
                                      std::vector<T*>(static_cast<size_t>(cols), nullptr));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto widget = factory(m, c, r, x0 + c * (cellW + gapX), y0 + r * (cellH + gapY),
                                  cellW, cellH);
            T* raw = widget.get();
            parent.addChild(std::move(widget));
            grid[static_cast<size_t>(r)][static_cast<size_t>(c)] = raw;
        }
    }
    return grid;
}

template <typename T, typename Factory>
std::vector<std::vector<T*>> makeGrid(GUIManager& manager, int cols, int rows,
                                      int x0, int y0, int cellW, int cellH,
                                      int gapX, int gapY, Factory&& factory) {
    std::vector<std::vector<T*>> grid(static_cast<size_t>(rows),
                                      std::vector<T*>(static_cast<size_t>(cols), nullptr));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto widget = factory(manager, c, r, x0 + c * (cellW + gapX),
                                  y0 + r * (cellH + gapY), cellW, cellH);
            T* raw = widget.get();
            manager.addElement(std::move(widget));
            grid[static_cast<size_t>(r)][static_cast<size_t>(c)] = raw;
        }
    }
    return grid;
}

// ---- slider fan-out (one refresh() for N sliders) ----

// Wires the same refresh() to every slider and runs it once immediately,
// so readouts never show stale text. The multi-slider demos' shared-refresh
// idiom (RGB mixer, brush, remap, text scaler).
void onAnyChange(std::initializer_list<Slider*> sliders, std::function<void()> refresh);
void onAnyChange(std::initializer_list<RangeSlider*> sliders, std::function<void()> refresh);

// Mirrors a slider value into a plain variable (simulation speed, gravity,
// mixer gain, …) — linkLabel's sibling for non-label targets.
void bindSliderValue(Slider& slider, int& out);
void bindSliderValue(Slider& slider, float& out, float scale = 1.0f);

// ---- status bars (top info strip / bottom status line) ----

struct StatusBar {
    Panel* bar = nullptr;
    Label* label = nullptr;
};

// Full-width bar with a single label child (inset 8px, vertically ~centered).
// Manager overloads take explicit screen dims; parent overloads derive the
// width (top) / width + bottom offset (bottom) from the parent's size.
// Add extra children (clock, hints) to bar as needed — all pointers below
// are owned by the parent/manager.
StatusBar makeTopBar(GUIManager& manager, int screenW, int h,
                     std::string text = {}, int fontSize = 14);
StatusBar makeBottomBar(GUIManager& manager, int screenW, int screenH, int h,
                        std::string text = {}, int fontSize = 14);
StatusBar makeTopBar(GUIElement& parent, int h, std::string text = {}, int fontSize = 14);
StatusBar makeBottomBar(GUIElement& parent, int h, std::string text = {}, int fontSize = 14);
