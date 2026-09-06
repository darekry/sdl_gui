# Pierwsze kroki z biblioteką SDL2 GUI

Ten przewodnik przeprowadzi Cię przez konfigurację podstawowego projektu SDL2 GUI, od instalacji zależności po uruchomienie prostej aplikacji "Hello World" z przyciskiem.

## 1. Wymagania systemowe i zależności

Biblioteka SDL2 GUI opiera się na SDL2 i jej bibliotekach rozszerzających. Będziesz musiał zainstalować je w swoim systemie.

### Instalacja w systemach opartych na Debianie/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev clang libc++-dev
```

W przypadku innych systemów operacyjnych, zapoznaj się z oficjalną dokumentacją SDL2 w celu uzyskania instrukcji instalacji dla `SDL2`, `SDL2_image` i `SDL2_ttf`. Będziesz także potrzebował kompilatora zgodnego z C++23 (np. Clang lub GCC).

## 2. Struktura projektu

W przypadku minimalnego projektu, zazwyczaj będziesz mieć pliki źródłowe (np. `main.cpp`) i linkować je z biblioteką SDL2 GUI.

```
twoj_projekt/
├── main.cpp
└── (link do biblioteki sdl_gui i plików nagłówkowych)
```

Będziesz potrzebował dostępu do plików nagłówkowych biblioteki SDL2 GUI (z katalogu `src/` biblioteki) oraz skompilowanego pliku biblioteki (np. `libsdl_gui.a` lub `libsdl_gui.so`).

## 3. Minimalny przykład "Hello World"

Oto prosty przykład, który inicjuje SDL, tworzy okno i wyświetla przycisk. Po kliknięciu przycisk wypisze wiadomość w konsoli.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "label.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Inicjalizacja SDL i utworzenie okna
    SDLApp app("Witaj SDL2 GUI", 800, 600);

    // Utworzenie menedżera GUI
    GUIManager guiManager(app.getRenderer());

    // Utworzenie przycisku
    auto button = std::make_unique<Button>(300, 250, 200, 50, "Kliknij mnie!");
    button->setOnClickCallback([](GUIElement*) {
        std::cout << "Przycisk kliknięty!" << std::endl;
    });
    guiManager.addElement(std::move(button));

    // Utworzenie etykiety
    auto label = std::make_unique<Label>(300, 150, "Witaj, SDL2 GUI!", 32, SDL_Color{0, 0, 0, 255});
    guiManager.addElement(std::move(label));

    // Główna pętla aplikacji
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Przekazywanie zdarzeń do GUI
            guiManager.processEvent(e);
        }

        // Bezpieczne usuwanie elementów i konserwacja
        guiManager.cleanup();

        SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
        SDL_RenderClear(app.getRenderer());
        guiManager.render();
        SDL_RenderPresent(app.getRenderer());
    }

    return 0;
}
```

## 4. Kompilacja i linkowanie

Aby skompilować i linkować aplikację z biblioteką SDL2 GUI, należy podać ścieżkę do plików nagłówkowych i pliku biblioteki, wraz z niezbędnymi zależnościami SDL2.

Zakładając, że skompilowałeś bibliotekę SDL2 GUI i masz dostępne `libsdl_gui.a` (statyczną) lub `libsdl_gui.so` (dynamiczną) oraz nagłówki z `src/`.

### Linkowanie statyczne (`.a`)

Podczas linkowania z biblioteką statyczną (`libsdl_gui.a`), należy podać ścieżkę do plików nagłówkowych i pliku biblioteki, wraz z niezbędnymi zależnościami SDL2.

```bash
g++ your_app.cpp -o your_app -I/sciezka/do/sdl_gui/src -L/sciezka/do/sdl_gui/output -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Zastąp `/sciezka/do/sdl_gui/src` rzeczywistą ścieżką do katalogu `src` biblioteki SDL2 GUI, a `/sciezka/do/sdl_gui/output` ścieżką, w której znajduje się `libsdl_gui.a` (zazwyczaj `output/` w katalogu głównym biblioteki).

### Linkowanie dynamiczne (`.so`)

Podczas linkowania z biblioteką współdzieloną (`libsdl_gui.so`), upewnij się, że linker może ją znaleźć w czasie wykonania.

```bash
g++ your_app.cpp -o your_app -I/sciezka/do/sdl_gui/src -L/sciezka/do/sdl_gui/output -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Upewnij się, że plik `.so` znajduje się w katalogu znanym linkerowi dynamicznemu (np. `/usr/local/lib`) lub ustaw zmienną środowiskową `LD_LIBRARY_PATH`:

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/sciezka/do/sdl_gui/output
./your_app
```

Teraz powinieneś być w stanie skompilować i uruchomić swoją pierwszą aplikację SDL2 GUI!