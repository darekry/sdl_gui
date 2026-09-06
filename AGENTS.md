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
| **Widgety (22)** | Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, RangeSlider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu, Cursor, ArcContainer, ProgressBar, ScrollArea, ShaderPanel |
| **Composite** | DialogBox, MessageBox, FileDialog (`src/composite/`) |
| **Editor** | EditorWindow, EditorState, PreviewWindow, LayoutImporter, LayoutExporter (`src/editor/`) |
| **Ekrany/okna** | ScreenManager (gry), WindowManager (wiele okien systemowych) |
| **Zasoby** | TextureManager, FontManager, TimerManager, AnimationManager |
| **Parsery** | JsonParser, SGMLParser, LayoutParser — definicja GUI z JSON/XML |
| **Style** | Style + Theme — `unordered_map<string, array<optional<Style>, 4>>` per typ per stan (Normal/Hovered/Pressed/Disabled) |
| **Layout** | Viewport (NonZero w ctorze GUIManagera) + Anchor (`HAnchor`/`VAnchor` enum + marginesy px) + LayoutPass Measure/Arrange (`ILayoutManager`: `AnchorLayout` domyślny, `StackLayout`; `layoutChildren()` per widget) |

### Źródła

```
src/           — implementacja (C++23, moduły)
src/composite/ — gotowe dialogi
src/editor/    — edytor wizualny GUI
examples/      — 49 przykładów (00–48); examples/c/ — 10 przykładów C
tests/         — 43 binarki testowe (Catch2)
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

        GUIManager guiManager(renderer, Viewport{800, 600});   // viewport NonZero w ctorze
        guiManager.setTheme(Theme::createDefaultTheme());   // KONIECZNE

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

### Anchor + LayoutPass (responsywny layout)

```cpp
Anchor::center()             // centruj w rodzicu
Anchor::fill(0)              // wypełnij cały rodzic
Anchor::topBar(50, 10, 10)   // pełna szerokość, 50px od góry, 10px marginesy boczne
Anchor::bottomBar(50, 10, 10)
Anchor::leftSidebar(60, 70)   // szerokość z konstruktora elementu
Anchor::horizontalStretch(5, 5)
Anchor::bottomRightAt(12, 34)  // osobne marginesy prawa/dół
Anchor::topCenter(40)          // środek poziomy, 40px od góry
Anchor::pinned(HAnchor::Right, VAnchor::Bottom, 0, 0, 12, 34)  // escape hatch
```

Enum per oś (`HAnchor::{None,Left,Center,Right,Stretch}`, `VAnchor::{None,Top,Center,Bottom,Stretch}`) + marginesy int w px. Brak magicznych floatów: `1px` osiągalne, center to wariant (nie `0.5`). Silnik: jeden pass Measure/Arrange (`ILayoutManager`: domyślny `AnchorLayout`, `StackLayout` do pasów/kolumn); widgety z własną geometrią nadpisują `layoutChildren()` (Button: label, Slider: track, ScrollArea: viewport/slidery, TabControl: zakładki, DialogBox/FileDialog: pas przycisków). Parser czyta `anchorH/anchorV` (`none|left|center|right|stretch` / `none|top|center|bottom|stretch`) + `marginLeft/Top/Right/Bottom` (px) i tworzy od razu docelowy rect (bez dummy `(0,0)`).

Dla resize: okno z `SDLApp("Tytuł", 800, 600, true)` (resizable), w pętli obsłuż `SDL_EVENT_WINDOW_RESIZED` → `guiManager.handleResize(w, h)` (propaguje do WSZYSTKICH top-level, też bez anchorów; wymiary <= 0 ignorowane — niezmiennik NonZero).

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
- **Widgety testowane (22)**: Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, RangeSlider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor, ArcContainer, ProgressBar, ScrollArea, ShaderPanel (CPU)
- **Menedżery (4)**: FontManager, TextureManager, TimerManager, AnimationManager
- **Systemy (11)**: GUIElement, GUIManager, Theme, Easing, UTF8, Anchor (enum H/V + marginesy px, presety, Viewport NonZero, LayoutPass, StackLayout), Style, TextEditable (bazowa klasa przez podklasę testową), RenderCache, RenderPixel (pikselowa walidacja renderowania), Performance
- **Screen/Window (2)**: ScreenManager, WindowManager
- **Parsery (3)**: JsonParser, SGMLParser, LayoutParser (fixture'y w `tests/data/` — `layout.json`, `layout.xml`, `widgets.json`, `win95_bevel.json/xml`, `bad.*`)
- **C API (1)**: test_sdl_gui_c_api (Phase 0+1+2+3; + pixel test renderowania kursora — pozycja myszy ze syntetycznego motion eventu, bez warpowania wskaźnika)

Testy integracyjne: 49 przykładów (`examples/`, 00–48) + 10 przykładów C (`examples/c/`) do manualnej weryfikacji wizualnej.

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

### Layout / Anchor — Viewport NonZero + enum + LayoutPass (2026-09-05)
- **Nowość**: `Viewport{w,h}` wstrzykiwany do `GUIManager` w ctorze (niezmiennik NonZero — brak `0x0`, koniec `setWindowSize()` i fallbacku `800x600` w `ContextMenu`; `handleResize(<=0)` ignorowane, np. minimalizacja). `Anchor` jako enum per oś (`HAnchor`/`VAnchor` + marginesy int px) zamiast magicznych floatów (`<0/0-1/>1/==0.5`): `1px` osiągalne, center to wariant; nowe `at/pinned/topCenter/bottomRightAt`. Jeden `LayoutPass` Measure/Arrange (`src/layout.hpp`: `ILayoutManager`, `AnchorLayout` domyślny, `StackLayout` z `arrangeStrip` do pasów przycisków). `layoutChildren()` zamiast `onSizeChanged()` (Button: label, Slider: track+przyciski, ScrollArea: viewport/slidery — koniec shadowowania `updateLayout`, TabControl: zakładki+panele, DialogBox/FileDialog: pas przycisków). Usunięte `m_originalW/H`/`storeOriginalSize` (center z bieżącego rozmiaru). Parser tworzy od razu docelowy rect (bez dummy `(0,0)` + `setSize`), kotwice z `anchorH/anchorV` + `margin*` (px). `handleResize` propaguje do WSZYSTKICH top-level (fix: rodzic bez anchora blokował resize zakotwiczonych dzieci). DialogBox/FileDialog centrują z realnego viewportu (koniec hardcode `800x600`), C-API: `sdlgui_anchor_t{h,v,l,t,r,b}` + `sdlgui_anchor_make` (koniec `anchor_raw`/floatów).
- **Zero shimów**: stare API usunięte (flota `Anchor`, `applyAnchor`, `onSizeChanged`, `onParentResize`, `setWindowSize`, `AnchorMode`, `anchorLeft/...` w plikach) — poprawione 48 przykładów, fixture'y JSON/XML, C-API i testy. `getAbsolutePosition`-cache i współdzielony render-cache bez zmian (perf).
- **Przy okazji (pre-existing, blokowały `./nob release`)**: `text_area.cpp` nie includował `gui_manager.hpp` (sypał per-file compile), `nob.c` — `layout.hpp` w combined header + `theme_presets.hpp` po `gui.hpp` (undeclared `applyBevelToStyle` w `dist/sdl_gui.hpp`); `WindowManager::createWindow` łapie `std::exception` (okno 0x0 → deterministyczny `nullptr` przez NonZero Viewport).
- Efekt: 44/44 testów przechodzi (test_anchor: 103 asercje/7 case'ów — nowe regresje: 1px, center bez historii, setSize rodzica bez anchora, propagacja przez rodzica bez anchora, brak (0,0) przed resize, NonZero przy resize 0x0, StackLayout dialogów), 48/48 przykładów + release (47_standalone, C smoke) zielone.
- Zmienione pliki: src/anchor.hpp (rewrite), src/layout.hpp/cpp (nowe), src/gui.hpp/cpp, src/gui_manager.hpp/cpp, src/button.hpp/cpp, src/slider.hpp/cpp, src/tab_control.hpp/cpp, src/scroll_area.hpp/cpp, src/context_menu.cpp, src/composite/dialog_box.hpp/cpp, src/composite/file_dialog.hpp/cpp, src/layout_parser.hpp/cpp, src/window.cpp, src/window_manager.cpp, src/gui_context.hpp, src/text_area.cpp, src/sdl_gui.h, src/sdl_gui_c_api.cpp, nob.c, examples/* (48), examples/layouts/*, tests/data/win95_bevel.json, tests/test_anchor.cpp (rewrite), tests/test_gui_manager.cpp, tests/test_window_manager.cpp, tests/test_sdl_gui_c_api.cpp, tests/test_text_area.cpp, tests/test_render_*.cpp, tests/test_context_menu.cpp, tests/test_helper.cpp, docs/release/{core,resources,managers,patterns,getting_started,c_api}.md + skeletony w docs/release/widgets/*.md

### StyleResolver faza 1 — ComponentType + cache scalonego stylu + BorderRenderer (2026-09-05)
- **Nowość**: `src/component_type.hpp` — enum `ComponentType:uint8_t` jako **jedyny** klucz typu w libie (stringi `"Button"…` tylko na granicy: pliki layoutu przez `componentTypeFromString`, C-API przez `componentTypeToString`). Usunięte: wirtualny `getComponentType()` ze wszystkich widgetów, stringowa mapa i overloady w `Theme` (została tablica `O(1)` `[typ][stan]` + `epoch()`). Klucz render-cache używa ID. `GUIElement::getComposedStyle()` cache'uje scalony styl per stan, przelicza tylko przy zmianie epoki themu/lokalnej.
- **Globalny współdzielony cache zachowany**: `TextureManager::m_renderCache` bez zmian — identyczne widgety dają identyczny klucz i współdzielą 1 teksturę (test: 100 identycznych paneli → 1 wpis; inny rozmiar → nowy wpis).
- **BorderRenderer**: `drawResolvedBorder()` jeden kod bevel-vs-plain (bevel ma priorytet, ostry jak Win95); `RadioButton` używa go zamiast własnej kopii brancha (semantyka texture-jako-indykator zachowana). `Style::hasBevel()`.
- **Koniec nadużycia `borderColor`**: nowe `thumbColor` (Slider/RangeSlider uchwyt) i `fillColor` (ProgressBar wypełnienie) z fallbackiem do `borderColor` — stare motywy/przykłady działają bez zmian; `setThumbColor/setFillColor`, klucz cache je domiesza.
- Efekt: 44/44 testów przechodzi (nowe case'y sharingu RTS w test_style.cpp), 48/48 przykładów się buduje bez zmian kodu.
- Zmienione pliki: src/component_type.hpp (nowy), src/style.hpp, src/theme.hpp, src/theme.cpp, src/gui.hpp, src/gui.cpp, src/slider.cpp, src/range_slider.cpp, src/progress_bar.cpp, src/radio_button.cpp, tests/test_style.cpp, docs/release/resources.md, docs/release/widgets/Slider.md, docs/release/widgets/ProgressBar.md

### Unifikacja TextArea z TextEditable — jeden model tekstu char-index (2026-09-05)
- **Refaktor**: `TextArea : public TextEditable` (wcześniej `: GUIElement` z równoległym API byte-index). Usunięte duplikaty: tekst/selekcja/schowek/focus/blink/menu/locked — wszystko dziedziczone; zostały tylko linie/wrap/scroll/nawigacja góra/dół. `setLocked/isLocked` i `setContextMenuEnabled` przeniesione do bazy (`TextInput::setLocked` deleguje do `TextEditable::setLocked`). Wszystkie pozycje w **znakach UTF-8** (fix: `erase/insert/strlen/m_cursorPos++/--`, mieszane `length()+1` z `charToByteIndex`, `substr` w `getSelection`).
- **Edge-case'y zlikwidowane**: pisanie wymaga fokusu (było `m_isHovered || hasFocus` — hover wystarczał); drag działa poza bounds (`hasKeyboardFocus` zamiast `m_isHovered`, jak w TextInput); `recalculateLines` zawija **wszystkie paragrafy** (wcześniej tylko ostatni), zachowuje wielokrotne spacje i łamie zbyt długie słowa po znakach; `\r\n` jak pojedynczy break; strzałki na granicach zwracają `false` (jak TextInput); Ctrl+C/V/X/A przez wspólne `handleClipboard*`/`selectAll`.
- **Przykłady**: bez zmian kodu (API kompatybilne — `setText/setOnTextChanged/setWordWrap/setLocked` zachowane), 48/48 się buduje.
- Efekt: 44/44 testów przechodzi (nowe case'y UTF-8 char-index w test_text_area.cpp).
- Zmienione pliki: src/text_editable.hpp, src/text_editable.cpp, src/text_input.hpp, src/text_input.cpp, src/text_area.hpp, src/text_area.cpp, tests/test_text_area.cpp, docs/release/widgets/TextArea.md, docs/release/widgets/TextEditable.md

### RMB callback + domyślne menu kontekstowe pól tekstowych (2026-09-05)
- **Nowość**: `GUIElement::setOnRightClickCallback(fn(element, x, y))` — callback prawego przycisku z pozycją kliknięcia (współrzędne okna); RMB jest konsumowane, więc przodkowie nie odpalają swoich callbacków. Widgety nadpisujące `handleEvent` (Button, TextInput, TextArea) wołają chroniony helper `processRightClick(e)`.
- **Domyślne menu kontekstowe**: `GUIManager::showContextMenu(items, x, y)` — jedno leniwie tworzone `ContextMenu` współdzielone (przebudowa itemów przy każdym pokazaniu). TextInput i TextArea: RMB **nie startuje już drag-selekcji**, tylko otwiera menu Wytnij/Kopiuj/Wklej/Zaznacz wszystko z enabled-state zależnym od selekcji i clipboardu; per-widget `setContextMenuEnabled(false)` wyłącza. `TextEditable` dostaje publiczne `copyToClipboard/cutToClipboard/pasteFromClipboard/selectAll` (virtual); TextArea **nie dziedziczy po TextEditable** (byte-index zamiast char-index) — ma równoległe własne API. Akcje menu guardują żywotność widgetu przez `isElementAlive` (menu może przeżyć widget).
- **ContextMenu**: `positionMenu` używa realnego rozmiaru okna (`GUIManager::getWindowSize`, fallback 800×600 gdy nieustawione) zamiast hardcode'u — bez tego (rozmiar 0×0) clamp zerował pozycję do (0,0); wysokość liczona z separatorami. Gettery `getItemCount()`/`isItemEnabled(i)` dla testów.
- **Przykład 12**: menu otwiera się prawdziwym RMB na pozycji kursora (wcześniej fake przez `setOnClickCallback`); przykład woła `setWindowSize` (bez niego clamp menu liczył na rozmiarze 0×0 i zerował pozycję do (0,0)).
- **Bugfix (duch klikniętego itemu)**: klik w item kradł focus klawiatury (`Button` robi `setKeyboardFocus` przy BUTTON_DOWN), a `closeMenu()` go nie oddawało — `GUIManager::render` malował potem przycisk przez focus-overlay mimo ukrytego menu (z ramką focusu, znikał dopiero po kliknięciu gdzie indziej). Fix: `ContextMenu::hide()` zwalnia focus, gdy wskazuje w głąb menu (focus spoza menu, np. pisane pole tekstowe, zostaje nietknięty).
- Efekt: 43/43 testów przechodzi, 48 przykładów się buduje.
- Zmienione pliki: src/gui.hpp, src/gui.cpp, src/gui_manager.hpp, src/gui_manager.cpp, src/context_menu.hpp, src/context_menu.cpp, src/text_editable.hpp, src/text_editable.cpp, src/text_input.cpp, src/text_area.hpp, src/text_area.cpp, examples/12_context_menu.cpp, tests/test_gui_element.cpp, tests/test_text_input.cpp, tests/test_text_area.cpp, tests/test_context_menu.cpp

### Label wieloliniowy — `\n` łamie linię (2026-08-25)
- **Nowość**: `Label` obsługuje wiele linii — tekst zawierający `\n` dzielony jest na linie (`updateLines()` → `m_lines`); szerokość elementu = najszersza linia, wysokość = liczba linii × `TTF_GetFontHeight()` (konwencja z TextArea). Rysowanie: jedna cache'owana tekstura per linia (`createTextureFromText`), każda w naturalnym rozmiarze, lewe wyrównanie; puste linie tylko przesuwają yOffset. `\r\n` traktowane jak pojedynczy break. Tekst jednolinijkowy bez zmian (stara ścieżka: `TTF_GetStringSize`, rozciągnięcie do rozmiaru elementu przy ręcznym `setSize`). Render-cache key bez zmian (hash całego tekstu).
- **Testy**: nowe case'y w test_label.cpp ("Label multi-line support": wysokość = n×font height, szerokość najszerszej linii, puste/wiodące/kończące `\n`, CRLF, przełączanie single↔multi przez setText, render smoke).
- Efekt: 43/43 testów przechodzi, 48 przykładów się buduje.
- Zmienione pliki: src/label.hpp, src/label.cpp, tests/test_label.cpp, docs/release/widgets/Label.md

### Bugfix: tooltip za mały dla tekstu wieloliniowego (2026-08-25)
- **Problem**: `GUIManager::showTooltip()` wymiarował panel przez `FontManager::getTextSize` (jedna linia, `TTF_GetStringSize`) — przy tekście z `\n` panel był za niski/wąski i linie wystawały poza tło (przykład 11, tooltip checkboxa). Sam Label renderował już wieloliniowo poprawnie.
- **Fix**: panel wymiarowany z faktycznych wymiarów `m_tooltipLabel` po `setText()` (jedno źródło prawdy, zero duplikacji logiki pomiaru). Stałe `TOOLTIP_FONT_SIZE`/`TOOLTIP_PADDING` przeniesione do `constants::kTooltipFontSize/kTooltipPadding`; nowy publiczny getter `GUIManager::getActiveTooltip()`.
- **Testy**: test_gui_manager.cpp — "Multi-line tooltip panel fits all lines" (szerokość = najszersza linia + 2×padding, wysokość = 3×font height + 2×padding) + sekcja "Single-line tooltip shrinks back".
- Efekt: 43/43 testów przechodzi, 48 przykładów się buduje.
- Zmienione pliki: src/gui_manager.cpp, src/gui_manager.hpp, src/constants.hpp, tests/test_gui_manager.cpp, docs/release/core.md, docs/release/managers.md

Starsza historia zmian (do 2026-08-02): [CHANGELOG.md](CHANGELOG.md).

### Theme::createWindows95Theme() + audyt bevel w widgetach (2026-08-25)
- **Nowość**: `Theme::createWindows95Theme()` (deleguje do `ThemePresets::createWindows95Theme()`) — autentyczny Win95/98 na systemie faz 3D: Button Raised w Normal/Hover/Disabled i Sunken w Pressed; TextInput/TextArea/ListView/ComboBox/Canvas/StringGrid białe + Sunken; ProgressBar biały + Sunken z navy wypełnieniem (`borderColor` = kolor fill w widgetcie); Slider/RangeSlider `borderColor` = kolor suwaka; ContextMenu biało-granatowe; Panel/TabControl/ScrollArea płaskie. Helper `ThemePresets::withBevel(Style, BevelType)` owija `applyBevelToStyle`. Disabled pola edycji gubią bevel (szary tekst). Bevel NIE przecieka: typy bez fazy mają czyste `optional` (Panel flat, unknown type → default bez bevel).
- **Audyt custom-draw**: tylko `RadioButton` rysował ramkę ręcznie — dostał gałąź `drawStyleBevel` przed fallbackiem na `RenderRect` (semantyka texture-jako-indykator zachowana, celowo bez `drawBackgroundAndBorder`, bo base traktuje `style.texture` jako tło). Pozostałe: Checkbox/ComboBox/TextInput/TextArea/StringGrid wołają `drawBackgroundAndBorder` wprost; Slider/RangeSlider/ProgressBar/DialogBox przez `Panel::draw`; ScrollArea/TabControl dziedziczą Panel — wspierają bevel automatycznie. Label/Cursor/AnimatedImage/Canvas/ShaderPanel bez ramek z definicji.
- **Przykład 48**: przepisany na `createWindows95Theme()` — usunięte ręczne helpery `applyWin95Button`/`applyWin95Sunken` (dialog/status bar trzymają lokalny bevel jako ramy okna).
- **Testy/docs**: nowe case'y w test_theme.cpp ("Windows95 theme": kolory faz Raised/Sunken, Disabled drop bevel, ProgressBar navy, StringGrid gridlines, brak przecieku do Panel/unknown); dokumentacja (core.md, resources.md, patterns.md).
- Efekt: 43/43 testów przechodzi (5 pełnych przebiegów), 48 przykładów się buduje.
- Zmienione pliki: src/theme.hpp, src/theme.cpp, src/theme_presets.hpp, src/radio_button.cpp, examples/48_win95_bevel.cpp, tests/test_theme.cpp, docs/release/core.md, docs/release/resources.md, docs/release/patterns.md

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
- Następne kroki: ~~Theme::createWindows95Theme()~~, ~~audyt widgetów z własnym draw()~~ (zrealizowane 2026-08-25, patrz wyżej). Pozostało: pressed content offset 1px dla Buttona.

### Cleanup: usunięcie redundantnych null-checków i martwej obsługi błędów (2026-08-22)
- **Niezmienniki udokumentowane w kodzie**: `m_children`/`m_elements`/`m_windows` nigdy nie zawierają nulli (filtrowane przed push); `getTimerManager()` zawsze nie-null (ctor); wartości `PreviewWindow::m_widgetMap` nigdy null; `ScrollArea::m_viewport/m_content`, członkowie FileDialog/DialogBox przypisani tylko w ctorach.
- Usunięto: checki iteracyjne `if (child && ...)` / `if (element && ...)` nad kontenerami unique_ptr; straż na `getTimerManager()` w `startTimer/stopTimer`; `if (!self)` w callbackach timerów; martwy licznik `total_removed_count` w `GUIManager::cleanup()`; O(n) skan duplikatów w `addElement()` (osiągalny tylko przez UB); podwójne checki `m_selectedCell`, `m_cellEditor+m_isEditing`, `m_messageLabel`, `m_pathLabel/m_titleLabel/m_filenameInput`, `m_dirGrid/m_fileGrid`, widget map w preview_window, dead `if (element)` w `LayoutParser::parseNode` + straż w `parseStyle`.
- Zachowano: straż na granicach publicznego API (`addElement`, `detachElement`, `register/unregisterElement`, `isElementAlive`, parametry renderer), obsługę błędów SDL/fontów/IO, walidację wejścia parserów.
- Efekt: 42/43 testów przechodzi (1 porażka = pre-existing flake środowiskowy warp myszy w test_sdl_gui_c_api, pada też na czystym drzewie), wszystkie przykłady się budują.
- Zmienione pliki: src/gui.cpp, src/gui_manager.cpp, src/scroll_area.cpp, src/tab_control.cpp, src/window_manager.cpp, src/string_grid.cpp, src/animated_image.cpp, src/composite/dialog_box.cpp, src/composite/file_dialog.cpp, src/editor/editor_window.cpp, src/editor/preview_window.cpp, src/layout_parser.cpp

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
