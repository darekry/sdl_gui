# ContextMenu — menu kontekstowe

Krótkie wprowadzenie
-------------------
`ContextMenu` to widget implementujący menu kontekstowe, które pojawia się po kliknięciu prawym przyciskiem myszy lub w odpowiedzi na inne zdarzenia. Pozwala na:
- Dynamiczne tworzenie pozycji menu z akcjami
- Dodawanie separatorów między grupami opcji
- Włączanie/wyłączanie poszczególnych pozycji
- Automatyczne pozycjonowanie i zamykanie menu
- Pełną integrację z systemem zdarzeń SDL

Wymagania i zależności
--------------------
- Dziedziczy po `GUIElement` — podstawowa funkcjonalność: [`src/gui.hpp`](../../src/gui.hpp:19)
- Używa `Panel` jako kontenera dla elementów menu: [`src/panel.hpp`](../../src/panel.hpp:13)
- Tworzy `Button` dla każdej klikalnej pozycji menu: [`src/button.hpp`](../../src/button.hpp:13)
- Zarządzany przez `GUIManager`: [`src/gui_manager.hpp`](../../src/gui_manager.hpp:19)

Konstrukcja i podstawowe użycie
-------------------------------
Przykład minimalnego użycia (kompilowalny fragment):

```cpp
#include "gui_manager.hpp"
#include "context_menu.hpp"

int main() {
    // Inicjalizacja SDL i GUIManager
    SDLApp app("ContextMenu Example", 800, 600);
    GUIManager manager(app.getRenderer());

    // Tworzenie menu kontekstowego
    auto contextMenu = std::make_unique<ContextMenu>(manager);
    ContextMenu* menuPtr = contextMenu.get();

    // Dodawanie pozycji menu
    menuPtr->addItem("Copy", []() {
        std::cout << "Copy action executed!" << std::endl;
    });

    menuPtr->addItem("Paste", []() {
        std::cout << "Paste action executed!" << std::endl;
    }, false); // disabled

    menuPtr->addSeparator();

    menuPtr->addItem("Delete", []() {
        std::cout << "Delete action executed!" << std::endl;
    });

    // Dodanie do managera
    manager.addElement(std::move(contextMenu));

    // W aplikacji - pokazanie menu na pozycji kursora
    menuPtr->showAt(mouseX, mouseY);

    return 0;
}
```

Dodawanie pozycji menu
---------------------
Menu kontekstowe pozwala na dodawanie różnych typów pozycji:

### Pozycje standardowe
```cpp
// Dodanie pozycji z akcją
menuPtr->addItem("Open File", []() {
    openFileDialog();
});

// Dodanie wyłączonej pozycji
menuPtr->addItem("Save", []() {
    saveCurrentFile();
}, false); // disabled

// Dodanie pozycji bez akcji (tylko etykieta)
menuPtr->addItem("Version 1.0");
```

### Separatory
```cpp
menuPtr->addSeparator(); // Dodaje wizualny separator
```

### Czyszczenie menu
```cpp
menuPtr->clearItems(); // Usuwa wszystkie pozycje
```

Pokazywanie i ukrywanie menu
---------------------------
Menu może być wyświetlane w dowolnej pozycji na ekranie:

```cpp
// Pokazanie menu
menuPtr->showAt(x, y);

// Sprawdzenie widoczności
if (menuPtr->isVisible()) {
    std::cout << "Menu is currently shown" << std::endl;
}

// Ukrycie menu
menuPtr->hide();
```

Menu automatycznie:
- Pozycjonuje się, aby nie wychodzić poza granice okna
- Zamyka się po wybraniu opcji
- Zamyka się po kliknięciu poza obszarem menu

Obsługa zdarzeń
---------------
ContextMenu automatycznie obsługuje zdarzenia myszki:

```cpp
// W głównej pętli aplikacji
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_RIGHT) {

        // Pokaż menu na pozycji kursora
        menuPtr->showAt(event.button.x, event.button.y);
    }

    manager.processEvent(event);
}
```

Menu automatycznie:
- Przechwytuje kliknięcia na swoich pozycjach
- Wykonuje przypisane akcje
- Zamyka się po wykonaniu akcji
- Zamyka się po kliknięciu poza obszarem menu

Pełna lista publicznych metod
----------------------------
Poniżej lista publicznych metod wraz z sygnaturami i krótkim opisem. Sygnatury pochodzą z nagłówka: [`src/context_menu.hpp`](../../src/context_menu.hpp:25).

### Zarządzanie pozycjami
- `void addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)` — dodaje pozycję menu z opcjonalną akcją i stanem włączenia. ([`src/context_menu.hpp`](../../src/context_menu.hpp:30))
- `void addSeparator()` — dodaje separator między grupami pozycji. ([`src/context_menu.hpp`](../../src/context_menu.hpp:31))
- `void clearItems()` — usuwa wszystkie pozycje z menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:32))

### Kontrola wyświetlania
- `void showAt(int x, int y)` — pokazuje menu na podanej pozycji, automatycznie pozycjonując je w granicach okna. ([`src/context_menu.hpp`](../../src/context_menu.hpp:34))
- `void hide()` — ukrywa menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:35))
- `bool isVisible() const` — zwraca true jeśli menu jest widoczne. ([`src/context_menu.hpp`](../../src/context_menu.hpp:36))

### Dziedziczone z GUIElement
- `bool handleEvent(const SDL_Event& event) override` — obsługuje zdarzenia myszki dla menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:38))
- `const char* getComponentType() const override` — zwraca "ContextMenu". ([`src/context_menu.hpp`](../../src/context_menu.hpp:39))

Mechanizmy wewnętrzne istotne dla użytkownika
--------------------------------------------
### Struktura ContextMenuItem
```cpp
struct ContextMenuItem {
    std::string text;              // Tekst wyświetlany
    std::function<void()> action;  // Akcja do wykonania
    bool enabled = true;           // Czy pozycja jest włączona
    bool separator = false;        // Czy to separator
};
```

### Architektura kompozytowa
ContextMenu używa wzorca kompozytowego:
- Główny `ContextMenu` dziedziczy po `GUIElement`
- Wewnętrzny `Panel` (`m_panel`) zawiera wszystkie elementy menu
- Każda pozycja menu to albo `Button` (dla akcji), albo `Panel` (dla separatorów)

### Opóźnione tworzenie przycisków
Przyciski menu są tworzone dopiero w `showAt()`:
- Metoda `createMenuButtons()` jest wywoływana tylko gdy potrzebne
- Ustawia flagę `m_needsUpdate` do synchronizacji
- Tworzy odpowiedni widget dla każdej pozycji

### Automatyczne pozycjonowanie
Metoda `positionMenu()`:
- Sprawdza granice okna (domyślnie 800x600)
- Przesuwa menu w lewo/górę jeśli wychodzi poza prawy/dolny brzeg
- Zapewnia, że menu jest zawsze w pełni widoczne

### Obsługa kliknięć poza menu
Metoda `shouldCloseOnClick()`:
- Sprawdza czy kliknięcie jest poza prostokątem menu
- Używa `SDL_PointInRect()` do sprawdzenia pozycji
- Zwraca true jeśli menu powinno zostać zamknięte

Tips & Gotchas (najczęściej przydatne wskazówki)
-----------------------------------------------
### Tworzenie dynamicznych menu
```cpp
// Menu z pozycjami zależnymi od kontekstu
void showFileMenu(int fileType) {
    menuPtr->clearItems();

    menuPtr->addItem("Open");
    menuPtr->addItem("Edit");

    if (fileType == IMAGE_FILE) {
        menuPtr->addSeparator();
        menuPtr->addItem("View Image");
        menuPtr->addItem("Resize");
    }

    menuPtr->showAt(mouseX, mouseY);
}
```

### Zarządzanie stanem pozycji
```cpp
// Włącz/wyłącz pozycje w zależności od stanu aplikacji
bool canSave = hasUnsavedChanges();
menuPtr->addItem("Save", []() { saveFile(); }, canSave);

bool canUndo = !undoStack.empty();
menuPtr->addItem("Undo", []() { undoLastAction(); }, canUndo);
```

### Obsługa prawego przycisku myszy
```cpp
// W pętli zdarzeń
if (event.type == SDL_MOUSEBUTTONDOWN &&
    event.button.button == SDL_BUTTON_RIGHT) {

    // Sprawdź czy kliknięto na jakiś element
    GUIElement* clickedElement = getElementAt(event.button.x, event.button.y);

    if (clickedElement != nullptr) {
        showContextMenuFor(clickedElement, event.button.x, event.button.y);
    }
}
```

### Problemy z pozycjonowaniem
- Menu domyślnie zakłada okno 800x600 - dostosuj w `positionMenu()` jeśli potrzebne
- Upewnij się, że `GUIManager` jest prawidłowo skonfigurowany przed tworzeniem menu
- Menu może wymagać ręcznego wywołania `markDirty()` po zmianach

### Problemy z widocznością
- Menu jest początkowo ukryte - zawsze wywołuj `showAt()` aby je pokazać
- Po wybraniu opcji menu automatycznie się zamyka
- Sprawdź czy wszystkie callbacki są prawidłowo podłączone

Dodatkowe odwołania / przykłady
-------------------------------
- Zobacz kompletny przykład w katalogu examples: [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp:1)
- Konstruktor widgetu: [`src/context_menu.hpp`](../../src/context_menu.hpp:27)
- Implementacja `addItem`: [`src/context_menu.cpp`](../../src/context_menu.cpp:18)
- Implementacja `showAt`: [`src/context_menu.cpp`](../../src/context_menu.cpp:37)
- Implementacja `handleEvent`: [`src/context_menu.cpp`](../../src/context_menu.cpp:56)

Częste linie kodu do przeglądnięcia (ważne miejsca implementacji)
-----------------------------------------------------------------
- Konstruktor / destruktor: [`src/context_menu.cpp`](../../src/context_menu.cpp:5)
- addItem: [`src/context_menu.cpp`](../../src/context_menu.cpp:18)
- addSeparator: [`src/context_menu.cpp`](../../src/context_menu.cpp:24)
- showAt: [`src/context_menu.cpp`](../../src/context_menu.cpp:37)
- hide: [`src/context_menu.cpp`](../../src/context_menu.cpp:50)
- handleEvent: [`src/context_menu.cpp`](../../src/context_menu.cpp:56)
- createMenuButtons: [`src/context_menu.cpp`](../../src/context_menu.cpp:88)
- positionMenu: [`src/context_menu.cpp`](../../src/context_menu.cpp:125)
- shouldCloseOnClick: [`src/context_menu.cpp`](../../src/context_menu.cpp:153)

Zakończenie
-----------
ContextMenu to wszechstronny widget, który można łatwo dostosować do różnych scenariuszy użycia. Jego kompozytowa architektura pozwala na łatwe rozszerzanie funkcjonalności, a automatyczne zarządzanie cyklem życia czyni go niezawodnym w codziennym użytkowaniu. Widget doskonale sprawdza się zarówno w prostych menu podręcznych, jak i złożonych systemach kontekstowych.