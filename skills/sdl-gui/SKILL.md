---
name: sdl-gui
description: "Building desktop applications and RTS games in C++23 with the SDL GUI library (SDK shipped in the dist/ directory: sdl_gui.hpp, libsdl_gui.a/.so, docs/). This skill should be used when the user wants to write a GUI application, desktop tool, prototype, game UI (menus, HUD, screens) or an RTS strategy game based on SDL3 with this library — and when they need to understand usage patterns, compile a program against dist/, or troubleshoot widgets, layout (anchors), themes or the event loop. Widget creation uses manager.create or make_unique plus addElement; details in dist/docs/getting_started.md and dist/docs/core.md."
---

# SDL GUI — Building RTS Games and GUI Applications

## Purpose

Teach an agent to build complete applications (RTS games, GUI tools,
prototypes) in C++23 with the SDL GUI library. The library is shipped as a
ready-made SDK in the `dist/` directory — **do not rewrite its API or
documentation into code**: read from `dist/` on demand (full docs in
`dist/docs/`). This skill provides mandatory rules, a workflow, architecture
patterns, and a documentation map.

## Locating the library

Before starting, locate the SDK (usually `dist/` in the project directory or
the user's working directory):

```
dist/
├── sdl_gui.hpp        # entire C++ API — the ONLY project include
├── sdl_gui.h          # C API (sdlgui_ prefix) — for C code
├── libsdl_gui.a       # static library
├── libsdl_gui.so      # shared library
└── docs/              # full documentation
```

Verify the directory exists (`ls dist/`). If the SDK lives elsewhere, use the
found path in all commands and includes instead of `dist/`. If it cannot be
found, ask the user for the path before writing code.

## Foundations — mandatory rules

These rules apply to EVERY program. Breaking them yields invisible widgets,
stuck tooltips, leaks, or crashes inside callbacks.

1. **One include**: `#include "sdl_gui.hpp"` — that is all; do not include anything else from the SDK.
2. **A theme is required**: without `manager.setTheme(...)` widgets have no colors and are invisible. Use `ThemePresets::createDarkTheme()` (or another preset), or `GUIContext` (applies a theme automatically).
3. **Event loop order is mandatory**:
   `processEvent` → `update` → `cleanup` → `render`. Skipping `update()` breaks tooltips/animations/timers; skipping `cleanup()` leaks elements marked with `markForDeletion()`. If you do not write the loop manually, use the built-in `SDLApp::run(manager, clearColor, onEvent)` or `GUIContext::run()`.
4. **Coordinate systems**: children (`addChild`/`create(parent, ...)`) use coordinates relative to the parent; top-level elements are relative to the window.
5. **`ElementRef` before `std::move`**: callbacks must not capture raw pointers to other widgets — create `ElementRef<T>` BEFORE transferring ownership, then check `if (ref)`.
6. **`markDirty()` after direct `Style` mutation**: setters (`setBackgroundColor` etc.) do this automatically; direct mutation of `Style` fields does not.

## Application-building workflow

1. **Locate the SDK** (`dist/`) and open `dist/docs/getting_started.md` — compilation/linking, Hello World pattern.
2. **Choose the architecture** (see "Architecture selection" below) — an RTS game uses `ScreenManager` (menu/pause/gameplay) and renders the game world with low-level SDL3; a tool application uses panels, anchors, and dialogs.
3. **Write the skeleton**: `SDLApp` (or `GUIContext`) → `GUIManager` → `setTheme` → `setWindowSize`. For resizable windows: `SDLApp(title, w, h, true)` + handle `SDL_EVENT_WINDOW_RESIZED` → `manager.handleResize(w, h)`.
4. **Build the UI with widgets** per the docs (map below). Create widgets via `manager.create<T>(...)` (returns `T*`) or `std::make_unique` + `addElement(std::move(...))`. Attach callbacks with `ElementRef` when a callback touches other widgets.
5. **Run the loop**: `app.run(manager, {40,42,54,255}, onEvent)` or a manual loop per rule 3.
6. **Compile and verify** (commands below). The program must build with just `-I dist`; test by running it (if there is no display, report that to the user — do not block delivering the code).

### Architecture selection

| Goal | Architecture | Details |
|------|-------------|---------|
| RTS game (menu, gameplay, pause, settings) | `ScreenManager` + `Screen` (one screen per game phase) | `references/rts-game.md` |
| Tool application (editor, dashboard, forms) | Single window: panels + anchors + dialogs; optionally `WindowManager` for multiple windows | `references/gui-app.md` |
| Screen + overlay (pause, settings over the game) | `ScreenManager::pushScreen/popScreen` (overlay stack) | `references/rts-game.md` |

## Documentation map (`dist/docs/`)

Read the relevant file BEFORE using an API — signatures are 1:1 with the header:

| Need | Document |
|------|----------|
| Compilation, linking, first program | `getting_started.md` |
| `GUIElement` (position, styles, anchor, focus, deletion) and `GUIManager` (create, ElementRef, captureMouse) | `core.md` |
| Patterns: call order, ElementRef, styles, anchors, dialogs, GPU | `patterns.md` |
| A widget — e.g. Button, Slider, ListView, Canvas | `widgets/<Name>.md` (index: `widgets/README.md`) |
| DialogBox, MessageBox, FileDialog | `composites.md` |
| GUIManager, FontManager, TextureManager, TimerManager, AnimationManager, ScreenManager, WindowManager | `managers.md` |
| Style/Theme/ThemePresets, Anchor, SDLApp, GUIContext, layout parsers (JSON/XML), logging, Easing | `resources.md` |
| C API (`sdlgui_*`) | `c_api.md` |

In addition, the dev repo has examples in `examples/` (00–47, numbered simple →
complex, e.g. `37_screen_manager.cpp`, `43_gamepad_controller.cpp`,
`29_resize.cpp`) — use them as reference; in target code use only `sdl_gui.hpp`.

## Compilation and linking

```bash
# shared
clang++ -std=c++23 -stdlib=libc++ -I dist app.cpp -L dist -lsdl_gui \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)

# static (requires -flto — the library is built with LTO)
clang++ -std=c++23 -stdlib=libc++ -O3 -flto -I dist app.cpp dist/libsdl_gui.a \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)

# C: compile with gcc -std=c11 -pedantic-errors -I dist -c app.c, link with clang++ (the library is C++)
```

If SDL3 is installed in a non-standard location:
`export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`.
Running with a shared library outside system paths: `LD_LIBRARY_PATH=dist ./app`.

## Common errors and their causes

| Symptom | Cause |
|---------|-------|
| Widgets invisible | Missing `setTheme(...)` |
| Tooltip never disappears / animations frozen | Missing `update()` in the loop |
| Elements marked with `markForDeletion()` never disappear | Missing `cleanup()` in the loop |
| Crash in a callback | Raw pointer captured after `std::move` — use `ElementRef` |
| Children in wrong places | Child coordinates are relative to the parent, not the window |
| `Style` change has no effect | Missing `markDirty()` after direct mutation |
| Tab navigation does nothing | Missing `setCanGetKeyboardFocus(true)` on the element |
| Anchors do not respond to resize | Missing `setWindowSize` at startup or missing `handleResize` on `SDL_EVENT_WINDOW_RESIZED` |
| ShaderPanel does not work | Requires GPU renderer: `SDLApp(title, w, h, false, GPU_VULKAN)` |

## Resources

### references/

- `references/rts-game.md` — RTS game architecture: screens, game loop (fixed timestep), rendering the game world alongside the GUI, HUD, input (keyboard/mouse/gamepad), dialogs, performance.
- `references/gui-app.md` — tool application architecture: layout (anchors), toolbars, forms, dialogs, file open/save, themes, multiple windows (`WindowManager`).

Load the relevant file when working on a game or an application project.
