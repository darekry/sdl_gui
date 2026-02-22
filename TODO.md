# Znalezione problemy i niespójności w SDL GUI

## TextArea

### 1. Brak aktualizacji `m_state` po kliknięciu
**Plik:** [`src/text_area.cpp`](src/text_area.cpp:92-108)

**Opis:** TextArea nie aktualizuje `m_state` (przez `setState()`) po kliknięciu, w przeciwieństwie do innych widgetów jak Button. TextArea używa wewnętrznej zmiennej `m_isHovered` do śledzenia stanu aktywności.

**Oczekiwane zachowanie:** Po kliknięciu w TextArea, `getState()` powinno zwracać `ElementState::Hover`, podobnie jak w innych widgetach.

**Rzeczywiste zachowanie:** `getState()` zwraca `ElementState::Normal` nawet po kliknięciu wewnątrz TextArea.

**Wpływ na testy:** Testy nie mogą używać `getState()` do weryfikacji aktywacji TextArea.

**Sugerowana naprawa:** Dodać `setState(ElementState::Hover)` w obsłudze `SDL_MOUSEBUTTONDOWN` gdy kliknięcie jest wewnątrz elementu.

```cpp
// W handleEvent(), linia ~94:
if (contains(e.button.x, e.button.y)) {
    setState(ElementState::Hover);  // DODAĆ TO
    m_isHovered = true;
    SDL_StartTextInput();
    // ...
}
```

### 2. Inicjalizacja `m_lines` dopiero w `draw()`
**Plik:** [`src/text_area.cpp`](src/text_area.cpp:173-215)

**Opis:** Wektor `m_lines` jest inicjalizowany dopiero w metodzie `recalculateLines()`, która jest wywoływana z `draw()`. Jeśli użytkownik spróbuje edytować tekst przed pierwszym renderem, może wystąpić SIGSEGV w `update_text_offset()`.

**Problem:** Metoda `update_text_offset()` (linia 274-316) iteruje po `m_lines` bez sprawdzania czy jest pusty.

**Wpływ na testy:** Testy muszą wywołać `render()` przed próbą edycji tekstu, co jest nieintuicyjne.

**Sugerowana naprawa:** 
1. Zainicjalizować `m_lines` w konstruktorze z jedną pustą linią.
2. Lub dodać sprawdzenie `m_lines.empty()` w `update_text_offset()`.

```cpp
// W konstruktorze:
TextArea::TextArea(...) {
    m_lines.push_back("");  // Zainicjalizuj z pustą linią
    // ...
}

// Lub w update_text_offset():
void TextArea::update_text_offset() {
    if (m_lines.empty()) return;  // Dodaj guard
    // ...
}
```

### 3. Brak callbacków
**Plik:** [`src/text_area.hpp`](src/text_area.hpp)

**Opis:** TextArea nie posiada callbacków dla zmian tekstu (w przeciwieństwie do TextInput, który ma `setOnTextChanged`).

**Wpływ:** Użytkownik nie może reagować na zmiany tekstu w czasie rzeczywistym.

**Sugerowana poprawa:** Dodać callback `setOnTextChanged` podobnie jak w TextInput.

---

## Uwagi do testów

### Wymagane wywołanie `render()` przed edycją
W testach TextArea, przed próbą edycji tekstu (dodawanie/usuwanie znaków), konieczne jest wywołanie:
```cpp
area->render(manager.getRenderer());
```
Dzieje się tak z powodu problemu #2 powyżej - `m_lines` jest inicjalizowany dopiero w `draw()`.

---

## Data analizy
**Data:** 2026-02-22  
**Analizowane pliki:**
- `src/text_area.hpp`
- `src/text_area.cpp`
- `tests/test_text_area.cpp`
