# GUIManager API

## Przeznaczenie komponentu

`GUIManager` jest centralnym kontrolerem biblioteki SDL GUI. Odpowiada za inicjalizację i zarządzanie cyklem życia kluczowych menedżerów (FontManager, TextureManager, TimerManager, AnimationManager), przechowywanie i renderowanie elementów GUI najwyższego poziomu, obsługę zdarzeń SDL, zarządzanie globalnym motywem oraz mechanizmami fokusu i przechwytywania myszy.

### Nawigacja klawiaturą

`GUIManager` automatycznie obsługuje nawigację klawiaturą pomiędzy elementami:
- **Tab** — przełącza fokus na następny element focusowalny (Button, Checkbox, TextInput itp.) w kolejności DFS po drzewie widgetów
- **Shift+Tab** — przełącza fokus na poprzedni element focusowalny (z zawijaniem)
- Element z fokusem otrzymuje wszystkie zdarzenia klawiatury (KEY_DOWN, KEY_UP, TEXT_INPUT)
- Brak elementu z fokusem — zdarzenia klawiatury są ignorowane

## Publiczne metody

*   ### `explicit GUIManager(SDL_Renderer* renderer)`
    *   **Opis**: Konstruktor klasy `GUIManager`. Inicjalizuje menedżery zasobów i motyw.
    *   **Parametry**:
        *   `renderer`: Wskaźnik do obiektu `SDL_Renderer`, który będzie używany do renderowania.
    *   **Przykład użycia**:
        ```cpp
        SDL_Renderer* renderer = ...; // Inicjalizacja renderera
        GUIManager guiManager(renderer);
        ```

*   ### `~GUIManager()`
    *   **Opis**: Destruktor klasy `GUIManager`. Zwolnienie zasobów jest automatycznie obsługiwane przez `std::unique_ptr` i `std::shared_ptr`.

*   ### `GUIElement* addElement(std::unique_ptr<GUIElement> element)`
    *   **Opis**: Dodaje nowy element GUI do zarządzania przez `GUIManager`. Menedżer przejmuje własność nad elementem.
    *   **Parametry**:
        *   `element`: Unikalny wskaźnik do elementu GUI do dodania.
    *   **Zwraca**: Wskaźnik surowy do dodanego elementu, lub `nullptr` jeśli element był pusty.
    *   **Przykład użycia**:
        ```cpp
        auto button = std::make_unique<Button>(guiManager, 10, 10, 100, 50, "Click Me");
        GUIElement* btnPtr = guiManager.addElement(std::move(button));
        ```

*   ### `bool processEvent(const SDL_Event& e)`
    *   **Opis**: Przetwarza zdarzenie SDL, przekazując je do odpowiednich elementów GUI. Obsługuje przechwytywanie myszy i fokus klawiatury.
    *   **Parametry**:
        *   `e`: Referencja do zdarzenia `SDL_Event`.
    *   **Zwraca**: `true`, jeśli zdarzenie zostało obsłużone przez któryś z elementów GUI; `false` w przeciwnym razie.
    *   **Przykład użycia**:
        ```cpp
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (guiManager.processEvent(event)) {
                // Zdarzenie zostało obsłużone przez GUI
            } else {
                // Obsługa innych zdarzeń aplikacji
            }
        }
        ```

*   ### `void render()`
    *   **Opis**: Renderuje wszystkie zarządzane elementy GUI na ekranie. Elementy overlay są renderowane na końcu.
    *   **Przykład użycia**:
        ```cpp
        SDL_RenderClear(renderer);
        guiManager.render();
        SDL_RenderPresent(renderer);
        ```

*   ### `void cleanup()`
    *   **Opis**: Czyści elementy GUI oznaczone do usunięcia oraz aktualizuje timery i animacje.
    *   **Przykład użycia**:
        ```cpp
        guiManager.cleanup();
        ```

*   ### `SDL_Renderer* getRenderer() const`
    *   **Opis**: Zwraca wskaźnik do renderera SDL używanego przez menedżera.
    *   **Zwraca**: Wskaźnik `SDL_Renderer*`.

*   ### `FontManager& getFontManager()`
    *   **Opis**: Zwraca referencję do menedżera czcionek.

*   ### `const FontManager& getFontManager() const`
    *   **Opis**: Zwraca stałą referencję do menedżera czcionek.

*   ### `TextureManager& getTextureManager()`
    *   **Opis**: Zwraca referencję do menedżera tekstur.

*   ### `const TextureManager& getTextureManager() const`
    *   **Opis**: Zwraca stałą referencję do menedżera tekstur.

*   ### `TimerManager* getTimerManager()`
    *   **Opis**: Zwraca wskaźnik do menedżera timerów.
    *   **Zwraca**: Wskaźnik `TimerManager*`.

*   ### `AnimationManager* getAnimationManager()`
    *   **Opis**: Zwraca wskaźnik do menedżera animacji.
    *   **Zwraca**: Wskaźnik `AnimationManager*`.

*   ### `void showTooltip(GUIElement* target, const std::string& text)`
    *   **Opis**: Wyświetla tooltip dla danego elementu GUI z określonym tekstem.
    *   **Parametry**:
        *   `target`: Wskaźnik do elementu, dla którego ma być wyświetlony tooltip.
        *   `text`: Tekst tooltipa.
    *   **Przykład użycia**:
        ```cpp
        guiManager.showTooltip(myButton, "To jest przycisk");
        ```

*   ### `void hideTooltip()`
    *   **Opis**: Ukrywa aktualnie wyświetlany tooltip.
    *   **Przykład użycia**:
        ```cpp
        guiManager.hideTooltip();
        ```

*   ### `void setTheme(Theme theme)`
    *   **Opis**: Ustawia globalny motyw dla wszystkich elementów GUI.
    *   **Parametry**:
        *   `theme`: Obiekt `Theme` do ustawienia.
    *   **Przykład użycia**:
        ```cpp
        Theme myCustomTheme = Theme::createDefaultTheme();
        // ... modyfikacja motywu ...
        guiManager.setTheme(std::move(myCustomTheme));
        ```

*   ### `Theme& getTheme()`
    *   **Opis**: Zwraca referencję do aktualnie używanego motywu.
    *   **Zwraca**: Referencja `Theme&`.

*   ### `GUIElement* findElementAt(int x, int y)`
    *   **Opis**: Znajduje element GUI znajdujący się pod danymi współrzędnymi (x, y).
    *   **Parametry**:
        *   `x`: Współrzędna X.
        *   `y`: Współrzędna Y.
    *   **Zwraca**: Wskaźnik do znalezionego elementu `GUIElement*` lub `nullptr`, jeśli nie znaleziono.

*   ### `void captureMouse(GUIElement* element)`
    *   **Opis**: Przechwytuje zdarzenia myszy do określonego elementu GUI. Wszystkie przyszłe zdarzenia myszy będą wysyłane tylko do tego elementu.
    *   **Parametry**:
        *   `element`: Wskaźnik do elementu, który ma przechwycić mysz.
    *   **Przykład użycia**:
        ```cpp
        guiManager.captureMouse(myDraggableElement);
        ```

*   ### `void releaseMouse()`
    *   **Opis**: Zwalnia przechwytywanie myszy. Zdarzenia myszy będą ponownie rozsyłane do wszystkich elementów.
    *   **Przykład użycia**:
        ```cpp
        guiManager.releaseMouse();
        ```

*   ### `void setKeyboardFocus(GUIElement* element)`
    *   **Opis**: Ustawia fokus klawiatury na określony element GUI. Zdarzenia klawiatury będą wysyłane tylko do tego elementu. Wywołuje `onFocusLost()` na poprzednim elemencie i `onFocusGained()` na nowym.
    *   **Parametry**:
        *   `element`: Wskaźnik do elementu, który ma otrzymać fokus klawiatury. `nullptr` usuwa fokus.
    *   **Przykład użycia**:
        ```cpp
        guiManager.setKeyboardFocus(myTextInput);
        ```

*   ### `[[nodiscard]] GUIElement* getKeyboardFocus() const`
    *   **Opis**: Zwraca wskaźnik do elementu, który aktualnie posiada fokus klawiatury.
    *   **Zwraca**: Wskaźnik `GUIElement*` do elementu z focusem lub `nullptr`, jeśli żaden element nie ma fokusu.

*   ### `void focusNextElement(bool forward)`
    *   **Opis**: Przełącza fokus klawiatury na następny (lub poprzedni) element focusowalny. Elementy są przeszukiwane w kolejności DFS po drzewie widgetów. Zawija na koniec/początek listy.
    *   **Parametry**:
        *   `forward`: `true` — następny element; `false` — poprzedni element.
    *   **Przykład użycia**:
        ```cpp
        guiManager.focusNextElement(true);   // następny
        guiManager.focusNextElement(false);  // poprzedni
        ```