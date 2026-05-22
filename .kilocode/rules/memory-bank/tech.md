# Technologie i narzędzia

## Środowisko i zależności

Projekt jest napisany w C++ i opiera się na następujących bibliotekach:
-   **SDL2**: Główna biblioteka do obsługi okien, zdarzeń i renderowania.
-   **SDL2_image**: Rozszerzenie do ładowania różnych formatów obrazów.
-   **SDL2_ttf**: Rozszerzenie do renderowania czcionek TrueType.
-   **SDL2_gfx**: Rozszerzenie do rysowania figur geometrycznych (zaokrąglone rogi).
-   **Catch2**: Framework do testów jednostkowych (wersja amalgamated w [`lib/`](lib/:1)).

### Instalacja zależności (przykład dla systemów Debian/Ubuntu):
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev clang libc++-dev
```

## Proces budowania

Projekt wykorzystuje `nob.c` (skrypt budujący w C) z biblioteką [`nob.h`](nob.h:1) (v3.8.0) do automatyzacji kompilacji.

### Bootstrap i uruchomienie:
```bash
# Bootstrap (kompilacja nob.c - wykonaj raz)
cc -o nob nob.c

# Budowanie
./nob              # build examples (debug)
./nob examples     # build examples
./nob test         # build + run tests
./nob release      # build dist/ artifacts
./nob clean        # remove output/, dist/, modules_cache/
./nob non_unity    # compile each .cpp separately
./nob -r examples  # build examples (release mode)
```

### Główne cele:
-   `nob` / `nob examples`: Kompiluje wszystkie przykłady z katalogu [`examples/`](examples/:1) do katalogu `output/`.
-   `nob test`: Kompiluje i uruchamia wszystkie testy jednostkowe z katalogu [`tests/`](tests/:1).
-   `nob release`: Buduje biblioteki statyczne/dynamiczne i połączony header w katalogu `dist/`.
-   `nob clean`: Usuwa skompilowane pliki (`output/`, `dist/`, `modules_cache/`).

**compile_commands.json** jest generowany automatycznie podczas każdego buildu (jeśli były kompilacje).

### Standard C++ i kompilator
-   **Standard**: Projekt używa **C++23** (`-std=c++23`).
-   **Kompilator**: Domyślnie skonfigurowany jest `clang++-22` z biblioteką standardową `libc++`.

## Techniki i optymalizacje

-   **Unity Build**: Domyślna kompilacja (`./nob examples`) wykorzystuje technikę "unity build". Wszystkie pliki `.cpp` z katalogu [`src/`](src/:1) są łączone w jeden duży plik (`output/all.cpp`), co znacząco przyspiesza proces kompilacji.
-   **Optymalizacje kompilatora**: Używane są flagi `-O3`, `-march=native` (optymalizacje pod architekturę maszyny budującej) oraz `-flto` (Link-Time Optimization) w trybie release.
-   **Moduły C++23**: Projekt eksperymentalnie wykorzystuje prekompilowane moduły dla biblioteki standardowej (`std.pcm`, `std.compat.pcm`), co może dodatkowo przyspieszyć kompilację.
-   **Zarządzanie pamięcią**: Biblioteka intensywnie korzysta z inteligentnych wskaźników:
    -   `std::unique_ptr` do zarządzania hierarchią i cyklem życia elementów GUI.
    -   `std::shared_ptr` (`SharedTexture`, `SharedFont`) z niestandardowymi deleterami ([`src/sdl_deleters.hpp`](src/sdl_deleters.hpp:1)) do automatycznego zwalniania zasobów SDL.
-   **Cache'owanie zasobów**: `TextureManager` i `FontManager` przechowują załadowane zasoby w mapach, aby uniknąć wielokrotnego ładowania tych samych plików.
-   **Cache'owanie renderowania**: Każdy `GUIElement` domyślnie renderuje swoją zawartość do osobnej tekstury (`m_cachedTexture`), która jest odświeżana tylko w razie potrzeby (`m_isDirty = true`). To kluczowa optymalizacja, która minimalizuje liczbę operacji rysowania.

## Narzędzia deweloperskie

-   **System budowania**: `nob.c` z `nob.h` (Go Rebuild Urself™ Technology - automatyczne przebudowanie).
-   **Analiza kodu**: `clangd` lub inne narzędzia oparte na LSP, które wykorzystują plik [`compile_commands.json`](compile_commands.json:1) do precyzyjnej analizy kodu. Generuj przez `./nob compile_commands`.
-   **Testowanie**: `Catch2` do testów jednostkowych.
-   **Formatowanie kodu**: Projekt zawiera plik `.clang-format`, który definiuje styl formatowania kodu.