# Plan: cache lifetime, cleanup, mouse capture, clip dedupe

Data: 2026-11-22
Status: gotowy do implementacji (po decyzjach poniżej)

## Kontekst

Audyt wydajności (release, OpenGL, 1280×800) wykazał m.in.:

- `update()` co klatkę woła `pruneUnused()` przy `renderCache > 256`; w scenie StringGrid + 300 labeli to 0.21 ms → 8.76 ms/frame, bo text cache jest kasowany i tekst jest rasteryzowany od zera.
- `cleanup()` przechodzi całe drzewo co klatkę: 82–100 µs/frame przy 4000 widgetach, mimo braku usunięć.
- Slider/RangeSlider/TextInput/TextArea/StringGrid nie używają `captureMouse`; drag motion dalej leci przez cały DFS (42.8 µs/event przy 4000).
- Bezwarunkowy `SDL_SetRenderClipRect(&parent_clip_rect)` na końcu każdego `GUIElement::render()` powoduje toggling clipa i 1614 alokacji/frame w SDL (0.58–0.62 ms → 0.37–0.38 ms po wyłączeniu clipChildren w tej samej scenie).

## Decyzje użytkownika (wiążące)

1. **Render/text cache**: `weak_ptr` w cache, widget jest właścicielem tekstury. Zero `pruneUnused`, zero byte-budget, zero globalnego GC ani limitu pamięci. Tekstura ginie z ostatnim holderem.
2. **Path cache (assety)**: zostaje `strong` i bez prune/budżetu — assety PNG/TTF są statyczne i nieliczne; `Image`/`Style` trzymają teksturę.
3. **Wygasłe wpisy metadanych**: mapa + **jawne release** (bez okresowego sweepa). Dla widgetów trzymających tekstury — RAII `TextLease`; dla render cache — release klucza przy zmianie/zniszczeniu.
4. **Cleanup**: licznik/flag pending w `GUIManager` + natychmiastowy early return; gdy >0 zostaje obecny rekurencyjny pass. Vector<ElementHandle> — poza zakresem.
5. **Mouse capture**: jedna umowa dla wszystkich dragów (capture na down, release na up/lost, reset stanu w `onMouseCaptureLost`).
6. **Clip dedupe**: bez zmiany semantyki `m_clip_children`; overlaye/tooltipy/context menu są top-level i muszą pozostać nieucięte przez clip ostatniego elementu.

## Poza zakresem tego planu

- Event DFS O(N) dla hover/click (osobny temat).
- Reuse prywatnej tekstury cache dla non-shared widgetów (ProgressBar itd.).
- Optymalizacja rasteryzacji dynamicznego tekstu / atlas glifów.
- `TimerManager` allokacje per update.

---

## A. Cache lifetime: `weak_ptr` + jawny release

### A1. `TextureManager` — nowy model

Pliki: `src/texture_manager.hpp`, `src/texture_manager.cpp`.

- Path cache (`m_textureCache`): pozostaje `unordered_map<string, SharedTexture>` (strong). Usunąć z niego księgowanie bajtów i LRU.
- Render/text cache (`m_renderCache`, `m_textCache`): zmienić na `unordered_map<uint64_t, std::weak_ptr<SDL_Texture>>`.
- Usunąć całkowicie:
  - `CacheEntry`, `lastUse`, `m_lruClock`, `touch()`;
  - `m_bytesUsed`, `m_byteBudget`, `kDefaultByteBudget`, `getBytesUsed()`, `getByteBudget()`, `setByteBudget()`;
  - `pruneUnused()`, `enforceByteBudget()`, `textureBytes()`, `insertEntry()` (lub uprościć do path cache);
  - `kRenderCachePruneThreshold` i blok prune z `GUIManager::update()` (`src/gui_manager.cpp:14`, `:193-196`).
- `renderCache(key, w, h, draw)`:
  - `weak.lock()` → hit;
  - jeśli wpis wygasł: utwórz teksturę, narysuj, **nadpisz** weak_ptr w istniejącym wpisie;
  - brak wpisu: utwórz i wstaw weak_ptr; zwróć `SharedTexture` (holderem jest widget).
- Dodać `void releaseRenderCache(uint64_t key)`: usuń wpis **tylko jeśli weak_ptr wygasł**; jeśli inny widget nadal trzyma teksturę, wpis zostaje (zachowanie sharingu).
- `createTextureFromText(text, font, color)` → **uncached**: zawsze rasteryzuje nową teksturę, bez wpisu w mapie. Wydzielić prywatny helper `rasterizeTextTexture(...)`.
- Dodać cached acquire dla trwałych holderów:
  - `class TextureManager::TextLease` (move-only, default ctor, dtor, `get()`, `operator bool`, `reset()`, `key()`);
  - `TextLease acquireTextTexture(text, font, color)`: weak lock albo rasteryzacja + weak wpis; lease trzyma `SharedTexture` + key + `TextureManager*`;
  - `~TextLease()`: `m_texture.reset();` potem `owner->releaseTextCache(key)`;
  - `releaseTextCache(key)` (private, friend `TextLease`): usuń wpis tylko jeśli weak wygasł.
- `clearCache()`: czyści mapy (path/render/text). Żywe tekstury trzymane przez widgety pozostają; kolejne lookupi mogą utworzyć duplikaty. Udokumentować.
- `getRenderCacheSize()` / `getTextCacheSize()`: bez zmian (liczą wpisy; wygasłe wpisy znikają po release).
- `hasTexture/getTexture/queryTexture/loadTexture/addTexture`: path cache bez zmian semantyki.

### A2. `GUIElement` — release render cache

Plik: `src/gui.cpp` (`~GUIElement`, `renderToCache`).

- Dodać prywatny helper np. `void releaseCachedRenderTexture()`:
  ```cpp
  if (m_cacheKey != 0) {
      const uint64_t old = m_cacheKey;
      m_cachedTexture.reset();          // najpierw drop own strong ref
      m_cacheKey = 0;
      m_manager.getTextureManager().releaseRenderCache(old);
  }
  ```
- `~GUIElement()`: wywołać helper (przed zniszczeniem `m_cachedTexture`; destruktor `TextureManager` żyje — `GUIManager::~GUIManager` czyści `m_elements` w ciele, przed memberami).
- `renderToCache()`:
  - early-out `!m_visible || w<=0 || h<=0`: helper zamiast samego `reset()` (`gui.cpp:856-861`);
  - shared path (`gui.cpp:865-874`): jeśli `key != m_cacheKey`, po pobraniu nowej tekstury zrobić drop starej + release starego klucza, dopiero potem przypisać nową;
  - przejście shared → private (`gui.cpp:877-885`): helper przed ustawieniem `m_cacheKey = 0`;
  - private → shared: `m_cacheKey` jest 0, nic do zwolnienia.
- Uwaga: `AnimatedImage::setUseCache(false)` resetuje `m_cachedTexture` bez klucza (canShare=false, key=0) — bez zmian.

### A3. Widgety trzymające tekstury tekstowe

Zasada:
- **Bake-time draw** (Label, ProgressBar, ComboBox, MenuPopup, MenuBar — rysują do własnej tekstury cache): używają uncached `createTextureFromText`; nie tworzą wpisów w `m_textCache`.
- **Trwałe holdery** (rysują bezpośrednio lub trzymają teksturę między klatkami): `TextureManager::TextLease`.

Zmiany:
- `src/text_input.cpp/.hpp`: `SharedTexture m_textTexture` → `TextureManager::TextLease m_textLease`; `refreshTextTexture()` robi `reset()` i `acquireTextTexture`; draw używa `m_textLease.get()`.
- `src/text_area.cpp/.hpp`: `std::vector<SharedTexture> m_line_textures` → `std::vector<TextureManager::TextLease>`; `refreshTextures()` najpierw `clear()` (lease release), potem acquire per linia; draw używa `.get()`.
- `src/string_grid.cpp/.hpp` (+ `ListView` dziedziczy):
  - dodać `std::vector<std::vector<TextureManager::TextLease>> m_cellTextures` (mirror `m_data`);
  - `m_columnHeaderTextures`, `m_rowHeaderTextures`;
  - `drawCell`/`drawColumnHeaders`/`drawRowHeaders` używają lease (lazy acquire przy pierwszym rysowaniu komórki/nagłówka);
  - `setCellText`, `setRowCount`, `setColumnCount`, `clear`, `sortByColumn`, zmiana fontu/koloru → reset odpowiednich lease;
  - nie trzymać tekstur wszystkich możliwych komórek na zapas — lease powstaje przy pierwszym użyciu i żyje z widgetem (zgodnie z modelem „widget właścicielem”).
- `src/menu_list.cpp/.hpp`: dodać lease dla tytułu i pozycji; budować przy `setItems`/zmianie fontu, resetować przy przebudowie; drawDirect używa gotowych tekstur zamiast `menu_render::drawText` per frame.
- `src/menu_popup.cpp` + `src/menu_bar.cpp`:
  - popup: lease per item (tekst + skrót) budowane przy `setItems`/refresh; draw z gotowych tekstur;
  - bar: lease per tytuł menu; przebudowa przy `setMenus`/zmianie fontu/theme.
  - `menu_render::drawText` może zostać dla ścieżek bake-time, ale draw per-frame ma używać lease.
- `src/label.cpp`, `src/progress_bar.cpp`, `src/combobox.cpp`, `examples/34_texture_font_previewer.cpp`: bez zmian wywołań — `createTextureFromText` staje się uncached; example 34 trzyma teksturę w `Style`, więc jest bezpieczny.

### A4. Testy cache

Plik: `tests/test_texture_manager.cpp`; dostosować też `tests/test_render_cache.cpp`, `tests/test_style.cpp`, `tests/test_gui_manager.cpp`.

- Usunąć sekcje byte-budget/LRU/prune.
- Nowe/zmienione przypadki:
  - dwa identyczne widgety żyją → 1 wpis render cache i ten sam `SDL_Texture*`;
  - zniszczenie jednego z dwóch → wpis zostaje;
  - zniszczenie ostatniego → wpis znika (`getRenderCacheSize()` maleje);
  - zmiana tekstu dynamicznego Label zmienia klucz → stary wpis znika po release (brak narastania mapy);
  - `acquireTextTexture` × 2 → jedna tekstura; release pierwszego → wpis zostaje; release drugiego → wpis znika;
  - `createTextureFromText` jest uncached (dwa wywołania → różne tekstury; brak wpisu w `getTextCacheSize()`);
  - `GUIManager::update()` **nie** czyści text cache przy >256 wpisach render cache;
  - path cache nadal strong: `loadTexture` + upuszczenie lokalnego shared_ptr nadal daje hit (bo cache trzyma strong) — zgodnie z decyzją.
- Test regresyjny wydajnościowy (opcjonalny, w `bench`): StringGrid + 300 labeli, `render+update` ≈ `render` (bez 8–12 ms).

### A5. Dokumentacja

- `docs/release/managers.md`: usunąć wiersze `pruneUnused`, `setByteBudget`, `getBytesUsed`, `getByteBudget`; opisać weak cache, `TextLease`, uncached `createTextureFromText`, path cache strong.
- `AGENTS.md`: zaktualizować sekcje o `TextureManager`/„Punkt 6 plaster 4”/Memory i wpis w „Bieżący stan” po wdrożeniu.
- `docs/archive/texture_font_manager_review.md` — nie ruszać (archiwum).

---

## B. Cleanup: licznik pending + early return

Pliki: `src/gui_manager.hpp/.cpp`, `src/gui.cpp`.

- `GUIManager`: dodać `size_t m_pendingDeletionCount = 0;` oraz wewnętrzne `notifyDeletionPending()` / `clearDeletionPending()`.
- `GUIElement::markForDeletion()`:
  ```cpp
  if (m_isMarkedForDeletion) return;
  m_isMarkedForDeletion = true;
  m_manager.notifyDeletionPending();
  ```
- `GUIManager::detachElement()`: jeśli element był oznaczony, wyczyścić flagę i decrementować licznik (inaczej oznaczony, ale odłączony element zostawiłby licznik > 0 na zawsze).
- `GUIManager::cleanup()`:
  - na początku: jeśli `m_pendingDeletionCount == 0` → `return;` (focus/capture stale po `clearChildren`/`detach` i tak resolvują się do null);
  - gdy >0: obecny rekurencyjny pass bez zmian;
  - na końcu: `m_pendingDeletionCount = 0` (elementy nieosiągalne: zniszczone przez `clearChildren`, odłączone — zostały już policzone).
- `GUIElement::cleanup()` (rekurencyjny) bez zmian; nadal czyści dzieci, `markDirty(false)`, `layoutChildren()`.
- Testy:
  - brak oznaczeń → cleanup nie zmienia drzewa (test funkcjonalny; brak łatwej asercji na liczbę odwiedzin);
  - oznaczone dziecko / rodzic+dzieci / duplikat `markForDeletion`;
  - `detachElement` oznaczonego elementu, potem ponowne dodanie i cleanup;
  - focus/capture/tooltip nadal czyszczone przy usunięciu;
  - licznik wraca do 0 po cleanupie.

---

## C. Mouse capture: jedna umowa i migracja dragów

### C1. API/umowa

Pliki: `src/gui.hpp`, `src/gui_manager.hpp/.cpp`.

- `GUIElement` (nievirtualne helpery):
  ```cpp
  void captureMouse();        // m_manager.captureMouse(this)
  void releaseMouse();        // tylko jeśli to ja mam capture
  bool hasMouseCapture() const;
  ```
- Udokumentować kontrakt przy `onMouseCaptureGained/Lost`:
  - LMB down w obszarze drag → `captureMouse()`;
  - LMB up → obsłuż i `releaseMouse()` (manager i tak auto-release na up);
  - `onMouseCaptureLost()` → wyzerować `m_isDragging`/`m_activeThumb`/`m_isSelecting` itd.;
  - podczas capture eventy trafiają tylko do właściciela; hover wraca przy następnym realnym motion (bez syntetycznych eventów).
- `GUIManager::processEvent`: przed routingiem capture, jeśli captured element jest `!isVisible()` lub `!isEnabled()` → `releaseMouse()` (ochrona przed dragiem na ukrytym/disabled widgecie). Zachować obecne auto-release na LMB up.

### C2. Migracje

- `src/button.cpp`, `src/checkbox.cpp`, `src/panel.cpp`, `src/canvas.cpp`, `src/editor/preview_window.cpp`: zamienić bezpośrednie `m_manager.captureMouse/releaseMouse` na helpery; dodać/resetować stan w `onMouseCaptureLost` (Button/Checkbox: `Pressed` → `Normal`).
- `src/slider.cpp`:
  - na down w track: `m_isDragging = true; captureMouse();`
  - na up: `m_isDragging = false; releaseMouse();`
  - `onMouseCaptureLost()`: `m_isDragging = false;`
  - motion poza widgetem podczas drag ma dalej aktualizować wartość (już clampuje).
- `src/range_slider.cpp`:
  - capture przy down na thumb/track; release przy up;
  - **usunąć** early-out `contains` w motion gdy `m_activeThumb != None` (linie ~67-72) — drag ma działać poza widgetem i clampować;
  - `onMouseCaptureLost()`: `m_activeThumb = None; m_hoveredThumb = None;`
- `src/text_input.cpp`:
  - down w polu: `captureMouse()`;
  - up: `releaseMouse()`;
  - `onMouseCaptureLost()`: `m_isDragging = false;`
  - zachować obecne rozszerzanie selekcji poza polem (focus-based).
- `src/text_area.cpp`: analogicznie.
- `src/string_grid.cpp`:
  - przy rozpoczęciu selekcji (`m_isSelecting = true`) `captureMouse()`; przy up `releaseMouse()`;
  - `onMouseCaptureLost()`: `m_isSelecting = false;`
  - zachować obecne zachowanie poza obszarem (getCellAtPosition invalid → brak zmiany); to tylko perf.
- `src/menu_popup.cpp`, `src/menu_list.cpp`, `src/context_menu.cpp`: brak drag state — bez zmian.

### C3. Testy capture

- Slider: down → motion poza widget → wartość się zmienia/clampuje → up poza → drag kończy się; `slider_drag` bench @4000 zbliża się do @200.
- RangeSlider: drag poza widgetem kontynuuje (nowe zachowanie vs obecne); up poza kończy.
- TextInput/TextArea: drag poza pole rozszerza selekcję; up poza kończy; focus/capture czyszczone.
- StringGrid: selekcja + capture; up poza kończy; slider w gridzie nadal działa.
- Capture lost: `setVisible(false)`/`markForDeletion`/`setEnabled(false)` w trakcie drag → stan wyzerowany, brak podwójnego `onMouseCaptureLost`.
- Auto-release: zawsze jeden `onMouseCaptureLost` na up (owner release + manager fallback nie dubluje).
- Hover po dragu: następny motion ustawia prawidłowy stan; brak syntetycznych eventów.

---

## D. Clip dedupe bez zmiany semantyki

### D1. Usunięcie redundantnego restore

Pliki: `src/gui.cpp`, `src/gui_manager.cpp/.hpp`.

- Usunąć bezwarunkowe `SDL_SetRenderClipRect(renderer, &parent_clip_rect)` na końcu `GUIElement::render()` (`src/gui.cpp:796`).
- Gałęzie, które faktycznie zmieniają clip, już go przywracają:
  - direct draw (`gui.cpp:709-713`),
  - draw hooks (`gui.cpp:704-709`),
  - rotated blit (`gui.cpp:736-749`),
  - focus overlay (`gui.cpp:764-766`).
  - Zweryfikować każdą z nich testem pikselowym.
- `GUIElement::render(SDL_Renderer*)` (public wrapper, `gui.cpp:619-623`) może zachować restore viewportu dla zewnętrznych wywołań. `GUIManager::render()` ma wołać nową ścieżkę **bez** końcowego restore dla każdego top-level (np. `friend class GUIManager` + protected `renderTree(renderer, clip)`, albo publiczne `renderForManager`). Dzięki temu nie ma 1 wywołania clipa na każdy top-level.
- `GUIManager::render()`: po głównym passie i **przed** tooltipem/overlayami oraz przed kursorem ustawić `SDL_SetRenderClipRect(m_renderer, nullptr)` (jeden raz). Końcowe `nullptr` zostaje.
- Overlaye/tooltipy/context menu/menu popupy są top-level — nie mogą dziedziczyć clipa ostatniego elementu.

### D2. StringGrid: clip raz, nie per cell

Plik: `src/string_grid.cpp`.

- `drawDirect`:
  - zapamiętać clip wejściowy (`SDL_RenderClipEnabled` + `SDL_GetRenderClipRect`);
  - ustawić cell-area clip raz przed `drawCells`;
  - `drawCell` **bez** `SDL_SetRenderClipRect` per komórka (`:986-992`); zamiast tego przycinać tekst przez `src_rect` (jak `GUIElement::render`), gdy tekst szerszy/wyższy niż komórka;
  - po komórkach przywrócić zapamiętany clip (nie `nullptr`) przed rysowaniem nagłówków/grid lines; na końcu przywrócić clip wejściowy.
- Zachować obecne wizualne przycinanie tekstu komórek i nieprzycinanie nagłówków do cell area.
- Sprawdzić `ListView` (dziedziczy) i inne direct widgety pod kątem `SDL_SetRenderClipRect(nullptr)` — zamienić na restore zapamiętanego clipa.

### D3. Testy clip/overlay

- Context menu, tooltip, dialog, menu popup rysowane poza bounds parenta — nadal w pełni widoczne (pixel test).
- Focus outline po cached sibling nie zostaje w clipie poprzedniego elementu (istniejący regres; dodać cursor).
- Cursor po sfokusowanym/direct widgecie nie jest ucinany.
- ScrollArea/WorldView nadal przycina dzieci (istniejące testy).
- Rotated content zwis poza oś-aligned rect nadal widoczny (istniejący test).
- StringGrid: długi tekst komórki nie wychodzi do sąsiedniej komórki; nagłówki nadal widoczne nad cell area; grid w ScrollArea respektuje viewport.
- Pixel test: brak clipa po top-level nie zmienia wyglądu scen bez focus/direct.

---

## E. Walidacja

### Testy

```
./nob test
./output/test_texture_manager
./output/test_render_cache
./output/test_style
./output/test_gui_manager
./output/test_gui_element
./output/test_slider
./output/test_range_slider
./output/test_text_input
./output/test_text_area
./output/test_string_grid
./output/test_menu_bar
./output/test_render_pixel
```

### Przykłady/release

```
./nob examples
./nob -r bench
```

### Benchmarki (oczekiwane efekty)

- `cleanup` @4000 bez usunięć: ~100 µs → ~0 (early return).
- `update` @4000 przy >256 wpisach: brak prune; StringGrid + 300 labeli `render+update` ≈ `render` (usunięcie 8–12 ms/frame).
- Render 4000 (480 widocznych): 0 alokacji/frame w steady state; czas ~0.6 ms → ~0.37 ms (clip dedupe).
- `slider_drag` @4000: ~42.8 µs/event → rząd 0.5–1 µs/event (capture; motion O(1)).
- Brak wzrostu `getRenderCacheSize()`/`getTextCacheSize()` przy powtarzanym create/destroy i zmianie tekstu dynamicznego Label po release.

### Static check

- `rg "pruneUnused|setByteBudget|getByteBudget|getBytesUsed|kRenderCachePruneThreshold|m_bytesUsed|lastUse|m_lruClock"` — poza docs/archive/CHANGELOG nie powinno być trafień.

---

## Ryzyka i uwagi implementacyjne

- **Kolejność zniszczeń**: `TextLease`/`GUIElement` release używają `TextureManager`; `GUIManager::~GUIManager` już niszczy `m_elements` w ciele przed memberami. Detached element zniszczony po `GUIManager` to istniejące UB (unregister) — nie pogarszać.
- **Release po drop**: zawsze `m_texture.reset()` przed `releaseTextCache/releaseRenderCache`; release sprawdza `weak.expired()`, więc sharing z innym holderem nie jest psuty.
- **Podwójny release**: brak wpisu = no-op; bezpieczne.
- **Cache key collisions**: bez zmian (64-bit hash); nie pogarszać.
- **TextLease i font pointer**: klucz nadal używa `font.get()`; `FontManager` nie evictuje fontów — bez zmian.
- **Direct-render text**: każdy widget rysujący tekst co klatkę musi trzymać lease; inaczej uncached `createTextureFromText` będzie rasteryzował każdą klatkę (StringGrid/ListView/MenuList/MenuPopup/MenuBar).
- **Clip**: usunięcie końcowego restore jest bezpieczne tylko jeśli każda gałąź zmieniająca clip go przywraca; dodać testy pikselowe dla overlays i kursora. Nie zmieniać `m_clip_children` default.
- **Capture**: RangeSlider zmienia zachowanie (drag poza widgetem kontynuuje) — świadoma zmiana; zaktualizować testy. Hover po dragu wraca przy następnym motion.
- **Cleanup**: oznaczony, ale odłączony element musi wyczyścić flagę/licznik w `detachElement`; `clearChildren` może zostawić licznik >0 do najbliższego cleanupu (jednorazowy pełny pass, potem reset).
- **API break**: usunięcie `pruneUnused`/budżetu i zmiana `createTextureFromText` na uncached wymagają aktualizacji docs/tests; C API nie eksponuje tych funkcji.
