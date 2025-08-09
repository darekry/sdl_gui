Strategia testów

Ramowy cel:
- Zapewnić, że krytyczne komponenty (widgety, menedżery zasobów, cache renderu, animacje, timery) działają stabilnie i regresje są wykrywane w CI.

Narzędzia:
- Catch2 (amalgamated) używany w [`lib/catch_amalgamated.hpp`](lib/catch_amalgamated.hpp:1) i testach w [`tests/`](tests/:1).
- Makefile dostarcza target `make test` do uruchamiania zestawu testów.

Rodzaje testów:
- Jednostkowe: testy funkcjonalne menedżerów (`FontManager`, `TextureManager`) i logiki (`AnimationManager`, `TimerManager`) — przykładowe pliki: [`tests/test_helper.cpp`](tests/test_helper.cpp:1), [`tests/test_button.cpp`](tests/test_button.cpp:1).
- Integracyjne: przykłady w [`examples/`](examples/:1) mogą służyć jako manualne testy integracyjne; warto dodać skrypty CI do uruchamiania headless (jeśli środowisko CI obsługuje SDL w trybie headless).

Przykładowy przepływ uruchomienia testów lokalnie:
1. make
2. make test

Przykłady istotnych przypadków testowych:
- TextureManager: ładowanie nieistniejącej ścieżki powinno zwracać null i nie wypadać.
- FontManager: loadDefaultFont oraz getTextSize zwracają poprawne wymiary dla znanych czcionek.
- GUIElement: przy zmianie rozmiaru m_cachedTexture jest rekreowana i renderToCache nie zgłasza błędów.
- AnimationManager: animacje osiągają końcowe wartości i wywołują callbacki on_complete.
- TimerManager: timery singleShot i intervalowe są wywoływane w oczekiwanym czasie (testy mogą symulować czas lub użyć realnego czasu z niewielkim marginesem).

Punkty do automatyzacji w CI:
- Instalacja zależności SDL w obrazie CI.
- Uruchomienie `make` i `make test`.
- Opcjonalna analiza statyczna (clang-tidy) wykorzystująca `compile_commands.json`.

Ograniczenia testów:
- Testy GUI i renderowania wymagają środowiska graficznego; w CI może być konieczne uruchomienie Xvfb lub użycie backendu renderera kompatybilnego z headless.