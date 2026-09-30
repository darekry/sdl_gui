# AGENTS.md — SDL GUI

## Build system (nob)

```
./nob test          # build + run all tests
./nob test <filter> # run tests matching substring
./nob examples      # build all examples (default)
./nob bench         # build synthetic benchmark (output/bench_gui, real window)
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
examples/      — 65 przykładów (00–64); examples/c/ — 10 przykładów C
tests/         — 43 binarki testowe (Catch2)
docs/          — release/ (kanon end-user → dist/docs/), refactor_plan.md, archive/ (nieaktualne)
skills/sdl-gui/ — skill agenta (SKILL.md + references/); `./nob release` kopiuje do dist/skills/
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
            Uint64 frameStart = SDL_GetTicks();   // do limitowania FPS (patrz app.endFrame)
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
            app.endFrame(frameStart);   // cap ~60 FPS — BEZ TEGO pętla kręci się tysiące FPS i zjada 1 rdzeń CPU
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

1. Tabela `g_embedded_assets[]` w `nob.c` — lista plików do osadzenia
2. `nob.c:build_embedded_assets()` — generowany `output/embedded_assets_data.c` (tablice bajtów `_binary_<nazwa>_start` + `_binary_<nazwa>_size`, bez GNU `ld` — przenośne na Windows/MinGW) → kompilowany zwykłym `CC` do `.o`
3. `src/embedded_assets.hpp` — auto-generowany header z `g_embeddedAssets[]`
4. `TextureManager::loadTextureFromMemory` / `FontManager::loadFontFromMemory` — `SDL_IOFromConstMem` → `IMG_Load_IO` / `TTF_OpenFontIO`
5. Po zarejestrowaniu (`registerEmbeddedAssets`, przykład 32) działa transparentnie: `loadTexture("assets/button1.png")` zwraca cache'owany embedded asset
6. Shadery GPU analogicznie: `glslc` → SPIR-V → `output/shader_spirv_data.c` + `output/gpu_shader_spirv.hpp` (nazwy `gpu_shader::<name>` bez zmian)
7. Warunkowość: brak assetu / brak `glslc` to WARNING + skip zależnych przykładów (tabela `example_needs()`: 32→`NEED_EMBEDDED`, 40/41→`NEED_SHADERS`, `*mixer*`/`*sqlite*`), nie błąd buildu. Embed/SPIR-V linkowane tylko do binarek, które ich potrzebują — testy i bench nie linkują ich wcale

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

Testy integracyjne: 65 przykładów (`examples/`, 00–64) + 10 przykładów C (`examples/c/`) do manualnej weryfikacji wizualnej.

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

### ModalDialog — cienka baza dla DialogBox/FileDialog (2026-09-30)
- **Refactor bez zmiany API**: nowe `src/composite/modal_dialog.{hpp,cpp}` (`ModalDialog : Panel`, `isOverlay`, `close/isOpen`, `centerInViewport`, `handleEvent`: dzieci → Esc→`onCancel` → Enter/KP_ENTER→`onConfirm`); `DialogBox`/`FileDialog` dziedziczą, ich `close/isOpen/isOverlay/Esc` usunięte (`DialogBox::onCancel` woła `callback(-1)`, `FileDialog::onConfirm=confirmSelection`, brak Enter w DialogBox = no-op). Bez nowego `ComponentType`, bez zmian w `GUIManager`.
- Efekt: 49/49 testów (nowy `test_modal_dialog.cpp`: Esc/Enter w obu dialogach, overlay, `close→markForDeletion`, `centerInViewport`, API `createConfirm`), release + smoke 35/36 zielone.
- Zmienione pliki: src/composite/modal_dialog.{hpp,cpp} (nowe), src/composite/dialog_box.{hpp,cpp}, src/composite/file_dialog.{hpp,cpp}, tests/test_modal_dialog.cpp (nowy), nob.c (`hpp_order` + `includes_to_remove`)

### DockLayout — dokowanie do krawędzi (2026-09-30)
- **Nowość**: `Dock` (`None/Top/Bottom/Left/Right/Fill` w `anchor.hpp`) + `DockLayout` (`src/layout.{hpp,cpp}`, chain `Fill`, skip `None`/`hidden`, padding+spacing, zero alokacji) + `setDock/getDock` w `GUIElement` (tani re-layout rodzica); parser: `dock=` per node + `layout="dock"` na kontenerze; C-API: `sdlgui_dock_t` + `set_dock/get_dock/set_dock_layout`.
- Efekt: 48/48 testów (nowy `test_dock_layout.cpp` + fixture'y JSON/XML), przykład `66_dock_layout.cpp`, release zielone.
- Zmienione pliki: src/anchor.hpp, src/layout.{hpp,cpp}, src/gui.{hpp,cpp}, src/layout_parser.{hpp,cpp}, src/sdl_gui.{h,c_api.cpp}, tests/test_dock_layout.cpp, examples/66_dock_layout.cpp, nob.c

### Embed bez ld + warunkowe przykłady + zalążek Windows (2026-09-21)
- **Embed/SPIR-V bez GNU `ld`**: `build_embedded_assets`/`build_gpu_shaders` emitują generowany `.c` z tablicami bajtów (`_start` + `_size`, koniec symboli `_end` i prefiksowych problemów PE/COFF) kompilowany zwykłym `CC` — ten sam kod zadziała pod MinGW cross i natywnym Windows. Publiczne nazwy bez zmian (`g_embeddedAssets[]`, `gpu_shader::<name>(+_size)`), przykład 32/40/41 nietknięte.
- **Warunkowość**: brak assetu / fail `glslc` to WARNING + skip (nowa tabela `example_needs()`: 32→`NEED_EMBEDDED`, 40/41→`NEED_SHADERS`, mixer/sqlite jak dotąd) zamiast kłaść cały build; embed/SPIR-V linkowane tylko tam, gdzie potrzebne (testy i bench już ich nie linkują).
- **Przenośność `src/`**: GPU-konstruktor `SDLApp` bez `dirent`/`setenv` (skan ICD przez `std::filesystem`, tylko `#ifndef _WIN32`); `std.hpp` na `_WIN32` używa klasycznych includów zamiast `import std.compat` (koniec zależności od prekompilowanego libc++ `.pcm`); `dist/sdl_gui.hpp` bez `<dirent.h>`.
- Efekt: 47/47 testów, 65/65 przykładów, release zielone; smoke 32 headless OK; symulacja braku assetu → `32 skipped (missing optional deps)`, reszta buduje się.
- Zmienione pliki: nob.c, src/sdl_app.hpp, src/std.hpp, src/embedded_assets.hpp (regenerowany)
- Następny krok: toolchain Windows w `nob.c` (llvm-mingw/clang `--target=`, prefix SDL z `vendor/` przez CMake) + `download_sdl_deps.sh` do przepisania na SDL3.

### Focus-outline wystawał poza WorldView/ScrollArea (2026-09-10)
- **Przyczyna (2×)**: (1) `renderFocusOverlay()` rysował obrys bez clipa (`gui.cpp`) — treść cięta do `clipped_rect`, obrys nie. (2) `GUIManager::render()` dorysowywał fokusowany widget drugi raz przez bazowy `renderOverlay()` (= `render()` bez clipa przodków) — cała binarka z fokusem lądowała na wierzchu poza viewportem.
- **Fix**: obrys + rotowane blity cięte do `clipped_rect` (ustaw/odtwórz jak w `drawDirect`); bazowy `renderOverlay()` to no-op (overlay mają tylko TextInput/TextArea-kursor, StringGrid-edytor, Cursor, CanvasPanel — ten zachował jawny `render()`); clip kursora TextInput/TextArea zawężony o clip przodków.
- Efekt: 47/47 testów (nowy regresyjny `WorldView focus-outline clip`: piksel na obrysie za viewportem nie-niebieski + kontrolny w środku niebieski), 65/65 przykładów.
- Zmienione pliki: src/gui.{hpp,cpp}, src/text_input.cpp, src/text_area.cpp, src/editor/preview_window.cpp, tests/test_world_view.cpp

### WorldView — kamera 2D na świat gry (2026-09-10)
- **Co**: nowy `src/world_view.{hpp,cpp}` (`WorldView : public Panel`, pan-only bez zoomu) — dzieci w world coords przez `addWorldChild`, kamera jako jeden shift contentu `(-camX,-camY)` + clip viewportu (mechanizm jak `ScrollArea`, bez sliderów). API: `setWorldSize/setCamera/panBy/centerOn`, `worldToScreen/screenToWorld`, clamp `max(0, world-view)`, `layoutChildren` re-clampuje przy resize. Cel: decoupling mapy od widoku pod `EntityWidget` (plan: `.kilo/plans/worldview-camera-plan.md` w grze). Wpięcie: `ComponentType::WorldView` (+toString/fromString), `WidgetFactory` (default `300x200`, gałąź `create` reuse `contentWidth/Height` jako world size, `knownTypes`), `nob.c` (`hpp_order` + `includes_to_remove`).
- **Uwaga**: izometria świadomie poza zakresem — `ArcContainer::rotateChild` obraca teksturę widgetu, a iso wymaga projekcji `2:1` + depth sortu `x+y` + rombowego pickingu; zostawiony punkt zaczepienia (wymienna projekcja w `WorldView`).
- Efekt: weryfikacja częściowa — syntax-check wszystkich dotkniętych plików zielony + `world_view.cpp` kompiluje się do `.o`; pełne `./nob test/examples/release` ZABLOKOWANE pre-existing brakami w checkoucie (`assets/button_bg.png`, `assets/fonts/font.ttf` nie istnieją → `build_embedded_assets` failuje przed kompilacją). Test jednostkowy `test_world_view.cpp` + przykład `65_world_view.cpp` (WASD/strzałki pan, label `cam + screenToWorld`) czekają na odblokowanie assetów.
- Zmienione pliki: src/world_view.{hpp,cpp} (nowe), src/component_type.hpp, src/widget_factory.cpp, tests/test_world_view.cpp (nowy), tests/test_lifetime.cpp (tabela fabryki), examples/65_world_view.cpp (nowy), docs/release/widgets/WorldView.md (nowy), docs/release/widgets/README.md, nob.c

### Pakiet z gier: Button::setText + ui_helpers + after()/every() (2026-09-07)
- **Co**: (1) `Button::setText/getText` — podmiana (lub leniwe utworzenie, gdy caption był pusty) wycentrowanego Label-dziecka + `layoutChildren/markDirty`; surowy `Label*` ZOSTAJE (ownership `unique_ptr` + lifetime związany z rodzicem, jak `Button*` w Sliderze; `weak_ptr` wymagałby `shared_ptr` w całym libie — libowy odpowiednik słabej referencji to `ElementRef`/handle'e). Przy okazji naprawiło to przykład w `docs/release/managers.md:237`, który używał nieistniejącego `setText`. (4) Nowy `src/ui_helpers.{hpp,cpp}` — baza wolnych funkcji: `linkLabel` (prefix/suffix i formatter) + `linkRangeLabel` (jw.), odświeżenie natychmiastowe, label trzymany przez `ElementRef`; uwaga "jedno wiązanie na widget" (nadpisuje jak `setOnChangeCallback` — złapane testem). `ui_helpers.hpp` dopisane do `hpp_order` w `nob.c`. (5) `GUIManager::after(ms,fn)/every(ms,fn)/cancelTimer(id)` — cienkie wrappery na `TimerManager` (target `nullptr`, wzorzec już używany w testach); NIEpowiązane z lifetimem widgetów (udokumentowane) + sekcje w `managers.md`/`patterns.md`/skillu. Dogfood: 62_memory przepisane z ręcznego deadlinu na `after(700)` z guardem `dealId` na restart.
- **Generatory w ui_helpers (dogrywka tego samego dnia)**: `gridPanels` (siatka Paneli, dzieci lub top-level), `faceGrid` (Button + centrowany Label-face — obchodzi brak `Button::setText` w grach), `makeStrip<T>` (szablon z fabryką `(manager,x,y,w,h,index)` na paski widgetów), `shuffledPairs(n)` (talia do Memory). Wpadki: overload `create<T>(parent,…)` wymaga ctora `(manager,parent,…)` — żaden widget tak nie ma, więc impl przez `make_unique` + `addChild/addElement`; szablon z `GUIManager&` wymaga pełnego typu → `ui_helpers.hpp` includuje `gui_manager.hpp` (stripowany w amalgamacie), a wpis w `hpp_order` wylądował ZA `gui_manager.hpp`. Dogfood: 59→`gridPanels`, 60→`faceGrid` (cyferki centrowane zamiast (12,6)), 62→`shuffledPairs`.
- Efekt: 46/46 testów (nowe: `test_ui_helpers.cpp` — 3 sekcje bindingów + 4 sekcje generatorów; case'y setText w `test_button.cpp`; `after/every` w `test_timer_manager.cpp`), 65/65 przykładów, smoke 59/60/62 headless czysty.
- Zmienione pliki: src/button.{hpp,cpp}, src/ui_helpers.{hpp,cpp} (nowe), src/gui_manager.{hpp,cpp}, tests/test_button.cpp, tests/test_ui_helpers.cpp (nowy), tests/test_timer_manager.cpp, examples/{59_snake,60_minesweeper,62_memory}.cpp, nob.c, docs/release/{widgets/Button,managers,patterns}.md, skills/sdl-gui/references/gui-app.md

### Audyt przykładów → 9 nowych helperów w ui_helpers (2026-09-08)
- **Co**: audyt wszystkich 65 przykładów (3 subagenty po ~20 plików, weryfikacja grepem) z progiem uniwersalności 3+ użycia w różnych domenach. Nowe w `ui_helpers`: Pakiet A (fundament) — `addTracked<T>` (fuzja `makeRef`-przed-`move` + `add`, zwraca `pair<T*, ElementRef<T>>`; zabija udokumentowany crash-pitfall, 40+ call sites w 20+ plikach), `addButton`/`addLabel` (one-linery z tekstem/callbackiem/kolorem), `styleCard` (kanoniczny ciemny blob z defaultami) + `addDarkPanel` (fabryka). Pakiet B (formularze) — `addLabeledCheckbox` (box + caption, opcjonalny kolor tekstu — potrzebny na ciemnych panelach) + `makeTopBar`/`makeBottomBar` (struct `StatusBar{bar,label}`, label-dziecko z insetem 8px). Pakiet C (siatki+suwaki) — `makeGrid<T>` (fabryka `(manager,col,row,x,y,w,h)`, spójna z `makeStrip`; uogólnia `gridPanels`/`faceGrid`), `onAnyChange` (wspólny `refresh()` + natychmiastowe odpalenie, overloady Slider/RangeSlider), `bindSliderValue` (int + float ze skalą — rodzeństwo `linkLabel`). Konwencja jak dotąd: impl z `(manager, parent*)` + cienkie publiczne overloady; fabryki szablonów w hpp.
- **Niche odrzucone**: status `"Selected: …"` (16,17), kolumna Add/Remove/Clear (16,17 — kryje `makeStrip`), theme-switcher RadioGroup (27,28), d-pad focus-nav (43,44), bochenki ShaderPanel (40,41), captiony nad/pod sliderem (niespójna geometria), `setTooltip`-jednolinijkowce. 48 (Win95, status bar na anchorach) i 29 (Anchor::bottomBar) świadomie nietknięte — helpery barów nie wspierają anchorów.
- Efekt: 46/46 testów (`test_ui_helpers.cpp`: +7 TEST_CASE — tracked/card/builders/grid/sliders/status bars, asercje przez `getComposedStyle` i syntetyczne click-eventy), 65/65 przykładów, smoke 10 migrowanych headless czysty (wycieki 42/44 z `SDL_X11_SetWindowTitle` — szum SDLa, nie nasz kod), release zielone.
- Zmienione pliki: src/ui_helpers.{hpp,cpp}, tests/test_ui_helpers.cpp, examples/{04_text_input,05_slider,06_checkbox,10_range_slider,36_file_dialog,42_mobile_touch,44_tv_remote,49_arc_sliders,51_bounce_lab,55_gravity_box}.cpp, docs/release/patterns.md, skills/sdl-gui/references/gui-app.md

### 6 nowych przykładów 59–64: proste gry + 3rd-party (2026-09-07)
- **Co**: 4 gry bez nowych zależności — 59_snake (siatka 20x14 Paneli, strzałki/WASD, slider kroków/s, repaint tylko na kroku logiki), 60_minesweeper (9x9 Buttonów + Label-faces jak Button nie ma setText; LPM z flood-fill, PPM flaga przez `setOnRightClickCallback`), 61_breakout (mysz steruje paletką, fizyka kulki `dt`, cegły chowane przez `setVisible(false)` żeby surowe wskaźniki nie wisiały, slider prędkości), 62_memory (16 kart, mismatch wraca po 700 ms przez deadline w pętli, restart tasuje `std::shuffle`). 2 przykłady 3rd-party: 63_mixer_beeps (SDL3_mixer: syntezowane WAV-y sinus w pamięci → `MIX_LoadAudio_IO` + `MIX_PlayAudio`, slider → `MIX_SetMixerGain`, klawisze 1/2/3), 64_highscores_sqlite (systemowy sqlite3: `TextInput` + slider wyniku → INSERT, TOP 5 w multiline-Label, `highscores.db` w cwd).
- **Lekcje buildowe**: (1) zero dodatkowych include'ów STL (`<deque>`/`<vector>`/`<algorithm>`/`<random>` też sypią `requires clause differs` — wszystko jest w `std.hpp`); (2) callbacki klików MUSZĄ brać `GUIElement*` (lambdy `[](){}` się nie konwertują); (3) `MIX_DestroyMixer` PRZED końcem `try` — dtor `SDLApp` woła `SDL_Quit` i niszczenie miksera po nim to SEGV w `MIX_StopAllTracks` (złapane ASan-em).
- **nob.c**: opt-in zależności per przykład (reszta binarek szczupła) — `*mixer*` dostaje flagi z `pkg-config sdl3-mixer`, `*sqlite*` dostaje `-lsqlite3` (sqlite3-dev już w systemie, nic nie instalowano); `cc -o nob nob.c` po edycji.
- Efekt: testów nie ruszano (lib bez zmian), 65/65 przykładów się buduje, smoke headless 59–64 (timeout = pętla chodziła; 64 utworzył `highscores.db`, 63 po fixie czysty ASan na dummy-audio).
- Zmienione pliki: examples/59_snake.cpp … examples/64_highscores_sqlite.cpp (nowe), nob.c, AGENTS.md, docs/index.md, skills/sdl-gui/SKILL.md (liczniki 59→65)

### 10 nowych przykładów 49–58: slidery sterują wszystkim (2026-09-07)
- **Co**: 10 przykładów 100–170 linii, każdy pokazuje nietypową, prostą interakcję ze sliderami: 49_arc_sliders (3 poziome slidery obrócone na łuku przez `ArcContainer::addChildAtAngle(rotate=true)` mieszają RGB podglądu; środkowy stoi pionowo), 50_servo_panel (slidery X/Y/rotacja jeżdżą panelem jak serwami wprost przez `setPosition/setRotation`), 51_bounce_lab (ręczna integracja `dt` z `SDL_GetTicks`: slider prędkości w px/s + slider rozmiaru kulki + pauza), 52_range_remap (`RangeSlider` definiuje okno [lo,hi], zwykły slider mapuje 0–100 w to okno, `ProgressBar` pokazuje wynik), 53_smooth_follow (wzór "smooth follow" bez AnimationManagera: `cur += (target-cur)*min(1,dt*speed)`), 54_brush_mixer (slidery R/G/B przezbrajają `Canvas::setPenColor` na żywo + swatch + Clear), 55_gravity_box (pionowy slider grawitacji + slider tłumienia odbicia, fizyka w pętli), 56_dial_morph (slider kąta kręci wskazówką przez `setRotationCenter`, slider promienia woła `ArcContainer::setRadius` na żywo), 57_text_scaler (slidery rozmiaru/rotacji/jasności labelki przez jeden wspólny `refresh()` budujący pełny `Style`), 58_equalizer (5 par pionowy slider + pionowy `ProgressBar` jako VU-metry z wobble `sin`, `ElementRef` w wektorach).
- **Lekcja buildowa**: nie doklejać `#include <vector>`/`<cmath>` — `std.hpp` już je zawiera, a mieszanie z modułami (`import std`) sypie `requires clause differs` w libc++.
- Efekt: testów nie ruszano (kod lib bez zmian), 59/59 przykładów się buduje (`./nob examples` zielone), smoke headless 49/55/58 (4 s każdy, `SDL_GUI_HIDDEN=1`, exit przez timeout = pętla chodziła, zero crashy).
- Zmienione pliki: examples/49_arc_sliders.cpp … examples/58_equalizer.cpp (nowe), AGENTS.md, docs/index.md, skills/sdl-gui/SKILL.md (liczniki 49→59)

### Unifikacja Win9x → Windows95 + checkbox Win95 (2026-09-07)
- **Co**: jeden kanoniczny preset Win95 — `createWin9xTheme()` to teraz cienki alias wołający `createWindows95Theme()` (bez `[[deprecated]]`, żeby nie zaśmiecać buildu warningami z C-API; sama adnotacja słowna), `Theme::createDefaultTheme()` deleguje do kanonu. Paleta kanonu podpięta pod `constants::kWin95*` (nowe `kWin95Navy`), kolory-widma `{64}/{160}/{212,208,200}` wycięte. Checkbox dostał wygląd Win95: białe pudełko + Sunken, check w `textColor` (Normal czarny via default, Disabled szary z zachowanym sunken — Hover/Pressed fallbackują do Normal jak w oryginale). Domknięte pokrycie: jawne flat-Face `DialogBox`/`FileDialog` + komentarz o świadomie pominiętych typach. Koniec nadużycia `borderColor`: Slider/RangeSlider → `thumbColor`, ProgressBar → `fillColor` (te same piksele przez fallback, poprawna semantyka). C-API: nowe `sdlgui_theme_windows95()`, `sdlgui_theme_win9x()` to alias. Przykłady 27/28 dostały trzeci radio "Win95".
- Efekt: 45/45 testów (przepisana sekcja Default-Button na semantykę "Pressed = ten sam bg, inny bevel", nowy `TEST_CASE "Win9x alias parity"` pinujący identyczność styli alias/kanon/default, ProgressBar na `fillColor`, nowe case'y checkbox/DialogBox, pikselowy test checkboxa w `test_render_pixel.cpp`: biały środek + rogi Sunken TL-Shadow/BR-Highlight/inner-TL-black), 48/48 przykładów, release (standalone + C smoke) zielone.
- Zmienione pliki: src/theme_presets.hpp, src/theme.cpp, src/constants.hpp, src/sdl_gui.{h,c_api.cpp}, tests/test_theme.cpp, examples/27_themes.cpp, examples/28_theme_playground.cpp, docs/release/{core,resources,patterns,c_api}.md, docs/refactor_plan.md, docs/win9x_theme_unification.md, skills/sdl-gui/references/gui-app.md, readme.md

### Render punkt 6 — plaster 6: rotacja/blit + fokus poza cache (2026-09-06)
- **Co**: `renderFocusOverlay()` — obrys fokusu wyekstraktowany z `render()`; usunięta wpiekana gałąź fokusu z `renderToCache()` (zmiana fokusu nie unieważnia cache'a treści). Usunięte mylące `m_isDirty = false` w gałęzi `wantsDirectRender`. Bugfix przy okazji: direct-dziecko rotowanego rodzica było NIEWIDZIALNE (pętla dzieci zgate'owana rotacją + bake kompozytował stub `draw()`); teraz bake pomija direct-dzieci, a one renderują się osobno bez rotacji.
- **Regresja AABB i fix (ten sam dzień)**: pierwsza wersja rysowała obrys osiowo (AABB) dla rotowanych — w demie ArcContainera wyglądało to jak błąd (ramka nie podążała za obrotem buttona). Fix: obrys rotowanego trzymany w OSOBNEJ teksturze `m_focusTexture` (sam obrys, lokalne coords; odbudowa w `renderToCache()` — te same inputy co cache treści; niezmiennik: istnieje ⟺ element rotowany), blitowanej z IDENTYCZNYMI parametrami co treść (`SDL_RenderTextureRotated`, ten sam dst/center) — wizualnie 1:1 ze starą wpiekanką, ale zmiana fokusu nie tyka cache'a treści. Bake rotowanego rodzica kompozytuje też focus-textury dzieci z fokusem (stary kod to miał via tekstura główna). Nierotowane bez zmian (direct draw, zero tekstur).
- Efekt: 45/45 testów (2 pikselowe w `test_render_pixel.cpp`: outline na rotowanej krawędzi niesymetrycznego panelu 120x60@90° — punkt leży W ŚRODKU AABB, więc fail i na braku outline'u, i na wersji AABB; czerwone direct-dziecko na niebieskim rotowanym rodzicu), 48/48 przykładów (demo ArcContainera startuje, brak crasha), release zielone. Bench: runda niemiarodajna (sama sekcja `frame_loop` +70/+120% przy reszcie w szumie, scena bez rotacji/fokusu — delta kodu to null-`reset()`; winny present na żywym desktopie, nie kod) — neutralność z konstrukcji: gorąca ścieżka sceny bez zmian.
- **PUNKT 6 ZAMKNIĘTY** (6/6 plastrów: OverlayStack, TextShaper, koniec mutacji w draw(), LRU bajtowe, precyzyjny dirty, rotacja/blit).
- Zmienione pliki: src/gui.{hpp,cpp}, tests/test_render_pixel.cpp, docs/release/core.md

### Render punkt 6 — plaster 5: precyzyjny dirty (2026-09-06)
- **Co**: reguła — cached texture zależy tylko od własnych inputów `draw()` (klucz cache'a nie miesza dzieci ani pozycji — zaudytowane; `draw()` w lokalnych coords). Stąd: `markDirty(cascade)` brudzi self + wyłącznie rotowanych przodków (nowy helper `markBakedAncestorsDirty()` — tylko oni wpiekają dzieci we własną teksturę); koniec kaskady do root przy każdym `markDirty()` (~40 call sites naprawionych jednym ruchem). `setPosition`: early-out + zero self-dirty (cache niezależny od pozycji; drag Panela nie przerysowuje już łańcucha przodków na każdy MOTION) + rotowani przodkowie. `setSize`: early-out przy tych samych wymiarach (wcześniej zawsze dirty — m.in. `AnchorLayout`, slidery, tooltip). `addChild/clearChildren/cleanup`: `markDirty(false)` (dodanie/usunięcie dziecka nie zmienia pikseli rodzica). Nieruszone: `markDirtyRecursively` (theme — semantycznie O(n)), `wantsDirectRender` (redraw co klatkę z natury), `setVisible` (już no-op), focus-outline (poza cache; rotowany-focus przez `onFocus*` self-dirty jak dotąd).
- **Przy okazji**: usunięty pojedynczy U+00AD (soft hyphen) zawleczony we własnym komentarzu w `gui.hpp` (weryfikacja bajtowa: 0 w dotkniętych plikach).
- Efekt: 45/45 testów (poprawiony `setPosition() marks element dirty` → nowa semantyka + 6 nowych sekcji: no-op same-position/same-size, brak propagacji do zwykłych przodków, rotowani przodkowie przy `markDirty` i `setPosition`, brak kaskady `addChild`), 48/48 przykładów, release zielone. Bench vs `pre6.json`: `frame_loop` +0.8/+0.1/+5.3% (neutralnie-do-plus; eventy w zwykłym szumie).
- Zmienione pliki: src/gui.{hpp,cpp}, tests/test_gui_element.cpp, docs/release/core.md

### Render punkt 6 — plaster 4: LRU bajtowe w TextureManager (2026-09-06)
- **Co**: wpisy wszystkich trzech cache'y (`texture/render/text`) to `CacheEntry{texture, bytes, lastUse}` — bajty = `w*h*4` (render-cache z parametrów, reszta przez `SDL_GetTextureSize`). Każde wstawienie księguje bajty i egzekwuje budżet (`enforceByteBudget()`: wyrzuca martwe `use_count==1` od najstarszego użycia, we wszystkich mapach naraz); żywe wpisy nigdy nie wypadają (budżet może chwilowo overshootować). Hit odświeża recency we WSZYSTKICH ścieżkach (`loadTexture`, `loadTextureFromMemory`, oba `createTextureFromText`, `renderCache`, `addTexture`, const `getTexture` — `lastUse`/zegar są `mutable`, sygnatury API bez zmian). Nowy limit `kDefaultByteBudget = 128 MiB` + `setByteBudget()` (egzekwuje natychmiast) / `getByteBudget()` / `getBytesUsed()`; `pruneUnused()`/`clearCache()` odejmują/zerują bajty. `GUIManager::update()` tripuje prune'a `getBytesUsed() > getByteBudget()` zamiast samego licznika (próg 256 wpisów zostaje jako hygiene-backstop).
- Efekt: 45/45 testów (nowy `byte-budget LRU`: accounting, eksmisja najstarszego martwego, zakaz eksmisji żywych, hit-odświeża-recency przez porównanie surowych wskaźników, shrink-budżetu, `pruneUnused` zeruje bajty), 48/48 przykładów, release zielone. Bench vs `pre6.json`: `frame_loop` −0.4/−2.8/−1.0% (szum; koszt to bump licznika na hicie + query rozmiaru na insert — na ścieżce klatki dominują hity).
- Zmienione pliki: src/texture_manager.{hpp,cpp}, src/gui_manager.cpp, tests/test_texture_manager.cpp, docs/release/managers.md

### Render punkt 6 — plaster 3: koniec mutacji w draw() (2026-09-06)
- **Co**: `ComboBox::createDropdownButtons()` buduje się eager w `toggleDropdown()` (expand) i `addItem()` (gdy rozwinięte) — z `draw()` wycięty blok `addChild/setSize/markDirty` w środku passa renderu; usunięta flaga `m_needs_update`. `ContextMenu::createMenuButtons()` buduje się eager w `addItem/addSeparator/clearItems` gdy menu widoczne (ścieżka `showAt` bez zmian — już była w event path); `draw()` to czysty no-op. `TextArea` świadomie nietknięta: jej `recalculateLines/refreshTextures` w `draw()` dotyka tylko własnych wektorów (bez hierarchii, bez `markDirty` — idempotentny self-cache, potrzebny też w handlerach klików do hit-testu).
- Efekt: 45/45 testów (nowe case'y `No draw-path mutation`: buttony ComboBoxa i pozycje menu widoczne przez `findElementAt`/dzieci panelu BEZ `render()` — na starym kodzie fail), 48/48 przykładów, release zielone. Bench vs `pre6.json`: `frame_loop` −1.7/−0.7/−0.9% (neutralnie — scena nie używa combo/menu; zysk to brak hitchy pierwszej klatki menu, niemierzalny obecną sceną).
- Zmienione pliki: src/combobox.{hpp,cpp}, src/context_menu.cpp, tests/test_combobox.cpp, tests/test_context_menu.cpp

### Render punkt 6 — plaster 2: TextShaper (2026-09-06)
- **Co**: `TextureManager::m_textCache` (`uint64 → SharedTexture`) — jeden współdzielony magazyn `text/font/color → texture` zamiast string-kluczy (`text|ptr|rgba` budowanych per wywołanie, czyli per klatkę dla ComboBox/ProgressBar/StringGrid). Klucz: FNV-1a po bajtach tekstu + tożsamość fontu + kolor, zero alokacji na hicie. Oba publiczne overloady `createTextureFromText` (API bez zmian) chodzą po tym samym store; lifecycle 1:1 jak dotąd (`pruneUnused`/`clearCache` obejmują nowy magazyn, trigger progu bez zmian) + nowe `getTextCacheSize()`. `StringGrid`: skasowany nielimitowany `m_localTextureCache` (klucz bez fontu — latentna kolizja) i `createLocalTextTexture/clearLocalTextureCache`; helpery rysujące biorą `const SharedFont&` i wołają managera (font domieszany do klucza).
- Efekt: 45/45 testów (nowy case `TextureManager TextShaper`: sharing, rozróżnienie koloru/fontu/tekstu, 100× ten sam tekst → 1 wpis), 48/48 przykładów, release zielone. Bench vs `pre6.json` (visible, load ~5): `frame_loop` −0.3/+0.1/+1.3% (neutralnie — scena bench była już cache-hit; zysk to tempo alokacji + bound pamięci, nie fps). Uwaga: jeden przelotny fail suity (44/45) nie do odtworzenia w 3 kolejnych runach — flake.
- Zmienione pliki: src/texture_manager.{hpp,cpp}, src/string_grid.{hpp,cpp}, tests/test_texture_manager.cpp, docs/release/managers.md

### Render punkt 6 — plaster 1: OverlayStack (2026-09-06)
- **Co**: nowe `src/overlay_stack.{hpp,cpp}` — cache nieposiadających widoków (`GUIElement*`) na top-level elementy z `isOverlay()==true`. Ownership i kolejność Z zostają w jednym `m_elements` (brak przenoszenia `unique_ptr`); overlay-pass `render()`, `getActiveOverlay()` i focus-modal chodzą po cache `O(K)` zamiast skanu `O(N)`. Invalidacja: `addElement/detachElement/cleanup` + publiczne `notifyOverlayChanged()` (dynamiczny flip `StringGrid::isOverlay()` przy `start/stopEditing`). Pierwszy pass renderu celowo nadal filtruje live (poprawność przy dynamicznym statusie). Zmiana semantyki: przy wielu overlayach `getActiveOverlay()` zwraca **górny** (ostatni), nie dolny — poprawny modal (test: dwa dialogi, focus w górnym, po close wraca dolny).
- **Przy okazji**: `overlay_stack.hpp` dopisane do `hpp_order`/`includes_to_remove` w `nob.c` (jak `component_type.hpp` — inaczej `dist/sdl_gui.hpp` wołał niezdefiniowany typ); `nob` trzeba przebudować (`cc -o nob nob.c`) i wymusić regenerację dist touche'm — sama zmiana `nob.c` nie invaliduje `dist/sdl_gui.hpp`.
- Efekt: 45/45 testów (nowy case `GUIManager OverlayStack`: modal-Tab, topmost-wins, grid editing-flip), 48/48 przykładów, release (standalone + C smoke) zielone. Bench vs `bench/pre6.json` (3 runy visible, sober): `frame_loop` ±1% na 100/200/1000w (neutralnie, zgodnie z analizą); sekcje eventowe pływają ±40% run-to-run na tym samym binarniku (szum, nie regresja). Uwaga: run z `--hidden` ucina `frame_loop` do ~57 fps (vsync na ukrytym oknie) — porównywać tylko runy visible.
- Zmienione pliki: src/overlay_stack.{hpp,cpp} (nowe), src/gui_manager.{hpp,cpp}, src/string_grid.cpp, tests/test_gui_manager.cpp, nob.c

### Event punkt 1 — plastry 1b+1c: Focus/Capture + Cursor-serwis (2026-09-06)
- **1b Focus**: nowe `GUIManager::requestFocus()` — jedyna droga widgetów do brania fokusu (polityka: focusable + enabled + visible + alive; `nullptr` zawsze czyści). Migracja: Button/Checkbox/TextInput/TextArea. `setKeyboardFocus()` zostaje mechanicznym setterem (testy/Tab go używają). Click-outside w managerze sprawdza teraz focusowalność głębokim hit-testem (`findElementAt` + spacer w górę po `canGetKeyboardFocus` — klik w Button-z-Labelką trzyma fokus), więc klik w Panel/Label czyści fokus; zdublowany handler w TextArea usunięty (został lokalny reset hover/drag bez wczesnego `return`, żeby event doszedł do managera).
- **1b Capture**: `PointerCapture{autoReleaseOnUp}` — manager zwalnia capture po LEFT-UP nawet gdy właściciel zapomniał `releaseMouse()` (podwójny release jest cichy). Drag poza bounds działał już przez forward capture — dopięty testem (Button + ruch poza + UP poza → brak klika, capture null).
- **1b FocusScope**: `ContextMenu::{showAt,hide}` trzyma `m_returnFocus` (handle generacyjny): show snapshotuje fokus spoza menu, hide przywraca go gdy fokus jest w menu (zamiast gołego clear — koniec ducha focus-overlay *i* utraty fokusu pola tekstowego po kliknięciu itemu).
- **Unifikacje (testy poprawione)**: ukryty TextInput ignoruje klik/typing jak TextArea i baza (koniec testów przypinających buga „hidden still receives focus"); disabled-odstępstwa z 1a domknięte polityką requestFocus.
- **1c Cursor**: `Cursor::handleEvent` skasowany → `updatePosition(event)` (MOTION + BUTTON — jedno źródło prawdy) + `getPosition()` + `render()`; manager karmi pozycję centralnie na górze `processEvent()` (skasowane oba forwardy: gałąź capture i ogon), render przez `render()`. Dziedziczenie po `GUIElement`, slot lifetime, `ElementRef`, C-API (`sdlgui_cursor_*`, `get_type=="Cursor"`) i przykład 33 bez zmian — świadomie, dla stabilnego ABI. Jedyny fallback `SDL_GetMouseState` to udokumentowany cold-start (przed pierwszym eventem).
- Efekt: 45/45 testów (nowe: `requestFocus` polityka 4×, click-outside panel/button, FocusScope-restore, cursor-pod-capture + autoRelease), 48/48 przykładów. Bench vs baseline: wszystko na + (maszyna cichsza niż przy baseline) — jak w 1a, pojedyncze runy niekonkluzywne; brak sygnatury regresji.
- Zmienione pliki: src/gui_manager.{hpp,cpp}, src/context_menu.{hpp,cpp}, src/cursor.{hpp,cpp}, src/button.cpp, src/checkbox.cpp, src/text_input.cpp, src/text_area.cpp, tests/test_cursor.cpp, tests/test_gui_manager.cpp, tests/test_context_menu.cpp, tests/test_text_input.cpp

### Event punkt 1 — plaster 1a: Template Method + contains() z eventu (2026-09-06)
- **Szkielet**: `GUIElement::handleEvent()` to stały potok `propagateToChildren()` → `handleSelf()` → `processRightClick()`; widgety nadpisują `handleSelf()` (nowe, domyślnie hover+button-state), nie `handleEvent()`. `processButtonEvent()` liczy `inside` z koordynatów eventu (MOTION/DOWN/UP) zamiast zgate'owanego `m_isHovered`; reszta eventów po staremu. `updateHoverState()` (protected) — hover/tooltip z MOTION-a w 1× `contains()`.
- **Migracja**: Button/Checkbox/Panel/RadioButton przeszły na `handleSelf()` (DFS i RMB w bazie; Button stracił własny `processRightClick`, Panel ręczny hover-set). Kwalifikowane `Panel::handleEvent` (StringGrid/ScrollArea/dialogi/...) działają bez zmian — trafiają w bazę + wirtualny `Panel::handleSelf`. TextInput/TextArea i reszta celowo nietknięte (plaster 1b/1c).
- **Unifikacja**: disabled Button/Checkbox wchodzi w `Disabled` przy evencie jak reszta widgetów (koniec odstępstwa `return false` przed DFS); test_button poprawiony.
- Efekt: 45/45 testów, 48/48 przykładów. Bench A/B pod obciążeniem (IDE ~60% CPU) niekonkluzywny — szum ±20% w obie strony, sekcje bez eventów (label_text/typing) pływają identycznie, więc brak sygnatury regresji; pliki `/tmp/base_loaded.json` (pre-1a, pod obciążeniem) do wyrzucenia po powtórce na idle. Powtórzyć A/B na spokojnej maszynie przed 1b.
- Zmienione pliki: src/gui.{hpp,cpp}, src/button.{hpp,cpp}, src/checkbox.{hpp,cpp}, src/panel.{hpp,cpp}, src/radio_button.{hpp,cpp}, tests/test_button.cpp

### Bench syntetyczny na prawdziwym oknie — baseline przed punktem 1 (2026-09-06)
- **Nowość**: `bench/bench_gui.cpp` + target `./nob bench` → `output/bench_gui` (prawdziwe okno przez `SDLApp`, flaga `--backend vulkan` na GPU renderer, `--hidden` pod CI). Deterministyczna scena (seed 42): siatka Button/Label/Slider/Checkbox/TextInput/TextArea/Panel-z-dzieckiem, co 5. widget zakotwiczony — 8 sekcji: `hover` (MOTION/DFS, tylko processEvent), `click`, `slider_drag`, `label_text` (setText), `typing` (TEXT_INPUT), `create_destroy`, `resize` (prawdziwe `SDL_SetWindowSize` + `handleResize`), `frame_loop` (mieszany ruch + update/cleanup/render/present, fps + p95). Parametry: `--widgets N`, `--sweep` (100/200/1000), `--seconds S`, `--save/--compare F.json`.
- **Baseline**: `bench/baseline.json` (sweep 5 s, debug+ASan, backend sdl): hover 5.8k/3.7k/0.68k ev/s dla 100/200/1000 widgetów (171 µs → 1.47 ms/ev — superliniowo), frame_loop 278/261/66 fps. Porównanie `--compare` pokazuje delty % per_sec. Uwagi: pętla celowo bez capa (surowy throughput), run-to-run szum ~15% na sekcjach CPU — porównywać na tej samej maszynie/obciążeniu; release (`./nob --release bench`) da stabilniejsze liczby; ASan wypisuje leak-noise SDLa na końcu (nieszkodliwe, testy je tłumią przez `detect_leaks=0`).
- Zmienione pliki: bench/bench_gui.cpp (nowy), bench/baseline.json (nowy), nob.c (`BENCH_DIR`, `build_bench()`, target `bench`)

### Lifetime — SlotMap/Handle + WidgetFactory + diff edytora (2026-09-06)
- **Nowość (punkt 5 planu)**: `src/element_handle.hpp` — `ElementHandle{index,generation}`; `GUIManager` trzyma sloty (generacja rośnie przy `unregister`, brak ABA przy reużyciu adresu). `ElementRef<T>` rozwiązuje się przez slot + weryfikację `raw*` (stary kod z `isAlive`-guardami działa bez zmian, guardy nie są już potrzebne w nowych lambdach). Focus/capture jako handle'e — `cleanup()` bez spaceru `hasAncestorMarkedForDeletion`, powiadomienia `onFocusLost/onMouseCaptureLost` tylko na żywym obiekcie. Tooltip bez ping-ponga własności (stały panel + `setVisible`), `ContextMenu` przez handle (`getContextMenu()` nigdy nie wisi), `ContextMenu::hide()` przez `isFocusInside()`.
- **WidgetFactory** (`src/widget_factory.{hpp,cpp}`): jeden rejestr `string↔ComponentType` + domyślne rozmiary + konstrukcja z `WidgetProps` (wszystkie skalarne propsy z parsera). `LayoutParser::parseNode` buduje tylko `WidgetProps` (strukturalne dzieci — zakładki przez nowy `TabControl::getTabContent/getTabCount`, treść scrolla, kąty łuku — zostały w parserze), `PreviewWindow::createWidget` i `EditorState::addElement` (rozmiary) korzystają z fabryki — koniec 3 kopii `if type==`. Podgląd zyskał `RadioGroup/RangeSlider/ProgressBar/ScrollArea/ArcContainer` (wcześniej cichy fallback do `Panela`).
- **Edytor diff**: `PreviewWindow::m_widgetMap` kluczowane stabilnym `EditorElement.id` (nie indeksem — `onElementDeleted` przychodzi PO `deleteElement`, stary kod usuwał zły widget); nowe `syncAll()` — update w miejscu (geometria/style/skalary, focus/scroll/selekcja przetrwają), recreate tylko przy zmianie strukturalnej (typ/items/tabs), przycinanie usuniętych id. `refreshElement/removeElementWidget` to wrappery (API przykładu 45 bez zmian).
- **C-API**: `checked_elem<T>` (`dynamic_cast` — podtypy jak Slider-jako-Panel dalej działają) w fazach Button/Label/Panel/Slider/Checkbox/TextInput/ListView/TextArea/ComboBox — zły typ to no-op/default + `sdlgui_last_error()` (sukces czyści błąd); nowe `sdlgui_element_is_alive`, `sdlgui_create_widget` (fabryka), warianty `*_get_text_buf`/`*_get_item_text_buf` kopiujące do bufora callera (koniec wiszących `c_str()`). Reszta faz (Progress/Grid/Tab/Cursor/...) zostaje na surowych castach — do dokończenia w tym samym wzorcu. Świadomy kompromis: uchwyty C zostają surowymi wskaźnikami (stabilne ABI Padre-10 przykładów C), walidacja jest po stronie wywołań.
- **Przy okazji**: `component_type.hpp` (i nowe nagłówki) dopisane do `hpp_order`/`includes_to_remove` w `nob.c` — `dist/sdl_gui.hpp` naprawdę samowystarczalny (`-I dist` syntax-check zielony; wcześniej `component_type.hpp` wisiał jako include od punktu 3).
- Efekt: 45/45 testów (nowy `tests/test_lifetime.cpp`: handle/ref/focus/capture/ABA, fabryka 19 typów, diff edytora, C-API błędy/buf/factory), 48/48 przykładów + release (standalone, C smoke) zielone.
- Zmienione pliki: src/element_handle.hpp (nowy), src/widget_factory.{hpp,cpp} (nowe), src/gui.{hpp,cpp}, src/gui_manager.{hpp,cpp}, src/context_menu.cpp, src/layout_parser.{hpp,cpp}, src/tab_control.hpp, src/editor/editor_state.cpp, src/editor/preview_window.{hpp,cpp}, src/sdl_gui.{h,c_api.cpp}, tests/test_lifetime.cpp (nowy), nob.c, docs/release/c_api.md, docs/refactor_plan.md

### Idle CPU — VSync + cap ~60 FPS w pętli głównej (2026-09-06)
- **Przyczyna**: `SDL_CreateRenderer` w SDL3 domyślnie wyłącza vsync (`SDL_RENDERER_VSYNC_DISABLED`), a pętle przykładów nie miały żadnego limitera — przykład 29 kręcił się z ~270 FPS (zmierzone; czysty renderer do 11000+ FPS), co na wielordzeniowym CPU widać jako ~7% (1 rdzeń na 100%). Sam `SDL_SetRenderVSync(renderer, 1)` nie wystarczył — sterownik na X11 go ignorował (nadal 271 FPS przy `vsync=1`).
- **Fix**: `SDLApp` (CPU) i `Window` wołają `SDL_SetRenderVSync(renderer, 1)` po utworzeniu renderera (pomaga tam, gdzie sterownik respektuje vsync + eliminuje tearing) + nowa `SDLApp::endFrame(frameStart)` — dosypia resztę budżetu 16 ms; nie podwaja throttlingu, gdy vsync zadziałał. `SDLApp::run()` i przykład 29 jej używają; przykład 29 dostał też brakujące `guiManager.cleanup()`. Boilerplate pętli w AGENTS.md zaktualizowany.
- Efekt: przykład 29 w idle: 14–20% → 0–3% CPU (widoczne okno, build debug+ASan). Pozostałe ~44 przykłady nadal mają starą pętlę bez capa — migracja to 2 linijki (`frameStart` + `app.endFrame(frameStart)`).
- Zmienione pliki: src/sdl_app.hpp, src/window.cpp, examples/29_resize.cpp, AGENTS.md

### Porządki docs + skill sdl-gui do projektów zewnętrznych (2026-09-06)
- **AGENTS.md → CHANGELOG.md**: 8 starszych wpisów (2026-08-22–08-25: Label multiline, tooltip multiline, Win95Theme, Cursor-flake, Button-parser, anchory-przy-add, Bevel, cleanup null-checków) przeniesionych na górę CHANGELOG.md; w AGENTS.md zostały 4 wpisy z 2026-09-05. Usunięty też wiszący link "do 2026-08-02" w środku listy.
- **docs/**: martwe pliki przeniesione `git mv` do `docs/archive/` (api/, en/, pl/ — era SDL2; getting_started.md root — SDL2; pigulka.md — usunięte `setWindowSize()`; mouse_cursor.md — stara nazwa `MouseCursor`; propozycje/plany: responsive_layout, text_input_text_area, texture_font_review, wysiwyg — zrealizowane lub zastąpione). Zostały: `release/` (kanon), `refactor_plan.md` (w toku), `index.md` (linki do archive/, poprawione 48→49 przykładów).
- **Skill**: naprawione rozjazdy z kodem po refaktorach — `gui-app.md`: podwójna deklaracja `GUIManager` (brak Viewport) → jeden ctor z `Viewport`, stara konwencja float-anchorów (`0–1/>1/0.5`) → enum `HAnchor`/`VAnchor` + int px, `setStyle("Button"…)` → `ComponentType::Button`, wymiary pasków/sidebara z ctora; `rts-game.md`: niekompilujące się `create<Label>(bar,…)` → `make_unique` + `addChild` z `makeRef` przed move, `onExit` → `markForDeletion` zamiast gołego `cleanup()`; SKILL.md: przykłady `00–47` → `00–48`.
- **Dystrybucja**: `nob.c build_release()` kopiuje `skills/sdl-gui/` → `dist/skills/sdl-gui/` (marker: SKILL.md); SKILL.md dostał sekcję instalacji (`cp -r <sdk>/skills/sdl-gui <gra>/.kilo/skills/`) — skill samowystarczalny, gra nie kopiuje źródeł biblioteki. Walidacja `quick_validate.py` PASS dla `skills/` i `dist/skills/`.
- Efekt: `./nob release` zielone (dist zawiera skills/), `diff -r skills/sdl-gui dist/skills/sdl-gui` identyczne. Testów nie ruszano (bez zmian kodu lib).
- Zmienione pliki: AGENTS.md, CHANGELOG.md, docs/index.md, docs/archive/* (git mv), nob.c, skills/sdl-gui/SKILL.md, skills/sdl-gui/references/gui-app.md, skills/sdl-gui/references/rts-game.md

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

Starsza historia zmian: [CHANGELOG.md](CHANGELOG.md).

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
