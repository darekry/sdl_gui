# Panel API

## Przeznaczenie komponentu

Klasa `Panel` to podstawowy element GUI, który służy jako kontener dla innych elementów. Może być przezroczysty, mieć tło i ramkę. Obsługuje również funkcję przeciągania, pozwalając użytkownikowi na przesuwanie panelu po ekranie.

## Publiczne metody

*   ### `Panel(GUIManager& manager, int x, int y, int width, int height)`
    *   **Opis**: Konstruktor klasy `Panel`. Tworzy nowy panel o określonej pozycji i rozmiarze.
    *   **Parametry**:
        *   `manager`: Referencja do `GUIManager` zarządzającego elementem.
        *   `x`: Pozycja X lewego górnego rogu panelu.
        *   `y`: Pozycja Y lewego górnego rogu panelu.
        *   `width`: Szerokość panelu.
        *   `height`: Wysokość panelu.
    *   **Przykład użycia**:
        ```cpp
        Panel myPanel(guiManager, 100, 100, 300, 200);
        ```

*   ### `Panel(GUIManager& manager, SDL_Rect rect)`
    *   **Opis**: Konstruktor klasy `Panel`. Tworzy nowy panel na podstawie struktury `SDL_Rect`.
    *   **Parametry**:
        *   `manager`: Referencja do `GUIManager` zarządzającego elementem.
        *   `rect`: Struktura `SDL_Rect` definiująca pozycję i rozmiar panelu.
    *   **Przykład użycia**:
        ```cpp
        SDL_Rect panelRect = {100, 100, 300, 200};
        Panel myPanel(guiManager, panelRect);
        ```

*   ### `bool handleEvent(const SDL_Event& event) override`
    *   **Opis**: Obsługuje zdarzenia SDL dla panelu. Przekazuje zdarzenia do swoich dzieci i obsługuje logikę przeciągania, jeśli panel jest przeciągalny. Ta metoda jest wywoływana wewnętrznie przez `GUIManager`.
    *   **Parametry**:
        *   `event`: Referencja do zdarzenia `SDL_Event`.
    *   **Zwraca**: `true`, jeśli zdarzenie zostało obsłużone; `false` w przeciwnym razie.

*   ### `void setDraggable(bool draggable)`
    *   **Opis**: Ustawia, czy panel może być przeciągany myszą.
    *   **Parametry**:
        *   `draggable`: `true`, aby umożliwić przeciąganie; `false`, aby je wyłączyć.
    *   **Przykład użycia**:
        ```cpp
        myPanel.setDraggable(true);
        ```

*   ### `ComponentType getComponentTypeId() const override`
    *   **Opis**: Zwraca typ komponentu jako ciąg znaków.
    *   **Zwraca**: Ciąg znaków `"Panel"`.