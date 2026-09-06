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

## Archiwum (`docs/archive/` — nieaktualne, tylko historyczne)

### Widget Documentation (EN/PL, era SDL2)

- [Getting Started Guide](archive/getting_started.md) - Setup and basic usage
- [AnimatedImage](archive/en/animated_image.md) - Frame-based animations from sprite sheets
- [ContextMenu](archive/en/context_menu.md) - Right-click context menus
- [Using SDL GUI for RTS Games](archive/en/for_rts.md) - Guide for real-time strategy game UIs
- [Pierwsze kroki](archive/pl/getting_started.md) - Przewodnik konfiguracji
- [Użycie SDL GUI w grach RTS](archive/pl/for_rts.md) - Przewodnik dla interfejsów RTS

### API Reference (nieaktualne, zastąpione przez `release/`)

- [Button](archive/api/Button.md), [Checkbox](archive/api/Checkbox.md), [GUIManager](archive/api/GUIManager.md), [Panel](archive/api/Panel.md)

### Ściąga (nieaktualna — używa usuniętego `setWindowSize()`)

- [SDL GUI — ściąga](archive/pigulka.md)

## Technical Documentation (aktualne)

- [Plan dużego refaktoru](refactor_plan.md) — status prac strukturalnych (Event, Layout, Style, TextModel, Lifetime, Factory)

## Archiwum techniczne (`docs/archive/` — zrealizowane propozycje, tylko historyczne)

- [Mouse Cursor](archive/mouse_cursor.md) - Custom cursor system (PL, stara nazwa `MouseCursor`)
- [Responsive Layout System](archive/responsive_layout_proposal.md) - propozycja anchorów (zrealizowana inaczej: enum `HAnchor`/`VAnchor` + `LayoutPass`)
- [Texture & Font Manager Review](archive/texture_font_manager_review.md) - Technical code review
- [TextInput/TextArea plan](archive/text_input_text_area_implementation_plan.md) - zrealizowany (unifikacja `TextEditable`, char-index UTF-8)
- [WYSIWYG Editor plan](archive/wysiwyg_editor_plan.md) - zrealizowany (`src/editor/`, przykład 45)

## Project Structure

| Directory | Description |
|-----------|-------------|
| `src/` | Library implementation (C++23) |
| `src/composite/` | DialogBox, MessageBox, FileDialog |
| `examples/` | 49 example applications (00–48) |
| `tests/` | Unit tests (Catch2) |
| `docs/` | Documentation (EN/PL) |
| `docs/release/` | End-user docs, copied to `dist/docs/` by `./nob release` |
| `dist/` | Release artifacts: `sdl_gui.hpp`, `sdl_gui.h`, `libsdl_gui.a/.so`, `docs/` |
