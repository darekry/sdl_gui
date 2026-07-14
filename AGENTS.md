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
| **Widgety (21)** | Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu, Cursor, ArcContainer, ProgressBar, ScrollArea, ShaderPanel |
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
tests/         — 29 plików testowych (Catch2)
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
- **Widgety testowane (17)**: Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor
- **Menedżery (4)**: FontManager, TextureManager, TimerManager, AnimationManager
- **Systemy (5)**: GUIElement, GUIManager, Theme, Easing, UTF8
- **Screen/Window (2)**: ScreenManager, WindowManager
- **C API (1)**: test_sdl_gui_c_api (17 przypadków, Phase 0+1)
- **C API (1)**: test_sdl_gui_c_api (36 przypadków, Phase 0+1+2, 306 asercji)
- **C API (1)**: test_sdl_gui_c_api (40 przypadków, Phase 0+1+2, 316 asercji)
- **C API (1)**: test_sdl_gui_c_api (45 przypadków, Phase 0+1+2+timers, 331 asercji)
- **Brakujące testy**: ArcContainer, ProgressBar, ScrollArea, ShaderPanel, TextEditable, Style, parsery
- **Znane bugi**: Combobox — heap-use-after-free (pre-existing); Button::onMouseOver callback zdefiniowany ale nigdy nie wywoływany; Label nie ma override `getComponentType()` → naprawione w (2026-07-14)

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

### GUIContext + parent-in-create + C API refactor (2026-07-14)
- **GUIContext** (`src/gui_context.hpp`) — klasa łącząca `SDLApp` + `GUIManager` + `Theme` w jeden obiekt RAII. Konstruktor auto-aplikuje theme i ustawia window size. Metoda `run()` hermetyzuje całą pętlę zdarzeń.
- **SDLApp::run()** — nowa metoda szablonowa: `app.run(guiManager, clearColor, onEventCallback)`. Obsługuje PollEvent → processEvent → update → cleanup → clear → render → present. Opcjonalny callback `onEvent(SDL_Event&)` do dodatkowej obsługi zdarzeń.
- **Parent-in-create** — `GUIManager::create<T>(args...)` i `GUIManager::create<T>(parent, args...)`: tworzy widget przez `make_unique`, auto-dodaje do managera (top-level) lub rodzica (child), zwraca surowy wskaźnik `T*`.
- **addChild** zwraca `GUIElement*` (wcześniej `void`) — eliminuje antypattern `auto* ptr = widget.get()` przed `std::move`.
- **C API refactor** — `CContext` (wewnętrzny struct w `sdl_gui_c_api.cpp`) zastąpiony aliasem na `GUIContext`, usuwając duplikację kodu.
- **Przykłady**: `45_run_basic.cpp` (pusta pętla run), `46_run_callback.cpp` (run z obsługą klawiszy)
- Efekt: 46/46 examples, 31/31 tests
- Zmienione: `src/gui_context.hpp` (nowy), `src/gui.hpp`, `src/gui.cpp`, `src/sdl_app.hpp`, `src/gui_manager.hpp`, `src/sdl_gui_c_api.cpp`, `examples/45_run_basic.cpp` (nowy), `examples/46_run_callback.cpp` (nowy)

### C API wrapper — Phase 2 (2026-07-14)
- Dodane opakowania C API dla pozostałych 11 widgetów + dynamic reparenting
- **Nowe widgety C API**: ProgressBar, RadioButton, RadioGroup, TextArea, ComboBox, StringGrid, ScrollArea, AnimatedImage, Canvas, ArcContainer, TabControl, ContextMenu
- **Nowe callback typy**: `sdlgui_index_text_callback_t` (RadioGroup/ComboBox), `sdlgui_cell_callback_t` (StringGrid), `sdlgui_context_menu_callback_t`
- **Dynamic reparenting**: `sdlgui_element_add_child(parent, child)` — przenosi top-level element do rodzica
- **Gap filling**: `tab_control_set_active_tab`, `scroll_area_get_scroll_offset`, `animated_image_set_frame`, `string_grid_set_editable/is_editable`
- **Ownership transfer**: `GUIManager::detachElement()` — wyodrębnia `unique_ptr` bez usuwania, używane przez `scroll_area_set_content`, `arc_container_add_child_at_angle`, `add_child`
- **GUIElement::getManager()** — nowa publiczna metoda dostępu do GUIManager
- **ComboBox::clearItems()** — nowa metoda (wcześniej brakowało)
- **RadioGroup::getComponentType()** — dodany override zwracający "RadioGroup" (wcześniej dziedziczył "Panel")
- **ComboBox::getComponentType()** — dodany override zwracający "ComboBox" (wcześniej brakowało)
- **Testy**: 23 nowe przypadki testowe (40 łącznie, 316 asercji)
- **C przykłady**: 5 nowych (04-08), 8 łącznie — ProgressBar, RadioGroup, ComboBox, StringGrid, TabControl
- Efekt: 44/44 examples, 31/31 tests, release: `.a`, `.so`, `sdl_gui.h`, `sdl_gui.hpp`
- Zmienione: `src/sdl_gui.h`, `src/sdl_gui_c_api.cpp`, `src/gui_manager.hpp`, `src/gui_manager.cpp`, `src/gui.hpp`, `src/combobox.hpp`, `src/combobox.cpp`, `src/radio_group.hpp`, `src/radio_group.cpp`, `tests/test_sdl_gui_c_api.cpp`, `tests/test_radio_group.cpp`, `examples/c/04-08*` (nowe)

### C API wrapper — Phase 0+1 (2026-07-14)
- Dodana warstwa C API (`extern "C"`) do istniejącej biblioteki C++
- **Nowe pliki**: `src/sdl_gui.h` — publiczny nagłówek C (C11, `sdlgui_*` prefix), `src/sdl_gui_c_api.cpp` — implementacja
- **Context lifecycle**: `sdlgui_create/destroy` — convenience wrapper (SDLApp + GUIManager + Theme)
- **Core loop**: `sdlgui_process_event/update/cleanup/render/get_renderer/handle_resize/get_window_size`
- **Theme**: 4 presety (`sdlgui_theme_win9x/dark/light/high_contrast`), tooltip API
- **Element base API**: set_position/size/enabled/visible, style setters (bg/text/border), tooltip, id, anchor, rotation, focus
- **Anchor factories**: 14 funkcji (`none`, `top_left`, `center`, `fill`, `stretch`, `bar`, `sidebar`, `raw`)
- **Phase 1 widgets (Core 7)**: Button, Label, Panel, Slider, Checkbox, TextInput, ListView
  - Element ownership: GTK/Win32 pattern — parent w `create_*`, NULL = top-level
  - Callback types: `sdlgui_callback_t` (generic), `sdlgui_bool_callback_t` (checkbox), `sdlgui_size_callback_t` (listview)
  - String returns: `.c_str()` bezpieczne do następnej modyfikacji elementu
- **Build**: `build_release()` kopiuje `src/sdl_gui.h` → `dist/sdl_gui.h`, kompiluje C przykłady + smoke test
- **Testy**: `tests/test_sdl_gui_c_api.cpp` — 17 test case'ów (217 asercji), pokrycie wszystkich Phase 0+1 funkcji
- **C przykłady**: `examples/c/01_hello.c`, `02_buttons.c`, `03_slider_label.c` — czysty C11, kompilowane `clang -std=c11 -pedantic-errors`, linkowane z `libsdl_gui.so`
- **Integracja C++**: `examples/41_c_api_demo.cpp` — użycie C API na widgetach C++
- **Fix**: `label.hpp` — dodany brakujący override `getComponentType()` zwracający "Label" (poprzednio brakowało)
- Efekt: 44/44 examples, 31/31 tests, release: `.a`, `.so`, `sdl_gui.h`, `sdl_gui.hpp`
- Zmienione: `src/sdl_gui.h` (nowy), `src/sdl_gui_c_api.cpp` (nowy), `src/label.hpp`, `nob.c`, `tests/test_sdl_gui_c_api.cpp` (nowy), `examples/c/*.c` (nowe), `examples/41_c_api_demo.cpp` (nowy), `examples/14_list_view.cpp` (fix)


- `ThemePresets` namespace w `src/theme_presets.hpp` — 4 predefiniowane motywy:
  - `createWin9xTheme()` — klasyczny Windows 95/98: szare tło `{192,192,192}`, ostre krawędzie, białe inputy
  - `createLightTheme()` — jasny, nowoczesny z niebieskim akcentem
  - `createDarkTheme()` — ciemny (dark mode)
  - `createHighContrastTheme()` — czarne tło, żółte akcenty, duże fonty
- `Theme::createDefaultTheme()` deleguje do `ThemePresets::createWin9xTheme()`
- Pokrycie wszystkich typów widgetów (Button/Panel/TextInput/TextArea/Label/Slider/ProgressBar/StringGrid/ListView/ComboBox/TabControl/ContextMenu/ScrollArea/Canvas/AnimatedImage) + stany Normal/Hover/Pressed/Disabled
- Examples 21 i 36 zaktualizowane do używania `ThemePresets`
- Efekt: 40/40 examples, 30/30 tests (theme test zaktualizowany dla Win9x borderRadius=0)
- Zmienione: `src/theme_presets.hpp` (nowy), `src/theme.cpp`, `examples/21_themes.cpp`, `examples/36_theme_playground.cpp`, `tests/test_theme.cpp`

### Focus element rendering behind overlays fix (2026-07-07)
- Pass 3 `GUIManager::render()` (focus element overlay) wywołuje `m_keyboardFocusElement->renderOverlay()` tylko gdy nie ma aktywnego overlayu lub element z focusem jest w nim zagnieżdżony — inaczej element spoza dialogu był rysowany na wierzchu
- `collectFocusableElements()` teraz zbiera elementy tylko z aktywnego overlayu, gdy taki istnieje — Tab nie skacze do elementów schowanych za dialogiem
- Dodane `getActiveOverlay()` i `isDescendantOf()` jako helpery
- Efekt: 40/40 examples, wszystkie testy przechodzą (oprócz pre-existing ASan w test_text_area)
- Zmienione: `src/gui_manager.cpp`, `src/gui_manager.hpp`

### Keyboard focus system (2026-06-28)
- Focus visual: niebieska obwódka (`kFocusOutlineColor`) rysowana w `drawBackgroundAndBorder()`
- Button: `setCanGetKeyboardFocus(true)`, Enter/Space → aktywacja ze stanem Pressed
- Checkbox: `setCanGetKeyboardFocus(true)`, Space → toggle
- Tab navigation: `GUIManager::focusNextElement()` + `collectFocusableElements()` — DFS z zawijaniem, Tab/Shift+Tab
- `onFocusGained/onFocusLost` w `.cpp` z `markDirty()`; `TextEditable` woła wersję bazową
- Efekt: 39/39 examples, wszystkie testy przechodzą (timer_manager ma pre-existing timeout)
- Zmienione: `constants.hpp`, `gui.hpp`, `gui.cpp`, `gui_manager.hpp`, `gui_manager.cpp`, `text_editable.cpp`, `button.cpp`, `checkbox.cpp`, `docs/api/*`

### Container & data structure optimization (2026-06-23)
- **Phase A**: Theme: `map<string, map<ElementState, Style>>` → `unordered_map<string, array<optional<Style>, 4>>` — O(1)
- **Phase B**: StringGrid cache: `map` → `unordered_map`
- **Phases D-J**: ListView, TextArea, StringGrid, gui.cpp, Cursor, EditorElement, EditorWindow, PreviewWindow, EditorState — `unordered_map` zamiast `map`, `reserve()`, lazy rebuild indeksów
- **Phase F**: `loadFont()` wyciągnięty z pętli rysowania — font ładowany raz w `drawDirect()`
- **Phase G**: `verts.reserve(192)` w `drawRoundedRectBorder`
- Efekt: 39/39 examples, 28/29 tests (1 pre-existing combobox bug). ~30+ linii usuniętych

### Hover performance optimization (2026-06-21)
- Cache pozycji, eliminacja podwójnego DFS, SDL_GetMouseState → dane z eventu
- Efekt: kilka tysięcy elementów: ~300ms → 16ms/klatkę

### SDL2 → SDL3 Migration (2026-06-13) ✅ Complete

### Helper refactors
- `SDLRectToFRect()`, `RenderRect()`, `SetDrawColor()`, `TextureWidth()`/`TextureHeight()`
- `computeScaledDstRect()` w animated_image.cpp
- `drawRoundedTexturedRect()` — tekstury z zaokrąglonymi rogami

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
