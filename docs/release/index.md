# SDL GUI — dokumentacja

## Czym jest SDL GUI

SDL GUI to lekka biblioteka GUI dla narzędzi i prototypów desktopowych, zbudowana
na SDL3 w C++23. Zapewnia kompletny zestaw widgetów (przyciski, pola tekstowe,
listy, siatki, karty, suwaki, menedżery okien i wiele innych), system motywów
i stylów per stan, responsywny layout oparty na anchorach oraz gotowe okna
dialogowe. Nie wymaga żadnych frameworków poza SDL3 — renderowanie odbywa się
bezpośrednio przez `SDL_Renderer`.

## Zawartość pakietu (`dist/`)

| Plik | Opis |
|------|------|
| `sdl_gui.hpp` | Kompletne publiczne API C++ w jednym nagłówku — jedyny include potrzebny w projekcie |
| `sdl_gui.h` | Publiczne API C (prefiks `sdlgui_`) dla integracji z kodem w czystym C |
| `libsdl_gui.a` | Biblioteka statyczna |
| `libsdl_gui.so` | Biblioteka współdzielona |
| `docs/` | Ta dokumentacja |

## Spis dokumentów

| Dokument | Zawartość |
|----------|-----------|
| [getting_started.md](getting_started.md) | Instalacja SDL3, kompilacja i linkowanie (C++ i C), pierwszy program |
| [core.md](core.md) | Fundamenty: `GUIElement`, `GUIManager`, wspólne typy |
| [patterns.md](patterns.md) | Zalecane wzorce użycia i typowe pułapki |
| [widgets/](widgets/) | Dokumentacja poszczególnych widgetów (np. [widgets/Button.md](widgets/Button.md), [widgets/Slider.md](widgets/Slider.md)) |
| [composites.md](composites.md) | Gotowe okna dialogowe: `DialogBox`, `MessageBox`, `FileDialog` |
| [managers.md](managers.md) | Menedżery: `GUIManager`, `TimerManager`, `AnimationManager`, `FontManager`, `TextureManager` |
| [resources.md](resources.md) | Zasoby: tekstury, fonty, animacje, asset embedded, typy pomocnicze |
| [c_api.md](c_api.md) | API C (`sdl_gui.h`) — tworzenie kontekstu, widgety, animacje i timery z poziomu C |

## Wymagania systemowe

- System Linux (biblioteka korzysta z API SDL3; inne platformy nie są wspierane).
- SDL3 wraz z dodatkami: `SDL3`, `SDL3_image`, `SDL3_ttf` (dostępne przez
  `pkg-config`, patrz [getting_started.md](getting_started.md)).
- Dla C++: kompilator `clang` z `libc++` i standardem C++23 (`-std=c++23 -stdlib=libc++`).
- Dla C: dowolny kompilator C11 (`gcc -std=c11 -pedantic-errors`); linkowanie
  odbywa się kompilatorem C++ (biblioteka jest napisana w C++).

## Szybki start

Minimalny program z jednym przyciskiem znajdziesz w
[getting_started.md](getting_started.md#hello-world). Zanim zaczniesz:

1. Zainstaluj SDL3 i dodatki.
2. Skompiluj program z `sdl_gui.hpp` i `libsdl_gui.so` (lub `libsdl_gui.a`).
3. Pamiętaj o ustawieniu motywu — bez niego widgety są niewidoczne.
