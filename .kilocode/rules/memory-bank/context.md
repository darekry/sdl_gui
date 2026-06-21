# Aktualny stan projektu (2026-06-21)

## Status: Example simplification + Memory bank active

## Ostatnie zmiany

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
