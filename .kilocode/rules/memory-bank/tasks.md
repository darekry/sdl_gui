Powtarzalne procedury i checklisty deweloperskie

Jak dodać nowy widget:
1. Stwórz nagłówkowy plik klasy dziedziczącej po [`GUIElement`](src/gui.hpp:19), np. `src/my_widget.hpp`.
2. Zaimplementuj metody: konstruktor, destruktor, draw(SDL_Renderer*) oraz opcjonalnie wantsDirectRender()/drawDirect().
3. Dodaj implementację w `src/my_widget.cpp` i zarejestruj w build systemie (Makefile).
4. Dodaj przykład użycia do katalogu [`examples/`](examples/:1) (np. `examples/example_my_widget.cpp`) i zaktualizuj dokumentację w [`docs/`](docs/:1).
5. Uruchom lokalnie przykład: make && ./output/example_my_widget

Jak generować `compile_commands.json`:
- Jeśli używasz CMake: ustaw -DCMAKE_EXPORT_COMPILE_COMMANDS=ON podczas konfiguracji.
- W tym repozytorium `compile_commands.json` już istnieje; jeśli go brakuje, można użyć narzędzi typu bear lub edytować Makefile, aby wygenerować go przez CMake.

Jak uruchomić testy:
1. Zbuduj projekt z testami: make
2. Uruchom: make test
3. Testy bazują na Catch2; pliki testowe znajdują się w [`tests/`](tests/:1).

Debugowanie problemów z zasobami:
- Sprawdź ścieżki używane w kodzie (np. `assets/fonts/font.ttf`).
- Upewnij się, że `SDL_image` i `SDL_ttf` są zainicjowane przed użyciem `TextureManager`/`FontManager`.
- Używaj logów SDL_LogInfo/SDL_LogError które są używane w menedżerach zasobów.

Rutyna PR/Code review:
- Upewnij się, że dodany kod ma przykładowe użycie w `examples/` lub test.
- Drobne zmiany architektury omawiać przez issues/proposals w [`docs/proposals/`](docs/proposals/:1).