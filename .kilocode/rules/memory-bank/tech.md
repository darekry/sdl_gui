Technologie i narzędzia

Środowisko i zależności:
- Biblioteka bazuje na SDL2 oraz rozszerzeniach: `SDL_image` i `SDL_ttf`.
- Pliki źródłowe w C++ (standard zgodny z projektem; patrz [`Makefile`](Makefile:1) i [`compile_commands.json`](compile_commands.json:1)).
- Testy jednostkowe wykorzystują Catch2 (pliki w [`tests/`](tests/:1) i [`lib/catch_amalgamated.hpp`](lib/catch_amalgamated.hpp:1)).

Budowanie:
- `Makefile` dostępny w repozytorium (`Makefile`) uruchamia kompilację przykładów i testów.
- [`compile_commands.json`](compile_commands.json:1) jest obecny w repozytorium i może być użyty przez narzędzia typu clangd / IDE do analizy kodu.
- Wskazówki:
  - Upewnić się, że biblioteki deweloperskie SDL2, SDL2_image i SDL2_ttf są zainstalowane w systemie przed kompilacją.
  - Jeżeli potrzebne, użyć skryptu [`tools/download_sdl_deps.sh`](tools/download_sdl_deps.sh:1) do automatycznego pobrania zależności w środowarach CI lub lokalnym.

Techniki i optymalizacje:
- Cache'owanie zasobów (tekstur, czcionek) używa `SharedTexture`/`SharedFont` (`std::shared_ptr`) z niestandardowymi deleterami ([`src/sdl_deleters.hpp`](src/sdl_deleters.hpp:1)).
- `GUIElement` używa lokalnego cache'u renderu (`m_cachedTexture`) jako texture target, aby ograniczyć koszty redrawu.
- Prostota projektu preferuje czytelność nad złożonymi wzorcami. Możliwe do rozważenia optymalizacje:
  - Unity Build (scalanie plików źródłowych) dla szybszej kompilacji w CI; upewnić się, że nie ma konfliktów symboli.
  - Modularizacja plików i biblioteka statyczna/shared dla ponownego użycia w innych projektach.

Narzędzia do analizy i testów:
- clangd / ccls (używa [`compile_commands.json`](compile_commands.json:1)).
- narzędzia CI powinny instalować zależności SDL przed budowaniem.
- Testy: `make test` (Makefile powinien mapować do uruchomienia binarki testowej opartej na Catch2).

Krótkie wskazówki kompilacji lokalnej:
1. Zainstaluj: libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev (nazwa pakietów zależy od dystrybucji).
2. Uruchom: make
3. Uruchom testy: make test

Ograniczenia i punkty do weryfikacji:
- Nie zakładamy tu konkretnej wersji standardu C++ — proszę sprawdzić [`Makefile`](Makefile:1) lub [`compile_commands.json`](compile_commands.json:1) jeśli wymagana jest konkretna flaga (np. -std=c++20).
- Unity Build lub moduły mogą wymagać dodatkowej konfiguracji w `Makefile`.
- Upewnić się, czy CI ma dostęp do zasobów graficznych (Xvfb lub headless renderer), jeśli testy/rendering mają być uruchamiane w CI.

Dobre praktyki:
- Używaj `compile_commands.json` dla narzędzi IDE.
- Łącz logi SDL (SDL_LogInfo/SDL_LogError) z systemem CI, żeby mieć lepsze raporty błędów.
- Dokumentuj zmiany zależności w `readme.md` i/lub `docs/`.