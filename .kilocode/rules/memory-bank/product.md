# Dlaczego projekt istnieje i dla kogo

## Cel

Dostarczyć prostą, wydajną bibliotekę GUI opartą na SDL2, którą można łatwo użyć do tworzenia narzędzi, prototypów i lekkich aplikacji desktopowych w C++.

## Problemy, które projekt rozwiązuje

### Zarządzanie zasobami
Uproszczone zarządzanie kontekstem renderera i zasobami (tekstury, czcionki):
- [`TextureManager`](src/texture_manager.hpp) - cache'owanie tekstur
- [`FontManager`](src/font_manager.hpp) - cache'owanie czcionek

### Hierarchia elementów GUI
Cykl życia elementów z mechanizmem cache'a renderu:
- [`GUIElement`](src/gui.hpp) - klasa bazowa
- [`GUIManager`](src/gui_manager.hpp) - kontroler aplikacji

### Animacje i timery
Proste API do animacji i timerów:
- [`AnimationManager`](src/animation_manager.hpp)
- [`TimerManager`](src/timer_manager.hpp)

### Definicja GUI z plików
Parsery layoutów pozwalają definiować interfejs w JSON lub XML:
- [`JsonParser`](src/json_parser.hpp) - parsowanie JSON
- [`SGMLParser`](src/sgml_parser.hpp) - parsowanie XML/SGML

### Stylowanie i motywy
Przyjazne mechanizmy stylowania:
- [`Style`](src/style.hpp) - style elementów
- [`Theme`](src/theme.hpp) - globalne motywy

## Oczekiwany UX dla programistów

- **Prosty model dodawania widgetów**: utwórz instancję elementu dziedziczącego po `GUIElement`, dodaj do `GUIManager` przez `addElement`
- **Minimalne zarządzanie zasobami**: menedżery automatycznie cache'ują zasoby i zwracają współdzielone wskaźniki (`SharedTexture`, `SharedFont`)
- **Przykłady gotowego użycia**: katalog [`examples/`](examples/) zawiera 31 przykładów demonstrujących poszczególne widgety i funkcjonalności

## Ograniczenia i założenia

- Biblioteka bazuje na SDL2 (SDL_image, SDL_ttf)
- Projekt celuje w prostotę i czytelność, nie w pełną funkcjonalność produkcyjnego GUI frameworka
- Wymaga C++23 i kompilatora z obsługą modułów (clang++ z libc++)