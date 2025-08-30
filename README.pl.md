# Biblioteka GUI dla SDL2

## Opis

Prosta i rozszerzalna biblioteka GUI zbudowana w oparciu o SDL2 w standardzie C++23, umożliwiająca łatwe tworzenie interaktywnych elementów interfejsu użytkownika. Biblioteka została zaprojektowana z myślą o spójności, bezpieczeństwie pamięci i łatwości użycia.

## Architektura

Architektura biblioteki opiera się na kilku kluczowych klasach i wzorcach projektowych, które zapewniają elastyczność, reużywalność kodu i centralne zarządzanie zasobami.

### `GUIElement` - Klasa Bazowa

Sercem biblioteki jest abstrakcyjna klasa `GUIElement`, która definiuje wspólny interfejs i zachowanie dla wszystkich komponentów interfejsu.

*   **Hierarchia i własność:** Zarządza relacją rodzic-dziecko za pomocą `std::vector<std::unique_ptr<GUIElement>>`. Użycie `std::unique_ptr` zapewnia automatyczne i kaskadowe zwalnianie pamięci (RAII), co eliminuje ryzyko wycieków.
*   **Dostęp do kontekstu:** Każdy element przechowuje referencję do `GUIManager`, co daje mu dostęp do globalnych zasobów, takich jak renderer i managery.
*   **Podstawowe atrybuty:** Definiuje podstawowe właściwości, takie jak pozycja (`x`, `y`), rozmiar (`width`, `height`), widoczność (`m_visible`), aktywność (`m_enabled`), stan najechania myszą (`m_isHovered`).
*   **Obsługa zdarzeń:** Wirtualna metoda `handleEvent(const SDL_Event& e)` propaguje zdarzenia w dół drzewa komponentów. Komponenty mogą "skonsumować" zdarzenie, zatrzymując dalszą propagację.
*   **Renderowanie:** Metoda `render()` odpowiada za rysowanie elementu i jego dzieci. Wywołuje wirtualną metodę `draw()`, którą klasy pochodne muszą zaimplementować. Renderowanie uwzględnia również przycinanie (`clipping`) do granic rodzica.
*   **Odroczone usuwanie:** Mechanizm `markForDeletion()` pozwala na bezpieczne usuwanie elementów w głównej pętli aplikacji.
*   **Podpowiedzi (Tooltips):** Wbudowana obsługa tooltipów z wykorzystaniem `TimerManager` do opóźnionego wyświetlania.

### `GUIManager` - Centralny Zarządca

`GUIManager` pełni rolę głównego zarządcy i "dostawcy kontekstu" dla całej biblioteki.

*   **Zarządzanie cyklem życia:** Przechowuje elementy GUI najwyższego poziomu i jest odpowiedzialny za inicjowanie procesów obsługi zdarzeń (`processEvent`), renderowania (`render`) i czyszczenia (`cleanup`).
*   **Dostawca Kontekstu:** W konstruktorze tworzy instancje `FontManager`, `TextureManager` i `TimerManager`. Przechowuje również wskaźnik na `SDL_Renderer`. Każdy `GUIElement` ma dostęp do `GUIManager`, a przez niego do tych zasobów.
*   **Propagacja zdarzeń:** Metoda `processEvent(const SDL_Event& e)` odbiera zdarzenia z głównej pętli aplikacji i przekazuje je do zarządzanych elementów.
*   **Globalne funkcje:** Zarządza globalnymi elementami, takimi jak dynamicznie tworzone podpowiedzi.

### Managery Zasobów

Aby zoptymalizować użycie pamięci i unikać wielokrotnego ładowania tych samych zasobów, biblioteka wykorzystuje dedykowane managery.

*   **`FontManager`:** Zarządza czcionkami TTF. Cache'uje załadowane czcionki (`std::shared_ptr<TTF_Font>`), zapewniając automatyczne zwalnianie pamięci.
*   **`TextureManager`:** Zarządza teksturami `SDL_Texture`. Cache'uje tekstury (`std::shared_ptr<SDL_Texture>`) i pozwala na dodawanie tekstur stworzonych przez użytkownika (`addTexture`).
*   **`TimerManager`:** Zapewnia bezpieczną obsługę zdarzeń czasowych (timerów) w głównym wątku aplikacji, co jest używane m.in. do implementacji tooltipów.

## Dostępne Komponenty

Biblioteka oferuje zestaw gotowych do użycia, w pełni konfigurowalnych komponentów.

### `Panel`
*   **Przeznaczenie:** Kontener do grupowania innych elementów, idealny jako tło dla okien lub sekcji interfejsu.
*   **Przykład użycia:**
    ```cpp
    auto panel = std::make_unique<Panel>(50, 50, 300, 200);
    panel->setBackgroundColor({200, 200, 200, 255});
    panel->setBorder(2, {100, 100, 100, 255});
    panel->setDraggable(true);
    guiManager.addElement(std::move(panel));
    ```

### `Label`
*   **Przeznaczenie:** Statyczna etykieta tekstowa.
*   **Przykład użycia:**
    ```cpp
    auto label = std::make_unique<Label>(100, 100, "Witaj, świecie!", 24, SDL_Color{0, 0, 0, 255});
    guiManager.addElement(std::move(label));
    ```

### `Button`
*   **Przeznaczenie:** Standardowy przycisk reagujący na kliknięcia.
*   **Przykład użycia:**
    ```cpp
    auto button = std::make_unique<Button>(100, 150, 120, 40, "Kliknij mnie");
    button->setOnClickCallback([](GUIElement*){ 
        // Logika po kliknięciu
    });
    guiManager.addElement(std::move(button));
    ```

### `Checkbox`
*   **Przeznaczenie:** Pole wyboru (zaznaczone/odznaczone).
*   **Przykład użycia:**
    ```cpp
    auto checkbox = std::make_unique<Checkbox>(100, 200, "Zgoda na warunki");
    checkbox->setOnChange([](Checkbox* cb, bool isChecked){
        // Logika po zmianie stanu
    });
    guiManager.addElement(std::move(checkbox));
    ```

### `RadioButton` i `RadioGroup`
*   **Przeznaczenie:** Grupa przycisków, z których tylko jeden może być zaznaczony.
*   **Przykład użycia:**
    ```cpp
    auto radioGroup = std::make_unique<RadioGroup>(100, 250, 200, 100);
    radioGroup->addChild(std::make_unique<RadioButton>(10, 10, "Opcja 1", true)); // Domyślnie zaznaczony
    radioGroup->addChild(std::make_unique<RadioButton>(10, 40, "Opcja 2"));
    guiManager.addElement(std::move(radioGroup));
    ```

### `Slider`
*   **Przeznaczenie:** Suwak do wybierania wartości z przedziału.
*   **Przykład użycia:**
    ```cpp
    auto slider = std::make_unique<Slider>(100, 380, 200, 20, 0, 100);
    slider->setOnChangeCallback([](int value){
        // Logika po zmianie wartości
    });
    guiManager.addElement(std::move(slider));
    ```

### `TextInput`
*   **Przeznaczenie:** Jednoliniowe pole do wprowadzania tekstu.
*   **Przykład użycia:**
    ```cpp
    auto textInput = std::make_unique<TextInput>(100, 420, 200, 30);
    textInput->setOnEnterPressed([](const std::string& text){
        // Logika po wciśnięciu Enter
    });
    guiManager.addElement(std::move(textInput));
    ```

### `TextArea`
*   **Przeznaczenie:** Wieloliniowe pole tekstowe z zawijaniem wierszy.
*   **Przykład użycia:**
    ```cpp
    auto textArea = std::make_unique<TextArea>(400, 50, 300, 200);
    textArea->setText("To jest wieloliniowy\ntekst z obsługą\nzawijania wierszy.");
    textArea->setWordWrap(true);
    guiManager.addElement(std::move(textArea));
    ```

### `ComboBox`
*   **Przeznaczenie:** Rozwijana lista opcji.
*   **Przykład użycia:**
    ```cpp
    auto comboBox = std::make_unique<ComboBox>(400, 280, 150, 30);
    comboBox->addItem("Opcja A");
    comboBox->addItem("Opcja B");
    comboBox->addItem("Opcja C");
    comboBox->on_selection_changed = [](int index, const std::string& item){
        // Logika po wybraniu opcji
    };
    guiManager.addElement(std::move(comboBox));
    ```

### `TabControl`
*   **Przeznaczenie:** Kontener z zakładkami do przełączania widoków.
*   **Przykład użycia:**
    ```cpp
    auto tabControl = std::make_unique<TabControl>(400, 330, 300, 150);
    Panel* tab1 = tabControl->addTab("Zakładka 1");
    tab1->addChild(std::make_unique<Label>(10, 10, "Zawartość pierwszej zakładki."));
    Panel* tab2 = tabControl->addTab("Zakładka 2");
    tab2->addChild(std::make_unique<Button>(10, 10, 100, 30, "Przycisk"));
    guiManager.addElement(std::move(tabControl));
    ```

## Jak Używać

### Inicjalizacja
Zalecanym sposobem jest użycie klasy pomocniczej `SDLApp` (`examples/helpers/sdl_app.hpp`).

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp" 
// ... inne komponenty

SDLApp app("Moja Aplikacja", 800, 600);
GUIManager guiManager(app.getRenderer());
```

### Główna Pętla Aplikacji
Pętla zdarzeń jest odpowiedzialnością aplikacji, co daje większą elastyczność.

```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        guiManager.processEvent(e);
    }

    guiManager.cleanup(); // Bezpieczne usuwanie elementów

    SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
    SDL_RenderClear(app.getRenderer());
    guiManager.render();
    SDL_RenderPresent(app.getRenderer());
}
```

## Kompilacja

Projekt wykorzystuje `Makefile` i *unity build* dla szybkiej kompilacji.

### Wymagania
*   SDL2, SDL2_image, SDL2_ttf
*   Kompilator C++23 (rekomendowany Clang++)
*   `make`

### Polecenia
*   `make all`: Kompiluje przykłady i testy.
*   `make examples`: Kompiluje tylko przykłady.
*   `make test`: Kompiluje i uruchamia testy.
*   `make clean`: Usuwa skompilowane pliki.

Aby uruchomić przykład:
```bash
make examples
./output/example_button
```

## Testowanie

Biblioteka wykorzystuje framework **Catch2** do testów jednostkowych. Testy znajdują się w katalogu `tests/` i można je uruchomić za pomocą `make test`.

## Indeksy dokumentacji
- Archiwum (PL): [docs/pl/archive/README.md](docs/pl/archive/README.md)
- Archive (EN): [docs/en/archive/README.md](docs/en/archive/README.md)