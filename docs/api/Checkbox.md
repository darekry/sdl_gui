# Checkbox API

## Przeznaczenie komponentu

Klasa `Checkbox` reprezentuje pole wyboru (checkbox) w interfejsie użytkownika. Dziedziczy po `GUIElement` i obsługuje zdarzenia myszy oraz klawiatury. Wyświetla wizualny znacznik zaznaczenia (✓). Checkbox automatycznie rejestruje się jako element focusowalny (`canGetKeyboardFocus = true`).

### Obsługa klawiatury

Checkbox można przełączać za pomocą klawiatury:
- **Spacja** — naciśnięcie zmienia stan wizualny na `Pressed`, puszczenie przełącza zaznaczenie i przywraca stan `Hover`
- **Tab** / **Shift+Tab** — przełącza fokus pomiędzy elementami focusowalnymi
- Aktywny fokus jest wizualnie sygnalizowany niebieską obwódką wokół checkboxa

## Publiczne metody

*   ### `Checkbox(GUIManager& manager, int x, int y, int w, int h)`
    *   **Opis**: Konstruktor klasy `Checkbox`. Tworzy nowy checkbox o określonej pozycji i rozmiarze. Automatycznie włącza możliwość uzyskania fokusu klawiatury.
    *   **Parametry**:
        *   `manager`: Referencja do `GUIManager` zarządzającego elementem.
        *   `x`: Pozycja X lewego górnego rogu.
        *   `y`: Pozycja Y lewego górnego rogu.
        *   `w`: Szerokość checkboxa.
        *   `h`: Wysokość checkboxa.
    *   **Przykład użycia**:
        ```cpp
        Checkbox myCheckbox(guiManager, 50, 50, 24, 24);
        ```

*   ### `~Checkbox() = default`
    *   **Opis**: Domyślny destruktor.

*   ### `bool isChecked() const`
    *   **Opis**: Sprawdza, czy checkbox jest zaznaczony.
    *   **Zwraca**: `true` jeśli zaznaczony; `false` w przeciwnym razie.

*   ### `void setChecked(bool checked)`
    *   **Opis**: Ustawia stan zaznaczenia checkboxa. Jeśli wartość się zmieni, wywołuje callback `onChange` i odświeża wygląd.
    *   **Parametry**:
        *   `checked`: Nowy stan zaznaczenia.
    *   **Przykład użycia**:
        ```cpp
        myCheckbox.setChecked(true);
        ```

*   ### `void setOnChange(OnChangeCallback callback)`
    *   **Opis**: Ustawia funkcję zwrotną wywoływaną przy każdej zmianie stanu checkboxa (myszą lub klawiaturą).
    *   **Parametry**:
        *   `callback`: Funkcja zwrotna typu `std::function<void(Checkbox*, bool)>`. Parametr `bool` to nowy stan zaznaczenia.
    *   **Przykład użycia**:
        ```cpp
        myCheckbox.setOnChange([](Checkbox* cb, bool checked) {
            std::cout << "Checkbox " << (checked ? "zaznaczony" : "odznaczony") << std::endl;
        });
        ```

*   ### `bool handleEvent(const SDL_Event& e) override`
    *   **Opis**: Obsługuje zdarzenia SDL dla checkboxa (mysz + klawiatura). Ta metoda jest wywoływana wewnętrznie przez `GUIManager`.
    *   **Parametry**:
        *   `e`: Referencja do zdarzenia `SDL_Event`.
    *   **Zwraca**: `true`, jeśli zdarzenie zostało obsłużone; `false` w przeciwnym razie.

*   ### `const char* getComponentType() const override`
    *   **Opis**: Zwraca typ komponentu jako ciąg znaków.
    *   **Zwraca**: Ciąg znaków `"Checkbox"`.
