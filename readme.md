# Biblioteka GUI dla SDL2

## Opis

Prosta i rozszerzalna biblioteka GUI zbudowana w oparciu o SDL2 w standardzie C++20, umożliwiająca łatwe tworzenie interaktywnych elementów interfejsu użytkownika. Biblioteka została zaprojektowana z myślą o spójności, bezpieczeństwie pamięci i łatwości użycia.

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

Elementy GUI nie przechowują bezpośrednio wskaźnika do `SDL_Renderer`. Zamiast tego, `GUIManager` jest inicjalizowany z potrzebnym kontekstem (renderer, managery zasobów), a następnie przekazuje wskaźnik na samego siebie w dół drzewa hierarchii. Każdy element może w dowolnym momencie "sięgnąć" w górę do `GUIManager`, aby uzyskać dostęp do renderera lub managerów, np. w celu dynamicznego utworzenia tekstury.

### 5. Odroczone Usuwanie (Deferred Deletion)

Aby uniknąć problemów z usuwaniem elementów w trakcie iteracji (np. przycisk usuwający okno, w którym się znajduje), biblioteka implementuje mechanizm odroczonego usuwania. Elementy są najpierw oznaczane do usunięcia (`markForDeletion()`), a następnie faktycznie usuwane w bezpiecznym momencie przez `GUIManager::cleanup()`.

## Użycie Biblioteki

### Inicjalizacja

```cpp
// Inicjalizacja SDL
SDL_Init(SDL_INIT_VIDEO);
SDL_Window* window = SDL_CreateWindow(...);
SDL_Renderer* renderer = SDL_CreateRenderer(...);

// Inicjalizacja managerów
TextureManager textureManager(renderer);
FontManager fontManager(renderer);
GUIManager guiManager(renderer, &fontManager, &textureManager);
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

`GUIManager::handleEvents()` centralizuje pętlę zdarzeń i zwraca `true`, jeśli aplikacja powinna zostać zamknięta (np. przez `SDL_QUIT`).

```cpp
bool quit = false;
while (!quit) {
    // 1. Obsługa zdarzeń
    quit = guiManager.handleEvents();

    // 2. Logika aplikacji (opcjonalnie)
    // ...

    // 3. Czyszczenie usuniętych elementów
    guiManager.cleanup();

    // 4. Renderowanie
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderClear(renderer);
    guiManager.render(); // Renderer jest już znany
    SDL_RenderPresent(renderer);
}
```

## Kompilacja i Uruchamianie

Projekt wykorzystuje `Makefile` z techniką *unity build* do szybkiej kompilacji.

### Wymagania

-   SDL2, SDL2_image, SDL2_ttf
-   Kompilator C++20 (rekomendowany Clang++)
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

## Testowanie

Biblioteka wykorzystuje framework **Catch2** do testów jednostkowych. Testy znajdują się w katalogu `tests/` i można je uruchomić za pomocą polecenia `make test`.