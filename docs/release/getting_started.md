# Getting started

## Instalacja SDL3

Biblioteka wymaga SDL3 oraz dodatków SDL3_image i SDL3_ttf. Zainstaluj pakiety
systemowe tak, aby były widoczne przez `pkg-config`:

```
pkg-config --modversion sdl3 sdl3-image sdl3-ttf
```

Jeśli SDL3 jest zainstalowane w niestandardowej lokalizacji (np. pod
`/usr/local`), ustaw ścieżkę plików `.pc`:

```
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

## Kompilacja i linkowanie

### C++ (biblioteka współdzielona)

```bash
clang++ -std=c++23 -stdlib=libc++ -I dist app.cpp \
    -L dist -lsdl_gui \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
```

`-I dist` wskazuje katalog z `sdl_gui.hpp`; `-L dist -lsdl_gui` linkuje
`libsdl_gui.so`. Jeśli `libsdl_gui.so` nie jest w systemowej ścieżce, przy
uruchomieniu wskaż ją przez `LD_LIBRARY_PATH=dist` (lub skonfiguruj
`ldconfig`).

### C++ (biblioteka statyczna)

```bash
clang++ -std=c++23 -stdlib=libc++ -I dist app.cpp \
    dist/libsdl_gui.a \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
```

Uwaga: `libsdl_gui.a` jest budowana w trybie release z optymalizacjami
`-O3 -flto -march=native`. Kompilując własny plik, użyj tych samych flag
co najmniej dla `-flto` (inaczej linker zgłosi niezgodność jednostek LTO):

```bash
clang++ -std=c++23 -stdlib=libc++ -O3 -flto -I dist app.cpp dist/libsdl_gui.a \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
```

### C (API C)

Kod w czystym C11 kompiluje się kompilatorem C, ale **linkuje** kompilatorem
C++ (biblioteka jest napisana w C++):

```bash
gcc -std=c11 -pedantic-errors -I dist -c app.c -o app.o

clang++ -std=c++23 -stdlib=libc++ app.o \
    -L dist -lsdl_gui \
    $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
```

## Hello World

Kompletny program: okno 800x600, jeden przycisk, który zwiększa licznik
w etykiecie. Kolejność w pętli zdarzeń jest obowiązkowa:
`processEvent` → `update` → `cleanup` → `render`.

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Hello SDL GUI", 800, 600);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());  // KONIECZNE
        manager.setWindowSize(800, 600);                    // dla anchorów

        auto label = manager.create<Label>(350, 240, "Kliknięć: 0");
        auto btn = manager.create<Button>(320, 300, 160, 40, "Kliknij");
        auto ref = manager.makeRef(label);

        btn->setOnClickCallback([ref](GUIElement*) {
            static int count = 0;
            if (ref) ref->setText("Kliknięć: " + std::to_string(++count));
        });

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);    // 1. zdarzenia
            }
            manager.update();               // 2. timery, animacje, tooltipy
            manager.cleanup();              // 3. usuwanie elementów z markForDeletion()
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();               // 4. rysowanie
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

Wyjaśnienie kroków:

- `SDLApp` inicjalizuje SDL3, tworzy okno i renderer (destruktor sprząta sam).
- `GUIManager` zarządza elementami, zdarzeniami i renderowaniem.
- `manager.create<T>(...)` tworzy widget, dodaje go do managera i zwraca
  surowy wskaźnik.
- `manager.makeRef(label)` tworzy bezpieczne odniesienie do elementu do użycia
  w callbacku — patrz [patterns.md](patterns.md#3-komunikacja-między-widgetami-callbacki-i-elementref).

## Wersja z GUIContext

`GUIContext` łączy `SDLApp` + `GUIManager` + motyw w jeden obiekt RAII
(motyw domyślny jest ustawiany automatycznie):

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        GUIContext ctx("Hello SDL GUI", 800, 600);
        GUIManager& manager = ctx.getGUIManager();

        auto label = manager.create<Label>(350, 240, "Kliknięć: 0");
        auto btn = manager.create<Button>(320, 300, 160, 40, "Kliknij");
        auto ref = manager.makeRef(label);

        btn->setOnClickCallback([ref](GUIElement*) {
            static int count = 0;
            if (ref) ref->setText("Kliknięć: " + std::to_string(++count));
        });

        ctx.run();  // cała pętla zdarzeń wewnątrz
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

## Wersja z SDLApp::run()

`SDLApp::run(manager, clearColor, onEvent)` hermetyzuje całą pętlę
(PollEvent → processEvent → update → cleanup → clear → render → present).
Opcjonalny trzeci argument to callback `void(SDL_Event&)` wołany po
`processEvent` — użyj go m.in. do obsługi resize:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Hello SDL GUI", 800, 600, true);  // resizable
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());
        manager.setWindowSize(800, 600);

        // ... tworzenie widgetów ...

        app.run(manager, {40, 42, 54, 255}, [&manager](SDL_Event& e) {
            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                manager.handleResize(e.window.data1, e.window.data2);
            }
        });
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

## Uruchamianie i zasoby

- Aplikacja szuka fontów i tekstur w bieżącym katalogu roboczym lub przez
  `TextureManager`/`FontManager` (ścieżki względne). Jeśli biblioteka została
  zbudowana z osadzonymi assetami (embedded assets), zasoby są ładowane
  transparentnie przez te same ścieżki bez plików na dysku — szczegóły
  w [resources.md](resources.md).
- Przy pierwszym uruchomieniu sprawdź, czy okno się pojawia i czy widgety są
  widoczne — jeśli nie, patrz niżej.

## Rozwiązywanie problemów

| Problem | Przyczyna i rozwiązanie |
|---------|--------------------------|
| Okno się otwiera, ale widgetów nie widać | Brak motywu — wywołaj `manager.setTheme(ThemePresets::createDarkTheme())` (lub inny preset, albo `Theme::createDefaultTheme()`). Bez motywu style są puste i elementy nie mają kolorów. |
| Tooltipy nie znikają / animacje stoją | Brak `manager.update()` w pętli zdarzeń. |
| Elementy oznaczone `markForDeletion()` pozostają | Brak `manager.cleanup()` w pętli. |
| Aplikacja nie startuje: `SDL_Init failed` / `TTF_Init failed` | SDL3 lub SDL3_ttf niepoprawnie zainstalowane — sprawdź `pkg-config --modversion sdl3 sdl3-ttf` i `PKG_CONFIG_PATH`. |
| Błąd linkowania `-lsdl_gui` | Ustaw `-L dist`; przy statycznej `libsdl_gui.a` podaj pełną ścieżkę do pliku. |
