# Button API

## Przeznaczenie komponentu

Klasa `Button` reprezentuje interaktywny przycisk w interfejsie użytkownika. Dziedziczy po `GUIElement` i obsługuje zdarzenia myszy oraz klawiatury. Może wyświetlać tekstową etykietę. Przycisk automatycznie rejestruje się jako element focusowalny (`canGetKeyboardFocus = true`).

### Obsługa klawiatury

Przycisk można aktywować za pomocą klawiatury:
- **Enter** lub **Spacja** — naciśnięcie zmienia stan wizualny na `Pressed`, puszczenie wywołuje callback `onClick` i przywraca stan `Hover`
- **Tab** / **Shift+Tab** — przełącza fokus pomiędzy elementami focusowalnymi (Button, Checkbox, TextInput itp.)
- Aktywny fokus jest wizualnie sygnalizowany niebieską obwódką wokół przycisku

## Publiczne metody

*   ### `Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label = "")`
    *   **Opis**: Konstruktor klasy `Button`. Tworzy nowy przycisk o określonej pozycji, rozmiarze i opcjonalnej etykiecie. Automatycznie włącza możliwość uzyskania fokusu klawiatury.
    *   **Parametry**:
        *   `manager`: Referencja do `GUIManager` zarządzającego elementem.
        *   `x`: Pozycja X lewego górnego rogu przycisku.
        *   `y`: Pozycja Y lewego górnego rogu przycisku.
        *   `width`: Szerokość przycisku.
        *   `height`: Wysokość przycisku.
        *   `label`: Opcjonalna etykieta tekstowa wyświetlana na przycisku.
    *   **Przykład użycia**:
        ```cpp
        Button myButton(guiManager, 50, 50, 150, 40, "Mój Przycisk");
        ```

*   ### `~Button() = default`
    *   **Opis**: Domyślny destruktor.

*   ### `void setOnClickCallback(OnClickCallback callback)`
    *   **Opis**: Ustawia funkcję zwrotną, która zostanie wywołana po kliknięciu przycisku (myszą lub klawiaturą).
    *   **Parametry**:
        *   `callback`: Funkcja zwrotna typu `std::function<void(GUIElement*)>`.
    *   **Przykład użycia**:
        ```cpp
        myButton.setOnClickCallback([](GUIElement* element) {
            std::cout << "Przycisk kliknięty!" << std::endl;
        });
        ```

*   ### `void setOnMouseOverCallback(OnMouseOverCallback callback)`
    *   **Opis**: Ustawia funkcję zwrotną, która zostanie wywołana, gdy kursor myszy najedzie na przycisk.
    *   **Parametry**:
        *   `callback`: Funkcja zwrotna typu `std::function<void(GUIElement*)>`.
    *   **Przykład użycia**:
        ```cpp
        myButton.setOnMouseOverCallback([](GUIElement* element) {
            std::cout << "Kursor nad przyciskiem!" << std::endl;
        });
        ```

*   ### `bool handleEvent(const SDL_Event& e) override`
    *   **Opis**: Obsługuje zdarzenia SDL dla przycisku (mysz + klawiatura). Ta metoda jest wywoływana wewnętrznie przez `GUIManager`.
    *   **Parametry**:
        *   `e`: Referencja do zdarzenia `SDL_Event`.
    *   **Zwraca**: `true`, jeśli zdarzenie zostało obsłużone; `false` w przeciwnym razie.

*   ### `const char* getComponentType() const override`
    *   **Opis**: Zwraca typ komponentu jako ciąg znaków.
    *   **Zwraca**: Ciąg znaków `"Button"`.