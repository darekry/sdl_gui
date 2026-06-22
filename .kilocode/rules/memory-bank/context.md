# Aktualny stan projektu (2026-06-21)

## Status: Hover performance optimized

## Ostatnie zmiany

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
