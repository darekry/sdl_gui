# Aktualny stan projektu

## Stan repozytorium

- **Kod źródłowy**: [`src/`](src/) - 32 plików nagłówkowych, 27 plików implementacji
- **Dokumentacja**: [`docs/`](docs/) - API docs, przewodniki (EN/PL), code review texture/font manager
- **Przykłady**: [`examples/`](examples/) - 26 przykładów demonstrujących widgety
- **Testy**: [`tests/`](tests/) - 21 plików testowych (Catch2)

## Kluczowe cechy

- Biblioteka GUI oparta na SDL2 z cache'owaniem renderowania
- Migracja na moduły C++23 (`import std.compat;`)
- Parser JSON/XML do definicji layoutów
- Kompletny zestaw widgetów: Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu
- System motywów i stylów

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

## Test Results Summary (2026-05-17)

**All 25 tests passed after fixes:**

| Test | Assertions | Test Cases | Status |
|------|------------|------------|--------|
| test_easing | 263 | 6 | ✅ PASSED |
| test_cursor | 90 | 10 | ✅ PASSED (fixed library bug) |
| test_gui_element | 102 | 12 | ✅ PASSED |
| test_gui_manager | 107 | 14 | ✅ PASSED |
| test_button | 77 | 12 | ✅ PASSED |
| test_checkbox | 86 | 10 | ✅ PASSED |
| test_label | 60 | 8 | ✅ PASSED |
| test_panel | 75 | 12 | ✅ PASSED |
| test_slider | 55 | 9 | ✅ PASSED |
| test_text_input | 81 | 11 | ✅ PASSED |
| test_text_area | 80 | 12 | ✅ PASSED |
| test_radio_button | 88 | 8 | ✅ PASSED (fixed library bug) |
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
| test_timer_manager | 13 | 1 | ✅ PASSED |

**Library bugs fixed during testing:**
1. `cursor.cpp:78-91` - setState() required texture to exist (removed check)
2. `gui.cpp:122-137` - Clicks without prior mouse motion didn't register (added position check for button events)

**Total: 1,726 assertions across 25 test files**

## Punkty do weryfikacji

- Polityka publikacji assetów domyślnych (`assets/fonts/font.ttf`)
- Zachowanie cache (`m_cachedTexture`) przy zmianie rozmiaru elementu
- Test compilation verification - all tests should link together now