Dlaczego projekt istnieje i dla kogo.

Cel: dostarczyć prostą, wydajną bibliotekę GUI opartą na SDL2, którą można łatwo użyć do tworzenia narzędzi, prototypów i lekkich aplikacji desktopowych w C++.

Problemy, które projekt rozwiązuje:
- Uproszczone zarządzanie kontekstem renderera i zasobami (teksturowanie, czcionki) — implementacja w [`src/texture_manager.hpp`](src/texture_manager.hpp:15) i [`src/font_manager.hpp`](src/font_manager.hpp:30).
- Hierarchia i cykl życia elementów GUI z mechanizmem cache'a renderu — podstawowe klasy w [`src/gui.hpp`](src/gui.hpp:19) i implementacja renderowania w [`src/gui.cpp`](src/gui.cpp:135).
- Proste API do animacji i timerów: [`src/animation_manager.hpp`](src/animation_manager.hpp:24) i [`src/timer_manager.hpp`](src/timer_manager.hpp:18).
- Przyjazne mechanizmy stylowania i motywów: [`src/style.hpp`](src/style.hpp:17) i [`src/theme.hpp`](src/theme.hpp:10).

Oczekiwany UX dla programistów:
- Prosty model dodawania widgetów: utwórz instancję elementu dziedziczącego po [`GUIElement`](src/gui.hpp:19), dodaj do [`GUIManager`](src/gui_manager.hpp:19) przez `addElement`.
- Minimalne zarządzanie zasobami: menedżery (`TextureManager`, `FontManager`) automatycznie cache'ują zasoby i zwracają współdzielone wskaźniki (`SharedTexture`, `SharedFont`).
- Przykłady gotowego użycia w katalogu [`examples/`](examples/:1), np. [`examples/example_button.cpp`](examples/example_button.cpp:1) i [`examples/example_tooltip.cpp`](examples/example_tooltip.cpp:1).

Ograniczenia i założenia:
- Biblioteka bazuje na SDL2 (SDL_image, SDL_ttf) — użytkownik musi mieć zależności środowiskowe.
- Projekt celuje w prostotę i czytelność, a nie w pełną funkcjonalność produkcyjnego GUI frameworka.
- W razie niejasności dotyczących API lub ścieżek zasobów, proszę zweryfikować implementację w plikach źródłowych wymienionych powyżej.