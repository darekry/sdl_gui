# Biblioteka GUI dla SDL2

## Opis

Prosta biblioteka GUI zbudowana w oparciu o SDL2, umożliwiająca tworzenie interaktywnych elementów interfejsu użytkownika, takich jak przyciski, panele i suwaki.

## Użycie biblioteki

Biblioteka składa się z kilku kluczowych komponentów: `GUIManager`, elementów GUI (takich jak `Button`, `Panel`, `Slider`) oraz `TextureManager`.

### Menedżer GUI (`GUIManager`)

`GUIManager` jest głównym punktem wejścia do zarządzania elementami GUI.

Inicjalizacja:
```cpp
GUIManager gui_manager;
```

Dodawanie elementów:
Elementy GUI są dodawane do menedżera za pomocą metody [`addElement()`](src/gui_manager.hpp:17).
```cpp
gui_manager.addElement(std::unique_ptr<GUIElement>(new Button(...)));
```

Obsługa zdarzeń:
Menedżer obsługuje zdarzenia SDL i przekazuje je do elementów GUI za pomocą metody [`handleEvents()`](src/gui_manager.hpp:23).
```cpp
SDL_Event e;
while (SDL_PollEvent(&e) != 0) {
    gui_manager.handleEvents(e);
    // ... inne zdarzenia
}
```

Renderowanie:
Menedżer renderuje wszystkie elementy za pomocą metody [`render()`](src/gui_manager.hpp:26), przyjmując wskaźnik na `SDL_Renderer`.
```cpp
gui_manager.render(renderer);
```

### Elementy GUI (`GUIElement`, `Button`, `Panel`, `Slider`)

Klasa bazowa [`GUIElement`](src/gui.hpp:9) definiuje podstawowe metody do zarządzania pozycją, rozmiarem, relacją rodzic-dziecko oraz wirtualne metody do obsługi zdarzeń (`handleEvent`) i renderowania (`render`). Klasy pochodne implementują specyficzne zachowania.

Elementy są tworzone dynamicznie, zazwyczaj jako wskaźniki zarządzane przez `std::unique_ptr` lub `std::shared_ptr`.

Przykładowe tworzenie suwaka:
```cpp
std::unique_ptr<Slider> slider = std::make_unique<Slider>(/* argumenty */);
gui_manager.addElement(std::move(slider));
```

### Pole tekstowe (`TextInput`)

Klasa [`TextInput`](src/text_input.hpp:9) reprezentuje pole tekstowe, w którym użytkownik może wprowadzać i edytować tekst. Jest to interaktywny element GUI, który przechwytuje zdarzenia klawiatury i myszy w celu umożliwienia wprowadzania danych.

### Menedżer tekstur (`TextureManager`)

`TextureManager` służy do ładowania i zarządzania teksturami używanymi przez elementy GUI.

Inicjalizacja:
```cpp
TextureManager texture_manager(renderer);
```

Ładowanie tekstur:
Tekstury są ładowane za pomocą metody [`loadTexture()`](src/texture_manager.hpp:32), która zwraca współdzielony wskaźnik (`SharedTexture`).
```cpp
SharedTexture button_texture = texture_manager.loadTexture("assets/button.png");
```

## Kompilacja

Projekt wykorzystuje plik [`Makefile`](Makefile) do automatyzacji procesu kompilacji.

### Wymagania wstępne

Do kompilacji projektu potrzebne są następujące biblioteki i narzędzia:
*   SDL2
*   SDL2_image
*   SDL2_ttf
*   Kompilator C++ zgodny ze standardem C++20 (np. clang++)
*   make

Upewnij się, że biblioteki SDL2, SDL2_image i SDL2_ttf są zainstalowane w systemie i dostępne dla `sdl2-config`.

### Instrukcje kompilacji

1.  Otwórz terminal w głównym katalogu projektu (`/home/darekry/code/sdl_gui`).
2.  Użyj polecenia `make` z odpowiednim celem:

    *   **`make all`**: Kompiluje pliki źródłowe (`.c`, `.cpp`) z katalogu `src/`, tworzy pliki obiektowe (`.o`) w katalogu `output/`, a następnie linkuje je do pliku wykonywalnego `main.exe` w katalogu `output/`. Jest to domyślny cel, uruchamiany po prostu przez `make`.
    *   **`make clean`**: Usuwa katalog `output/` wraz ze wszystkimi skompilowanymi plikami.
    *   **`make run`**: Kompiluje projekt (jeśli nie jest skompilowany) i uruchamia plik wykonywalny `output/main.exe`.
    *   **`make debug`**: Dodaje flagę `-DDEBUG` do flag kompilacji CFLAGS, kompiluje projekt i uruchamia go w debuggerze `gdb`.

Przykład użycia:
```bash
make all
# lub po prostu
make
```

Aby uruchomić skompilowany program:
```bash
make run
```

Aby wyczyścić skompilowane pliki:
```bash
make clean