# Aktualny stan projektu (2026-06-23)

## Status: Container optimization pass completed

## Ostatnie zmiany

### Container & data structure optimization (2026-06-23)
- **Phase A (Theme)**: `map<string, map<ElementState, Style>>` → `unordered_map<string, array<optional<Style>, 4>>` — O(1) zamiast O(log n) przy każdym `getComposedStyle()`. Usunięto `ThemeTypeCompare`.
- **Phase B (StringGrid cache)**: `map<string, SharedTexture>` + `map<size_t, CompareFunc>` → `unordered_map` — spójne z TextureManager.
- **Phase C**: ~~GUIManager addElement~~ — REVERTED (konstruktor GUIElement już rejestruje w m_liveElements, false-positive).
- **Phase D (ListView)**: Usunięto zbędne `std::string()` kopie przy `insertItem`/`removeItem`.
- **Phase E (TextArea)**: Dodano `m_line_textures.reserve(m_lines.size())`.
- **Phase F (StringGrid)**: `loadFont()` wyciągnięty z pętli `drawCells()`/`drawColumnHeaders()`/`drawRowHeaders()` — font ładowany raz w `drawDirect()` i przekazywany jako `TTF_Font*`.
- **Phase G (gui.cpp)**: Dodano `verts.reserve(192)` w `drawRoundedRectBorder`.
- **Phase I**: `Cursor::m_cursors` map→array, `EditorElement::properties` map→unordered_map, `EditorWindow` 5× map→unordered_map, `PreviewWindow::m_widgetMap` map→unordered_map, `timer_manager` reserve.
- **Phase J (EditorState)**: Dodano `m_idToIndex` (unordered_map) i `m_parentToChildren` (unordered_map z lazy rebuild) — O(1) findElementById i getElementsByParent.
- **Efekt**: 39/39 examples, 28/29 tests pass (1 pre-existing combobox bug).
- **Phase 3**: Zastąpiono 4 manualne `static_cast<float>` na członach `SDL_Rect` helperem `SDLRectToFRect()` w `gui.cpp`, `preview_window.cpp`. Dodatkowo `preview_window.cpp` używa teraz `RenderRect()` zamiast `SDL_RenderRect()`.
- **Phase 4**: Wyekstrahowano `computeScaledDstRect(offsetX, offsetY)` w `animated_image.cpp` — eliminuje ~50 linii zduplikowanej logiki skalowania między `draw()` i `drawDirect()`.
- **Phase 7**: Zastąpiono 4 wystąpienia `SDL_GetTextureSize` + `static_cast<int>` helperami `TextureWidth()`/`TextureHeight()` w `text_area.cpp`, `cursor.cpp`, `combobox.cpp`. (`gui.cpp:430` i `texture_manager.cpp:264` pominięte — mają error checking).
- **Phase 10**: Zastąpiono ~21 wywołań `SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a)` helperem `SetDrawColor(renderer, c)` w: `canvas.cpp`, `slider.cpp`, `radio_button.cpp`, `progress_bar.cpp`, `combobox.cpp`, `window.cpp`, `string_grid.cpp`, `preview_window.cpp`.
- **Efekt**: 39/39 examples, 28/29 tests pass (1 pre-existing combobox heap-use-after-free). ~30+ linii usuniętych, znacząco mniej powtórzeń.

### Hover performance optimization (2026-06-21)
- **Problem**: przy dużej ilości zagnieżdżonych komponentów, przeliczanie pozycji i hit-testing podczas ruchu myszy powodowało klatki po kilkaset ms
- **getAbsolutePosition() cache** (`src/gui.cpp:209-235`): dodane `mutable m_cachedAbsPos` + `m_absPosValid`, invalidowane rekurencyjnie przy `setPosition()`/`setParent()`/`addChild()`. Po pierwszym wyliczeniu — O(1)
- **processHoverTooltip()** (`src/gui.cpp:349-364`): wyekstraktowana logika tooltipu, przyjmuje już obliczony stan hover — eliminuje podwójne `contains()`
- **processButtonEvent()** (`src/gui.cpp:338-347`): wyekstraktowana obsługa przycisków myszy
- **Panel::handleEvent()** (`src/panel.cpp:22-79`): usunięte wywołanie `GUIElement::handleEvent(event)` które powodowało **podwójny DFS** po dzieciach; zastąpione `processHoverTooltip()`+`processButtonEvent()`
- **SDL_GetMouseState()** → dane z eventu: `panel.cpp`, `scroll_area.cpp`, `string_grid.cpp` (SDL3 ma `mouse_x/y` w `SDL_MouseWheelEvent`)
- Button, Checkbox, TextInput, TextArea — ten sam wzorzec: jedno `contains()` → `processHoverTooltip()` + `processButtonEvent()`
- **Efekt**: przy kilku tysiącach elementów i intensywnym ruchu myszy: z kilkuset ms → 16 ms/klatkę (5 ms bez ruchu)

### Simplified examples (2026-06-21)
- **examples/15_widgets_combo.cpp**: New replacement for old sprite_animator. Demonstrates multiple widgets (Button, Checkbox, Slider, ComboBox) with tooltips, ContextMenu, Theme::createDefaultTheme(), and Anchor::center() in a cohesive mini-application. 118 lines.
- **examples/24_button.cpp**: 139 → 89 lines. Reduced 5 variants to 3 (default themed, colored green, textured with border-radius). Added tooltips and Theme::createDefaultTheme(). Removed image-only and styled-border variants.
- **examples/33_hover_animation.cpp**: 319 → 160 lines. Reduced 4 classes to 2 (HoverAnimatedLift + HoverStaticLift). Removed HoverAnimatedScale and HoverStaticScale. Added Theme::createDefaultTheme(). Kept labels for animated/static comparison and AnimationManager info.

### Rounded corner texture clipping fix (2026-06-20)
- **Problem**: Texture rendered by drawBackgroundAndBorder ignored rounded corners
- **Fix**: drawRoundedTexturedRect in src/gui.cpp uses SDL_RenderGeometry with normalized UV coordinates
- **Build**: 39/39 examples, 29/29 tests compile (1 pre-existing test fail: combobox heap-use-after-free)

### SDL2 → SDL3 Migration (2026-06-13) ✅ Complete

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 41 (all compile, numbered 00–40 by complexity) |
| Test files | 31 total (29 test + 2 infra) |
| Widget types | 21 (+ 3 composite) |
| Editor modules | 5 |
