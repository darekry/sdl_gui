# Strategia Testowania Biblioteki GUI

## 1. Podejście i Wybór Frameworka

Do testowania biblioteki zostanie użyty framework **Catch2**.

### Uzasadnienie

*   **Lekkość i prostota integracji:** Catch2 może być dołączony do projektu jako jeden plik nagłówkowy (`catch.hpp`), co eliminuje potrzebę skomplikowanej konfiguracji i zarządzania dodatkowymi zależnościami.
*   **Nowoczesna składnia C++:** Testy pisane w Catch2 są czytelne, zwięzłe i dobrze integrują się ze standardami C++20.
*   **Automatyczna rejestracja testów:** Catch2 automatycznie wykrywa i rejestruje przypadki testowe zdefiniowane za pomocą makra `TEST_CASE`, co upraszcza zarządzanie zestawem testów.
*   **Bogate możliwości asercji:** Framework dostarcza intuicyjne makra, takie jak `REQUIRE` (przerywa test w razie błędu) i `CHECK` (raportuje błąd i kontynuuje), co daje dużą elastyczność w weryfikacji wyników.

## 2. Struktura Plików Testowych

Wszystkie pliki testowe będą umieszczone w katalogu `tests/`.

```
tests/
├── catch.hpp               # Plik nagłówkowy Catch2 (do pobrania i umieszczenia)
├── test_helper.hpp         # Plik nagłówkowy dla wspólnych funkcji testowych
├── test_helper.cpp         # Implementacja wspólnych funkcji testowych
├── test_button.cpp         # Testy dla przycisku (i panelu, bo są w tym samym pliku gui.cpp)
├── test_slider.cpp         # Testy dla suwaka
├── test_text_input.cpp     # Testy dla pola tekstowego
├── test_checkbox.cpp       # Testy dla checkboxa
└── test_radio_button.cpp   # Testy dla radio buttona i radio group
```

### `test_helper.hpp` / `test_helper.cpp`

Pliki te będą zawierać klasę lub zbiór funkcji pomocniczych do:
*   Inicjalizacji i zamykania podsystemów SDL w tle.
*   Tworzenia "fałszywego" okna i renderera, które nie będą widoczne na ekranie, ale umożliwią testowanie logiki renderowania i obsługi zdarzeń.
*   Symulowania zdarzeń SDL, np. `simulate_mouse_click(x, y)` czy `simulate_key_press(SDL_Keycode)`.

### Przykładowy plik testowy (`tests/test_button.cpp`)

```cpp
#define CATCH_CONFIG_MAIN // Informuje Catch, że ma stworzyć własną funkcję main()
#include "catch.hpp"
#include "test_helper.hpp"
#include "gui.hpp" // Zakładając, że Button jest w gui.hpp
#include "texture_manager.hpp"
#include "font_manager.hpp"

TEST_CASE("Button Functionality", "[button]") {
    TestHelper helper; // Inicjalizuje SDL, okno i renderer w tle
    auto renderer = helper.getRenderer();
    TextureManager textureManager(renderer);
    FontManager fontManager(renderer);

    SECTION("Initialization") {
        Button button(renderer, 10, 20, 100, 50, "Click me", &fontManager, &textureManager);
        SDL_Rect rect = button.getRect();
        REQUIRE(rect.x == 10);
        REQUIRE(rect.y == 20);
        REQUIRE(rect.w == 100);
        REQUIRE(rect.h == 50);
        REQUIRE(button.isEnabled() == true);
    }

    SECTION("Event Handling - Click") {
        bool clicked = false;
        Button button(renderer, 10, 10, 100, 50, "Click me", &fontManager, &textureManager);
        button.setOnClick([&]() { clicked = true; });

        // Symulacja kliknięcia wewnątrz przycisku
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        button.handleEvent(&event);
        
        REQUIRE(clicked == true);

        // Symulacja kliknięcia na zewnątrz
        clicked = false;
        event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 200, 200);
        button.handleEvent(&event);
        REQUIRE(clicked == false);
    }

    SECTION("State - Disabled") {
        bool clicked = false;
        Button button(renderer, 10, 10, 100, 50, "Click me", &fontManager, &textureManager);
        button.setOnClick([&]() { clicked = true; });
        button.setEnabled(false);

        REQUIRE(button.isEnabled() == false);

        // Symulacja kliknięcia, gdy przycisk jest wyłączony
        SDL_Event event = helper.create_mouse_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        button.handleEvent(&event);
        REQUIRE(clicked == false);
    }
}
```

## 3. Modyfikacje w `Makefile`

Poniżej znajdują się proponowane zmiany w `Makefile` w celu zautomatyzowania kompilacji i uruchamiania testów.

```makefile
# ... (istniejące zmienne CXX, CXXFLAGS, LDFLAGS, etc.)

# Pliki obiektowe biblioteki (bez main.o)
LIB_SRC_FILES := $(filter-out src/main.cpp, $(wildcard src/*.cpp))
LIB_OBJ_FILES := $(patsubst src/%.cpp,output/%.o,$(LIB_SRC_FILES))

# Pliki źródłowe testów
TEST_SRC_FILES := $(wildcard tests/test_*.cpp)
TEST_HELPER_OBJ := output/test_helper.o

# Pliki wykonywalne testów
TEST_EXECS := $(patsubst tests/%.cpp,output/%,$(TEST_SRC_FILES))

# Cel główny
all: output/main

# Cel do uruchamiania testów
test: $(TEST_EXECS)
	@echo "Running all tests..."
	@for t in $(TEST_EXECS); do \
		./$$t; \
	done
	@echo "All tests passed successfully."

# Reguła kompilacji dla plików wykonywalnych testów
# Każdy plik testowy jest kompilowany do osobnego pliku wykonywalnego
# i linkowany z całą biblioteką oraz pomocnikiem testów.
output/test_%: tests/test_%.cpp $(LIB_OBJ_FILES) $(TEST_HELPER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Reguła kompilacji dla pomocnika testów
$(TEST_HELPER_OBJ): tests/test_helper.cpp tests/test_helper.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Reguła kompilacji dla plików obiektowych biblioteki
output/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Reguła kompilacji dla głównej aplikacji
output/main: output/main.o $(LIB_OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# ... (istniejące cele clean, etc.)
# Należy zaktualizować cel 'clean', aby usuwał również pliki testowe
clean:
	rm -f output/*
```

## 4. Podsumowanie

Powyższa strategia zapewnia solidne podstawy do testowania biblioteki. Wprowadza minimalne zależności, jest łatwa w utrzymaniu i skalowalna. Umożliwia testowanie poszczególnych komponentów w izolacji oraz weryfikację ich poprawnego działania w odpowiedzi na symulowane zdarzenia, co znacząco podniesie jakość i stabilność projektu.