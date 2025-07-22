# Biblioteka GUI dla SDL2

## Opis

Prosta i rozszerzalna biblioteka GUI zbudowana w oparciu o SDL2 w standardzie C++23, umożliwiająca łatwe tworzenie interaktywnych elementów interfejsu użytkownika. Biblioteka została zaprojektowana z myślą o spójności, bezpieczeństwie pamięci i łatwości użycia.

## Kluczowe Koncepcje Architektoniczne

### 1. `GUIManager` - Centralny Punkt Zarządzania

`GUIManager` jest sercem biblioteki. Odpowiada za:
-   Przechowywanie elementów GUI najwyższego poziomu.
-   Propagację zdarzeń SDL do odpowiednich elementów.
-   Renderowanie całego interfejsu.
-   Zarządzanie cyklem życia elementów, w tym ich bezpieczne usuwanie.
-   Dostarczanie kontekstu (Renderer, Managery zasobów) do wszystkich elementów w drzewie GUI.

### 2. `GUIElement` - Klasa Bazowa

Wszystkie elementy interfejsu (przyciski, panele, suwaki itp.) dziedziczą po `GUIElement`. Klasa ta definiuje:
-   Podstawowe właściwości (pozycja, rozmiar, widoczność, stan włączenia).
-   Hierarchię rodzic-dziecko, umożliwiając budowanie złożonych interfejsów.
-   Wirtualne metody do obsługi zdarzeń (`handleEvent`) i renderowania (`render`).
-   Mechanizm przeciągania (`setDraggable(true)`).

### 3. Zarządzanie Pamięcią

Biblioteka intensywnie wykorzystuje inteligentne wskaźniki, aby zapewnić bezpieczeństwo i automatyczne zarządzanie pamięcią:
-   `std::unique_ptr`: Używany do relacji własności, gdzie jeden obiekt jest wyłącznym właścicielem drugiego (np. `GUIManager` jest właścicielem elementów, a element-rodzic jest właścicielem swoich dzieci).
-   `std::shared_ptr`: Używany do zasobów współdzielonych, takich jak tekstury (`SharedTexture`) i czcionki (`SharedFont`), aby uniknąć ich duplikacji.

### 4. Dostęp do Kontekstu Renderowania

Elementy GUI nie przechowują bezpośrednio wskaźnika do `SDL_Renderer`. Zamiast tego, `GUIManager` jest inicjalizowany z potrzebnym kontekstem (renderer), a następnie przekazuje wskaźnik na samego siebie w dół drzewa hierarchii. Każdy element może w dowolnym momencie "sięgnąć" w górę do `GUIManager`, aby uzyskać dostęp do renderera lub managerów, np. w celu dynamicznego utworzenia tekstury.

### 5. Odroczone Usuwanie (Deferred Deletion)

Aby uniknąć problemów z usuwaniem elementów w trakcie iteracji (np. przycisk usuwający okno, w którym się znajduje), biblioteka implementuje mechanizm odroczonego usuwania. Elementy są najpierw oznaczane do usunięcia (`markForDeletion()`), a następnie faktycznie usuwane w bezpiecznym momencie przez `GUIManager::cleanup()`.

## Dostępne Komponenty (Widgety)

Biblioteka oferuje zestaw gotowych do użycia komponentów:

-   **`Panel`**: Kontener do grupowania innych elementów. Może służyć jako tło lub proste okno. Posiada opcję przeciągania.
-   **`Button`**: Standardowy przycisk z tekstem, który reaguje na kliknięcie.
-   **`Checkbox`**: Pole wyboru, które może być zaznaczone lub odznaczone.
-   **`Slider`**: Suwak pozwalający na wybór wartości z określonego przedziału.
-   **`TextInput`**: Jednoliniowe pole do wprowadzania tekstu przez użytkownika.
-   **`TextArea`**: Wieloliniowe pole tekstowe, przydatne do wyświetlania większych bloków tekstu z zawijaniem wierszy.
-   **`RadioButton`** i **`RadioGroup`**: Przyciski opcji, które pozwalają na dokonanie jednego wyboru w ramach grupy.
-   **`ComboBox`**: Rozwijana lista, z której użytkownik może wybrać jedną opcję.
-   **`TabControl`**: Kontener z zakładkami, umożliwiający przełączanie się między różnymi widokami/grupami elementów.

## Użycie Biblioteki

### Inicjalizacja

Zalecanym sposobem inicjalizacji jest użycie klasy pomocniczej `SDLApp` (dostępnej w `examples/helpers/sdl_app.hpp`), która upraszcza zarządzanie oknem i rendererem. Następnie `GUIManager` jest inicjalizowany z posiadanym rendererem.

```cpp
#include "helpers/sdl_app.hpp"
#include "gui_manager.hpp"

// Inicjalizacja za pomocą klasy pomocniczej
SDLApp app("Moja Aplikacja", 800, 600);
GUIManager guiManager(app.getRenderer());
```

### Tworzenie i Dodawanie Elementów

Elementy tworzy się za pomocą `std::make_unique` i dodaje do managera lub innego elementu za pomocą `std::move`.

```cpp
// Tworzenie panelu (okna)
auto window_panel = std::make_unique<Panel>(100, 100, 400, 300);
window_panel->setDraggable(true); // Umożliwia przeciąganie

// Tworzenie przycisku
auto my_button = std::make_unique<Button>(150, 200, 100, 50, "Kliknij!");
my_button->setOnClick([]() {
    std::cout << "Przycisk kliknięty!" << std::endl;
});

// Dodanie przycisku jako dziecko panelu
GUIElement* panel_ptr = window_panel.get(); // Pobranie surowego wskaźnika przed przeniesieniem
panel_ptr->addChild(std::move(my_button));

// Dodanie panelu do GUIManager
guiManager.addElement(std::move(window_panel));
```

### Główna Pętla Aplikacji

W nowej architekturze pętla zdarzeń jest odpowiedzialnością aplikacji, co daje większą elastyczność. `GUIManager` nie kontroluje już pętli, lecz jedynie przetwarza przekazywane do niego zdarzenia.

```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    // 1. Przetwarzanie zdarzeń w pętli
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        // Przekaż zdarzenie do GUIManager
        guiManager.processEvent(e);
    }

    // 2. Logika aplikacji (opcjonalnie)
    // ...

    // 3. Czyszczenie usuniętych elementów
    guiManager.cleanup();

    // 4. Renderowanie
    SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
    SDL_RenderClear(app.getRenderer());
    guiManager.render();
    SDL_RenderPresent(app.getRenderer());
}
```

## Kompilacja i Uruchamianie

Projekt wykorzystuje `Makefile` z techniką *unity build* do szybkiej kompilacji.

### Wymagania

-   SDL2, SDL2_image, SDL2_ttf
-   Kompilator C++23 (rekomendowany Clang++)
-   `make`

### Dostępne Polecenia

-   `make all`: Kompiluje przykłady i testy (domyślne).
-   `make examples`: Kompiluje tylko pliki z katalogu `examples/`.
-   `make test`: Kompiluje i uruchamia wszystkie testy z katalogu `tests/`.
-   `make clean`: Usuwa wszystkie skompilowane pliki.

Aby uruchomić konkretny przykład:
```bash
make examples
./output/example_button
```

## Integracja z Własnym Projektem

Aby użyć biblioteki w swoim projekcie, masz dwie główne opcje:

### Opcja 1: Kompilacja do biblioteki statycznej (zalecane)

1.  **Skompiluj bibliotekę**: Możesz zmodyfikować `Makefile`, aby tworzył bibliotekę statyczną (`libsdl_gui.a`).
    ```makefile
    # Przykład docelowej reguły w Makefile
    lib:
        $(CXX) $(CXXFLAGS) -c $(UNITY_SRC) -o unity.o
        ar rcs output/libsdl_gui.a unity.o
    ```
2.  **Linkowanie w Twoim projekcie**:
    -   Dodaj katalog `src` do ścieżek include (`-I/sciezka/do/sdl_gui/src`).
    -   Dodaj bibliotekę do linkera (`-L/sciezka/do/sdl_gui/output -lsdl_gui`).

### Opcja 2: Bezpośrednie dołączenie źródeł

1.  **Skopiuj katalog `src`** do swojego projektu.
2.  **Dodaj pliki `*.cpp`** z tego katalogu do swojego systemu budowania (np. Makefile, CMake).
3.  **Upewnij się, że katalog `src` jest w ścieżkach include (`-I` w flagach kompilatora).

W obu przypadkach musisz również linkować zależności: `SDL2`, `SDL2_image` i `SDL2_ttf`.

```bash
# Przykładowa flaga kompilacji dla Twojego projektu
g++ -std=c++23 -I/path/to/sdl_gui/src your_app.cpp -L/path/to/sdl_gui/output -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf -o my_app
```

## Testowanie

Biblioteka wykorzystuje framework **Catch2** do testów jednostkowych. Testy znajdują się w katalogu `tests/` i można je uruchomić za pomocą polecenia `make test`.