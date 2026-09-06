# ListView

Lista tekstowa w jednej kolumnie, zbudowana na bazie `StringGrid` — ukrywa nagłówki i pasek poziomy, zostawiając przejrzystą, przewijaną listę wierszy. Użyj go do prostych list wyboru, logów, list plików itp.

## Przeznaczenie

ListView upraszcza `StringGrid` do jednej kolumny: wiersze dodaje się przez `addItem`, a cała obsługa (przewijanie kółkiem, pasek pionowy, zaznaczanie, edycja) działa jak w StringGrid. Kliknięcie wiersza wywołuje `setOnRowClick`, podwójne kliknięcie — `setOnRowDoubleClick` i `setOnRowActivate` (aktywacja). Nadaje się do list, z których wybiera się jeden element lub które służą jako wejście do dalszej akcji (np. otwarcie pliku po podwójnym kliknięciu).

## Tworzenie

```cpp
ListView(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto list = std::make_unique<ListView>(manager, 10, 10, 300, 200);
manager.addElement(std::move(list));

// albo przez skrót:
ListView* list = manager.create<ListView>(10, 10, 300, 200);
```

## Najważniejsze metody

### Zarządzanie wierszami

| Metoda | Opis |
|--------|------|
| `void addItem(const std::string& text)` | Dopisuje wiersz na końcu listy |
| `void insertItem(size_t index, const std::string& text)` | Wstawia wiersz na pozycji `index` (indeks poza zakresem = dopisanie na końcu) |
| `void removeItem(size_t index)` | Usuwa wiersz o indeksie `index` (ignoruje błędny indeks) |
| `void clearItems()` | Usuwa wszystkie wiersze |
| `size_t getItemCount() const` | Liczba wierszy |
| `std::string_view getItem(size_t index) const` | Tekst wiersza (widok ważny do następnej modyfikacji) |
| `void setItem(size_t index, const std::string& text)` | Nadpisuje tekst wiersza |

### Zaznaczanie

| Metoda | Opis |
|--------|------|
| `void setSelectedRow(size_t row)` | Zaznacza wiersz programowo |
| `std::optional<size_t> getSelectedRow() const` | Indeks zaznaczonego wiersza (puste, gdy brak) |
| `void clearSelection()` | Czyści zaznaczenie |

## Callbacki / zdarzenia

| Metoda | Typ callbacka | Kiedy wywoływany |
|--------|---------------|------------------|
| `void setOnRowClick(RowCallback cb)` | `std::function<void(ListView*, size_t)>` | Kliknięcie wiersza (indeks w drugim argumencie) |
| `void setOnRowDoubleClick(RowCallback cb)` | `std::function<void(ListView*, size_t)>` | Podwójne kliknięcie wiersza |
| `void setOnRowActivate(RowCallback cb)` | `std::function<void(ListView*, size_t)>` | Aktywacja wiersza (wywoływana przy podwójnym kliknięciu, zaraz po `setOnRowDoubleClick`) |

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ListView", 800, 600);
        GUIManager manager(app.getRenderer(), Viewport{800, 600});
        manager.setTheme(ThemePresets::createDarkTheme());

        ListView* list = manager.create<ListView>(10, 10, 300, 250);
        list->addItem("main.cpp");
        list->addItem("gui.hpp");
        list->addItem("theme.cpp");
        list->addItem("CMakeLists.txt");

        auto info = manager.create<Label>(10, 280, "Wybierz plik");
        auto infoRef = manager.makeRef(info);

        list->setOnRowClick([infoRef, list](ListView*, size_t row) {
            if (infoRef) {
                infoRef->setText("Zaznaczono: " + std::string(list->getItem(row)));
            }
        });
        list->setOnRowActivate([infoRef, list](ListView*, size_t row) {
            if (infoRef) {
                infoRef->setText("Otwieram: " + std::string(list->getItem(row)));
            }
        });

        manager.create<Button>(330, 10, 120, 30, "Dodaj plik")->setOnClickCallback([list](GUIElement*) {
            list->addItem("nowy_plik.txt");
        });
        manager.create<Button>(330, 50, 120, 30, "Usuń zaznaczony")->setOnClickCallback([list](GUIElement*) {
            if (auto sel = list->getSelectedRow()) {
                list->removeItem(*sel);
            }
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

- ListView dziedziczy wszystkie metody `StringGrid` (m.in. `setCellText`, `setEditable`, `setOnCellEdit`, `setVerticalScrollEnabled`) — działa na kolumnie 0.
- Nagłówki i poziomy pasek przewijania są domyślnie ukryte; pionowy pasek i przewijanie kółkiem działają od razu.
- `setSelectedRow` ustawia stan z kodu (nie wywołuje callbacków wiersza — `setOnRowClick`/`setOnRowActivate` reagują tylko na kliknięcia myszą).
- W callbackach nie trzymaj surowych wskaźników do widgetów, które mogą zostać usunięte — używaj `ElementRef<T>`.
- `getItem` zwraca `std::string_view` — skopiuj tekst, jeśli chcesz go przechować po modyfikacji listy.
