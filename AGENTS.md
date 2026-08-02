# AGENTS.md — SDL GUI

## Build system (nob)

```
./nob test          # build + run all tests
./nob test <filter> # run tests matching substring
./nob examples      # build all examples (default)
./nob release       # build release artifacts (.a, .so, combined header)
./nob clean         # clean output directories
./nob non_unity     # compile each .cpp separately (for IDE)
```

Po buildzie testy można uruchomić bezpośrednio:
```
./output/test_slider
./output/test_slider "Value Initialization"
./output/test_slider "[slider]"
```

## Projekt w pigułce

SDL GUI to lekka biblioteka GUI oparta na SDL3. Cel: ułatwić tworzenie narzędzi i prototypów desktopowych w C++.

### Kluczowe komponenty

| Warstwa | Elementy |
|---------|----------|
| **Core** | GUIManager (kontekst, renderowanie), GUIElement (hierarchia + cache tekstur), TextEditable (selekcja, clipboard) |
| **Widgety (23)** | Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu, Cursor, ArcContainer, ProgressBar, ScrollArea, ShaderPanel, RangeSlider, Splitter |
| **Composite** | DialogBox, MessageBox, FileDialog (`src/composite/`) |
| **Editor** | EditorWindow, EditorState, PreviewWindow, LayoutImporter, LayoutExporter (`src/editor/`) |
| **Ekrany/okna** | ScreenManager (gry), WindowManager (wiele okien systemowych) |
| **Zasoby** | TextureManager, FontManager, TimerManager, AnimationManager |
| **Parsery** | JsonParser, SGMLParser, LayoutParser — definicja GUI z JSON/XML |
| **Style** | Style + Theme — `unordered_map<string, array<optional<Style>, 4>>` per typ per stan (Normal/Hovered/Pressed/Disabled) |
| **Layout** | Anchor — responsywne pozycjonowanie (procenty 0-1, piksele >1, stretch, presety: center/fill/topBar/leftSidebar itd.) |

### Źródła

```
src/           — implementacja (C++23, moduły)
src/composite/ — gotowe dialogi
src/editor/    — edytor wizualny GUI
examples/      — 39 przykładów (numbered 00–40)
tests/         — 32 plików testowych (Catch2)
docs/          — dokumentacja (EN/PL)
lib/           — Catch2 amalgamated, tinyxml2
```

## Wzorce użycia

### SDLApp — helper RAII

`SDLApp` (`src/sdl_app.hpp`) inicjalizuje SDL3 i tworzy okno + renderer. Dwa konstruktory:

```cpp
// CPU renderer (standardowy)
SDLApp app("Tytuł", 800, 600);

// GPU renderer (dla ShaderPanel / Vulkan)
SDLApp app("Tytuł", 800, 600, false, GPU_VULKAN);
```

Dostęp: `app.getRenderer()`, `app.getWindow()`, `app.getGPUDevice()` (tylko GPU).  
Destruktor automatycznie sprząta — nie trzeba ręcznie niszczyć.

### Struktura każdego przykładu (boilerplate)

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "std.hpp"          // zamiast <iostream>, <memory> itd.

int main(int, char**) {
    try {
        // 1. Inicjalizacja
        SDLApp app("Tytuł", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();

        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());   // KONIECZNE
        guiManager.setWindowSize(800, 600);                 // dla anchorów

        // 2. Tworzenie widgetów
        auto widget = std::make_unique<Panel>(guiManager, x, y, w, h);
        widget->setBackgroundColor(ElementState::Normal, {45, 48, 58, 255});
        // ... konfiguracja ...
        guiManager.addElement(std::move(widget));

        // 3. Pętla główna — KOLEJNOŚĆ MA ZNACZENIE
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();    // timery, animacje, tooltipy
            guiManager.cleanup();   // usuwa elementy z markForDeletion()
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }

    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

**Krytyczne**: `processEvent` → `update` → `cleanup` → `render` — ta kolejność jest obowiązkowa.  
Pominięcie `update()` psuje tooltipy. Pominięcie `cleanup()` powoduje wyciek elementów z `markForDeletion()`.

### Tworzenie i dodawanie widgetów

Widgety tworzy się jako `std::unique_ptr`, konfiguruje, potem przekazuje ownership:

```cpp
// Wzorzec A: stwórz → skonfiguruj → dodaj
auto btn = std::make_unique<Button>(guiManager, 10, 10, 120, 40, "Kliknij");
btn->setBorder(ElementState::Normal, {100, 100, 255, 255}, 2);
btn->setBorderRadius(ElementState::Normal, 8);
btn->setOnClickCallback([](GUIElement*) { /* ... */ });
guiManager.addElement(std::move(btn));

// Wzorzec B: dzieci przed rodzicem
auto panel = std::make_unique<Panel>(guiManager, 0, 0, 200, 100);
panel->addChild(std::move(label));
panel->addChild(std::move(button));
guiManager.addElement(std::move(panel));
```

**Uwaga**: widgety top-level (dodane do `GUIManager`) mają współrzędne względem okna.  
Dzieci (`addChild()`) mają współrzędne względem rodzica.

### Style i Theme

`Style` ma same `optional<T>` — brak wartości = dziedziczenie z themu:

```cpp
Style s;
s.backgroundColor = {45, 48, 58, 255};
s.borderColor     = {98, 114, 164, 255};
s.borderWidth     = 2;
s.borderRadius    = 10;
widget->setStyle(ElementState::Normal, s);

// Skróty z GUIElement:
widget->setBackgroundColor(ElementState::Hover, {60, 63, 73, 255});
widget->setBorder(ElementState::Pressed, {200, 100, 100, 255}, 3);
widget->setTextColor(ElementState::Disabled, {128, 128, 128, 255});
```

Kaskada: `m_localStyles[state]` → `Theme[type][state]` → `Theme[type][Normal]` → `m_defaultStyle`.  
`ElementState`: `Normal`, `Hover`, `Pressed`, `Disabled`.

### Callbacki i ElementRef

Każdy widget ma własne callbacki. Do komunikacji między widgetami używa się `ElementRef<T>`:

```cpp
auto label = std::make_unique<Label>(guiManager, 10, 10, 100, 30, "0");
auto ref = guiManager.makeRef(label.get());   // PRZED std::move!

auto slider = std::make_unique<Slider>(guiManager, 10, 50, 200, 30, 0, 100, 50);
slider->setOnChangeCallback([ref](GUIElement* e) {
    auto* s = static_cast<Slider*>(e);
    if (s && ref) ref->setText(std::to_string(s->getValue()));
});

panel->addChild(std::move(label));
panel->addChild(std::move(slider));
```

`ElementRef` sprawdza `isElementAlive()` przy każdym dostępie — bezpieczny dangling pointer.

### Anchor (responsywny layout)

```cpp
Anchor::center()             // centruj w rodzicu
Anchor::fill(0)              // wypełnij cały rodzic
Anchor::topBar(50, 10, 10)   // pełna szerokość, 50px wysokości, 10px od góry i boków
Anchor::bottomBar(50, 10, 10)
Anchor::leftSidebar(200, 60, 70)
Anchor::horizontalStretch(5, 5)
```

Konwencja kodowania float: `<0` = nieustawione, `0-1` = procent, `>1` = piksele, `0.5` = centrum.

Dla resize: okno z `SDLApp("Tytuł", 800, 600, true)` (resizable), w pętli obsłuż `SDL_EVENT_WINDOW_RESIZED` → `guiManager.handleResize(w, h)`.

### Najczęstsze pułapki

| Problem | Przyczyna |
|---------|-----------|
| Widget się nie rysuje | Brak `guiManager.setTheme(Theme::createDefaultTheme())` |
| Tooltip nie znika | Brak `guiManager.update()` w pętli |
| Elementy nie są usuwane | Brak `guiManager.cleanup()` w pętli |
| Crash przy callbacku | `ElementRef` nie utworzony przed `std::move()` |
| Dziwne pozycje dzieci | Dzieci mają współrzędne względem rodzica, nie okna |
| Zmiana stylu nie działa | Trzeba wywołać `markDirty()` (settery robią to automatycznie, ale bezpośrednia zmiana `Style` nie) |

## Architektura szczegółowa

### Render flow
1. `GUIManager::render()` → `GUIElement::render()`
2. `wantsDirectRender()?` → `drawDirect()` : `renderToCache()` → `m_cachedTexture`
3. Dzieci renderowane z `parent_clip_rect`
4. GPU: elementy używają SDL_gpu przez `m_gpuState`

### Memory
- Hierarchia elementów: `unique_ptr`
- SDL resources: `SharedTexture`/`SharedFont` (`shared_ptr` z custom deleterami w `sdl_deleters.hpp`)
- Render cache: `m_cachedTexture` invalidowany przez `m_isDirty`
- Element lifecycle tracking: `m_liveElements` (unordered_set) dla bezpieczeństwa `ElementRef`

### Embedded Assets
System osadzania assetów (PNG, TTF) bezpośrednio w binarkach:

1. Manifest `assets.embed` — lista plików do osadzenia
2. `nob.c:build_embedded_assets()` — `ld -r -b binary` → `.o` z symbolami `_binary_<nazwa>_start/_end`
3. `output/embedded_assets.hpp` — auto-generowany header z `g_embeddedAssets[]`
4. `TextureManager::loadTextureFromMemory` / `FontManager::loadFontFromMemory` — `SDL_IOFromConstMem` → `IMG_Load_IO` / `TTF_OpenFontIO`
5. Po załadowaniu działa transparentnie: `loadTexture("assets/button1.png")` zwraca cache'owany embedded asset

### Hover performance (kluczowe)

Optymalizacje wprowadzone 2026-06-21:
- `getAbsolutePosition()` cache: `m_cachedAbsPos` + `m_absPosValid`, invalidowane rekurencyjnie
- `processHoverTooltip()` + `processButtonEvent()` — wyekstraktowana logika, jedno `contains()` zamiast podwójnego
- **Panel i widgety**: usunięte nadmiarowe `GUIElement::handleEvent()` powodujące podwójny DFS
- `SDL_GetMouseState()` → dane z eventu (SDL3 ma `mouse_x/y` w eventach)
- Efekt: przy tysiącach elementów: z kilkuset ms → 16 ms/klatkę

## Technologie

| Kategoria | Szczegóły |
|-----------|----------|
| **Język** | C++23 z modułami |
| **Kompilator** | `clang++-22` z `libc++` (LLVM-23) |
| **Zależności** | SDL3, SDL3_image, SDL3_ttf, tinyxml2 (wbudowany), Catch2 (amalgamated) |
| **Build** | `nob.c` + `nob.h` v3.8.0, unity build, `Nob_Procs` do równoległego linkowania |
| **Moduły** | Prekompilowane `std.pcm` i `std.compat.pcm` w `modules_cache/` |
| **Optymalizacje** | Release: `-O3 -march=native -flto`. Debug: `-g -O0 -fsanitize=address,undefined` |
| **Formatowanie** | `.clang-format` w korzeniu projektu |

### SDL3 API helpers
- `SDLRectToFRect()` — zamiast manualnych `static_cast<float>` na `SDL_Rect`
- `RenderRect()` — opakowanie `SDL_RenderRect`
- `SetDrawColor(renderer, c)` — zamiast rozwlekłego `SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a)`
- `TextureWidth()` / `TextureHeight()` — zamiast `SDL_GetTextureSize` + `static_cast<int>`
- `drawRoundedTexturedRect()` — renderowanie tekstury z zaokrąglonymi rogami (UV clipping przez `SDL_RenderGeometry`)

Zasoby muszą być dostępne przez `pkg-config sdl3 sdl3-image sdl3-ttf`. `PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`.

## Testy

- **Framework**: Catch2 (amalgamated: `lib/catch_amalgamated.hpp`)
- **Helper**: `tests/test_helper.hpp/cpp` — headless SDL init, `createMouseEvent()`, `createKeyboardEvent()`
- **Widgety testowane (18)**: Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, Splitter, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor
- **Menedżery (4)**: FontManager, TextureManager, TimerManager, AnimationManager
- **Systemy (5)**: GUIElement, GUIManager, Theme, Easing, UTF8
- **Screen/Window (2)**: ScreenManager, WindowManager
- **C API (1)**: test_sdl_gui_c_api (17 przypadków, Phase 0+1)
- **C API (1)**: test_sdl_gui_c_api (36 przypadków, Phase 0+1+2, 306 asercji)
- **C API (1)**: test_sdl_gui_c_api (40 przypadków, Phase 0+1+2, 316 asercji)
- **C API (1)**: test_sdl_gui_c_api (45 przypadków, Phase 0+1+2+timers, 331 asercji)
- **C API (1)**: test_sdl_gui_c_api (49 przypadków, Phase 0+1+2+3, 407 asercji)
- **C API (1)**: test_sdl_gui_c_api (50 przypadków, Phase 0+1+2+3, 415 asercji; + pixel test potwierdzający renderowanie kursora)
- **Brakujące testy**: ArcContainer, ProgressBar, ScrollArea, ShaderPanel, TextEditable, Style, parsery
- **Znane bugi**: Combobox — heap-use-after-free (pre-existing); Label nie ma override `getComponentType()` → naprawione w (2026-07-14)

Testy integracyjne: 44 przykładów w `examples/` do manualnej weryfikacji wizualnej (w tym 9 przykłady C i 1 demo C/C++).

## Powtarzalne zadania

### Jak dodać nowy widget

1. Stwórz `src/nazwa_widgetu.hpp` + `src/nazwa_widgetu.cpp`, klasa dziedziczy po `GUIElement`
2. Zaimplementuj `draw()` (rysowanie do cache) lub `wantsDirectRender()` + `drawDirect()`
3. Nob.c automatycznie wykrywa nowe pliki w `src/`, `src/composite/`, `src/editor/`
4. Stwórz `examples/example_nazwa_widgetu.cpp`, uruchom `./nob` i przetestuj
5. (Opcjonalnie) Stwórz `tests/test_nazwa_widgetu.cpp` z `TestHelper`, uruchom `./nob test`

### Jak dodać test jednostkowy

```cpp
#define CATCH_CONFIG_MAIN
#include "lib/catch_amalgamated.hpp"
#include "tests/test_helper.hpp"
#include "nazwa_widgetu.hpp"

TEST_CASE("NazwaWidget - opis", "[tag]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    SECTION("nazwa sekcji") {
        REQUIRE(warunek);
    }
}
```

Uruchom: `./nob test`

### Debugowanie

- **Problemy z zasobami**: sprawdź ścieżki i logi `SDL_LogError` z menedżerów
- **Widget się nie rysuje**: sprawdź flagę `m_isDirty`
- **Skomplikowany build**: `./nob non_unity` — kompiluje każdy `.cpp` osobno, łatwiej znaleźć błędy

## Bieżący stan i ostatnie zmiany

<!--
  ═══════════════════════════════════════════════════════════════════
  Ta sekcja jest NAJWAŻNIEJSZA dla ciągłości pracy między sesjami.
  Po KAŻDEJ znaczącej zmianie (nowy widget, optymalizacja, bugfix,
  refactor) dopisz zwięzły wpis z datą. Format:

  ### Krótki tytuł (YYYY-MM-DD)
  - Co zmieniono i dlaczego
  - Efekt: X/Y testów przechodzi, wszystkie przykłady się budują
  - Zmienione pliki: lista najważniejszych

  Nie duplikuj wpisów. Stare wpisy też mają wartość — pokazują
  historię decyzji projektowych. Jeśli lista staje się za długa
  (>20 wpisów), przenieś najstarsze do osobnego pliku CHANGELOG.md.
  ═══════════════════════════════════════════════════════════════════
-->

Starsza historia zmian (przed 2026-07-31): [CHANGELOG.md](CHANGELOG.md).

### C API Phase 3 — RangeSlider, Cursor, ShaderPanel + kontekst GPU (2026-08-02)
- **Luka**: C API (sdl_gui.h) nie opakowywał 3 widgetów istniejących w C++ — RangeSlider, Cursor, ShaderPanel (GPU). Splitter nie istnieje w tym checkout (osobna sesja).
- **RangeSlider**: pełne opakowanie — create (z orientacją), get/set lower/upper (clamp `lower ≤ upper`), setRange/setMin/setMax/getMin/getMax, wheelStep (min 1), setOnChange.
- **Cursor**: `sdlgui_cursor_create` instaluje kursor przez `GUIManager::setCursor` (dodany getter `getCursor()` — cursor nie jest w `m_elements`, tylko w `m_liveElements`); enum `sdlgui_cursor_state_t` (9 stanów 1:1 z CursorState), `set_texture`/`set_animated_texture` (sprite-sheet z fps), set/get state/offset/scale, `set_visible`/`is_visible` (osobne — `GUIElement::setVisible` nie jest wirtualne, Cursor::setVisible steruje SDL_HideCursor/ShowCursor), `set_on_state_changed` (nowy callback `sdlgui_cursor_state_callback_t`).
- **ShaderPanel + GPU**: `GUIContext` dostał konstruktor GPU (`SDLApp` z `GPU_VULKAN`) + `getGPUDevice()`; nowe `sdlgui_create_gpu()` (zwraca NULL bez Vulkan) i `sdlgui_get_gpu_device()`. Na kontekście CPU ShaderPanel renderuje się jako zwykły panel, shader ignorowany (`set_shader` z NULL/0-size bezpiecznie no-op). `sdl_gui.h` dołącza teraz `<SDL3/SDL_gpu.h>`.
- **Testy**: +4 case'y (RangeSlider — 9 sekcji w tym clampy/swap/callback; Cursor — 7 sekcji; ShaderPanel — 4 sekcje + render CPU; GPU context — soft test, pomija gdy brak Vulkan). 50/50 case'ów, 415 asercji (w tym pixel test renderowania kursora); pełny suite 33/33.
- **Przykłady**: nowy `examples/c/10_range_slider_cursor.c` (pure C — **jeden kontekst**: GPU-first z fallbackiem na CPU; na systemach z Vulkanem RangeSlider+Cursor+ShaderPanel w jednym oknie); `examples/41_c_api_demo.cpp` rozszerzony o Phase 3 (RangeSlider/Cursor przez C API na widgetach C++).
- **Bugfix przykłady (feedback użytkownika)**: kursor niewidoczny w `c_10` i `41_c_api_demo` — przyczyny: (a) `41` tworzył Cursor bez żadnej tekstury (systemowy kursor ukryty → brak kursora); (b) `c_10` tworzył DRUGIE okno GPU i puszczał tylko jego pętlę — okno z RangeSlider/kursorem nigdy się nie renderowało; (c) `assets/button2.png` nie istnieje (użyte w c_10). Fix: tekstury w obu przykładach (button1.png/button_bg.png), c_10 przepisany na jeden kontekst. Nowy `sdlgui_get_window()` w API (potrzebny do `SDL_WarpMouseInWindow`); pixel test `Cursor renders pixels` — warp myszy → render → odczyt piksela spod kursora (odczyt PRZED `RenderPresent` — po present backbuffer jest niezdefiniowany).
- **Docs**: `docs/release/c_api.md` — nowe sekcje (GPU context, RangeSlider, Cursor, ShaderPanel), callback w tabeli Callbacki, nota o Cursor w ownership.
- **Uwaga**: LeakSanitizer zgłasza wyciek ~50 KB w X11_VideoInit SDL3 przy tworzeniu kontekstu GPU (Vulkan) — zewnętrzny (SDL), nie nasz kod, nie zmienia exit code.
- Efekt: 33/33 testów, 50/50 case'ów C API, 48/48 examples (w tym nowy C), release + C smoke test + standalone OK. Zmienione: `src/sdl_gui.h` (+~135 linii API, w tym sdlgui_get_window), `src/sdl_gui_c_api.cpp` (+~260 linii), `src/gui_context.hpp` (GPU ctor + getGPUDevice), `src/gui_manager.hpp` (getCursor), `tests/test_sdl_gui_c_api.cpp`, `examples/c/10_range_slider_cursor.c` (nowy), `examples/41_c_api_demo.cpp`, `docs/release/c_api.md`, `AGENTS.md` (ten wpis)

### Shared render cache — dedup per (style, state, size) (2026-08-02)
- **Problem**: `GUIElement::m_cachedTexture` był per-elementowym render-targetem — 30 identycznych przycisków = 30 tekstur w VRAM, a każde najechanie myszą przeliczało teksturę od nowa dla każdego elementu z osobna.
- **Fix**: cache renderowania przeniesiony do TextureManagera jako **współdzielony, niezmienny wpis** — `TextureManager::renderCache(uint64_t key, w, h, draw)` w osobnej mapie `m_renderCache`: hit → zwraca istniejący wpis bez rysowania, miss → render-once + wstaw. Wpisy nigdy nie są nadpisywane (bezpieczne współdzielenie), usuwane przez `pruneUnused()` (use_count==1).
- **Klucz** — numeryczny FNV-1a 64-bit (zero alokacji, ~20 ns): typ + w×h + stan + pełny skomponowany styl (wszystkie pola z tagiem obecności) + sufiks widgetu. GUIElement trzyma `m_cacheKey` i przy dirty tylko robi lookup — zmiana stanu to zmiana klucza, nie re-render; 30 identycznych przycisków = 1 wpis Normal + 1 Hover + 1 Pressed (każdy wyrenderowany raz).
- **Poprawność**: focus outline usunięty z `drawBackgroundAndBorder` (zatruwałby klucz — wpis sfokusowanego elementu wyciekłby do niefokusowanych) → rysowany jako overlay w `render()` po blicie cache (dla rotacji zostaje pieczony). Nowe wirtuale: `canShareRenderCache()` (domyślnie true — draw zależy tylko od stylu; opt-out: Checkbox, RadioButton, Slider, RangeSlider, ProgressBar, ComboBox, Cursor, AnimatedImage, TextEditable) i `getRenderCacheKeySuffix()` (Label: hash m_text+m_font_size). Rotacja (`m_rotation != 0`, piecze dzieci) → niesdzielona ścieżka jak dawniej.
- **Sprzątanie**: `pruneUnused()` (wcześniej martwy) wołany z `GUIManager::update()` gdy `getRenderCacheSize() > 256` — stany Hover/Pressed żyją między najechaniami, porzucone wpisy znikają. Usunięty podwójny cache tekstu w Label (`m_cachedTextTexture`/`m_cachedTextContent`/`m_cachedTextColor`/`m_cachedFontSize`/`m_textTextureDirty` — nigdy nie trafiał, bo `draw()` wywołuje się raz na unikalny klucz widget-cache, a `createTextureFromText` jest sam kluczowany). `renderToCache()` przeniesiony do `protected` (brak zewnętrznych wywołań).
- Efekt: 33/33 testy (nowy `tests/test_render_cache.cpp` — 4 sekcje: dedup identycznych, wspólny wpis Hover, oddzielny wpis dla innego stylu, bypass opt-out), 47/47 examples, `non_unity` OK. Zmienione: `src/texture_manager.hpp`/`.cpp` (renderCache, m_renderCache, prune/clear obu map), `src/gui.hpp`/`.cpp` (SharedTexture cache, klucz, focus overlay), 9 nagłówków widgetów (opt-out/sufiks), `src/gui_manager.cpp` (prune policy), `tests/test_render_cache.cpp` (nowy), `AGENTS.md` (ten wpis)

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
