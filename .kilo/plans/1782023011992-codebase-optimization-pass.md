# Codebase Optimization Pass

## Goal
Reduce code duplication, eliminate magic numbers, and consolidate common patterns across the entire SDL GUI codebase. No behavioral changes. Excludes `TextArea::handleEvent()` refactoring (kept for separate task).

---

## Phase 1: `sdl_rect_helpers.hpp` — Extend & Standardize

**Problem:** ~55 manual `SDL_FRect` cast patterns across 14 files. `sdl_rect_helpers.hpp` already has `RenderFillRect`, `RenderRect`, `RenderTexture`, `RenderLine`, `RenderPoint` overloads, but only `RenderLine`/`RenderPoint` are used.

**Changes to `src/sdl_rect_helpers.hpp`:**

1. Add `SDL_Color` → `SDL_FColor` conversion helper:
   ```cpp
   inline SDL_FColor ColorToFColor(SDL_Color c) {
       return {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
   }
   ```

2. Add `GetTextureWidth` / `GetTextureHeight` helpers (eliminates the repeated `SDL_GetTextureSize` + `static_cast<int>` pattern):
   ```cpp
   inline int TextureWidth(SDL_Texture* t) {
       float w = 0, h = 0;
       SDL_GetTextureSize(t, &w, &h);
       return static_cast<int>(w);
   }
   inline int TextureHeight(SDL_Texture* t) {
       float w = 0, h = 0;
       SDL_GetTextureSize(t, &w, &h);
       return static_cast<int>(h);
   }
   ```

3. Add a `RenderSetDrawColor` helper that takes `SDL_Color` directly (reduces `.r, .g, .b, .a` repetition):
   ```cpp
   inline void SetDrawColor(SDL_Renderer* r, SDL_Color c) {
       SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
   }
   ```

---

## Phase 2: Consolidated Geometry Constants (`gui.cpp`)

**Problem:** `drawRoundedFilledRect`, `drawRoundedTexturedRect`, `drawRoundedRectBorder` duplicate:
- `const int segments = 8;` (3 times)
- `float ba[4] = {3.1415927f, 4.712389f, 0, 1.5707963f};` (3 times)
- Corner geometry constants arrays `cx[4]`, `cy[4]` computed identically in each

**Changes:**

1. Add to namespace scope in `gui.cpp`:
   ```cpp
   namespace {
       constexpr int kCornerSegments = 8;
       constexpr float kPi       = 3.1415927f;
       constexpr float kPiHalf   = 1.5707963f;
       constexpr float k3PiHalf  = 4.712389f;
       constexpr float kBa[4]    = {kPi, k3PiHalf, 0, kPiHalf};
       constexpr SDL_FColor kTransparent = {0,0,0,0};
   }
   ```

2. Replace `const int segments = 8;` with `kCornerSegments` in all 3 functions.
3. Replace `float ba[4]` arrays with `kBa` in all 3 functions.
4. Replace `1.5707963f` in angle calculations with `kPiHalf`.
5. Replace `{0, 0, 0, 0}` in `renderToCache()` with `kTransparent`.

---

## Phase 3: Replace All Manual `SDL_FRect` Cast Patterns

**Problem:** ~55 occurrences of:
```cpp
{ SDL_FRect _fr = {static_cast<float>(r.x), ...}; SDL_RenderFillRect(renderer, &_fr); }
```

**Replace with existing helpers from `sdl_rect_helpers.hpp`:**

| Pattern | Replacement |
|---------|-------------|
| `{ SDL_FRect _fr = {static_cast<float>(rect.x),...static_cast<float>(rect.h)}; SDL_RenderFillRect(renderer, &_fr); }` | `RenderFillRect(renderer, rect)` |
| `{ SDL_FRect _fr = {static_cast<float>(r.x),...}; SDL_RenderRect(renderer, &_fr); }` | `RenderRect(renderer, r)` |
| `{ SDL_FRect _sr = {...}; SDL_FRect _dr = {...}; SDL_RenderTexture(renderer, tex, &_sr, &_dr); }` | `RenderTexture(renderer, tex, &srcRect, &dstRect)` |
| `{ SDL_FRect _dr = {...}; SDL_RenderTexture(renderer, tex, nullptr, &_dr); }` | `RenderTexture(renderer, tex, dstRect)` |

**Files to update:**
- `src/gui.cpp` — lines 10, 19-23, 48, 105, 124, 128-131, 373, 441 (16 occurrences)
- `src/animated_image.cpp` — lines 136, 196 (2 occurrences — also see Phase 4)
- `src/composite/dialog_box.cpp` — line 281
- `src/composite/file_dialog.cpp` — line 420
- `src/canvas.cpp` — lines 99, 202
- `src/combobox.cpp` — line 68
- `src/cursor.cpp` — line 158
- `src/progress_bar.cpp` — lines 117, 149
- `src/string_grid.cpp` — lines 758, 988, 1007, 1030, 1038, 1048, 1070, 1098, 1106, 1119, 1165, 1173 (12 occurrences)
- `src/text_area.cpp` — lines 119, 723, 756
- `src/text_input.cpp` — lines 126, 179, 209
- `src/slider.cpp` — lines 153, 171
- `src/radio_button.cpp` — lines 71, 78, 84
- `src/label.cpp` — line 90

---

## Phase 4: Deduplicate `animated_image.cpp`

**Problem:** `draw()` (line 136) and `drawDirect()` (line 196) have identical render calls. `draw()` and `drawDirect()` also share the `computedFrame` logic (lines 90-96 and 150-156).

**Changes:**

1. Extract common frame advance logic:
   ```cpp
   void AnimatedImage::advanceFrameIfNeeded() {
       int computedFrame = static_cast<int>(std::round(m_animFrame));
       computedFrame = std::clamp(computedFrame, 0, std::max(0, m_totalFrames - 1));
       if (computedFrame != m_currentFrame) {
           m_currentFrame = computedFrame;
           updateSrcRect();
           if (m_onFrameChanged) m_onFrameChanged(m_currentFrame);
       }
   }
   ```

2. Call `advanceFrameIfNeeded()` from both `draw()` and `drawDirect()` instead of the duplicate 7-line block.

3. Extract common destination rect computation into a helper that takes an offset `(dx, dy)`:
   ```cpp
   SDL_Rect AnimatedImage::computeDstRect(int offsetX, int offsetY) const;
   ```
   Both `draw()` (offset 0,0) and `drawDirect()` (offset absPos.x, absPos.y) can use it.

4. Replace both `{ SDL_FRect _sr = ...; SDL_FRect _dr = ...; }` blocks with `RenderTexture(renderer, m_texture.get(), &m_srcRect, &dst)`.

---

## Phase 5: Consolidate Hardcoded Strings & Colors as Constants

**Problem:** `"assets/fonts/font.ttf"` appears ~20 times. Default colors duplicated.

**Changes:**

1. Create `src/constants.hpp`:
   ```cpp
   #pragma once
   #include <SDL3/SDL.h>
   
   namespace constants {
       constexpr const char* kDefaultFontPath = "assets/fonts/font.ttf";
       constexpr SDL_Color kDefaultTextColor  {0,   0,   0,   255};
       constexpr SDL_Color kDefaultFillColor  {0,   120, 215, 255};
       constexpr SDL_Color kSelectionColor    {100, 150, 255, 180};
       constexpr SDL_Color kTitleBarColor     {200, 200, 200, 255};
       constexpr SDL_Color kTitleBarLineColor {150, 150, 150, 255};
       constexpr int      kTooltipDelayMs     = 500;
       constexpr int      kDefaultFontSize    = 16;
   }
   ```

2. Replace all occurrences of `"assets/fonts/font.ttf"` with `constants::kDefaultFontPath` in:
   - `src/combobox.cpp`
   - `src/editor/preview_window.cpp`
   - `src/editor/layout_exporter.cpp`
   - `src/gui_manager.cpp`
   - `src/layout_parser.cpp`
   - `src/progress_bar.cpp`
   - `src/string_grid.cpp`
   - `src/theme.cpp`
   - `src/text_input.cpp`
   - `src/label.cpp`

3. Replace `SDL_Color{0, 0, 0, 255}` with `constants::kDefaultTextColor` where used as default text color.

4. Replace `{100, 150, 255, 180}` selection highlight in `text_area.cpp:722` and `text_input.cpp:178` with `constants::kSelectionColor`.

5. Replace title bar colors in `dialog_box.cpp:277,284` and `file_dialog.cpp:416,423` with `constants::kTitleBarColor` and `constants::kTitleBarLineColor`.

6. Replace `500` (tooltip delay) in `gui.cpp:280` with `constants::kTooltipDelayMs`.

---

## Phase 6: Replace `SDL_Color → SDL_FColor` Conversions

**Problem:** Manual `{c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f}` appears in multiple places.

**Changes:**

Use `ColorToFColor()` from Phase 1 in:
- `src/gui.cpp:616-619` — `drawBackgroundAndBorder` fill color
- `src/gui.cpp:629-632` — `drawBackgroundAndBorder` border color
- `src/progress_bar.cpp:110-113` — progress bar fill color

---

## Phase 7: Replace `SDL_GetTextureSize` + Cast Pattern

**Problem:** 7+ occurrences of:
```cpp
float _fw=0,_fh=0; SDL_GetTextureSize(tex, &_fw, &_fh); int tw=static_cast<int>(_fw); int th=static_cast<int>(_fh);
```

**Changes:**

Use `TextureWidth()`/`TextureHeight()` (or a single `TextureSize()` returning a pair) from Phase 1 in:
- `src/animated_image.cpp:48`
- `src/combobox.cpp:66`
- `src/progress_bar.cpp:142`
- `src/string_grid.cpp:752, 998, 1060, 1113`
- `src/text_area.cpp:115`
- `src/text_input.cpp:120`

---

## Phase 8: Deduplicate Title Bar Rendering

**Problem:** `dialog_box.cpp:274-291` and `file_dialog.cpp:414-429` have identical title bar rendering (fill rect + separator line).

**Changes:**

Extract a common function, either:
- A free function in a new `sdl_helpers.hpp`, or
- A protected/static method on a shared base

Since both already inherit `Panel`, add to `Panel` or as a free function in `gui.hpp`:
```cpp
inline void drawTitleBar(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SetDrawColor(renderer, constants::kTitleBarColor);
    RenderFillRect(renderer, SDL_Rect{x, y, w, h});
    SetDrawColor(renderer, constants::kTitleBarLineColor);
    RenderLine(renderer, x, y + h, x + w, y + h);
}
```

Replace both `draw()` methods' title bar sections with `drawTitleBar(renderer, m_x, m_y, m_width, m_titleBarHeight)`.

---

## Phase 9: Remove Redundant `static_cast<float>` on `SDL_FRect` Members

**Problem:** In functions where `rect` is already `SDL_FRect`, the `static_cast<float>(rect.x)` is redundant (members are already float).

**Changes:**
- `gui.cpp:10` — `rect` is `SDL_FRect`, remove casts in direct `{static_cast<float>(rect.x), ...}` → use `rect` directly or `RenderFillRect(renderer, rect)`.
- `gui.cpp:48` — same for `drawRoundedTexturedRect` fallback.

---

## Phase 10: Cleanup & Replace `SDL_SetRenderDrawColor` with `SetDrawColor`

**Changes:**

Wherever `SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a)` appears and `c` is an `SDL_Color`, replace with `SetDrawColor(renderer, c)`:
- `canvas.cpp:54, 66, 110, 158`
- `composite/dialog_box.cpp:278, 284`
- `composite/file_dialog.cpp:417, 423`
- `editor/preview_window.cpp:310, 342`
- `window.cpp:126`
- `string_grid.cpp` — all `SDL_SetRenderDrawColor` with SDL_Color
- `text_area.cpp:722, 755`
- `text_input.cpp:178, 208`
- `slider.cpp:144, 156`
- `radio_button.cpp:69, 76, 87`
- `checkbox.cpp:74`
- `progress_bar.cpp:116` (if the color is SDL_Color; check)
- `combobox.cpp:74`

---

## Validation

After each phase or at completion:
```bash
./nob examples    # must compile all 39 examples
./nob test        # must pass 28/29 tests (1 pre-existing: combobox heap-use-after-free)
```

Also run the test suite while watching for runtime rendering correctness (no visual regression).

## Risks

- **Unused symbols after consolidation** — verify new helpers are actually called; remove old ones if they become dead code through the pass.
- **Lambda captures in `animated_image.cpp`** — when extracting `advanceFrameIfNeeded()`, ensure `m_onFrameChanged` call is preserved correctly.
- **`string_grid.cpp` is large (~1200 lines)** — the cast pattern replacements there need care to not break existing logic.

## Implementation Order

Recommended: Phase 1 → 2 → 5 → 9 → 6 → 3 → 7 → 10 → 4 → 8

Rationale: Build infrastructure first (Phase 1, 2, 5), then use it in cleanup phases (9, 6, 3, 7, 10), finishing with the more structural changes (4, 8).
