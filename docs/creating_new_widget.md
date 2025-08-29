# Jak stworzyć niestandardowy widżet w bibliotece SDL_GUI

Ten przewodnik krok po kroku wyjaśnia, jak stworzyć własny, niestandardowy widżet, integrując go z architekturą biblioteki SDL_GUI. Jako przykład posłuży nam prosty widżet o nazwie `MyWidget`.

## Wprowadzenie

Wszystkie elementy interfejsu użytkownika w tej bibliotece, nazywane widżetami, są klasami dziedziczącymi po `GUIElement`. Klasa ta dostarcza podstawową funkcjonalność, taką jak zarządzanie pozycją, rozmiarem, widocznością, hierarchią rodzic-dziecko oraz, co najważniejsze, dostęp do globalnego kontekstu aplikacji (`GUIManager`).

Aby stworzyć nowy widżet, wystarczy odziedziczyć po `GUIElement` i zaimplementować dwie kluczowe metody wirtualne:
*   `draw()`: Odpowiedzialna za rysowanie widżetu.
*   `handleEvent()`: Odpowiedzialna za obsługę zdarzeń (np. od myszy czy klawiatury).

## Krok 1: Utworzenie plików

Zgodnie z konwencją projektu, dla każdego nowego widżetu tworzymy dwa pliki: nagłówkowy (`.hpp`) i implementacyjny (`.cpp`).

1.  **Plik nagłówkowy:** Utwórz plik `src/MyWidget.hpp`.
2.  **Plik implementacyjny:** Utwórz plik `src/MyWidget.cpp`.

## Krok 2: Definicja klasy w pliku nagłówkowym

W pliku `src/MyWidget.hpp` umieść definicję klasy `MyWidget`, która dziedziczy po `GUIElement`.

```cpp
// src/MyWidget.hpp

#ifndef MYWIDGET_HPP
#define MYWIDGET_HPP

#include "gui.hpp" // Podstawowy nagłówek dla wszystkich elementów GUI

// Deklaracja naszej nowej klasy widżetu
class MyWidget : public GUIElement {
public:
    // Konstruktor przyjmuje managera, pozycję i rozmiar
    MyWidget(GUIManager& manager, int x, int y, int width, int height);

    // Domyślny destruktor jest wystarczający dzięki inteligentym wskaźnikom
    ~MyWidget() = default;

    // Przesłaniamy metodę do obsługi zdarzeń
    bool handleEvent(const SDL_Event& e) override;

    // Przesłaniamy metodę identyfikującą typ komponentu (opcjonalne, ale dobre dla debugowania)
    const char* getComponentType() const override;

protected:
    // Przesłaniamy metodę odpowiedzialną za rysowanie
    void draw() override;
};

#endif // MYWIDGET_HPP
```

## Krok 3: Implementacja w pliku `.cpp`

W pliku `src/MyWidget.cpp` implementujemy logikę naszego widżetu.

### Konstruktor
Konstruktor musi wywołać konstruktor klasy bazowej `GUIElement`, przekazując do niego niezbędne parametry.

```cpp
// src/MyWidget.cpp

#include "MyWidget.hpp"
#include "gui_manager.hpp" // Potrzebny do dostępu do renderera

// Implementacja konstruktora
MyWidget::MyWidget(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
    // Tutaj możemy dodać dodatkową inicjalizację specyficzną dla naszego widżetu
}
```

### Metoda `draw()`
To serce wizualnej reprezentacji widżetu. Używamy jej do narysowania wyglądu widżetu. Dostęp do renderera SDL uzyskujemy poprzez `GUIManager`.

```cpp
void MyWidget::draw() {
    // 1. Zamiast rysować bezpośrednio, pozwalamy klasie bazowej narysować tło
    //    i ramkę na podstawie aktualnego stylu. To najlepsza praktyka.
    GUIElement::draw();

    // 2. Jeśli chcemy dodać coś ponad standardowe rysowanie (np. niestandardową ikonę),
    //    możemy to zrobić tutaj. Dla przykładu narysujemy diagonalną linię.
    
    // Uzyskujemy dostęp do renderera
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Point abs_pos = getAbsolutePosition();

    // Ustawiamy kolor rysowania na czerwony
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    
    // Rysujemy linię
    SDL_RenderDrawLine(renderer, abs_pos.x, abs_pos.y, abs_pos.x + m_width, abs_pos.y + m_height);
}
```

### Metoda `handleEvent()`
Ta metoda odpowiada za logikę interakcji. Klasa bazowa `GUIElement` już implementuje logikę sprawdzania, czy kursor jest nad widżetem i aktualizuje jego stan (`m_currentState`). My musimy jedynie zareagować na te zmiany.

```cpp
bool MyWidget::handleEvent(const SDL_Event& e) {
    // Zapamiętujemy stan przed obsługą zdarzenia
    auto previousState = m_currentState;
    
    // Wywołujemy implementację z klasy bazowej, aby zaktualizowała stan
    // (np. isHovered, m_currentState)
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        // Sprawdzamy, czy nastąpiło kliknięcie - stan zmienił się z wciśniętego na "nad"
        if (previousState == ElementState::Pressed && m_currentState == ElementState::Hover) {
            printf("MyWidget clicked!\n");
            return true; // Zdarzenie zostało obsłużone
        }
    }
    
    return false; // Zdarzenie nie zostało obsłużone przez ten widżet
}

const char* MyWidget::getComponentType() const {
    return "MyWidget";
}
```

## Krok 4: Użycie nowego widżetu

Aby użyć `MyWidget` w aplikacji, wystarczy dołączyć jego nagłówek, stworzyć instancję i dodać ją do `GUIManager`.

```cpp
// W pliku main lub w miejscu inicjalizacji GUI

#include "gui_manager.hpp"
#include "MyWidget.hpp" // Dołączamy nagłówek naszego widżetu
// ... inne nagłówki

// ... wewnątrz głównej funkcji lub klasy aplikacji ...

// Inicjalizacja GUIManager
GUIManager guiManager(app.getRenderer()); 

// Tworzymy instancję MyWidget za pomocą std::make_unique
auto myWidget = std::make_unique<MyWidget>(guiManager, 50, 50, 200, 80);

// Dodajemy widżet do managera
guiManager.addElement(std::move(myWidget));

// ... w głównej pętli aplikacji ...
while (!quit) {
    while (SDL_PollEvent(&e)) {
        // ... obsługa zdarzenia wyjścia ...
        guiManager.processEvent(e);
    }
    
    // ... czyszczenie ekranu ...
    
    guiManager.cleanup(); // Aktualizacja timerów i animacji
    guiManager.render(); // Renderowanie wszystkich widżetów
    
    // ... prezentacja renderera ...
}
```

## Dobre praktyki i dalsze kroki

*   **Używaj stylów:** Zamiast ręcznie rysować prostokąty, używaj systemu stylów. Ustawiaj kolory tła i ramek dla różnych stanów (`Normal`, `Hover`, `Pressed`).
    ```cpp
    // W konstruktorze MyWidget
    setBackgroundColor(ElementState::Normal, {200, 200, 200, 255});
    setBackgroundColor(ElementState::Hover, {220, 220, 220, 255});
    setBackgroundColor(ElementState::Pressed, {180, 180, 180, 255});
    ```
*   **Korzystaj z `TextureManager`:** Do rysowania obrazów zamiast prostych kształtów, użyj `GUIManager::getTextureManager()`, aby załadować teksturę, a następnie przypisz ją do widżetu za pomocą metody `setTexture()`.
## Przykład: ContextMenu

ContextMenu to doskonały przykład widgetu, który rozszerza `GUIElement` i wykorzystuje inne komponenty (takie jak `Panel` i `Button`) do stworzenia złożonej funkcjonalności. Oto jak został zaimplementowany:

### Struktura ContextMenu

ContextMenu używa wzorca kompozytowego, gdzie:
- Główny `ContextMenu` dziedziczy po `GUIElement`
- Wewnętrzny `Panel` służy jako kontener dla elementów menu
- Poszczególne pozycje menu są implementowane jako `Button` (dla elementów klikalnych) lub `Panel` (dla separatorów)

### Kluczowe aspekty implementacji

1. **Zarządzanie cyklem życia**: ContextMenu automatycznie pokazuje/ukrywa się i zarządza swoimi elementami-dziećmi.

2. **Opóźnione tworzenie**: Przyciski menu są tworzone dopiero przy pierwszym wywołaniu `showAt()`, co oszczędza zasoby.

3. **Pozycjonowanie automatyczne**: Metoda `positionMenu()` automatycznie dostosowuje pozycję menu, aby nie wychodziło poza granice okna.

4. **Obsługa zdarzeń**: ContextMenu przechwytuje kliknięcia poza swoim obszarem i automatycznie się zamyka.

### Przykładowy kod użycia

```cpp
// Tworzenie menu kontekstowego
auto contextMenu = std::make_unique<ContextMenu>(manager);
ContextMenu* menuPtr = contextMenu.get();

// Dodawanie pozycji menu
menuPtr->addItem("Copy", []() {
    std::cout << "Copy action!" << std::endl;
});

menuPtr->addItem("Paste", []() {
    std::cout << "Paste action!" << std::endl;
}, false); // disabled

menuPtr->addSeparator();

menuPtr->addItem("Delete", []() {
    std::cout << "Delete action!" << std::endl;
});

// Pokazywanie menu na pozycji kursora
menuPtr->showAt(mouseX, mouseY);

// Dodanie do managera
manager.addElement(std::move(contextMenu));
```

### Wskazówki projektowe

- **Użyj kompozycji**: ContextMenu pokazuje, jak łączyć proste widgety w bardziej złożone komponenty.
- **Lazy initialization**: Nie twórz zasobów, dopóki nie są potrzebne.
- **Automatyczne zarządzanie**: Implementuj automatyczne zachowania (jak zamykanie po kliknięciu poza obszarem).
- **Zachowaj prostotę API**: Użytkownik nie musi znać wewnętrznej struktury - API powinno być intuicyjne.

Pełną implementację ContextMenu można znaleźć w [`src/context_menu.hpp`](src/context_menu.hpp) i [`src/context_menu.cpp`](src/context_menu.cpp), a przykład użycia w [`examples/example_context_menu.cpp`](examples/example_context_menu.cpp).
*   **Dodawaj callbacki:** Wzorując się na klasie `Button`, możesz dodać `std::function` jako pole `MyWidget`, aby umożliwić użytkownikom przypisywanie własnych akcji na zdarzenie kliknięcia.
*   **Zarządzanie dziećmi:** Jeśli twój widżet ma być kontenerem na inne, użyj metody `addChild(std::unique_ptr<GUIElement> child)`. Klasa bazowa automatycznie zajmie się ich renderowaniem i przekazywaniem zdarzeń.