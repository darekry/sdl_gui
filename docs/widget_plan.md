# Plan implementacji widgetów Checkbox, RadioButton i klasy RadioGroup

## Cel
Dodać widgety `Checkbox` i `RadioButton` oraz klasę `RadioGroup` do biblioteki GUI opartej na SDL.

## Plan

### 1. Projektowanie klas

*   **Checkbox:**
    *   Dziedziczy po `GUIElement`.
    *   Przechowuje stan: zaznaczony/niezaznaczony (`bool isChecked()`, `void setChecked(bool)`).
    *   Posiada opcjonalną etykietę tekstową (`void setLabel(string)`).
    *   Obsługuje zdarzenia kliknięcia w celu przełączenia stanu.
    *   Posiada callback informujący o zmianie stanu (`void setOnChange(callback)`).

*   **RadioButton:**
    *   Dziedziczy po `GUIElement`.
    *   Przechowuje stan: zaznaczony/niezaznaczony (`bool isSelected()`, `void setSelected(bool)`).
    *   Posiada opcjonalną etykietę tekstową (`void setLabel(string)`).
    *   Obsługuje zdarzenia kliknięcia.
    *   Przechowuje wskaźnik lub referencję do swojej grupy (`RadioGroup* m_group`, `void setGroup(RadioGroup*)`).
    *   Kliknięcie powoduje powiadomienie grupy o zaznaczeniu (`m_group->buttonSelected(this)`).
    *   Posiada callback informujący o zaznaczeniu (`void setOnChange(callback)`).

*   **RadioGroup:**
    *   Zarządza kolekcją wskaźników do `RadioButton`ów (`std::vector<RadioButton*> m_buttons`).
    *   Posiada metodę do dodawania `RadioButton`ów do grupy (`void addRadioButton(RadioButton*)`).
    *   Posiada metodę `buttonSelected(RadioButton* selectedButton)`, która jest wywoływana przez `RadioButton` po kliknięciu. Ta metoda iteruje przez wszystkie przyciski w grupie i odznacza te, które nie są `selectedButton`.

### 2. Implementacja

*   Utworzenie plików nagłówkowych (`.hpp`) i źródłowych (`.cpp`) dla wszystkich trzech klas:
    *   `src/checkbox.hpp`
    *   `src/checkbox.cpp`
    *   `src/radio_button.hpp`
    *   `src/radio_button.cpp`
    *   `src/radio_group.hpp`
    *   `src/radio_group.cpp`
*   Implementacja metod `render` w `Checkbox` i `RadioButton` do rysowania odpowiednich kształtów (kwadrat/koło) i stanu (zaznaczenie/kropka), a także etykiety tekstowej.
*   Implementacja metody `handleEvent` w `Checkbox` do przełączania stanu po kliknięciu i wywołania callbacka.
*   Implementacja metody `handleEvent` w `RadioButton` do zmiany stanu na zaznaczony po kliknięciu, powiadomienia `RadioGroup` i wywołania callbacka.
*   Implementacja logiki w `RadioGroup::addRadioButton` do dodawania przycisków do wewnętrznej kolekcji.
*   Implementacja logiki w `RadioGroup::buttonSelected` do zarządzania stanem zaznaczenia przycisków w grupie.
*   Wykorzystanie `FontManager` i `TextureManager` do renderowania etykiet tekstowych.
*   Zapewnienie odpowiedniego zarządzania pamięcią (np. w destruktorach, jeśli używane są surowe wskaźniki lub wskaźniki współdzielone).

## Struktura klas (Mermaid)

```mermaid
classDiagram
    GUIElement <|-- Checkbox
    GUIElement <|-- RadioButton
    RadioGroup "1" -- "*" RadioButton : manages
    Checkbox : +bool isChecked()
    Checkbox : +void setChecked(bool)
    Checkbox : +void setLabel(string)
    Checkbox : +void setOnChange(callback)
    Checkbox : +render()
    Checkbox : +handleEvent()
    RadioButton : +bool isSelected()
    RadioButton : +void setSelected(bool)
    RadioButton : +void setLabel(string)
    RadioButton : +void setOnChange(callback)
    RadioButton : +setGroup(RadioGroup*)
    RadioButton : +render()
    RadioButton : +handleEvent()
    RadioGroup : +addRadioButton(RadioButton*)
    RadioGroup : +buttonSelected(RadioButton*)