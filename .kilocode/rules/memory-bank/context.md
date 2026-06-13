# Aktualny stan projektu (2026-06-13)

## Status: MIGRACJA SDL2 → SDL3 W TOKU

**Repozytorium:** Core library (src/), examples (37), i tests (31) skompilowane z SDL3.  
**Pre-existing issue:** `__promote_t` STL/module error rozwiązany przez `std::numbers::pi` zamiast `M_PI`.

## Ostatnie zmiany

### SDL2 → SDL3 Migration (2026-06-13)

**Completed (Stages 1-9):**
- **Stage 1:** `nob.c` — pkg-config SDL3, **bugfix: dangling pointer w pkg_config_cmd()** (`nob_temp_strdup`)
- **Stage 2:** Skrypty automatyzacyjne: rename_headers, rename_symbols, rename_macros
- **Stage 3:** Core files — SDL_WINDOWEVENT decomposition, SDL_CreateWindow/CreateRenderer API, SDL_INIT flags, **sdl_app.hpp: IMG_Init block removed**
- **Stage 4:** Widgets — float mouse coords casts, SDL_Rect → SDL_FRect w render API, TTF_RenderText_Blended, SDL_GetTextureSize
- **Stage 5:** SDL_gfx replacement — `drawRoundedFilledRect()`, `drawRoundedRectBorder()` via SDL_RenderGeometry
- **Stage 6:** Return type fixes — SDL_Init, TTF_Init, IMG_Init removed
- **Stage 7:** TimerManager — Uint64 ticks
- **Stage 8:** Examples — wszystkie 37 kompilują się (sdl_app.hpp, example_resize, example_window, example_arc_container)
- **Stage 9:** Tests — wszystkie 31 kompilują się (test_helper refactor na SDL3 event API, SDL_Rect→SDL_FRect, SDL_QueryTexture→SDL_GetTextureSize, SDL_INIT_EVERYTHING→SDL_INIT_VIDEO)

**Key API changes handled:**
| SDL2 | SDL3 |
|------|------|
| SDL_QueryTexture | SDL_GetTextureSize (float*) |
| SDL_RenderFillRect (SDL_Rect*) | SDL_RenderFillRect (SDL_FRect*) |
| SDL_RenderDrawRect / SDL_RenderCopy | SDL_RenderRect / SDL_RenderTexture (SDL_FRect*) |
| e.key.keysym.sym | e.key.key (flat struct) |
| e.button.x/y (int) | e.button.x/y (float) |
| SDL_WINDOWEVENT | individual SDL_EVENT_WINDOW_* events |
| SDL_CreateRenderer(-1, flags) | SDL_CreateRenderer(NULL) |
| SDL_ShowCursor(0/1) | SDL_HideCursor() / SDL_ShowCursor() |
| TTF_RenderUTF8_Blended | TTF_RenderText_Blended (+length param) |
| TTF_SizeUTF8 | TTF_GetStringSize (+length param) |
| TTF_GetError() | SDL_GetError() (removed from SDL3_ttf) |
| IMG_Init/IMG_Quit | removed (on-demand loading in SDL3_image) |
| roundedBoxRGBA/roundedRectangleRGBA | drawRoundedFilledRect/drawRoundedRectBorder (GPU) |
| SDL_CreateRGBSurface / SDL_MapRGBA | SDL_CreateSurface / SDL_MapSurfaceRGBA |
| e.button.state / e.key.state | implicit (event type: DOWN vs UP) |
| SDL_PRESSED / SDL_RELEASED | removed |
| SDL_TEXTINPUTEVENT_TEXT_SIZE | removed (text is const char*) |
| SDL_RENDERER_PRESENTVSYNC / ACCELERATED | removed (default behavior) |
| SDL_Init() < 0 | !SDL_Init() (returns bool) |
| SDL_INIT_EVERYTHING | SDL_INIT_VIDEO (must list flags explicitly) |
| SDL_StartTextInput(NULL) | SDL_StartTextInput(SDL_GetRenderWindow(renderer)) |
| TTF_SizeUTF8 returns int (0=success) | TTF_GetStringSize returns bool (true=success) |

**Remaining:**
- Stage 10: GPU shader example (nowe)

### Post-migration bugfixes (2026-06-13):
- **nob.c**: pkg_config_cmd() dangling pointer fix (`nob_temp_strdup`)
- **text_input.cpp**: `TTF_GetStringSize` return value check `!= 0` → `!` (SDL2's TTF_SizeUTF8 returned 0=success, SDL3_ttf returns true=success)
- **text_editable.cpp + text_area.cpp**: `SDL_StartTextInput(NULL)` → `SDL_GetRenderWindow(m_manager.getRenderer())` (SDL3 requires valid window)
- **text_input.cpp**: debug log messages updated from `TTF_SizeUTF8` to `TTF_GetStringSize`

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 37 (all compile) |
| Test files | 31 (all compile) |
| Widget types | 18 |
