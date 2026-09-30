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
| **Widgety (23)** | Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, RangeSlider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu, Cursor, ArcContainer, ProgressBar, ScrollArea, WorldView, ShaderPanel |
| **Composite** | ModalDialog (baza), DialogBox, MessageBox, FileDialog, ColorDialog (`src/composite/`) |
| **Editor** | EditorWindow, EditorState, PreviewWindow, LayoutImporter, LayoutExporter (`src/editor/`) |
| **Ekrany/okna** | ScreenManager (gry), WindowManager (wiele okien systemowych) |
| **Zasoby** | TextureManager, FontManager, TimerManager, AnimationManager |
| **Parsery** | JsonParser, SGMLParser, LayoutParser — definicja GUI z JSON/XML |
| **Style** | Style + Theme — tablica `[ComponentType][stan]` (`array<optional<Style>, 4>`) per typ per stan (Normal/Hovered/Pressed/Disabled) |
| **Layout** | Viewport (NonZero w ctorze GUIManagera) + Anchor (`HAnchor`/`VAnchor` enum + marginesy px) + Dock (`Dock` enum) + LayoutPass Measure/Arrange (`ILayoutManager`: `AnchorLayout` domyślny, `StackLayout`, `DockLayout`; `layoutChildren()` per widget) |

### Źródła

```
src/           — implementacja (C++23, moduły)
src/composite/ — gotowe dialogi
src/editor/    — edytor wizualny GUI
examples/      — 68 przykładów (00–67); examples/c/ — 10 przykładów C
tests/         — 50 binarek testowych (Catch2)
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
- **Widgety testowane (23)**: Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, RangeSlider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor, ArcContainer, ProgressBar, ScrollArea, WorldView, ShaderPanel (CPU)
- **Composite (2)**: ModalDialog, ColorDialog
- **Menedżery (4)**: FontManager, TextureManager, TimerManager, AnimationManager
- **Systemy (11)**: GUIElement, GUIManager, Theme, Easing, UTF8, Anchor (enum H/V + marginesy px, presety, Viewport NonZero, LayoutPass, StackLayout, DockLayout), Style, TextEditable (bazowa klasa przez podklasę testową), RenderCache, RenderPixel (pikselowa walidacja renderowania), Performance
- **Layout/helpery/lifetime (3)**: DockLayout (+fixture'y JSON/XML), UI helpers (`ui_helpers`), Lifetime (SlotMap/Handle + WidgetFactory + diff edytora)
- **Screen/Window (2)**: ScreenManager, WindowManager
- **Parsery (3)**: JsonParser, SGMLParser, LayoutParser (fixture'y w `tests/data/` — `layout.json`, `layout.xml`, `widgets.json`, `win95_bevel.json/xml`, `bad.*`)
- **C API (1)**: test_sdl_gui_c_api (Phase 0+1+2+3; + pixel test renderowania kursora — pozycja myszy ze syntetycznego motion eventu, bez warpowania wskaźnika)

Testy integracyjne: 68 przykładów (`examples/`, 00–67) + 10 przykładów C (`examples/c/`) do manualnej weryfikacji wizualnej.

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

### ColorDialog — modalny picker RGB/HSV (2026-09-30)
- **Nowość**: `src/composite/color_dialog.{hpp,cpp}` (`ColorDialog : ModalDialog`, `create()` jak `FileDialog`): 3x Slider RGB + 3x H/S/V + HEX `TextInput` + preview old/new + 12 presetów + OK/Cancel; sync dwukierunkowy z guardem `m_syncing` (setValue/setText wołają callbacki synchronicznie), konwersje int-math tylko w event path, zły HEX ignorowany. `ComponentType::ColorDialog` dopisany (bez fabryki — precedens pozostałych dialogów).
- Efekt: 50/50 testów (nowy `test_color_dialog.cpp`: roundtrip RGB↔HSV, guard/sync, HEX, preset-klik, OK/Cancel, Enter), 67 przykładów (`67_color_dialog.cpp`), release + smoke zielone.
- Zmienione pliki: src/composite/color_dialog.{hpp,cpp} (nowe), src/component_type.hpp, tests/test_color_dialog.cpp (nowy), examples/67_color_dialog.cpp (nowy), nob.c (`hpp_order` + `includes_to_remove`)

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

### Audyt przykładów → 9 nowych helperów w ui_helpers (2026-09-08)
- **Co**: audyt wszystkich 65 przykładów (3 subagenty po ~20 plików, weryfikacja grepem) z progiem uniwersalności 3+ użycia w różnych domenach. Nowe w `ui_helpers`: Pakiet A (fundament) — `addTracked<T>` (fuzja `makeRef`-przed-`move` + `add`, zwraca `pair<T*, ElementRef<T>>`; zabija udokumentowany crash-pitfall, 40+ call sites w 20+ plikach), `addButton`/`addLabel` (one-linery z tekstem/callbackiem/kolorem), `styleCard` (kanoniczny ciemny blob z defaultami) + `addDarkPanel` (fabryka). Pakiet B (formularze) — `addLabeledCheckbox` (box + caption, opcjonalny kolor tekstu — potrzebny na ciemnych panelach) + `makeTopBar`/`makeBottomBar` (struct `StatusBar{bar,label}`, label-dziecko z insetem 8px). Pakiet C (siatki+suwaki) — `makeGrid<T>` (fabryka `(manager,col,row,x,y,w,h)`, spójna z `makeStrip`; uogólnia `gridPanels`/`faceGrid`), `onAnyChange` (wspólny `refresh()` + natychmiastowe odpalenie, overloady Slider/RangeSlider), `bindSliderValue` (int + float ze skalą — rodzeństwo `linkLabel`). Konwencja jak dotąd: impl z `(manager, parent*)` + cienkie publiczne overloady; fabryki szablonów w hpp.
- **Niche odrzucone**: status `"Selected: …"` (16,17), kolumna Add/Remove/Clear (16,17 — kryje `makeStrip`), theme-switcher RadioGroup (27,28), d-pad focus-nav (43,44), bochenki ShaderPanel (40,41), captiony nad/pod sliderem (niespójna geometria), `setTooltip`-jednolinijkowce. 48 (Win95, status bar na anchorach) i 29 (Anchor::bottomBar) świadomie nietknięte — helpery barów nie wspierają anchorów.
- Efekt: 46/46 testów (`test_ui_helpers.cpp`: +7 TEST_CASE — tracked/card/builders/grid/sliders/status bars, asercje przez `getComposedStyle` i syntetyczne click-eventy), 65/65 przykładów, smoke 10 migrowanych headless czysty (wycieki 42/44 z `SDL_X11_SetWindowTitle` — szum SDLa, nie nasz kod), release zielone.
- Zmienione pliki: src/ui_helpers.{hpp,cpp}, tests/test_ui_helpers.cpp, examples/{04_text_input,05_slider,06_checkbox,10_range_slider,36_file_dialog,42_mobile_touch,44_tv_remote,49_arc_sliders,51_bounce_lab,55_gravity_box}.cpp, docs/release/patterns.md, skills/sdl-gui/references/gui-app.md

### Pakiet z gier: Button::setText + ui_helpers + after()/every() (2026-09-07)
- **Co**: (1) `Button::setText/getText` — podmiana (lub leniwe utworzenie, gdy caption był pusty) wycentrowanego Label-dziecka + `layoutChildren/markDirty`; surowy `Label*` ZOSTAJE (ownership `unique_ptr` + lifetime związany z rodzicem, jak `Button*` w Sliderze; `weak_ptr` wymagałby `shared_ptr` w całym libie — libowy odpowiednik słabej referencji to `ElementRef`/handle'e). Przy okazji naprawiło to przykład w `docs/release/managers.md:237`, który używał nieistniejącego `setText`. (4) Nowy `src/ui_helpers.{hpp,cpp}` — baza wolnych funkcji: `linkLabel` (prefix/suffix i formatter) + `linkRangeLabel` (jw.), odświeżenie natychmiastowe, label trzymany przez `ElementRef`; uwaga "jedno wiązanie na widget" (nadpisuje jak `setOnChangeCallback` — złapane testem). `ui_helpers.hpp` dopisane do `hpp_order` w `nob.c`. (5) `GUIManager::after(ms,fn)/every(ms,fn)/cancelTimer(id)` — cienkie wrappery na `TimerManager` (target `nullptr`, wzorzec już używany w testach); NIEpowiązane z lifetimem widgetów (udokumentowane) + sekcje w `managers.md`/`patterns.md`/skillu. Dogfood: 62_memory przepisane z ręcznego deadlinu na `after(700)` z guardem `dealId` na restart.
- **Generatory w ui_helpers (dogrywka tego samego dnia)**: `gridPanels` (siatka Paneli, dzieci lub top-level), `faceGrid` (Button + centrowany Label-face — obchodzi brak `Button::setText` w grach), `makeStrip<T>` (szablon z fabryką `(manager,x,y,w,h,index)` na paski widgetów), `shuffledPairs(n)` (talia do Memory). Wpadki: overload `create<T>(parent,…)` wymaga ctora `(manager,parent,…)` — żaden widget tak nie ma, więc impl przez `make_unique` + `addChild/addElement`; szablon z `GUIManager&` wymaga pełnego typu → `ui_helpers.hpp` includuje `gui_manager.hpp` (stripowany w amalgamacie), a wpis w `hpp_order` wylądował ZA `gui_manager.hpp`. Dogfood: 59→`gridPanels`, 60→`faceGrid` (cyferki centrowane zamiast (12,6)), 62→`shuffledPairs`.
- Efekt: 46/46 testów (nowe: `test_ui_helpers.cpp` — 3 sekcje bindingów + 4 sekcje generatorów; case'y setText w `test_button.cpp`; `after/every` w `test_timer_manager.cpp`), 65/65 przykładów, smoke 59/60/62 headless czysty.
- Zmienione pliki: src/button.{hpp,cpp}, src/ui_helpers.{hpp,cpp} (nowe), src/gui_manager.{hpp,cpp}, tests/test_button.cpp, tests/test_ui_helpers.cpp (nowy), tests/test_timer_manager.cpp, examples/{59_snake,60_minesweeper,62_memory}.cpp, nob.c, docs/release/{widgets/Button,managers,patterns}.md, skills/sdl-gui/references/gui-app.md

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


Starsza historia zmian: [CHANGELOG.md](CHANGELOG.md).

---

## Zasady utrzymania pliku

Po każdej **znaczącej** zmianie (nowa funkcjonalność, optymalizacja, istotny bugfix, refactor) zaktualizuj sekcję „Bieżący stan i ostatnie zmiany". Wpis powinien być zwięzły i zawierać: datę, co zmieniono, efekt (testy/examples), listę zmienionych plików.

**Nie aktualizuj** przy zmianach kosmetycznych (formatowanie, nazwy zmiennych bez zmiany logiki) ani tymczasowych branchach eksperymentalnych.

**Nie commituj sam** — agent nigdy nie woła `git commit` z własnej inicjatywy. Zmiany zostają w working tree do decyzji użytkownika; commit tylko na wyraźną prośbę.

Jeśli zmieniono architekturę (nowy wzorzec, nowy menedżer, nowa warstwa abstrakcji), zaktualizuj również odpowiednie sekcje powyżej.
