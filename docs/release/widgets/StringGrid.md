# StringGrid

Tabela komórek tekstowych z sortowaniem, zaznaczaniem zakresów, edycją w miejscu, nagłówkami kolumn i wierszy oraz wbudowanymi paskami przewijania. Użyj go wszędzie tam, gdzie potrzebujesz danych w formie siatki (tabele, arkusze, przeglądarki danych).

## Przeznaczenie

StringGrid renderuje dane jako siatkę wierszy × kolumn. Wiersze i kolumny dodaje się przez `setRowCount`/`setColumnCount`, a zawartość ustawia przez `setCellText`. Kliknięcie nagłówka kolumny sortuje dane (cykl: brak → rosnąco → malejąco → brak); można też sortować programowo i podpiąć własne funkcje porównujące per kolumna. Kliknięcie i przeciągnięcie zaznacza komórkę lub prostokątny zakres; `Ctrl+C` kopiuje zaznaczenie do schowka. Po włączeniu `setEditable(true)` komórkę edytuje się w miejscu (Enter rozpoczyna edycję, Escape przerywa). Poziome i pionowe przewijanie realizują wbudowane paski (`Slider`), działa też kółko myszy. Jest to największy widget biblioteki — większość jego zachowań można sterować przez settery i callbacki.

## Tworzenie

```cpp
StringGrid(GUIManager& manager, int x, int y, int width, int height,
           size_t initialRows = 0, size_t initialCols = 0);
```

`initialRows`/`initialCols` to opcjonalny wstępny rozmiar siatki (można pominąć i ustawić wymiary później). Widget dziedziczy po `Panel` (można go dodać jako dziecko innego elementu; współrzędne są wtedy względne rodzica).

```cpp
auto grid = std::make_unique<StringGrid>(manager, 10, 10, 560, 300, 5, 4);
manager.addElement(std::move(grid));

// albo przez skrót:
StringGrid* grid = manager.create<StringGrid>(10, 10, 560, 300, 5, 4);
```

Typy pomocnicze (w `sdl_gui.hpp`):

```cpp
enum class SortDirection { None, Ascending, Descending };

struct CellCoord {
    size_t row;
    size_t col;
    [[nodiscard]] bool isValid() const;              // row != SIZE_MAX && col != SIZE_MAX
    static CellCoord invalid();                      // {SIZE_MAX, SIZE_MAX}
    bool operator==(const CellCoord&) const;
    bool operator!=(const CellCoord&) const;
};

struct SelectionRange {
    CellCoord start;
    CellCoord end;
    [[nodiscard]] bool isValid() const;              // oba końce poprawne
    [[nodiscard]] SelectionRange normalized() const; // start <= end (po wierszach i kolumnach)
};
```

## Najważniejsze metody

### Dane

| Metoda | Opis |
|--------|------|
| `void setRowCount(size_t rows)` | Ustawia liczbę wierszy (dodaje/pomniejsza siatkę) |
| `void setColumnCount(size_t cols)` | Ustawia liczbę kolumn |
| `size_t getRowCount() const` | Liczba wierszy |
| `size_t getColumnCount() const` | Liczba kolumn |
| `void setCellText(size_t row, size_t col, std::string_view text)` | Wstawia tekst do komórki |
| `std::string_view getCellText(size_t row, size_t col) const` | Zwraca tekst komórki (widok ważny do następnej modyfikacji siatki) |
| `void clear()` | Czyści wszystkie dane (wymiary zostają) |

### Sortowanie

| Metoda | Opis |
|--------|------|
| `void sortByColumn(size_t col, SortDirection dir)` | Sortuje wiersze wg tekstu kolumny `col` |
| `SortDirection getSortDirection() const` | Aktualny kierunek sortowania |
| `size_t getSortColumn() const` | Indeks sortowanej kolumny |
| `void setCustomComparator(size_t col, CompareFunc func)` | Własny komparator dla kolumny; `CompareFunc = std::function<bool(const std::string& a, const std::string& b)>` — zwraca `true`, gdy `a` ma być przed `b` (dla sortowania rosnącego) |
| `void clearCustomComparator(size_t col)` | Usuwa komparator dla kolumny |
| `void clearAllCustomComparators()` | Usuwa wszystkie komparatory |
| `bool hasCustomComparator(size_t col) const` | Czy kolumna ma własny komparator |

Kliknięcie nagłówka kolumny również sortuje (cykl `None → Ascending → Descending → None`), z użyciem komparatora, jeśli ustawiony.

### Geometria i nagłówki

| Metoda | Opis |
|--------|------|
| `void setColumnWidth(size_t col, int width)` | Szerokość kolumny w pikselach |
| `void setRowHeight(int height)` | Wysokość wiersza (domyślnie 24) |
| `void setHeaderHeight(int height)` | Wysokość nagłówków kolumn (domyślnie 28) |
| `void setRowHeaderWidth(int width)` | Szerokość nagłówków wierszy (domyślnie 50) |
| `void setColumnHeader(size_t col, std::string_view text)` | Tekst nagłówka kolumny |
| `void setShowRowHeaders(bool show)` | Pokaż/ukryj nagłówki wierszy |
| `void setShowColumnHeaders(bool show)` | Pokaż/ukryj nagłówki kolumn |

### Zaznaczanie

| Metoda | Opis |
|--------|------|
| `void setSelectedCell(size_t row, size_t col)` | Zaznacza pojedynczą komórkę |
| `void setSelectionRange(size_t startRow, size_t startCol, size_t endRow, size_t endCol)` | Zaznacza prostokątny zakres |
| `void clearSelection()` | Czyści zaznaczenie |
| `std::optional<CellCoord> getSelectedCell() const` | Zaznaczona komórka (puste, gdy brak) |
| `std::optional<SelectionRange> getSelectionRange() const` | Zaznaczony zakres (puste, gdy brak) |

Zaznaczenie myszą: klik = pojedyncza komórka, przeciągnięcie = zakres.

### Edycja w miejscu

| Metoda | Opis |
|--------|------|
| `void setEditable(bool editable)` | Włącza/wyłącza edycję (domyślnie włączona) |
| `bool isEditable() const` | Czy edycja włączona |
| `void startEditing(size_t row, size_t col)` | Rozpoczyna edycję komórki (nad komórką pojawia się `TextInput`) |
| `void stopEditing()` | Kończy edycję (wywołuje callback `setOnCellEdit`) |
| `bool isEditing() const` | Czy trwa edycja |

Skróty klawiszowe: `Enter` na zaznaczonej komórce rozpoczyna edycję, `Escape` przerywa.

### Przewijanie

| Metoda | Opis |
|--------|------|
| `void setHorizontalScrollEnabled(bool enabled)` | Pokaż/ukryj poziomy pasek przewijania |
| `void setVerticalScrollEnabled(bool enabled)` | Pokaż/ukryj pionowy pasek przewijania |
| `bool isHorizontalScrollEnabled() const` | Czy poziomy pasek aktywny |
| `bool isVerticalScrollEnabled() const` | Czy pionowy pasek aktywny |
| `int getVerticalSliderMax() const` | Maksymalna wartość pionowego paska |
| `int getVerticalScrollOffset() const` | Aktualne przesunięcie pionowe w pikselach |
| `int getRowHeight() const` | Wysokość wiersza |

### Kolory

| Metoda | Opis |
|--------|------|
| `void setSelectionColor(SDL_Color color)` | Kolor tła zaznaczenia |
| `void setSelectedCellBorderColor(SDL_Color color)` | Kolor obramowania zaznaczonej komórki |
| `SDL_Color getSelectionColor() const` | Aktualny kolor zaznaczenia |
| `SDL_Color getSelectedCellBorderColor() const` | Aktualny kolor obramowania |

## Callbacki / zdarzenia

| Metoda | Typ callbacka | Kiedy wywoływany |
|--------|---------------|------------------|
| `void setOnCellClick(CellCallback cb)` | `std::function<void(StringGrid*, CellCoord)>` | Kliknięcie komórki |
| `void setOnCellDoubleClick(CellCallback cb)` | `std::function<void(StringGrid*, CellCoord)>` | Podwójne kliknięcie komórki |
| `void setOnCellEdit(EditCallback cb)` | `std::function<void(StringGrid*, CellCoord, std::string)>` | Zakończenie edycji (nowy tekst w trzecim argumencie) |
| `void setOnSelectionChange(SelectionCallback cb)` | `std::function<void(StringGrid*, SelectionRange)>` | Zmiana zaznaczenia (klik lub przeciągnięcie; dla pojedynczej komórki `start == end`) |

W callbackach nie trzymaj surowych wskaźników do innych widgetów, które mogą zostać usunięte — użyj `ElementRef<T>` (patrz przykład).

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("StringGrid", 800, 600);
        GUIManager manager(app.getRenderer(), Viewport{800, 600});
        manager.setTheme(ThemePresets::createDarkTheme());

        StringGrid* grid = manager.create<StringGrid>(10, 10, 560, 300, 5, 4);
        grid->setColumnHeader(0, "Nazwa");
        grid->setColumnHeader(1, "Typ");
        grid->setColumnHeader(2, "Wartość");

        const char* data[5][3] = {
            {"Płyn", "ciecz", "100"},
            {"Woda", "ciecz", "50"},
            {"Żelazo", "ciało stałe", "20"},
            {"Hel", "gaz", "5"},
            {"Stal", "ciało stałe", "80"},
        };
        for (size_t r = 0; r < 5; ++r)
            for (size_t c = 0; c < 3; ++c)
                grid->setCellText(r, c, data[r][c]);

        grid->setCustomComparator(2, [](const std::string& a, const std::string& b) {
            return std::stoi(a) < std::stoi(b);   // sortowanie numeryczne po "Wartość"
        });

        auto status = manager.create<Label>(10, 330, "Kliknij komórkę lub nagłówek");
        auto statusRef = manager.makeRef(status);

        grid->setOnCellClick([statusRef](StringGrid*, CellCoord c) {
            if (statusRef) {
                statusRef->setText("Klik: [" + std::to_string(c.row) + ", "
                                   + std::to_string(c.col) + "]");
            }
        });
        grid->setOnCellEdit([](StringGrid* g, CellCoord c, std::string text) {
            g->setCellText(c.row, c.col, text);   // zapis nowej wartości
        });

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);
            }
            manager.update();
            manager.cleanup();
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

## Uwagi

- Kliknięcie nagłówka kolumny sortuje automatycznie (cykl kierunków). Domyślnie sortowanie jest leksykograficzne; dla liczb ustaw komparator per kolumna (`setCustomComparator`).
- `Ctrl+C` kopiuje zaznaczenie do schowka; strzałki przesuwają zaznaczenie; `Enter`/`Escape` sterują edycją.
- `getCellText` zwraca `std::string_view`, który traci ważność po każdej modyfikacji siatki — skopiuj, jeśli chcesz przechować tekst.
- Kółko myszy przewija pionowo (gdy aktywny pionowy pasek); paski są wewnętrznymi `Slider`ami sterowanymi przez widget — nie dodawaj własnych.
- Zakończenie edycji zamyka wbudowany `TextInput` i woła `setOnCellEdit` — nowa wartość NIE jest wpisywana do siatki automatycznie, musisz zrobić to w callbacku (jak wyżej).
- `clear()` usuwa dane, ale nie zmienia rozmiarów siatki; do zmniejszenia użyj `setRowCount`/`setColumnCount`.
- StringGrid jest bazą dla `ListView` — zob. dokumentacja ListView.
