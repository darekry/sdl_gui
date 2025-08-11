# Strategia testów

## Cel i narzędzia

Głównym celem strategii testowania jest zapewnienie stabilności kluczowych komponentów (widgety, menedżery, cache) i szybkie wykrywanie regresji.

-   **Framework do testów jednostkowych**: **Catch2**. Wersja amalgamated znajduje się w [`lib/catch_amalgamated.hpp`](lib/catch_amalgamated.hpp:1).
-   **Uruchamianie testów**: `make test` w głównym katalogu projektu.

## Rodzaje testów

### 1. Testy jednostkowe

Testy jednostkowe weryfikują logikę poszczególnych komponentów w izolacji. Znajdują się w katalogu [`tests/`](tests/:1).

-   **Struktura**: Każdy plik `test_*.cpp` odpowiada za testowanie konkretnego widgetu lub modułu.
-   **Helper**: Pliki [`tests/test_helper.hpp`](tests/test_helper.hpp:1) i [`tests/test_helper.cpp`](tests/test_helper.cpp:1) dostarczają środowisko testowe, które inicjalizuje SDL i `GUIManager` w tle, umożliwiając testowanie logiki bez renderowania okna.
-   **Zasada**: Testy powinny symulować interakcje (np. kliknięcia, wprowadzanie tekstu) i weryfikować stan obiektu oraz wywołania zwrotne (callbacki).

**Przykładowy test dla `Button`:**
```cpp
#define CATCH_CONFIG_MAIN
#include "lib/catch_amalgamated.hpp"
#include "tests/test_helper.hpp"
#include "button.hpp"
#include "gui_manager.hpp"

TEST_CASE("Button Functionality", "[button]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Event Handling - Click") {
        bool clicked = false;
        auto button = std::make_unique<Button>(manager, 10, 10, 100, 50, "Click me");
        button->setOnClickCallback([&](GUIElement*) { clicked = true; });
        
        Button* button_ptr = button.get();
        manager.addElement(std::move(button));

        // Symulacja kliknięcia wewnątrz przycisku
        SDL_Event event = helper.createMouseEvent(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(event);
        
        // Weryfikacja stanu po zdarzeniu
        REQUIRE(button_ptr->getState() == ElementState::Pressed);

        // Symulacja zwolnienia przycisku
        event = helper.createMouseEvent(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 20, 20);
        manager.processEvent(event);

        REQUIRE(clicked == true);
        REQUIRE(button_ptr->getState() == ElementState::Hover);
    }
}
```

### 2. Testy integracyjne (manualne)

Katalog [`examples/`](examples/:1) pełni rolę zestawu manualnych testów integracyjnych. Każdy plik `example_*.cpp` demonstruje działanie jednego lub więcej komponentów w działającej aplikacji.

-   **Cel**: Weryfikacja wizualna i funkcjonalna. Sprawdzenie, czy widgety poprawnie się renderują, reagują na interakcje i współpracują ze sobą.
-   **Procedura**: Po wprowadzeniu znaczących zmian w bibliotece, deweloper powinien skompilować (`make examples`) i uruchomić wszystkie przykłady, aby upewnić się, że żaden z nich nie uległ regresji.
-   **Kluczowe przykłady do weryfikacji**:
    -   `example_button.cpp`: Podstawowa funkcjonalność i stylowanie.
    -   `example_window.cpp`: Zarządzanie hierarchią i dziećmi.
    -   `example_animated_image.cpp`: Złożone widgety, timery i animacje.

### 3. Testy wydajności (manualne)

Przykład [`examples/example_performance.cpp`](examples/example_performance.cpp:1) służy jako narzędzie do manualnego testowania wydajności biblioteki.

-   **Cel**: Ocena, jak biblioteka radzi sobie z renderowaniem i zarządzaniem dużą liczbą widgetów.
-   **Procedura**: Uruchomienie przykładu i dynamiczne dodawanie/usuwanie setek obiektów, obserwując przy tym licznik FPS i czas renderowania klatki. Pozwala to na wczesne wykrycie wąskich gardeł wydajnościowych.

## Podsumowanie

Strategia opiera się na trzech filarach:
1.  **Automatyczne testy jednostkowe (Catch2)** do weryfikacji logiki.
2.  **Manualne testy integracyjne (`examples/`)** do weryfikacji wizualnej i funkcjonalnej.
3.  **Manualne testy wydajności (`example_performance.cpp`)** do monitorowania optymalizacji.

Taki podział zapewnia dobre pokrycie kodu i minimalizuje ryzyko regresji, jednocześnie utrzymując proces testowania relatywnie prostym i szybkim.