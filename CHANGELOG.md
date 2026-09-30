# CHANGELOG

Historia zmian projektu — starsze wpisy przeniesione z AGENTS.md
(sekcja „Bieżący stan i ostatnie zmiany").

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
