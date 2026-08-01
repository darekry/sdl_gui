# Style guide dokumentacji SDL GUI (docs/release/)

Te pliki trafiają do `dist/docs/` — to JEDYNA dokumentacja, którą widzi użytkownik
końcowy. Pisze się ją dla kogoś, kto NIE MA dostępu do `src/` — tylko do
`dist/sdl_gui.hpp`, `dist/sdl_gui.h` i bibliotek.

## Zasady ogólne

- **Język: polski** (terminy techniczne po angielsku, np. hover, focus, callback).
- **Nie odwołuj się do ścieżek z repo** (`src/`, `tests/`, `examples/`, `nob.c`).
  Jedyne poprawne odwołania: `sdl_gui.hpp`, `sdl_gui.h`, `libsdl_gui.a`,
  `libsdl_gui.so`, `docs/`.
- Pisz zwięźle i konkretnie. Zero lania wody, zero emoji.
- **Każda metoda wymieniona w dokumencie musi istnieć w nagłówku** — weryfikuj
  w `src/<plik>.hpp`, który jest źródłem prawdy (generator łączy je w sdl_gui.hpp).
  Nie dokumentuj metod, których nie ma. Sygnatury podawaj 1:1 z nagłówka.
- Format: GitHub-flavored Markdown.

## Szablon dokumentacji widgetu (`widgets/Nazwa.md`)

```markdown
# NazwaWidget

Krótki opis (1-3 zdania): do czego służy, kiedy go użyć.

## Przeznaczenie

2-4 zdania o zastosowaniu + ewentualne ograniczenia.

## Tworzenie

Sygnatury konstruktorów z nagłówka (dokładnie), przykładowe wywołanie.
Widget tworzy się jako `std::make_unique<T>(manager, ...)` i dodaje przez
`guiManager.addElement(std::move(widget))` lub `manager.create<T>(...)`.

## Najważniejsze metody

Tabela: | Metoda | Opis | (parametry jeśli nieoczywiste)

## Callbacki / zdarzenia

Tabela lub lista callbacków z sygnaturami i wyjaśnieniem kiedy są wywoływane.
W callbackach używaj `ElementRef<T>` jeśli trzeba się odwołać do widgetu po `std::move`.

## Przykład

Kompletny, zwięzły przykład kodu (```cpp) oparty o `SDLApp` + `GUIManager`
(wzorzec z _TEMPLATE_EXAMPLE.md). Ma się kompilować koncepcyjnie z samym
`#include "sdl_gui.hpp"`. Pomijaj pętlę zdarzeń jeśli nie jest kluczowa.

## Uwagi

- Pułapki, zachowania specjalne, wydajność, interakcje z innymi widgetami.
```

## Szablon pozostałych dokumentów

- `managers.md` / `resources.md` / `composites.md`: sekcja `## Nazwa` per
  komponent, struktura jak wyżej (bez "Przeznaczenie" jako osobny nagłówek —
  jeden akapit otwierający).
- `c_api.md`: sekcja per obszar API z sygnaturami 1:1 z `src/sdl_gui.h`.

## Konwencje kodowe w przykładach

- `#include "sdl_gui.hpp"` — jedyny include projektu.
- Inicjalizacja: `SDLApp` lub `GUIContext` (GUIContext = SDLApp + GUIManager +
  Theme w jednym; pokazuj OBA wzorce — raz w getting_started, potem w miarę
  sensu).
- `guiManager.setTheme(ThemePresets::createDarkTheme())` gdy potrzebny motyw.
- Kolejność w pętli: `processEvent` → `update` → `cleanup` → `render`.
- Nazwy zmiennych: `btn`, `panel`, `slider`, `label`, `manager` itp.
- Kod C++23, `auto`/`std::make_unique`, żadnych gołych `new/delete`.

## Czego NIE robić

- Nie wymyślaj API. Jeśli czegoś nie ma w nagłówku — nie ma.
- Nie kopiuj starych dokumentów z `docs/pl/`, `docs/en/`, `docs/api/` — są
  nieaktualne (SDL2). Piszesz od zera.
- Nie wspominaj o Splitter — nie istnieje w tej wersji.
