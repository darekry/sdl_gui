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
- **Helper**: `tests/test_helper.hpp/cpp` — headless SDL init (okno tworzone jako `SDL_WINDOW_HIDDEN`), `createMouseEvent()`, `createKeyboardEvent()`
- **Uruchamianie**: `./nob test` uruchamia binarki testowe **równolegle** (dynamiczna kolejka, domyślnie `min(16, nprocs)` zadań; `NOB_TEST_JOBS=<n>` zmienia limit). Output każdego testu trafia do `output/test_logs/<nazwa>.log` — przy porażce wypisywany jest ogon logu. Okna SDL w testach są ukrywane przez zmienną `SDL_GUI_HIDDEN=1` (ustawianą przez runnera; respektują ją `SDLApp`, `Window` i `WindowManager`).
- **Widgety testowane (23)**: Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor, ArcContainer, ProgressBar, ScrollArea, ShaderPanel (CPU), TextEditable (bazowa klasa przez podklasę testową), Style
- **Menedżery (4)**: FontManager, TextureManager, TimerManager, AnimationManager
- **Systemy (6)**: GUIElement, GUIManager, Theme, Easing, UTF8, Anchor (presety, kodowanie 0-1/px, stretch, resize)
- **Screen/Window (2)**: ScreenManager, WindowManager
- **Parsery (3)**: JsonParser, SGMLParser, LayoutParser (fixture'y w `tests/data/` — `layout.json`, `layout.xml`, `widgets.json`, `bad.*`)
- **C API (1)**: test_sdl_gui_c_api (50 przypadków, Phase 0+1+2+3, 415 asercji; + pixel test potwierdzający renderowanie kursora — wymaga zmapowanego okna, więc przy `SDL_GUI_HIDDEN` jawnie woła `SDL_ShowWindow`)

Testy integracyjne: 47 przykładów w `examples/` do manualnej weryfikacji wizualnej (w tym 9 przykładów C i 1 demo C/C++).

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
  (>10 wpisów), przenieś najstarsze do osobnego pliku CHANGELOG.md.
  ═══════════════════════════════════════════════════════════════════
-->

Starsza historia zmian (przed 2026-07-31): [CHANGELOG.md](CHANGELOG.md).

### Bugfix: flaky test_sdl_gui_c_api — Cursor śledzi pozycję myszy z eventów (2026-08-24)
- **Problem**: test "Cursor renders pixels" wymuszał `SDL_WarpMouseInWindow` + polling `SDL_GetMouseState`; po migracji dev-maszyny na Wayland warp przestał działać (kompozytor nie pozwala aplikacjom przesuwać wskaźnika) → deterministyczny fail `0 == 50` (wcześniej losowy flake). `Cursor::renderOverlay` pollował `SDL_GetMouseState()` w renderze, niezależnie od systemu eventów.
- **Fix**: `Cursor::handleEvent()` śledzi `SDL_EVENT_MOUSE_MOTION` (`m_mouseX/Y`, flaga `m_hasMousePos`); `renderOverlay()` używa śledzonej pozycji, z fallbackiem na `SDL_GetMouseState()` dopóki nie przyszedł żaden motion (kompatybilność wstecz). `GUIManager::processEvent()` w gałęzi mouse-capture forwarduje event do kursora przed early-return — kursor nie zamiera podczas dragowania. Test: syntetyczny motion event przez `sdlgui_process_event` zamiast warp+mapowania okna (bez `SDL_ShowWindow`, bez `SDL_Delay`, bez pętli pollującej).
- Efekt: 43/43 testów przechodzi; test [pixel] 5× PASS pod rząd.
- Zmienione pliki: src/cursor.hpp, src/cursor.cpp, src/gui_manager.cpp, tests/test_sdl_gui_c_api.cpp

### Bugfix: etykiety Buttonów przycinane przy tworzeniu przez parser (2026-08-25)
- **Problem**: parsery layoutów tworzą widgety z rozmiarem (0,0) i dopiero potem wołają `setSize()`. `Button` centruje Label-dziecko tylko w konstruktorze — po `setSize` etykieta wisała na ujemnych współrzędnych i była przycinana przez clip rect (widać tylko ogonki tekstu: "okno", "uj", "K").
- **Fix**: nowy chroniony hook wirtualny `GUIElement::onSizeChanged(oldW, oldH)` wołany z `setSize()` przy faktycznej zmianie wymiarów; `Button` nadpisuje go i re-centruje etykietę (`m_label` — nowy członek). Przykłady 30/31 (JSON/XML parser) dostają `setWindowSize` + obsługę `SDL_EVENT_WINDOW_RESIZED` (anchory top-level reagują na resize).
- Efekt: 43/43 testów przechodzi; nowe case'y: test_button.cpp ("label re-centers when size changes"), test_layout_parser.cpp (etykieta OK wycentrowana po parse, dialog wycentrowany przez anchor z JSON).
- Zmienione pliki: src/gui.hpp, src/gui.cpp, src/button.hpp, src/button.cpp, examples/30_json_parser.cpp, examples/31_xml_parser.cpp, tests/test_button.cpp, tests/test_layout_parser.cpp

### Bugfix: anchory aplikowane od razu przy dodawaniu, nie dopiero przy resize (2026-08-24)
- **Problem**: `GUIManager::addElement()` i `GUIElement::addChild()` nie aplikowały anchorów — elementy dodane po starcie (np. dialog zbudowany w callbacku) miały dzieci na pozycjach konstrukcyjnych do pierwszego `SDL_EVENT_WINDOW_RESIZED`. Objaw: przyciski OK/Anuluj w (0,0), "brakujące" widgety (nakładka z-order), przykład 48_win95_bevel po zamknięciu i "Pokaż okno".
- **Fix**: `addElement()` woła `updateLayout(m_windowWidth, m_windowHeight)` (guard >0), `addChild()` woła `child->updateLayout(m_width, m_height)`. Dla elementów bez anchorów no-op. Uwaga: `getX()` zwraca współrzędne względem rodzica.
- Efekt: 42/43 (1 fail = pre-existing flake warp myszy w test_sdl_gui_c_api, potwierdzony na czystym drzewie przez git stash); nowe case'y w test_anchor.cpp (aplikacja przy add/addChild/subtree bez resize).
- Zmienione pliki: src/gui_manager.cpp, src/gui.cpp, tests/test_anchor.cpp, examples/48_win95_bevel.cpp

### Bevel 3D w stylu Windows 95/98 (2026-08-24)
- **Nowość**: fazowane obramowanie 3D — fundament pod look Win95/98. `Style` dostaje 4 opcjonalne kolory krawędzi (`borderColorOuter/InnerTopLeft/BottomRight`), `GUIElement::setBevel(state, BevelType::Raised/Sunken)` wypełnia je z palety systemowej (`constants::kWin95Face/Light/Highlight/Shadow/DarkShadow`). Rysowanie: wolne funkcje `drawBevelFrame()` / `drawStyleBevel()` w gui.cpp; bevel ma priorytet nad zwykłą ramką i rysuje się ostro (bez zaokrągleń). `buildRenderCacheKey()` domieszuje nowe pola; `mergeWith`/`operator==` rozszerzone. Paletę aplikuje wolna funkcja `applyBevelToStyle(Style&, BevelType)`.
- **Parsery**: `LayoutParser::parseStyle` obsługuje shorthand `"bevel": "Raised"|"Sunken"` (JSON attr / XML attr) + 4 jawne kolory `borderColorOuter/InnerTopLeft/BottomRight` (nadpisują shorthand). `getComposedStyle()` upublicznione (testy/debug). Fixture: `tests/data/win95_bevel.json` (layout przykładu 48 — ładuje go `./output/30_json_parser tests/data/win95_bevel.json`), `tests/data/win95_bevel.xml`.
- Efekt: 43/43 testów przechodzi; nowe case'y w test_style.cpp (mergeWith, równość, setBevel per stan, render smoke) i test_layout_parser.cpp (bevel JSON+XML).
- Zmienione pliki: src/constants.hpp, src/style.hpp, src/gui.hpp, src/gui.cpp, src/layout_parser.cpp, tests/test_style.cpp, tests/test_layout_parser.cpp, examples/48_win95_bevel.cpp
- Następne kroki (niezrealizowane): Theme::createWindows95Theme(), audyt widgetów z własnym draw() (Checkbox, Slider, ScrollArea…), pressed content offset 1px dla Buttona.

### Cleanup: usunięcie redundantnych null-checków i martwej obsługi błędów (2026-08-22)
- **Niezmienniki udokumentowane w kodzie**: `m_children`/`m_elements`/`m_windows` nigdy nie zawierają nulli (filtrowane przed push); `getTimerManager()` zawsze nie-null (ctor); wartości `PreviewWindow::m_widgetMap` nigdy null; `ScrollArea::m_viewport/m_content`, członkowie FileDialog/DialogBox przypisani tylko w ctorach.
- Usunięto: checki iteracyjne `if (child && ...)` / `if (element && ...)` nad kontenerami unique_ptr; straż na `getTimerManager()` w `startTimer/stopTimer`; `if (!self)` w callbackach timerów; martwy licznik `total_removed_count` w `GUIManager::cleanup()`; O(n) skan duplikatów w `addElement()` (osiągalny tylko przez UB); podwójne checki `m_selectedCell`, `m_cellEditor+m_isEditing`, `m_messageLabel`, `m_pathLabel/m_titleLabel/m_filenameInput`, `m_dirGrid/m_fileGrid`, widget map w preview_window, dead `if (element)` w `LayoutParser::parseNode` + straż w `parseStyle`.
- Zachowano: straż na granicach publicznego API (`addElement`, `detachElement`, `register/unregisterElement`, `isElementAlive`, parametry renderer), obsługę błędów SDL/fontów/IO, walidację wejścia parserów.
- Efekt: 42/43 testów przechodzi (1 porażka = pre-existing flake środowiskowy warp myszy w test_sdl_gui_c_api, pada też na czystym drzewie), wszystkie przykłady się budują.
- Zmienione pliki: src/gui.cpp, src/gui_manager.cpp, src/scroll_area.cpp, src/tab_control.cpp, src/window_manager.cpp, src/string_grid.cpp, src/animated_image.cpp, src/composite/dialog_box.cpp, src/composite/file_dialog.cpp, src/editor/editor_window.cpp, src/editor/preview_window.cpp, src/layout_parser.cpp

### Bugfix: TextArea nie uczestniczył w systemie keyboard focus — martwa edycja (2026-08-02)
- 
### Bugfix: TextArea pominięty w opt-out współdzielonego render cache (2026-08-02)


### Testy anchorów + 2 realne bugi w systemie Anchor (2026-08-02)
- **Nowy test**: `tests/test_anchor.cpp` (4 case'y, 47 asercji) —

### Test runner równoległy + ukryte okna + brakujące testy (2026-08-02)
- **Problem**: `./nob test` uruchamiał 33 binarki sekwencyjnie (~160 s; każdy test ~2-4 s startu SDL/ASAN) i zalewał konsolę logami.

### C API Phase 3 — RangeSlider, Cursor, ShaderPanel + kontekst GPU (2026-08-02)


### Shared render cache — dedup per (style, state, size) (2026-08-02)

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
