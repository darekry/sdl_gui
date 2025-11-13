# MouseCursor — Niestandardowy kursor myszy

Komponent `MouseCursor` pozwala zastąpić domyślny kursor systemowy niestandardową grafiką, w tym animowaną. Jest to komponent globalny, zarządzany przez `GUIManager`.

**Kluczowe funkcje:**
- Obsługa wielu stanów kursora (np. `Normal`, `Hover`, `Busy`).
- Wyświetlanie animowanych kursorów z arkuszy sprite'ów.
- Definiowanie "hotspotu", czyli aktywnego punktu kursora.
- Skalowanie grafiki kursora.

## Podstawowe użycie

Aby skorzystać z niestandardowego kursora, należy go włączyć w `GUIManager`, a następnie skonfigurować jego wygląd dla różnych stanów.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "mouse_cursor.hpp"

int main() {
    SDLApp app("Niestandardowy kursor", 800, 600);
    GUIManager gui(app.getRenderer());

    // Włącz niestandardowy kursor (ukrywa systemowy)
    gui.setCustomCursorEnabled(true);

    // Pobierz wskaźnik do obiektu kursora
    MouseCursor* cursor = gui.getMouseCursor();

    // Ustaw teksturę dla stanu domyślnego
    cursor->setCursorTexture(CursorState::Normal, "assets/cursor_normal.png", 8, 8);

    // Ustaw teksturę dla stanu najechania
    cursor->setCursorTexture(CursorState::Hover, "assets/cursor_hover.png", 16, 16);

    // Ustaw animowany kursor dla stanu zajętości
    cursor->setAnimatedCursor(CursorState::Busy, "assets/cursor_busy.png", 8, 2, 12.0f, 16, 16);

    // Pętla główna aplikacji
    while (app.isRunning()) {
        // ... obsługa zdarzeń i renderowanie
    }

    return 0;
}
```

## Zmiana stanu kursora

Stan kursora można zmieniać ręcznie lub automatycznie w odpowiedzi na interakcje z interfejsem.

```cpp
// Ręczna zmiana stanu
cursor->setState(CursorState::Busy);

// Automatyczna zmiana w pętli zdarzeń
if (e.type == SDL_MOUSEMOTION) {
    GUIElement* element = gui.findElementAt(e.motion.x, e.motion.y);
    if (element && element->getComponentType() == "Button") {
        cursor->setState(CursorState::Hover);
    } else {
        cursor->setState(CursorState::Normal);
    }
}
```

## Stany kursora

Dostępne stany kursora są zdefiniowane w `enum class CursorState`:
- `Normal`: Standardowy kursor.
- `Hover`: Kursor po najechaniu na interaktywny element.
- `Pressed`: Kursor podczas kliknięcia.
- `Disabled`: Kursor nad wyłączonym elementem.
- `Busy`: Kursor zajętości/ładowania.
- `Text`: Kursor do wprowadzania tekstu.
- `Custom1`, `Custom2`, `Custom3`: Stany do dowolnego wykorzystania.

## Referencje API

### Konfiguracja
- `setCursorTexture(CursorState state, const std::string& path, int hotspotX = 0, int hotspotY = 0)`: Ustawia statyczną teksturę dla danego stanu.
- `setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows = 1, float fps = 12.0f, int hotspotX = 0, int hotspotY = 0)`: Ustawia animowany kursor z arkusza sprite'ów.
- `setScale(float scale)`: Ustawia skalę kursora (1.0 to rozmiar oryginalny).
- `setOffset(int offsetX, int offsetY)`: Ustawia dodatkowe przesunięcie kursora względem pozycji myszy.

### Zarządzanie stanem
- `setState(CursorState state)`: Ustawia bieżący stan kursora.
- `getState() const`: Zwraca bieżący stan.
- `setVisible(bool visible)`: Pokazuje lub ukrywa kursor.
- `isVisible() const`: Sprawdza, czy kursor jest widoczny.

### Callbacki
- `setOnStateChanged(std::function<void(CursorState)> callback)`: Ustawia funkcję zwrotną wywoływaną przy zmianie stanu.

## Przykład

Kompletny, działający przykład użycia `MouseCursor` znajduje się w pliku:
- [`examples/example_mouse_cursor.cpp`](../examples/example_mouse_cursor.cpp)
