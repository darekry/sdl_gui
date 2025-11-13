Aktualny stan projektu i najważniejsze zmiany.

Stan repozytorium:
- Kod źródłowy i implementacja podstawowych widgetów znajdują się w [`src/`](src/:1).
- Dokumentacja i przykłady są w [`docs/`](docs/:1) i [`examples/`](examples/:1).
- Testy jednostkowe (Catch2) w [`tests/`](tests/:1).

Ostatnie istotne zmiany:
- Przeprowadzono pełną analizę projektu w celu aktualizacji banku pamięci.
- Zaktualizowano dokumentację architektury (`architecture.md`), produktu (`product.md`), technologii (`tech.md`), zadań (`tasks.md`) i strategii testów (`testing_strategy.md`).
- Potwierdzono, że architektura oparta na `GUIManager` i `GUIElement` z cache'owaniem renderowania jest stabilna.
- Potwierdzono, że przykłady w `examples/` są aktualne i stanowią dobrą bazę do testów manualnych.
- Naprawiono 8 warningów implicit conversion int to float w [`src/slider.cpp`](src/slider.cpp), dodając explicit cast do static_cast<float> dla dzielenia int przez int oraz mnożenia int przez float.
- Dodano słowo kluczowe override do deklaracji metody getComponentType w [`src/text_area.hpp`](src/text_area.hpp:21), aby poprawić czytelność kodu i zgodność z standardami C++.
- Naprawiono warning o nieużywanych parametrach argc i argv w [`examples/example_context_menu.cpp`](examples/example_context_menu.cpp:10), dodając [[maybe_unused]] przed parametrami funkcji main.
- Naprawiono warning o missing field 'textColor' initializer w [`examples/example_checkbox.cpp`](examples/example_checkbox.cpp:18), dodając .textColor = std::nullopt do inicjalizacji stylu panelu.
- Naprawiono warning o missing field 'backgroundColor' initializer w [`examples/example_tabs.cpp`](examples/example_tabs.cpp:27-29), dodając .backgroundColor = std::nullopt do inicjalizacji stylu paneli zakładek.
- Naprawiono warning o missing field 'backgroundColor' initializer w [`examples/example_window.cpp`](examples/example_window.cpp:28, 33, 46), dodając .backgroundColor = std::nullopt do inicjalizacji stylu paneli okna.
- Naprawiono warning o nieużywanych parametrach argc i argv w [`examples/example_themes.cpp`](examples/example_themes.cpp:76), dodając [[maybe_unused]] przed parametrami funkcji main.
- Naprawiono warning o missing field initializer w [`examples/example_radio_button.cpp`](examples/example_radio_button.cpp:19), dodając .borderWidth = std::nullopt, .fontSize = std::nullopt, .fontName = std::nullopt do inicjalizacji stylu RadioGroup.
- Naprawiono 2 warningi implicit conversion int to float w [`src/animated_image.cpp`](src/animated_image.cpp:127,185), dodając static_cast<float> dla wartości int przed mnożeniem przez float w celu uniknięcia utraty precyzji.
- Naprawiono 2 warningi implicit conversion w [`src/context_menu.cpp`](src/context_menu.cpp:94,131), dodając static_cast<size_t> dla konwersji int do size_type oraz static_cast<int> dla konwersji size_type do int.
- Zaktualizowano dokumentację dotyczącą tworzenia nowych widżetów ([`docs/creating_new_widget.md`](docs/creating_new_widget.md) oraz wersje `pl` i `en`). Poprawiono w niej przykład implementacji metody `draw()`, zaktualizowano jej sygnaturę i dodano informację o automatycznym wykrywaniu plików przez `Makefile`.
- Zaktualizowano pliki [`readme.md`](readme.md) i [`README.pl.md`](README.pl.md). W `readme.md` poprawiono opis polecenia `make all`, a w `README.pl.md` skorygowano ścieżkę do pliku `src/sdl_app.hpp`.
- **Refaktoryzacja dokumentacji dla użytkownika końcowego**:
    *   Zmodyfikowano `readme.md` i `README.pl.md`, usuwając sekcje dotyczące wewnętrznej architektury, kompilacji i testowania, a dodając szczegółowe instrukcje linkowania z bibliotekami statycznymi i dynamicznymi. Uproszczono sekcję "How to Use".
    *   Usunięto zbędne pliki i katalogi z `docs/`, które dotyczyły wewnętrznego rozwoju biblioteki (np. `creating_new_widget.md`, `testing_strategy.md`, `feature_proposals.md`, `proposals/`, `archive/`).
    *   Zrefaktoryzowano pozostałe pliki w `docs/` (`animated_image.md`, `context_menu.md`, `mouse_cursor.md`, `for_rts.md`), usuwając szczegóły implementacyjne i skupiając się na publicznym API.
    *   Utworzono nową dokumentację `docs/getting_started.md` (w wersjach `pl`/`en`), zawierającą kompletny przewodnik dla początkujących, w tym wymagania, strukturę projektu, przykład "Hello World" oraz instrukcje linkowania.
    *   Utworzono nową dokumentację API w `docs/api/` dla kluczowych komponentów (`GUIManager.md`, `Button.md`, `Panel.md`), opisującą ich przeznaczenie, publiczne metody i przykłady użycia.

Następne kroki (priorytety):
1.  Rozszerzenie pokrycia testami jednostkowymi dla kluczowych komponentów, które jeszcze nie są w pełni przetestowane.

Punkty wymagające weryfikacji:
- Dokładne ścieżki do zasobów domyślnych (`assets/fonts/font.ttf`) oraz polityka publikacji assetów.
- Czy oczekiwane zachowanie cache (`m_cachedTexture`) w sytuacji zmiany rozmiaru elementu jest zgodne z wymaganiami.
- Poziom wsparcia dla różnych formatów obrazów w [`TextureManager::TextureManager`](src/texture_manager.cpp:7).