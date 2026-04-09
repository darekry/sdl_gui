# Znalezione problemy i niespójności w SDL GUI

## StringGrid - Integracja z systemami biblioteki

### Priorytet 1 - Krytyczny (System styli)
- [ ] Dodać domyślny styl StringGrid w `src/theme.cpp` w metodzie `createDefaultTheme()`
- [ ] Zastąpić hardcoded kolory wartościami z `getComposedStyle(m_state)` w `src/string_grid.cpp`

### Priorytet 2 - Wysoki (Parser XML)
- [ ] Dodać `#include "string_grid.hpp"` w `src/sgml_parser.cpp`
- [ ] Dodać obsługę tagu "StringGrid" w metodzie `parseNode()` 
- [ ] Zaimplementować parsowanie atrybutów: rowCount, colCount, showRowHeaders, showColumnHeaders, editable

### Priorytet 3 - Średni (API)
- [ ] Dodać metody setter dla kolorów w `src/string_grid.hpp`:
  - `setCellBackgroundColor(SDL_Color)`
  - `setSelectionColor(SDL_Color)`
  - `setGridLineColor(SDL_Color)`
  - `setHeaderBackgroundColor(SDL_Color)`
  - `setHeaderTextColor(SDL_Color)`

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

---

## Data analizy
**Data:** 2026-02-22  
**Analizowane pliki:**
- `src/text_area.hpp`
- `src/text_area.cpp`
- `tests/test_text_area.cpp`
