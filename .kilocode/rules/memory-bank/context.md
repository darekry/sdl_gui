Aktualny stan projektu i najważniejsze zmiany.

Stan repozytorium:
- Kod źródłowy i implementacja podstawowych widgetów znajdują się w [`src/`](src/:1).
- Dokumentacja i przykłady są w [`docs/`](docs/:1) i [`examples/`](examples/:1).
- Testy jednostkowe (Catch2) w [`tests/`](tests/:1).

Ostatnie istotne zmiany:
- Refaktoryzacja renderowania elementów: wprowadzono tryb rysowania bezpośredniego (drawDirect) oraz domyślną ścieżkę buforowania do tekstury cache w [`src/gui.hpp`](src/gui.hpp:19) i [`src/gui.cpp`](src/gui.cpp:135).
- GUIManager inicjalizuje menedżery zasobów i motywów, tworzy domyślną teksturę i ładuje domyślną czcionkę (`src/gui_manager.cpp`).
- Dodano menedżery: [`TextureManager`](src/texture_manager.hpp:15, `src/texture_manager.cpp`), [`FontManager`](src/font_manager.hpp:30, `src/font_manager.cpp`), [`AnimationManager`](src/animation_manager.hpp:24) i [`TimerManager`](src/timer_manager.hpp:18).
- System stylów i motywów: [`src/style.hpp`](src/style.hpp:17) i [`src/theme.hpp`](src/theme.hpp:10).

Następne kroki (priorytety):
1. Przygotowanie i ujednolicenie `readme.md` z opisem uruchomienia i zależności (SDL2, SDL_image, SDL_ttf).
2. Uzupełnienie dokumentacji tworzenia widgetów na podstawie przykładów w [`docs/creating_new_widget.md`](docs/creating_new_widget.md:1) i [`examples/`](examples/:1).
3. Pokrycie testami krytycznych komponentów (render caching, menedżery zasobów, animacje, timery).
4. Przegląd i ewentualne ujednolicenie obsługi błędów inicjalizacji SDL_image i SDL_ttf.

Punkty wymagające weryfikacji:
- Dokładne ścieżki do zasobów domyślnych (`assets/fonts/font.ttf`) oraz polityka publikacji assetów.
- Czy oczekiwane zachowanie cache (m_cachedTexture) w sytuacji zmiany rozmiaru elementu jest zgodne z wymaganiami.
- Poziom wsparcia dla różnych formatów obrazów w [`TextureManager::TextureManager`](src/texture_manager.cpp:7).