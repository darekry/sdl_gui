# Powtarzalne procedury i checklisty deweloperskie

## Jak dodać nowy widget

1.  **Stwórz pliki**:
    *   Utwórz plik nagłówkowy `.hpp` w [`src/`](src/:1) dla nowej klasy widgetu, np. `my_widget.hpp`. Klasa musi dziedziczyć po [`GUIElement`](src/gui.hpp:19).
    *   Utwórz plik implementacji `.cpp` w [`src/`](src/:1), np. `my_widget.cpp`.

2.  **Zaimplementuj logikę**:
    *   W pliku `.hpp` zadeklaruj konstruktor i wymagane metody wirtualne, przede wszystkim `draw(SDL_Renderer* renderer)`.
    *   W pliku `.cpp` zaimplementuj logikę rysowania widgetu w metodzie `draw()`. Pamiętaj, że rysujesz do tekstury cache'u elementu, a nie bezpośrednio na ekran.
    *   Jeśli widget wymaga częstych aktualizacji (np. co klatkę), możesz zrezygnować z cache'owania, nadpisując metodę `wantsDirectRender()` tak, aby zwracała `true`, i implementując logikę rysowania w `drawDirect()`.

3.  **Zaktualizuj system budowania**:
    *   `nob.c` automatycznie wykrywa nowe pliki `.cpp` w katalogu `src/`, `src/composite/` i `src/editor/` (unity build), więc ręczna edycja nie jest konieczna.

4.  **Dodaj przykład użycia**:
    *   Stwórz plik `example_my_widget.cpp` w katalogu [`examples/`](examples/:1), aby pokazać, jak używać nowego widgetu.
    *   Uruchom `./nob` i przetestuj przykład: `./output/example_my_widget`.

5.  **Dodaj testy (opcjonalnie, ale zalecane)**:
    *   Stwórz plik `test_my_widget.cpp` w katalogu [`tests/`](tests/:1).
    *   Napisz testy sprawdzające kluczowe funkcjonalności widgetu.
    *   Uruchom testy za pomocą `./nob test`.

## Jak uruchomić testy

1.  **Zbuduj i uruchom testy**:
    *   Uruchom `./nob test` - kompiluje i automatycznie uruchamia wszystkie testy.

## Wskazówki dotyczące debugowania

-   **Problemy z zasobami (tekstury, czcionki)**:
    *   Sprawdź, czy ścieżki do plików zasobów są poprawne i dostępne z miejsca uruchomienia aplikacji.
    *   Upewnij się, że `TextureManager` i `FontManager` są poprawnie inicjalizowane przez `GUIManager`.
    *   Sprawdzaj logi błędów SDL. Menedżery zasobów używają `SDL_LogError` do raportowania problemów z ładowaniem.

-   **Problemy z renderowaniem**:
    *   Jeśli widget się nie rysuje, upewnij się, że flaga `m_isDirty` jest ustawiana na `true` po każdej zmianie, która powinna wywołać przerysowanie.
    *   Sprawdź, czy `render()` jest poprawnie wywoływane w hierarchii elementów.
    *   Użyj debuggera, aby prześledzić `GUIElement::render()` i `GUIElement::renderToCache()`.

## Jak dodać test jednostkowy

1.  **Stwórz plik testowy**:
    *   Utwórz plik `test_nazwa_widgetu.cpp` w katalogu [`tests/`](tests/:1).

2.  **Struktura testu**:
    ```cpp
    #define CATCH_CONFIG_MAIN
    #include "lib/catch_amalgamated.hpp"
    #include "tests/test_helper.hpp"
    #include "nazwa_widgetu.hpp"

    TEST_CASE("NazwaWidget - opis testu", "[tag]") {
        TestHelper helper;
        GUIManager& manager = helper.getManager();

        SECTION("Nazwa sekcji") {
            // kod testu
            REQUIRE(warunek);
        }
    }
    ```

3.  **Uruchom testy**:
    *   Wykonaj `./nob test` - nob.c automatycznie wykryje nowy plik testowy.

4.  **Ważne uwagi**:
    *   Użyj `TestHelper` do inicjalizacji SDL w trybie headless.
    *   Symuluj zdarzenia przez `helper.createMouseEvent()` i `helper.createKeyboardEvent()`.
    *   Testuj callbacki, stany elementów i reakcje na zdarzenia.