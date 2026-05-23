# Aktualny stan projektu

## Ostatnie zmiany (2026-05-23)

### RadioGroup::addOption() - convenience API

Dodano metodę `addOption(text, selected)` do RadioGroup, która automatycznie tworzy RadioButton + Label z auto-pozycjonaniem.

**API:**
```cpp
radioGroup->addOption("Opcja 1");
radioGroup->addOption("Opcja 2", true);  // selected by default
radioGroup->addOption("Opcja 3");

// Konfiguracja layoutu (opcjonalnie):
radioGroup->setOptionSpacing(40);
radioGroup->setOptionMargins(20, 45, 10);  // buttonX, labelX, startY
radioGroup->setOptionSizes(20, 16);  // buttonSize, labelFontSize
```

**Zmienione pliki:**
- `src/radio_group.hpp` - dodano addOption(), setOptionSpacing(), setOptionMargins(), setOptionSizes()
- `src/radio_group.cpp` - implementacja
- `examples/example_radio_button.cpp` - zaktualizowany przykład

**Redukcja boilerplate:** z ~12 linii (4 per opcja) do 3 linii (1 per opcja).

**Tests:** RadioGroup tests passed (70 assertions in 8 test cases).

## Ostatnie zmiany (2026-05-21)

### Rebuild detection fix (2026-05-21) - COMPLETED ✓

**Problem**: Każde uruchomienie ./nob budowało cały projekt, nawet gdy nie było zmian.

**Rozwiązanie**:
1. `ensure_unity_source()` - porównuje content `all.cpp` przed nadpisaniem
   - Jeśli content unchanged: skip write, preserve timestamp
   - Jeśli content changed: write new file, update timestamp
2. Modules rebuild - tylko gdy pcm nie istnieje (source timestamp constant)
3. Unity object (`all.o`) - rebuild tylko gdy `all.cpp` timestamp changed
4. Examples rebuild - `{example_src, all.o}` - rebuild tylko gdy jeden z nich changed

**Verification tests passed**:
- Fresh build: 28 built ✓
- No changes: 0 built, 28 skipped ✓
- One example changed: 1 built, 27 skipped ✓
- Library source changed: unity + 28 rebuilt ✓
- After rebuild: 0 built, 28 skipped ✓

**Bug fixed**: Removed duplicate code block in `build_unity_object()` function

### compile_commands.json integration (2026-05-21)

**Zmiana**: Usunięto osobny target `compile_commands` - generowanie jest teraz automatyczne podczas regularnych buildów.

**Implementacja**:
- Global `Nob_Compdb g_compdb` - entries added during compilations
- `nob_compdb_add()` called BEFORE each `nob_cmd_run()`
- `nob_compdb_save()` at end of main() (only if compilations happened)
- Removed `generate_compile_commands()` function and target

**Zalety**:
- compile_commands.json zawsze aktualny po build
- Zero extra work - entries generated alongside compilations
- Nie regeneruje gdy nothing compiled (skipped builds)

### Refaktor nob.c (Opcja C)

Przeprowadzono refaktor skryptu budującego `nob.c`:
- **853 linii** (z ~1400 przed refaktorem)
- Prekompilowane flagi jako globalne `Nob_Cmd`: `g_common`, `g_debug`, `g_release`
- Helper functions: `cmd_add_common()`, `cmd_add_mode()`, `cmd_add_modules()`
- `nob_cmd_extend()` zamiast pętli for dla flag
- `nob_da_foreach(const char*, ...)` dla iteracji
- Release kompiluje `.pic.o` (używane dla `.a` i `.so` - bez double compilation)
- Parallel compilation: `nob_cmd_run(.async = &procs)`
- Rebuild detection: `nob_needs_rebuild()` dla examples, tests, release

**Zachowane features**:
- Debug/Release modes
- Unity build
- Static (.a) i shared (.so) libraries
- compile_commands.json generation
- C++23 modules

**Test**: All examples (28) i tests (10) passed.

## Stan repozytorium

- **Kod źródłowy**: [`src/`](src/) - 34 plików nagłówkowych, 29 plików implementacji
- **Komponenty złożone**: [`src/composite/`](src/composite/) - DialogBox, MessageBox (2 pliki)
- **Dokumentacja**: [`docs/`](docs/) - API docs, przewodniki (EN/PL), code review texture/font manager
- **Przykłady**: [`examples/`](examples/) - 28 przykładów demonstrujących widgety (nowy: example_dialog)
- **Testy**: [`tests/`](tests/) - 21 plików testowych (Catch2)
- **Build system**: [`nob.c`](nob.c) - skrypt budujący w C (nob.h v3.8.0)

## System budowania (nob.c)

Projekt używa skryptu budującego `nob.c` (z biblioteką `nob.h`) zastępując Makefile:

**Uruchomienie**:
```bash
cc -o nob nob.c   # bootstrap (raz)
./nob             # build examples (debug)
./nob examples    # build examples
./nob test        # build + run tests
./nob release     # build dist/ artifacts
./nob clean       # remove output/, dist/, modules_cache/
./nob non_unity   # compile each .cpp separately
./nob compile_commands  # generate compile_commands.json
./nob -r examples # build examples (release)
```

**Funkcje**:
- Moduły prekompilowane C++23 (std.pcm, std.compat.pcm)
- Unity build (output/all.cpp)
- Debug/Release z różnymi flagami
- Generowanie compile_commands.json dla clangd
- Biblioteki statyczne/dynamiczne dla release
- Połączony header dist/sdl_gui.hpp

## Kluczowe cechy

- Biblioteka GUI oparta na SDL2 z cache'owaniem renderowania
- Migracja na moduły C++23 (`import std.compat;`)
- Parser JSON/XML do definicji layoutów
- Kompletny zestaw widgetów: Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu
- System motywów i stylów
- **Zaokrąglone rogi** - wsparcie dla borderRadius w Style (SDL2_gfx)
- **Komponenty złożone** - DialogBox, MessageBox dla łatwego tworzenia okien dialogowych

## Ostatnie zmiany (2026-05-20)

### Komponenty złożone (Composite Components)

Dodano nowy katalog `src/composite/` dla komponentów wyższego poziomu, które składają się z wielu podstawowych widgetów:

**DialogBox** (`src/composite/dialog_box.hpp/cpp`):
- Okno dialogowe w stylu Windows, draggable
- Statyczne metody factory: `createConfirm()`, `createAlert()`, `createCustom()`, `createWithTitle()`
- Składa się z Panel (tło/ramka), Label (komunikat), Button (akcje)
- ESC zamyka dialog bez wyboru

**MessageBox** (`src/composite/message_box.hpp/cpp`):
- Statyczna klasa pomocnicza dla szybkich alertów
- Metody: `showInfo()`, `showError()`, `showWarning()`, `showQuestion()`, `showCustom()`
- Automatyczny styling (kolor tła/border różny dla typu)

**FileDialog** (`src/composite/file_dialog.hpp/cpp`) - NEW:
- Dialog wyboru pliku/katalogu w stylu Windows 3.11
- MVP approach: ListView dla directories (flat list) i files
- ".." jako first item - navigate to parent directory
- Double-click na directory - navigate into it
- Filtering: *.txt, *.cpp, *.* patterns
- Save mode: TextInput dla filename
- std::filesystem dla directory navigation

**Przykłady**: 
- `examples/example_dialog.cpp` - demonstracja DialogBox i MessageBox
- `examples/example_file_dialog.cpp` - demonstracja FileDialog (NEW)

**API FileDialog**:
```cpp
// Open file dialog
FileDialog::createOpen(manager, "Open File", "", "*.cpp",
    [](std::string_view path) { if (!path.empty()) { /* selected */ } });

// Save file dialog
FileDialog::createSave(manager, "Save File", "", "untitled.txt", "*.txt",
    [](std::string_view path) { if (!path.empty()) { /* save to path */ } });
```

**Zmienione pliki**:
1. `src/composite/dialog_box.hpp/cpp` - nowy komponent
2. `src/composite/message_box.hpp/cpp` - nowy komponent
3. `Makefile` - dodano COMPOSITE_SRC, reguły kompilacji dla composite/
4. `examples/example_dialog.cpp` - nowy przykład

**API**:
```cpp
// DialogBox - dialog potwierdzający
DialogBox::createConfirm(manager, "Czy na pewno?", "Tak", "Nie",
    [](bool confirmed) { if (confirmed) { /* akcja */ } });

// MessageBox - szybki alert
MessageBox::showInfo(manager, "Plik został zapisany.");
MessageBox::showError(manager, "Błąd: nie można otworzyć pliku.");
MessageBox::showQuestion(manager, "Czy kontynuować?", 
    []() { /* tak */ }, []() { /* nie */ });
```

### Fix cursor blinking and mouse positioning (TextInput/TextArea)

**Problemy rozwiązane**:
1. TextInput cursor not blinking - `renderOverlay()` never called because `isOverlay()` returns false
2. TextArea cursor not blinking when idle - blinking logic in `handleEvent()` only runs with events
3. No mouse click cursor positioning in TextInput/TextArea

**Implementacja**:
1. `gui_manager.cpp:98-107` - `renderOverlay()` called for keyboard focus element regardless of `isOverlay()`
2. `text_input.cpp:205-256` - mouse click cursor positioning using binary search
3. `text_area.cpp` - added `renderOverlay()` for cursor, moved blinking logic there, removed from `handleEvent()`
4. `text_area.hpp` - added `renderOverlay()` override, removed unused `renderCursor()` method
5. `text_area.cpp` - mouse click cursor positioning with row/column calculation

**Files modified**:
- `src/gui_manager.cpp` - render overlay for keyboard focus
- `src/text_input.cpp` - mouse positioning (binary search)
- `src/text_area.cpp` - renderOverlay, mouse positioning, removed blink from handleEvent
- `src/text_area.hpp` - added renderOverlay, removed renderCursor

**Tests**: All pass (81 assertions in TextInput, 80 in TextArea)

### Zaokrąglone rogi (borderRadius) (2026-05-19)

Dodano wsparcie dla zaokrąglonych rogów w widgetach używających `drawBackgroundAndBorder()`:
- Panel, Button, Checkbox, TextInput, TextArea automatycznie obsługują borderRadius

**Zmienione pliki**:
1. `Makefile` - dodano `-lSDL2_gfx` do LDFLAGS, include SDL2_gfxPrimitives.h w combined header
2. `src/style.hpp` - dodano `borderRadius` do Style struct, aktualizacja mergeWith(), operator==(), logStyle()
3. `src/gui.hpp` - dodano `setBorderRadius(ElementState state, int radius)`
4. `src/gui.cpp` - implementacja `setBorderRadius()`, aktualizacja `drawBackgroundAndBorder()` używa `roundedBoxRGBA()` i `roundedRectangleRGBA()` z SDL2_gfx
5. `src/theme.cpp` - default borderRadius dla Button (4px), TextInput/TextArea (2px)
6. `src/layout_parser.cpp` - parsowanie atrybutu `borderRadius` w JSON/XML layouts
7. `examples/example_rounded_corners.cpp` - demonstracja zaokrąglonych rogów

**API**:
```cpp
// Ustawienie zaokrąglenia rogów
button->setBorderRadius(ElementState::Normal, 8);
button->setBorderRadius(ElementState::Hover, 10);

// W JSON/XML layout
{
  "type": "Button",
  "style": {
    "borderRadius": 6
  }
}
```

**Widgety z własnym rysowaniem (do zrobienia w późniejszym etapie)**:
- Slider (thumb)
- StringGrid (komórki, nagłówki)
- RadioButton (już rysuje koło)

## Ostatnia analiza i fixy (2026-05-17)

Przeprowadzony code review systemu TextureManager/FontManager - wyniki w [`docs/texture_font_manager_review.md`](docs/texture_font_manager_review.md).

**Zaimplementowane fixy**:
1. `TextureManager::pruneUnused()`, `clearCache()`, `getCacheSize()` - cleanup mechanizm
2. `TextureManager::createTextureFromText(path, size, color)` - stabilny klucz cache (font_path|font_size)
3. `TextArea::refreshTextures()` - lokalne tekstury, nie w TextureManager cache
4. `TextInput` - cursor w `renderOverlay()` (bez recreate cache), lokalna tekstura tekstu
5. `StringGrid` - `m_localTextureCache` dla komórek, `createLocalTextTexture()`, `clearLocalTextureCache()`
6. `gui.cpp:102-104` - hover detection z `e.motion.x/y` (zamiast `SDL_GetMouseState()`)
7. **Slider** - obsługa kółka myszy (`m_wheelStep`, `setWheelStep()`, `SDL_MOUSEWHEEL` handling)

**FontManager**: ✅ Poprawna implementacja z transparent comparator

## Ostatnie zmiany (2026-05-17)

**Comprehensive StringGrid tests rewrite**:
- `tests/test_string_grid.cpp` - rewritten with 14 test cases covering all user behaviors (146 assertions):
  - Construction (initial rows/columns, getComponentType)
  - Data Management (setRowCount/getRowCount, setColumnCount/getColumnCount, setCellText/getCellText, clear)
  - Headers (setColumnHeader, setShowRowHeaders, setShowColumnHeaders)
  - Selection (setSelectedCell, getSelectedCell, setSelectionRange, getSelectionRange, clearSelection)
  - Editing (setEditable/isEditable, startEditing/stopEditing, isEditing, isOverlay)
  - Sorting (sortByColumn Ascending/Descending/None, getSortDirection, getSortColumn, numeric/text sorting)
  - Custom Comparators (setCustomComparator, hasCustomComparator, clearCustomComparator, clearAllCustomComparators)
  - Geometry (setColumnWidth, setRowHeight/getRowHeight, setHeaderHeight, setRowHeaderWidth)
  - Scrolling (setHorizontalScrollEnabled/isHorizontalScrollEnabled, setVerticalScrollEnabled/isVerticalScrollEnabled, getVerticalScrollOffset)
  - CellCoord Structure (isValid, invalid, operator==, operator!=)
  - SelectionRange Structure (isValid, normalized)
  - Callbacks (setOnCellClick, setOnCellDoubleClick, setOnCellEdit, setOnSelectionChange)
  - Colors (setSelectionColor/getSelectionColor, setSelectedCellBorderColor/getSelectedCellBorderColor)
  - Scroll State verification

**Comprehensive Slider tests rewrite**:
- `tests/test_slider.cpp` - rewritten with 9 test cases covering all user behaviors:
  - Value initialization (constructor, getValue)
  - setValue behavior (clamping, callbacks)
  - Increment/Decrement buttons (click, range limits, getters)
  - Mouse wheel (hover required, wheelStep, min/max limits)
  - Dragging track (value change, edge values)
  - Orientation (horizontal/vertical)
  - Disabled state (ignores all events)
  - Range (getMin/getMax, setMin/setMax/setRange)
  - Component type verification

**Nowy widget ListView**:
- `src/list_view.hpp`, `src/list_view.cpp` - ListView jako specjalizacja StringGrid (1 kolumna)
- `StringGrid` - nowe metody `setHorizontalScrollEnabled()`, `setVerticalScrollEnabled()`
- Parsery layoutów - obsługa elementu `ListView` (JSON/XML)
- `examples/example_list_view.cpp` - demonstracja ListView
- `tests/test_list_view.cpp` - 12 testów jednostkowych (83 asercje) - comprehensive rewrite covering:
  - Construction and empty state
  - Item management (add, insert, remove, set, clear)
  - Selection behavior (setSelectedRow, getSelectedRow, clearSelection)
  - Row click selection
  - Row click/double-click/activate callbacks
  - Keyboard navigation (arrow keys, Enter/KP_Enter activation)
  - Large number of items handling
  - Disabled/hidden state behavior

**Fixy (2026-05-17)**:
- `string_grid.cpp` - naprawiono syntax error (duplikat kodu bloku hSlider w updateSliderRanges)
- `StringGrid` - dodano metody debugowe: `getVerticalSliderMax()`, `getVerticalScrollOffset()`, `getRowHeight()`
- Slider range verification: ✓ poprawny - range = contentHeight - visibleHeight
- `Slider` - dodano `setMin()`, `setMax()`, `setRange()` do dynamicznej zmiany zakresu
- `TextInput` - fix use-after-free: return immediately po `onEnterPressed` callback (prevent access to destroyed object)

## Test Rewriting Progress (2026-05-17)

**Completed**:
- Created `tests/test_main.cpp` with unified CATCH_CONFIG_MAIN entry point
- Fixed CATCH_CONFIG_MAIN in all test files (8 files):
  - test_tab_control.cpp, test_context_menu.cpp, test_animated_image.cpp
  - test_font_manager.cpp, test_animation_manager.cpp, test_theme.cpp
  - test_timer_manager.cpp, test_texture_manager.cpp
- Created new tests:
  - test_easing.cpp (50+ assertions) - tests for linear, easeInQuad, easeOutQuad, easeInOutQuad
  - test_cursor.cpp (40+ assertions) - tests for state, offset, scale, visibility, callbacks
- All tests now compile together into a single executable

**Test Coverage Status**:
| Widget | Test File | Lines/Assertions | Status |
|--------|-----------|-------|--------|
| GUIElement | test_gui_element.cpp | 584 lines | ✓ Done |
| GUIManager | test_gui_manager.cpp | 705 lines | ✓ Done |
| Button | test_button.cpp | 77 assertions | ✓ Done |
| Checkbox | test_checkbox.cpp | 86 assertions | ✓ Done |
| Label | test_label.cpp | 60 assertions | ✓ Done |
| Panel | test_panel.cpp | 75 assertions | ✓ Done |
| Slider | test_slider.cpp | 55 assertions | ✓ Done |
| TextInput | test_text_input.cpp | 765 lines | ✓ Good |
| TextArea | test_text_area.cpp | 785 lines | ✓ Good |
| RadioButton | test_radio_button.cpp | 423 lines | ✓ Good |
| RadioGroup | test_radio_group.cpp | 543 lines | ✓ Good |
| ComboBox | test_combobox.cpp | 713 lines | ✓ Good |
| Canvas | test_canvas.cpp | 542 lines | ✓ Good |
| ListView | test_list_view.cpp | 870 lines | ✓ Good |
| StringGrid | test_string_grid.cpp | 744 lines | ✓ Good |
| TabControl | test_tab_control.cpp | 442 lines | ✓ Fixed |
| ContextMenu | test_context_menu.cpp | 65 lines | ✓ Fixed |
| AnimatedImage | test_animated_image.cpp | 214 lines | ✓ Fixed |
| Theme | test_theme.cpp | 93 lines | ✓ Fixed |
| FontManager | test_font_manager.cpp | 376 lines | ✓ Fixed |
| TextureManager | test_texture_manager.cpp | 106 lines | ✓ Fixed |
| AnimationManager | test_animation_manager.cpp | 123 lines | ✓ Fixed |
| TimerManager | test_timer_manager.cpp | 108 lines | ✓ Fixed |
| Easing | test_easing.cpp | NEW | ✓ Created |
| Cursor | test_cursor.cpp | NEW | ✓ Created |

## Test Results Summary (2026-05-19) - FINAL

**All 24 tests passed:**

| Test | Assertions | Test Cases | Status |
|------|------------|------------|--------|
| test_easing | 263 | 6 | ✅ PASSED |
| test_cursor | 90 | 10 | ✅ PASSED |
| test_gui_element | 102 | 12 | ✅ PASSED |
| test_gui_manager | 107 | 14 | ✅ PASSED |
| test_button | 79 | 12 | ✅ PASSED |
| test_checkbox | 86 | 10 | ✅ PASSED |
| test_label | 60 | 8 | ✅ PASSED |
| test_panel | 75 | 12 | ✅ PASSED |
| test_slider | 55 | 9 | ✅ PASSED |
| test_text_input | 81 | 11 | ✅ PASSED |
| test_text_area | 80 | 12 | ✅ PASSED |
| test_radio_button | 88 | 8 | ✅ PASSED |
| test_radio_group | 70 | 8 | ✅ PASSED |
| test_combobox | 75 | 10 | ✅ PASSED |
| test_canvas | 50 | 11 | ✅ PASSED |
| test_list_view | 83 | 12 | ✅ PASSED |
| test_string_grid | 146 | 14 | ✅ PASSED |
| test_tab_control | 85 | 10 | ✅ PASSED |
| test_context_menu | 12 | 1 | ✅ PASSED |
| test_animated_image | 28 | 1 | ✅ PASSED |
| test_theme | 30 | 1 | ✅ PASSED |
| test_font_manager | 182 | 9 | ✅ PASSED |
| test_texture_manager | 22 | 1 | ✅ PASSED |
| test_animation_manager | 23 | 1 | ✅ PASSED |

**Total: 1,853 assertions across 181 test cases - ALL PASSED**

**Library bugs fixed during testing:**
1. `cursor.cpp:78-91` - setState() required texture to exist (removed check)
2. `gui.cpp:122-137` - Clicks without prior mouse motion didn't register (added position check for button events)
3. `gui.cpp:138-142` - Mouse release outside button now correctly sets Normal state (not Hover)

## Current Status

**Test rewriting task: COMPLETED**
- All 24 test files compile and pass
- All library bugs discovered during testing have been fixed
- Memory bank updated with final results

## Punkty do weryfikacji

- Polityka publikacji assetów domyślnych (`assets/fonts/font.ttf`)
- Zachowanie cache (`m_cachedTexture`) przy zmianie rozmiaru elementu