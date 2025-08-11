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
    *   Obecny `Makefile` automatycznie wykrywa nowe pliki `.cpp` w katalogu `src/` dzięki `wildcard`, więc ręczna edycja `Makefile` nie jest konieczna.

4.  **Dodaj przykład użycia**:
    *   Stwórz plik `example_my_widget.cpp` w katalogu [`examples/`](examples/:1), aby pokazać, jak używać nowego widgetu.
    *   Uruchom `make` i przetestuj przykład: `./output/example_my_widget`.

5.  **Dodaj testy (opcjonalnie, ale zalecane)**:
    *   Stwórz plik `test_my_widget.cpp` w katalogu [`tests/`](tests/:1).
    *   Napisz testy sprawdzające kluczowe funkcjonalności widgetu.
    *   Uruchom testy za pomocą `make test`.

## Jak uruchomić testy

1.  **Zbuduj projekt**:
    *   Uruchom `make` w głównym katalogu projektu. To polecenie skompiluje bibliotekę, przykłady oraz pliki wykonywalne testów.
2.  **Uruchom testy**:
    *   Wykonaj polecenie `make test`. Skrypt automatycznie uruchomi wszystkie pliki wykonywalne testów znajdujące się w katalogu `output/`.

## Wskazówki dotyczące debugowania

-   **Problemy z zasobami (tekstury, czcionki)**:
    *   Sprawdź, czy ścieżki do plików zasobów są poprawne i dostępne z miejsca uruchomienia aplikacji.
    *   Upewnij się, że `TextureManager` i `FontManager` są poprawnie inicjalizowane przez `GUIManager`.
    *   Sprawdzaj logi błędów SDL. Menedżery zasobów używają `SDL_LogError` do raportowania problemów z ładowaniem.

-   **Problemy z renderowaniem**:
    *   Jeśli widget się nie rysuje, upewnij się, że flaga `m_isDirty` jest ustawiana na `true` po każdej zmianie, która powinna wywołać przerysowanie.
    *   Sprawdź, czy `render()` jest poprawnie wywoływane w hierarchii elementów.
    *   Użyj debuggera, aby prześledzić `GUIElement::render()` i `GUIElement::renderToCache()`.