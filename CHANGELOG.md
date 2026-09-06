# CHANGELOG

Historia zmian projektu — starsze wpisy przeniesione z AGENTS.md
(sekcja „Bieżący stan i ostatnie zmiany").

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

### Bugfix: TextArea nie uczestniczył w systemie keyboard focus — martwa edycja (2026-08-02)

### Bugfix: TextArea pominięty w opt-out współdzielonego render cache (2026-08-02)

### Testy anchorów + 2 realne bugi w systemie Anchor (2026-08-02)
- **Nowy test**: `tests/test_anchor.cpp` (4 case'y, 47 asercji) —

### Test runner równoległy + ukryte okna + brakujące testy (2026-08-02)
- **Problem**: `./nob test` uruchamiał 33 binarki sekwencyjnie (~160 s; każdy test ~2-4 s startu SDL/ASAN) i zalewał konsolę logami.

### C API Phase 3 — RangeSlider, Cursor, ShaderPanel + kontekst GPU (2026-08-02)

### Shared render cache — dedup per (style, state, size) (2026-08-02)

### Refactor pass — dead code removal + dedup + simplification (2026-08-01)
- **Martwy kod usunięty** (skan 3 subagentów + weryfikacja `-Wall -Wextra -Wunused*`): `GUIElement::setParent`/`getCachedTexture`/`setGPUState`/`getGPUState`/`m_gpuState`/`m_style_dirty` (usunięte też 4 martwe gałęzie `SDL_SetGPURenderState` w `render()`), `StringGrid::renderText`/`getHeaderRect`/`getRowHeaderRect`, `logStyle()` ze `style.hpp` (usunięte wywołania z `setState` — znika warning o nieużywanych parametrach), stałe `kDefaultFillColor`/`kDefaultFontSize`, deklaracje/definicje w editorze (`EditorWindow::rebuild`/`updateCheckboxesFromElement`/`addPropertyField`/`addColorSliders`/dynamic-fields/members `m_propertyTextAreas`/`m_propertyCombos`/`m_selectedPaletteType`, `PreviewWindow::refreshAllElements`, `EditorState::updateElement`/`setGridSize`/const `getSelectedElement`, `LayoutImporter::parseJSONElement`/`parseStyleFromJSON`), `GUIElement::draw` pure-virtual → domyślna implementacja `drawBackgroundAndBorder` (usunięte 5 trywialnych override'ów: Button, Panel, ScrollArea, TabControl, CanvasPanel), martwe pliki `tests/test_main.cpp` (main i tak jest w catch_amalgamated), `fake.std.hpp` (0 B), `nob.old`, przypadkowo zacommitowany `.mp4` (2.7 MB), ~15 nieużywanych `#include`.
- **Deduplikacja**: `TextEditable::deleteSelection()` (5 identycznych bloków paste/cut/delete/backspace/input → 1); `TextEditable::charIndexAtX()` (4 kopie binarnego wyszukiwania klik→znak w TextInput/TextArea → 1, teraz char-based dla TextArea z konwersją `charToByteIndex` — kursor nie ląduje w środku znaku UTF-8); `ScopedRenderTarget` RAII w `sdl_rect_helpers.hpp` (4 kopie save/restore target+viewport+clip w gui.cpp/canvas.cpp/shader_panel.cpp — przy okazji naprawia wyciek viewport/clip po `SDL_SetRenderTarget` w `renderToCache`); `CenterRect()` helper (6 kopii centrowania dialogów); tekst renderowany przez `TextureManager::createTextureFromText` (TextInput/TextArea — zyskują cache); `extractKeyVal` (2 lambdy JSON w layout_importer — druga bez escape-handlingu, teraz wspólna i poprawna); `safeParseInt` → wspólne `src/editor/editor_utils.hpp`.
- **Naprawiony martwy callback**: `Button::m_onMouseOver` był write-only (znany bug z AGENTS.md) — teraz wywoływany przy wejściu kursora (test C API wzmocniony: `hoverCalls == 1`); zaktualizowana nota w `docs/release/widgets/Button.md`.
- **Krytyczny bug naprawiony (renderowanie)**: `ScopedRenderTarget` (dedup z tego pasa) czytał stan clipa przez return `SDL_GetRenderClipRect` — a to jest flaga SUKCESU (zawsze `true`), nie stanu. Przy braku clipa destruktor przywracał clip=WŁĄCZONY z rect `0,0,0x0` → cały kadr przycięty do niczego → połowa widgetów (te z cache'em) niewidoczna, zero zaokrąglonych rogów. Fix: `SDL_RenderClipEnabled()` do odczytu stanu (`src/sdl_rect_helpers.hpp`). Przy okazji naprawia pre-existing bug w `shader_panel.cpp` (ten sam wzorzec). Nowy test regresji: `tests/test_render_pixel.cpp` — czyta piksele po `render()` (Panel/Button opakowe, tło przezroczyste).
- Efekt: 32/32 testów, 47/47 examples, `non_unity` (każdy TU osobno) OK, release + smoke testy OK. Netto: −3040 linii (+781/−3821). Zmienione: ~40 plików w `src/`, `tests/test_render_pixel.cpp` (nowy), `tests/test_sdl_gui_c_api.cpp`, `.gitignore` (compile_commands.json, src/embedded_assets.hpp), `docs/release/widgets/Button.md`, `AGENTS.md` (ten wpis)

### End-user docs in release — dist/docs/ + self-contained sdl_gui.hpp (2026-08-01)
- **Problem**: `./nob release` dawało niekompletne dist/ — połączony `sdl_gui.hpp` nie zawierał 14 publicznych klas (StringGrid, ListView, ProgressBar, ScrollArea, ArcContainer, ShaderPanel, Screen/Manager, Window/Manager, GUIContext, ThemePresets, FileDialog), a inline'owane odwołania (`std.hpp`, `logger.hpp`, `constants.hpp`, `sdl_rect_helpers.hpp`, `tinyxml2.h`) nie istniały w dist/ — użytkownik bez src/ nie skompilowałby niczego.
- **Fix**: `hpp_order` rozszerzony do 52 plików (wszystkie publiczne nagłówki + pliki wspierające inline'owane do sdl_gui.hpp). `line_should_remove()` dostał tryb support-header (zostawia systemowe `#include <...>`); dla `std.hpp` usuwana jest konstrukcja `#ifdef __clangd__/#else/import std.compat/#endif` — release zostawia TYLKO tradycyjne includy, więc użytkownik NIE potrzebuje prekompilowanych modułów (-fmodule-file).
- **Smoke test wzmocniony**: standalone kompilowany tylko z `-I dist` (bez src/, lib/, modułów) — dowód samowystarczalności.
- **Dokumentacja**: nowe `docs/release/` (34 pliki, ~5400 linii) — pełna dokumentacja end-user: index, getting_started (linkowanie C++/.a z -flto/.so, C), core (GUIElement/GUIManager/ElementRef), patterns (wzorce + pułapki), 22 widgety (osobne pliki), composites (DialogBox/MessageBox/FileDialog), managers (7 menedżerów), resources (Style/Theme/Anchor/SDLApp/GUIContext/parsery/logowanie), c_api (referencja 200 funkcji). Pisana równolegle przez 8 subagentów wg `docs/release/_STYLE.md` + `_TEMPLATE_EXAMPLE.md` (spójny format, sygnatury 1:1 z nagłówków, język polski). `build_release()` kopiuje `docs/release/` → `dist/docs/` (marker: `_STYLE.md`).
- **Uwaga**: AGENTS.md wpis o Splitter (2026-08-01) nie ma odpowiedników w tym checkout (src/splitter.hpp nie istnieje) — praca z innej sesji/worktree; dokumentacja release celowo nie wspomina o Splitter.
- Efekt: release OK, standalone OK, ręczny test użytkownika (kompilacja+link tylko z dist/, bez modułów) OK. Zmienione: `nob.c` (hpp_order, filtry, smoke test, kopiowanie docs), `docs/release/*` (nowe, 34 pliki), `docs/index.md`, `AGENTS.md` (ten wpis)

### Vulkan slow startup on NVIDIA fixed — keep driver loaded (2026-08-01)
- **Problem**: `SDL_CreateGPUDevice` na NVIDIA RTX 2060 Mobile trwał ~4.5s (po aktualizacji sterowników; wcześniej "kilkanaście sekund"). Diagnoza przez wrapper ICD (`/tmp/kilo/icdwrap/`): init biblioteki `libGLX_nvidia.so.0` (dlopen) = ~1.85s — to przebudzenie dGPU z D3Cold (znany problem NVIDIA, potwierdzony na forums.developer.nvidia.com, bez fixa z ich strony). SDL3 robi PrepareVulkan dwukrotnie (VULKAN_PrepareDriver + VULKAN_CreateDevice), loader dlclose'uje ICD między przebiegami → 2× przebudzenie = ~4.5s.
- **Fix**: `setenv("VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING", "1", 0)` w konstruktorze GPU w `SDLApp` przed `SDL_CreateGPUDeviceWithProperties` (oficjalna opcja Vulkan-Loader od PR #1260, 2023). Loader nie wyładowuje ICD → drugi przebieg nie budzi dGPU ponownie.
- Efekt: NVIDIA ~4.5s → ~2.1s; Intel bez zmian (~46ms); 47/47 examples się buduje.
- Zmienione: `src/sdl_app.hpp` (setenv), `AGENTS.md` (ten wpis)
### ShaderPanel animated uniforms — drawDirect + vertex colors (2026-07-31)
- **Problem**: Intel Vulkan driver (ANV, UHD 630) crashował SEGV w `VULKAN_CreateGraphicsPipeline` gdy `ShaderPanel` blitował cache przez `SDL_RenderTexture` z render state + shaderem samplującym teksturę. Dodatkowo `SDL_SetGPURenderStateFragmentUniforms` (push constants) nie docierał do shadera (statyczny output nawet na llvmpipe).
- **Rozwiązanie**: `ShaderPanel` używa `wantsDirectRender()` + `drawDirect()` — rysuje content panelu do `m_tempTexture`, potem blituje przez `SDL_RenderGeometry` z aktywnym render state. Dane per-frame (czas, pozycja myszy) przekazywane przez kolory werteksów (`SDL_Vertex.color` → fragment input `location = 0`, uv → `location = 1`; interfejs SDL GPU renderera potwierdzony w `SDL_render_gpu.c`/`tri_texture.vert`).
- **Nowe API**: `setUniformTime(float)`, `setUniformMouse(x, y)`. Czas przez istniejący `AnimationManager::addAnimation()`, kursor przez event loop — zero nowych hooków.
- **Shadery bez samplera**: `time_water.frag` / `mouse_glow.frag` (przykład 45) są w pełni proceduralne (fragTexCoord + fragColor), nie próbkują tekstury — to unika problematycznej ścieżki samplowania na Intelu. `desaturate.frag` (przykład 34) działa bez zmian.
- Efekt: 47/47 examples, 31/31 tests. Przykład 45 działa na Intel i llvmpipe.
- Zmienione: `src/shader_panel.hpp`/`.cpp` (przepisane), `examples/shaders/time_water.frag` (nowy), `examples/shaders/mouse_glow.frag` (nowy), `examples/45_gpu_shader_animation.cpp` (nowy), `nob.c` (rejestracja shaderów)

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


### ThemePresets — 4 predefiniowane motywy (2026-07-14)

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
