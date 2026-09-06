# ContextMenu — menu kontekstowe

`ContextMenu` to widget implementujący menu kontekstowe, które pojawia się w odpowiedzi na działania użytkownika, najczęściej po kliknięciu prawym przyciskiem myszy.

**Kluczowe funkcje:**
- Dynamiczne tworzenie pozycji menu z przypisanymi akcjami.
- Dodawanie separatorów między grupami opcji.
- Włączanie i wyłączanie poszczególnych pozycji.
- Automatyczne pozycjonowanie i zamykanie.

## Konstrukcja i podstawowe użycie

Aby użyć `ContextMenu`, należy utworzyć instancję, dodać pozycje menu, a następnie wyświetlić je w odpowiednim momencie.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "context_menu.hpp"

int main() {
    SDLApp app("ContextMenu Example", 800, 600);
    GUIManager manager(app.getRenderer());

    // Tworzenie menu kontekstowego
    auto contextMenu = std::make_unique<ContextMenu>(manager);
    ContextMenu* menuPtr = contextMenu.get();

    // Dodawanie pozycji menu
    menuPtr->addItem("Kopiuj", []() {
        std::cout << "Akcja 'Kopiuj' wykonana!" << std::endl;
    });
    menuPtr->addItem("Wklej", []() {
        std::cout << "Akcja 'Wklej' wykonana!" << std::endl;
    }, false); // Pozycja wyłączona
    menuPtr->addSeparator();
    menuPtr->addItem("Usuń", []() {
        std::cout << "Akcja 'Usuń' wykonana!" << std::endl;
    });

    manager.addElement(std::move(contextMenu));

    // Pętla główna aplikacji
    while (app.isRunning()) {
        app.handleEvents();
        const SDL_Event& event = app.getEvent();

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
            menuPtr->showAt(event.button.x, event.button.y);
        }

        manager.processEvent(event);
        
        app.clearScreen();
        manager.render();
        app.present();
    }

    return 0;
}
```

## Zarządzanie pozycjami menu

Menu kontekstowe pozwala na elastyczne dodawanie i usuwanie pozycji.

```cpp
// Dodanie pozycji z akcją
menuPtr->addItem("Otwórz plik", []() { /* ... */ });

// Dodanie pozycji wyłączonej
menuPtr->addItem("Zapisz", []() { /* ... */ }, false);

// Dodanie separatora wizualnego
menuPtr->addSeparator();

// Usunięcie wszystkich pozycji
menuPtr->clearItems();
```

## Wyświetlanie i ukrywanie

Menu można wyświetlić w dowolnym miejscu na ekranie. Automatycznie dostosuje ono swoją pozycję, aby nie wyjść poza granice okna.

```cpp
// Wyświetl menu na podanych współrzędnych
menuPtr->showAt(x, y);

// Sprawdź, czy menu jest widoczne
if (menuPtr->isVisible()) {
    // ...
}

// Ukryj menu
menuPtr->hide();
```

Menu zamyka się automatycznie po wybraniu opcji lub po kliknięciu poza jego obszarem.

## Referencje API

### Zarządzanie pozycjami
- `ContextMenu(GUIManager& manager)`: Konstruktor.
- `addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)`: Dodaje pozycję menu.
- `addSeparator()`: Dodaje separator.
- `clearItems()`: Usuwa wszystkie pozycje.

### Kontrola wyświetlania
- `showAt(int x, int y)`: Wyświetla menu na podanej pozycji.
- `hide()`: Ukrywa menu.
- `isVisible() const`: Zwraca `true`, jeśli menu jest widoczne.

## Przykład

Kompletny, działający przykład użycia `ContextMenu` znajduje się w pliku:
- [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp)