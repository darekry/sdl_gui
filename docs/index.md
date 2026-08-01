# SDL GUI Library Documentation

Dokumentacja biblioteki SDL GUI (SDL3, C++23). Główna dokumentacja end-user
trafia do `dist/docs/` przy każdym `./nob release` — to ona jest źródłem
prawdy dla odbiorców biblioteki (nie mają dostępu do `src/`).

## Dokumentacja release (`docs/release/` → `dist/docs/`)

- [Getting Started](release/getting_started.md) — instalacja SDL3, kompilacja, linkowanie, Hello World
- [Fundamenty (core)](release/core.md) — GUIElement, GUIManager, ElementRef, cykl życia
- [Wzorce użycia](release/patterns.md) — zalecane wzorce i typowe pułapki
- [Widgety](release/widgets/README.md) — 22 widgety (Button, Slider, StringGrid, TextArea, ShaderPanel, ...)
- [Kompozyty](release/composites.md) — DialogBox, MessageBox, FileDialog
- [Menedżery](release/managers.md) — FontManager, TextureManager, TimerManager, AnimationManager, ScreenManager, WindowManager
- [Zasoby](release/resources.md) — Style/Theme, Anchor, SDLApp, GUIContext, parsery, logowanie
- [C API](release/c_api.md) — referencja `sdl_gui.h` (200 funkcji)

## Starsza dokumentacja (do archiwizacji)

### Widget Documentation (EN/PL)

- [Getting Started Guide](getting_started.md) - Setup and basic usage
- [AnimatedImage](en/animated_image.md) - Frame-based animations from sprite sheets
- [ContextMenu](en/context_menu.md) - Right-click context menus
- [Using SDL GUI for RTS Games](en/for_rts.md) - Guide for real-time strategy game UIs
- [Pierwsze kroki](pl/getting_started.md) - Przewodnik konfiguracji
- [Użycie SDL GUI w grach RTS](pl/for_rts.md) - Przewodnik dla interfejsów RTS

### API Reference (nieaktualne, zastąpione przez `release/`)

- [Button](api/Button.md), [Checkbox](api/Checkbox.md), [GUIManager](api/GUIManager.md), [Panel](api/Panel.md)

## Technical Documentation

- [Mouse Cursor](mouse_cursor.md) - Custom cursor system (PL)
- [Responsive Layout System](responsive_layout_proposal.md) - Anchor system for window resizing
- [Texture & Font Manager Review](texture_font_manager_review.md) - Technical code review

## Project Structure

| Directory | Description |
|-----------|-------------|
| `src/` | Library implementation (C++23) |
| `src/composite/` | DialogBox, MessageBox, FileDialog |
| `examples/` | 48 example applications |
| `tests/` | Unit tests (Catch2) |
| `docs/` | Documentation (EN/PL) |
| `docs/release/` | End-user docs, copied to `dist/docs/` by `./nob release` |
| `dist/` | Release artifacts: `sdl_gui.hpp`, `sdl_gui.h`, `libsdl_gui.a/.so`, `docs/` |
