# Znalezione problemy i niespójności w SDL GUI

## StringGrid - Integracja z systemami biblioteki

### Priorytet 1 - Krytyczny (System styli) - ✅ UKOŃCZONE
- [x] Dodać domyślny styl StringGrid w `src/theme.cpp` w metodzie `createDefaultTheme()`
- [x] Zastąpić hardcoded kolory wartościami z `getComposedStyle(m_state)` w `src/string_grid.cpp`

### Priorytet 2 - Wysoki (Parser XML) - ✅ UKOŃCZONE
- [x] Dodać `#include "string_grid.hpp"` w parserze (znajduje się w `src/layout_parser.cpp:13`)
- [x] Dodać obsługę tagu "StringGrid" w metodzie `parseNode()` (w `layout_parser.cpp:90-97`)
- [x] Zaimplementować parsowanie atrybutów: rowCount, colCount, showRowHeaders, showColumnHeaders, editable

### Priorytet 3 - Średni (API) - ⚠️ CZĘŚCIOWO UKOŃCZONE
- [x] `setSelectionColor(SDL_Color)` - istnieje w `src/string_grid.hpp:113`
- [ ] `setCellBackgroundColor(SDL_Color)` - brak
- [ ] `setGridLineColor(SDL_Color)` - brak
- [ ] `setHeaderBackgroundColor(SDL_Color)` - brak
- [ ] `setHeaderTextColor(SDL_Color)` - brak

---

## Błędy krytyczne i安全问题 (Security/Safety Issues)

### Priorytet 1 - Krytyczne - ✅ NAPRAWIONE

#### 1. TimerManager - Use-After-Free - ✅ NAPRAWIONE
**Plik:** [`src/timer_manager.cpp`](src/timer_manager.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** Zmieniono logikę `update()` - teraz zbiera indeksy timerów do wykonania przed iteracją, wykonuje callbacki, a następnie usuwa single-shot timery i aktualizuje repeating timery. Eliminuje iterator invalidation.

#### 2. AnimationManager - Dangling Pointer - ⚠️ DOKUMENTOWANE (wymaga zmiany API)
**Plik:** [`src/animation_manager.hpp`](src/animation_manager.hpp)

**Status:** Dokumentowano 2026-04-09

**Opis:** `createAnimation` przechowuje surowe wskaźniki do zmiennych zewnętrznych (`target_property`). To jest celowy design dla simplicity/performance, ale wymaga od użytkownika zapewnienia, że target variables pozostaną valid podczas animacji.

**Działanie:** Dodano warning w dokumentacji klasy. Pełna naprawa wymaga zmiany API na `std::shared_ptr` lub `std::weak_ptr` - to jest significant API change.

#### 3. Cursor Animation Callback - Dangling Pointer - ✅ NAPRAWIONE
**Plik:** [`src/cursor.cpp`](src/cursor.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** W destruktorze `Cursor` anulowane są wszystkie aktywne animacje dla każdego stanu cursora, zapobiegając wiszącym callbackom.

---

### Priorytet 2 - Wysokie - ✅ NAPRAWIONE

#### 4. Canvas - Ręczne zarządzanie teksturą - ✅ NAPRAWIONE
**Plik:** [`src/canvas.hpp`](src/canvas.hpp), [`src/canvas.cpp`](src/canvas.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** `m_canvasTex` zmieniono z `SDL_Texture*` na `std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>`. Automatyczne cleanup w destruktorze, spójne z resztą kodu.

#### 5. Brak sprawdzenia wartości zwracanej przez funkcje SDL/TTF - ✅ NAPRAWIONE
**Pliki:**
- `src/gui.cpp:196` - `SDL_QueryTexture` - dodano sprawdzenie
- `src/label.cpp:18` - `TTF_SizeText` - dodano sprawdzenie
- `src/text_input.cpp:73,87,131` - `TTF_SizeText` - dodano sprawdzenie
- `src/text_input.cpp:112` - `SDL_RenderSetClipRect` - dodano sprawdzenie

**Status:** Naprawiono 2026-04-09

**Naprawa:** Dodano sprawdzenia return values z logowaniem błędów przez `LOG_DEBUG` lub `SDL_LogError`.

#### 6. Brak bezpieczeństwa wątkowego - ✅ DOKUMENTOWANE
**Pliki:**
- `src/timer_manager.hpp`
- `src/animation_manager.hpp`
- `src/texture_manager.hpp`
- `src/font_manager.hpp`

**Status:** Dokumentowano 2026-04-09

**Naprawa:** Dodano `@warning` komentarze w dokumentacji każdej klasy menedżera, jasno określające, że klasa nie jest thread-safe i musi być używana z jednego wątku.

#### 7. TextureManager/FontManager - Inicjalizacja SDL - ✅ NAPRAWIONE
**Pliki:** [`src/texture_manager.cpp`](src/texture_manager.cpp), [`src/font_manager.cpp`](src/font_manager.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:**
- Dodano flagę `m_initialized` i metodę `isInitialized()` w obu menedżerach
- Zmieniono logowanie na `SDL_LogError` z prefixem "CRITICAL"
- Obiekt może działać w ograniczonym trybie jeśli inicjalizacja nie powiodła się

---

### Priorytet 3 - Średnie - ✅ NAPRAWIONE

#### 8. StringGrid Sorting - Undefined Behavior - ✅ NAPRAWIONE
**Plik:** [`src/string_grid.cpp`](src/string_grid.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** Komparator teraz zapewnia strict weak ordering - wiersze bez danej kolumny są porządkowane według rozmiaru, zapewniając spójną kolejność.

#### 9. Panel Drag State Not Reset - ✅ NAPRAWIONE
**Plik:** [`src/panel.cpp`](src/panel.cpp), [`src/panel.hpp`](src/panel.hpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** Dodano override `onMouseCaptureLost()` który resetuje `m_is_dragging = false`.

#### 10. Slider Division by Zero - ✅ NAPRAWIONE
**Plik:** [`src/slider.cpp`](src/slider.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** Dodano guard `if (m_trackSize <= 0) return;` w `updateValueFromMouse()`.

#### 11. Tooltip Timer Not Stopped on Element Destruction - ✅ NAPRAWIONE
**Plik:** [`src/gui.hpp`](src/gui.hpp), [`src/gui.cpp`](src/gui.cpp)

**Status:** Naprawiono 2026-04-09

**Naprawa:** Dodano destruktor `GUIElement::~GUIElement()` który zatrzymuje tooltip timer jeśli działa.

---

## Niespójności w kodzie

### API Naming Inconsistencies (DO ZROBIENIA - niski priorytet)
| Klasa | Metoda | Problem |
|-------|--------|---------|
| Button | `setOnClickCallback` | vs |
| Checkbox | `setOnChange` | vs |
| TextInput | `setOnTextChanged` | vs |
| TextArea | `setOnTextChanged` | vs |
| ComboBox | `on_selection_changed` (public member!) | brak metody setter |

**Rozwiązanie:** Ujednolicić nazewnictwo do wzorca `setOn<Event>Callback`.

### Duplicate Include - ✅ NAPRAWIONE
**Plik:** `src/text_input.cpp`

**Status:** Naprawiono 2026-04-09

**Naprawa:** Usunięto duplikat `#include "theme.hpp"`

### Redundant Style Assignment - ✅ NAPRAWIONE
**Plik:** `src/theme.cpp`

**Status:** Naprawiono 2026-04-09

**Naprawa:** Usunięto nadpisujące przypisanie `theme.setStyle("TextInput", textAreaStyle)` - TextInput zachowuje swój własny styl.

### Dead Code - ✅ NAPRAWIONE
**Status:** Naprawiono 2026-04-09

**Naprawa:**
- `src/combobox.cpp` - usunięto pustą metodę `updateMainButtonText()`
- `src/animated_image.cpp` - usunięto nieużywaną metodę `startFrameAnimation()`

**Uwaga:** `OnMouseOverCallback` w Button jest dokumentowanym API - to jest bug (callback nie jest wywoływany), nie dead code.

---

## Możliwości refaktoryzacji (DO ZROBIENIA - niski priorytet)

### Priorytet Wysoki

#### 1. Magic Numbers
Utworzyć `src/constants.hpp` z wartościami:
- Domyślne rozmiary fontów: 16, 24
- Padding: 5, 10
- Cursor blink interval: 500ms
- Tooltip delay: 500ms
- Min column width/row height w StringGrid: 20, 16

#### 2. Code Duplication - Font Loading
Wzorzec powtarza się w: `label.cpp`, `text_input.cpp`, `text_area.cpp`, `combobox.cpp`, `string_grid.cpp`

**Rozwiązanie:** Dodać metodę pomocniczą w `GUIElement`:
```cpp
SharedFont loadDefaultFont(int size = 16) const;
```

#### 3. Code Duplication - Text Rendering
Powtarzający się wzorzec tworzenia tekstur z tekstem.

**Rozwiązanie:** Utworzyć funkcję pomocniczą:
```cpp
void renderTextAt(SDL_Renderer* renderer, std::string_view text, int x, int y, 
                  const SharedFont& font, SDL_Color color, TextAlign align = TextAlign::Left);
```

#### 4. Code Duplication - Hover State Handling
Identyczna logika obsługi stanu hover w: `button.cpp`, `checkbox.cpp`, `panel.cpp`

**Rozwiązanie:** Wyekstrahować metodę `updateHoverState(const SDL_Event& e)` do `GUIElement`.

### Priorytet Średni

#### 5. Użyć Modern C++23
- `std::expected` dla obsługi błędów w `FontManager`, `TextureManager`
- `std::format` zamiast konkatenacji stringów
- `[[nodiscard]]` na getterach
- `std::ranges` algorytmy zamiast iteratorów

#### 6. Performance - Cache Composed Style
`getComposedStyle()` tworzy nowe obiekty Style i wykonuje lookup w mapie przy każdym wywołaniu.

**Rozwiązanie:** Cache'ować composed style per state z flagą `m_styleCacheDirty`.

#### 7. Performance - Texture Cache Key
Klucz cache tekstur jest generowany przez konkatenację stringów - operacja kosztowna.

**Rozwiązanie:** Użyć struktury z hashem jako klucza.

### Priorytet Niski

#### 8. Abstrakcje do rozważenia
- `ClickableWidget` - wspólna baza dla Button, Checkbox, RadioButton
- `TextEditingMixin` - wspólna logika dla TextInput, TextArea
- `SpriteSheetRenderer` - wspólna logika dla AnimatedImage, Cursor

#### 9. File Organization
- `src/string_grid.cpp` (1190 linii) - rozważyć podział na: `string_grid_data.cpp`, `string_grid_render.cpp`, `string_grid_events.cpp`
- Utworzyć `src/types.hpp` dla wspólnych typów

---

## TextArea

### 1. Brak aktualizacji `m_state` po kliknięciu - ✅ NAPRAWIONE
**Plik:** [`src/text_area.cpp`](src/text_area.cpp)

**Status:** Naprawiono 2026-02-22

**Naprawa:** Dodano `setState(ElementState::Hover)` w obsłudze `SDL_MOUSEBUTTONDOWN` gdy kliknięcie jest wewnątrz elementu oraz `setState(ElementState::Normal)` przy utracie fokusu.

### 2. Inicjalizacja `m_lines` dopiero w `draw()` - ✅ NAPRAWIONE
**Plik:** [`src/text_area.cpp`](src/text_area.cpp)

**Status:** Naprawiono 2026-02-22

**Naprawa:** Zainicjalizowano `m_lines.push_back("")` w konstruktorze TextArea, co eliminuje potrzebę wywoływania `render()` przed edycją tekstu.

### 3. Brak callbacków - ✅ NAPRAWIONE
**Plik:** [`src/text_area.hpp`](src/text_area.hpp)

**Status:** Naprawiono 2026-02-22

**Naprawa:** Dodano metodę `setOnTextChanged(const std::function<void(TextArea*)>& callback)` oraz wywołanie callbacka przy zmianach tekstu.

---

## Uwagi do testów

### ~~Wymagane wywołanie `render()` przed edycją~~ - NIEAKTUALNE
~~W testach TextArea, przed próbą edycji tekstu (dodawanie/usuwanie znaków), konieczne jest wywołanie:~~
```cpp
// area->render(manager.getRenderer());  // Już niepotrzebne po naprawie #2
```
Ten problem został rozwiązany przez naprawę #2 - `m_lines` jest teraz inicjalizowany w konstruktorze.

---

## Historia napraw

### Naprawy z 2026-02-22

#### 1. Brak aktualizacji `m_state` po kliknięciu - NAPRAWIONE
Dodano `setState(ElementState::Hover)` w obsłudze kliknięcia oraz `setState(ElementState::Normal)` przy utracie fokusu.

**Zmienione pliki:**
- [`src/text_area.cpp`](src/text_area.cpp) - dodano wywołania `setState()` w odpowiednich miejscach obsługi zdarzeń

#### 2. Inicjalizacja `m_lines` dopiero w `draw()` - NAPRAWIONE
Zainicjalizowano `m_lines.push_back("")` w konstruktorze TextArea.

**Zmienione pliki:**
- [`src/text_area.cpp`](src/text_area.cpp) - dodano inicjalizację `m_lines` w konstruktorze

#### 3. Brak callbacków - NAPRAWIONE
Dodano metodę `setOnTextChanged(const std::function<void(TextArea*)>& callback)` oraz wywołanie callbacka przy zmianach tekstu.

**Zmienione pliki:**
- [`src/text_area.hpp`](src/text_area.hpp) - dodano deklarację metody `setOnTextChanged()` i zmiennej składowej `m_onTextChanged`
- [`src/text_area.cpp`](src/text_area.cpp) - dodano implementację metody i wywołanie callbacka

### Naprawy z 2026-04-09

#### 4. TimerManager - Use-After-Free - NAPRAWIONE
Zmieniono logikę `update()` - zbieranie timerów do wykonania, wykonanie callbacków, usuwanie single-shot, aktualizacja repeating.

**Zmienione pliki:**
- [`src/timer_manager.cpp`](src/timer_manager.cpp) - przepisana metoda `update()`

#### 5. Cursor Animation Callback - Dangling Pointer - NAPRAWIONE
Dodano anulowanie animacji w destruktorze Cursor.

**Zmienione pliki:**
- [`src/cursor.cpp`](src/cursor.cpp) - zmieniony destruktor

#### 6. Canvas - Ręczne zarządzanie teksturą - NAPRAWIONE
Zmieniono `m_canvasTex` na `std::unique_ptr` z deleterem.

**Zmienione pliki:**
- [`src/canvas.hpp`](src/canvas.hpp) - zmieniony typ membera
- [`src/canvas.cpp`](src/canvas.cpp) - przepisane metody używające tekstury

#### 7. SDL/TTF Return Value Checks - NAPRAWIONE
Dodano sprawdzenia return values dla SDL_QueryTexture, TTF_SizeText, SDL_RenderSetClipRect.

**Zmienione pliki:**
- [`src/gui.cpp`](src/gui.cpp)
- [`src/label.cpp`](src/label.cpp)
- [`src/text_input.cpp`](src/text_input.cpp)

#### 8. Thread Safety Documentation - DOKUMENTOWANE
Dodano `@warning` w dokumentacji TimerManager, AnimationManager, TextureManager, FontManager.

**Zmienione pliki:**
- [`src/timer_manager.hpp`](src/timer_manager.hpp)
- [`src/animation_manager.hpp`](src/animation_manager.hpp)
- [`src/texture_manager.hpp`](src/texture_manager.hpp)
- [`src/font_manager.hpp`](src/font_manager.hpp)

#### 9. SDL Initialization Failures - NAPRAWIONE
Dodano flagi `m_initialized` i metody `isInitialized()`, zmieniono logowanie na SDL_LogError.

**Zmienione pliki:**
- [`src/texture_manager.hpp`](src/texture_manager.hpp), [`src/texture_manager.cpp`](src/texture_manager.cpp)
- [`src/font_manager.hpp`](src/font_manager.hpp), [`src/font_manager.cpp`](src/font_manager.cpp)

#### 10. Duplicate Include - NAPRAWIONE
Usunięto duplikat `#include "theme.hpp"` w text_input.cpp.

**Zmienione pliki:**
- [`src/text_input.cpp`](src/text_input.cpp)

#### 11. StringGrid Sorting UB - NAPRAWIONE
Komparator zapewnia teraz strict weak ordering dla wierszy o różnych rozmiarach.

**Zmienione pliki:**
- [`src/string_grid.cpp`](src/string_grid.cpp)

#### 12. Panel Drag State Reset - NAPRAWIONE
Dodano `onMouseCaptureLost()` override resetujący `m_is_dragging`.

**Zmienione pliki:**
- [`src/panel.hpp`](src/panel.hpp)
- [`src/panel.cpp`](src/panel.cpp)

#### 13. Slider Division by Zero - NAPRAWIONE
Dodano guard dla `m_trackSize <= 0`.

**Zmienione pliki:**
- [`src/slider.cpp`](src/slider.cpp)

#### 14. Tooltip Timer Cleanup - NAPRAWIONE
Dodano destruktor GUIElement zatrzymujący tooltip timer.

**Zmienione pliki:**
- [`src/gui.hpp`](src/gui.hpp)
- [`src/gui.cpp`](src/gui.cpp)

#### 15. Redundant Style Assignment - NAPRAWIONE
Usunięto nadpisujące przypisanie TextInput style.

**Zmienione pliki:**
- [`src/theme.cpp`](src/theme.cpp)

#### 16. Dead Code Removal - NAPRAWIONE
Usunięto nieużywane metody: `updateMainButtonText()`, `startFrameAnimation()`.

**Zmienione pliki:**
- [`src/combobox.hpp`](src/combobox.hpp), [`src/combobox.cpp`](src/combobox.cpp)
- [`src/animated_image.hpp`](src/animated_image.hpp), [`src/animated_image.cpp`](src/animated_image.cpp)

---

## Data analizy
**Ostatnia aktualizacja:** 2026-04-09

**Analizowane pliki:**
- `src/text_area.hpp`, `src/text_area.cpp`
- `src/string_grid.hpp`, `src/string_grid.cpp`
- `src/theme.cpp`, `src/layout_parser.cpp`
- `src/timer_manager.hpp`, `src/timer_manager.cpp`
- `src/animation_manager.hpp`
- `src/cursor.hpp`, `src/cursor.cpp`
- `src/canvas.hpp`, `src/canvas.cpp`
- `src/gui.hpp`, `src/gui.cpp`, `src/panel.hpp`, `src/panel.cpp`, `src/slider.cpp`
- `src/label.cpp`, `src/text_input.cpp`
- `src/button.hpp`, `src/combobox.hpp`, `src/combobox.cpp`, `src/animated_image.hpp`, `src/animated_image.cpp`
- `src/texture_manager.hpp`, `src/texture_manager.cpp`
- `src/font_manager.hpp`, `src/font_manager.cpp`