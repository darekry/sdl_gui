# Technologie i narzędzia

## Środowisko i zależności

Projekt jest napisany w C++ i opiera się na następujących bibliotekach:
-   **SDL3**: Główna biblioteka do obsługi okien, zdarzeń i renderowania (GPU).
-   **SDL3_image**: Rozszerzenie do ładowania różnych formatów obrazów (on-demand, bez IMG_Init).
-   **SDL3_ttf**: Rozszerzenie do renderowania czcionek TrueType.
-   **tinyxml2**: Parsowanie XML (wbudowane w `lib/tinyxml2.cpp`).
-   **Catch2**: Framework do testów jednostkowych (wersja amalgamated w [`lib/`](lib/:1)).

### Instalacja zależności
SDL3 instaluje się ze źródeł lub przez pkg-config. Biblioteki muszą być dostępne przez `pkg-config sdl3 sdl3-image sdl3-ttf`.
Kompilator: `clang++-22` z `libc++` (LLVM-23).

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
-   `nob` / `nob examples`: Kompiluje wszystkie przykłady z katalogu [`examples/`](examples/:1) do katalogu `output/`. Automatycznie osadza assety z `assets.embed` do każdego binarka.
-   `nob test`: Kompiluje i uruchamia wszystkie testy jednostkowe z katalogu [`tests/`](tests/:1).
-   `nob release`: Buduje biblioteki statyczne/dynamiczne i połączony header w katalogu `dist/`.
-   `nob clean`: Usuwa skompilowane pliki (`output/`, `dist/`, `modules_cache/`).

**compile_commands.json** jest generowany automatycznie podczas każdego buildu (jeśli były kompilacje).

### Standard C++ i kompilator
-   **Standard**: Projekt używa **C++23** (`-std=c++23`).
-   **Kompilator**: `clang++-22` z biblioteką standardową `libc++` (LLVM-23).
-   **Moduły**: Prekompilowane moduły `std.pcm` i `std.compat.pcm` z `/usr/lib/llvm-23/share/libc++/v1/`.
-   **PKG_CONFIG_PATH**: `/usr/local/lib/pkgconfig` dla SDL3.

## Techniki i optymalizacje

-   **Unity Build**: Domyślna kompilacja (`./nob examples`) wykorzystuje technikę "unity build". Wszystkie pliki `.cpp` z `src/`, `src/composite/` i `src/editor/` są łączone w jeden plik (`output/all.cpp`), co znacząco przyspiesza proces kompilacji.
-   **Optymalizacje kompilatora**: Używane są flagi `-O3`, `-march=native` oraz `-flto` (Link-Time Optimization) w trybie release. Debug mode używa `-g -O0 -fsanitize=address,undefined`.
-   **Moduły C++23**: Prekompilowane moduły `std.pcm` i `std.compat.pcm` w `modules_cache/`, budowane z plików `.cppm` LLVM.
-   **Nob_Procs**: Kompilacja przykładów i testów używa równoległego linkowania przez `Nob_Procs`.
-   **Zarządzanie pamięcią**: Biblioteka intensywnie korzysta z inteligentnych wskaźników:
    -   `std::unique_ptr` do zarządzania hierarchią i cyklem życia elementów GUI.
    -   `std::shared_ptr` (`SharedTexture`, `SharedFont`) z niestandardowymi deleterami ([`src/sdl_deleters.hpp`](src/sdl_deleters.hpp:1)) do automatycznego zwalniania zasobów SDL.
-   **GPU Renderer**: SDL_gpu używany przez ShaderPanel i elementy wymagające shaderów. Wymaga `SDL_CreateGPURenderer` (nie `SDL_CreateRenderer`).
-   **Cache'owanie zasobów**: `TextureManager` i `FontManager` przechowują załadowane zasoby w mapach, aby uniknąć wielokrotnego ładowania tych samych plików.
-   **Cache'owanie renderowania**: Każdy `GUIElement` domyślnie renderuje swoją zawartość do osobnej tekstury (`m_cachedTexture`), która jest odświeżana tylko w razie potrzeby (`m_isDirty = true`). To kluczowa optymalizacja, która minimalizuje liczbę operacji rysowania.
-   **Embedded Assets**: System osadzania plików binarnych (PNG, TTF) bezpośrednio w ELF:
    -   `ld -r -b binary` tworzy plik `.o` z symbolami `_binary_<nazwa>_start` / `_end` / `_size`
    -   `SDL_IOFromConstMem` → `IMG_Load_IO` / `TTF_OpenFontIO` do ładowania z pamięci
    -   Manifest `assets.embed` definiuje listę plików do osadzenia
    -   Auto-generowany header `output/embedded_assets.hpp` z tablicą `g_embeddedAssets[]`

## Narzędzia deweloperskie

-   **System budowania**: `nob.c` z `nob.h` (Go Rebuild Urself™ Technology - automatyczne przebudowanie).
-   **Analiza kodu**: `clangd` lub inne narzędzia oparte na LSP, które wykorzystują plik [`compile_commands.json`](compile_commands.json:1) do precyźnej analizy kodu. Generowany automatycznie podczas buildów.
-   **Testowanie**: `Catch2` do testów jednostkowych.
-   **Formatowanie kodu**: Projekt zawiera plik `.clang-format`, który definiuje styl formatowania kodu.