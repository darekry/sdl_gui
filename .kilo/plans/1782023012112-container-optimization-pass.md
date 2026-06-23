# Container & Data Structure Optimization Pass

## Goal
Replace suboptimal container choices with more efficient ones. Reduce allocations, eliminate O(log n) tree traversals in hot paths, and avoid unnecessary copies.

## Excluded
- **H1 (WindowManager O(n))**: Pominięte — okien maksymalnie 3-4, liniowe skanowanie jest OK.

---

## Phase A: Theme — `map<string, map<ElementState, Style>>` → `unordered_map<string, array<optional<Style>, 4>>`

**Problem:** `Theme::m_typeStyles` (theme.hpp:39) używa `std::map<string, map<ElementState, Style>>`. Każde `getComposedStyle()` (tysiące razy na klatkę) wykonuje O(log n) tree traversal z porównaniami stringów, plus wewnętrzny map dla 4 stanów.

**Fix:**
- `theme.hpp:39`: `std::map<std::string, std::map<ElementState, Style>, ThemeTypeCompare>` → `std::unordered_map<std::string, std::array<std::optional<Style>, 4>, StringHash, std::equal_to<>>`
- `theme.hpp:29`: usuń `ThemeTypeCompare` (już niepotrzebny z `unordered_map`)
- `theme.cpp`: dostosuj `getStyle()`, `setStyle()`, `createDefaultTheme()`

**Files:** `src/theme.hpp`, `src/theme.cpp`

---

## Phase B: `StringGrid::m_localTextureCache` — `map` → `unordered_map`

**Problem:** `string_grid.hpp:264` — `std::map<std::string, SharedTexture>` dla cache tekstur per-komórka. 100 widocznych komórek = 100 O(log n) lookupów na klatkę. `TextureManager` już używa `unordered_map`.

**Fix:**
- `string_grid.hpp:264`: `std::map<std::string, SharedTexture>` → `std::unordered_map<std::string, SharedTexture, StringHash, std::equal_to<>>`
- Dodaj include dla StringHash z `texture_manager.hpp` (lub przenieś do wspólnego miejsca)

**Files:** `src/string_grid.hpp`

---

## Phase C: ~~GUIManager::addElement() — redundantne O(n)~~ REVERTED

**Problem:** Pierwotna zmiana na `m_liveElements.contains(raw_ptr)` dawała false-positive — konstruktor `GUIElement` już woła `registerElement()` (gui.cpp:174), więc element jest w `m_liveElements` przed `addElement()`. Oryginalna pętla skanująca `m_elements` jest poprawna, bo sprawdza tylko top-level elementy dodane przez `addElement`, nie wszystkie żywe elementy.

**Decyzja:** Pozostawić bez zmian.

---

## Phase D: `ListView::insertItem/removeItem` — zbędna kopia `std::string`

**Problem:** `list_view.cpp:47,60` — `std::string(getCellText(...))` gdzie `getCellText` zwraca `string_view`, a `setCellText` przyjmuje `string_view`.

**Fix:** Usuń konstruktory `std::string(...)`.

**Files:** `src/list_view.cpp`

---

## Phase E: `TextArea::refreshTextures()` — brak `reserve()`

**Problem:** `text_area.cpp:808` — `m_line_textures` czyszczone i odbudowane bez `reserve()`. Rozmiar = `m_lines.size()`, który jest znany.

**Fix:** Dodaj `m_line_textures.reserve(m_lines.size());` po `clear()`.

**Files:** `src/text_area.cpp`

---

## Phase F: `StringGrid::drawCells()` — `loadFont()` wewnątrz pętli po komórkach

**Problem:** `string_grid.cpp:991` — każda widoczna komórka wywołuje `loadFont()`, który robi map lookup. Font jest ten sam dla wszystkich.

**Fix:** Font już jest dostępny — wyciągnij go do `drawDirect()` i przekaż jako parametr do `drawCells()` i helperów.

**Files:** `src/string_grid.cpp`, `src/string_grid.hpp`

---

## Phase G: `drawRoundedRectBorder()` — brak `verts.reserve()`

**Problem:** `gui.cpp:150` — `vector<SDL_Vertex>` tworzony wewnątrz pętli grubości bordera. Rozmiar znany: 4 × kCornerSegments × 6 = 192.

**Fix:** Dodaj `verts.reserve(192);` przed pętlą po cornerach.

**Files:** `src/gui.cpp`

---

## Phase H: ~~EditorElement::styles — map<ElementState, Style> → array<optional<Style>, 4>~~ POMINIĘTE

**Decyzja:** Zbyt duży refactor iteracji w 4+ plikach edytora przy minimalnym zysku (edytor nie jest hot path).

---

## Phase I: Drobne poprawki edytora i rzadkich ścieżek

- `editor_element.hpp:15`: `map<string, string> properties` → `unordered_map`
- `editor_window.hpp:82,95-98`: 5× `map<string, T*>` → `unordered_map`
- `cursor.hpp:70`: `map<CursorState, CursorData>` (9 wartości) → `array<optional<CursorData>, 9>`
- `timer_manager.cpp:30`: dodaj `timers_to_execute.reserve(timers.size())`
- `preview_window.hpp:36`: `map<size_t, GUIElement*>` → `vector<GUIElement*>` (indeksy ciągłe)

---

## Phase J: Edytor — indeksy dla szybszego lookupu

- `editor_state.cpp:177`: `findElementById()` O(n) → dodaj `unordered_map<string, size_t> m_idToIndex`
- `editor_state.cpp:163`: `getElementsByParent()` O(n×m) → dodaj `unordered_map<string, vector<size_t>> m_parentToChildren`

Obie mapy aktualizowane przy add/delete/clear elementów.

**Files:** `src/editor/editor_state.hpp`, `src/editor/editor_state.cpp`

---

## Validation

```bash
./nob examples    # must compile all 39 examples
./nob test        # must pass 28/29 tests (1 pre-existing: combobox heap-use-after-free)
```

## Implementation Order

A → B → C → D → E → F → G → H → I → J

Rationale: Zacznij od największego wpływu na wydajność (Theme, StringGrid cache), potem eliminacja alokacji i kopii, na końcu edytor.
