# Biblioteka GUI dla SDL2

## Opis

Prosta i rozszerzalna biblioteka GUI zbudowana w oparciu o SDL2 w standardzie C++23, umożliwiająca łatwe tworzenie interaktywnych elementów interfejsu użytkownika. Biblioteka została zaprojektowana z myślą o spójności, bezpieczeństwie pamięci i łatwości użycia.

## Jak Używać

### Inicjalizacja
Aby rozpocząć, dołącz główny plik nagłówkowy `gui_manager.hpp` oraz nagłówki potrzebnych widżetów. Zalecanym sposobem jest użycie klasy pomocniczej `sdl_app.hpp`.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp" 
// ... inne komponenty

// Inicjalizacja SDL i okna
SDLApp app("Moja Aplikacja", 800, 600);

// Stworzenie managera GUI
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

## Linkowanie Biblioteki

Aby użyć biblioteki, musisz zlinkować ją ze swoją aplikacją. Poniżej znajdują się przykłady dla linkowania statycznego i dynamicznego.

### Linkowanie Statyczne (`.a`)

Podczas linkowania z biblioteką statyczną (`libsdl_gui.a`) należy podać ścieżkę do plików nagłówkowych i pliku biblioteki, a także dołączyć wymagane zależności SDL2.

```bash
g++ twoja_aplikacja.cpp -o twoja_aplikacja -I/sciezka/do/naglowkow -L/sciezka/do/biblioteki -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

### Linkowanie Dynamiczne (`.so`)

Podczas linkowania z biblioteką współdzieloną (`libsdl_gui.so`) upewnij się, że linker będzie w stanie ją znaleźć w czasie wykonania.

```bash
g++ twoja_aplikacja.cpp -o twoja_aplikacja -I/sciezka/do/naglowkow -L/sciezka/do/biblioteki -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Upewnij się, że plik `.so` znajduje się w katalogu znanym linkerowi dynamicznemu (np. `/usr/local/lib`) lub ustaw zmienną środowiskową `LD_LIBRARY_PATH`.

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

## Indeksy dokumentacji
- Archiwum (PL): [docs/pl/archive/README.md](docs/pl/archive/README.md)
- Archive (EN): [docs/en/archive/README.md](docs/en/archive/README.md)