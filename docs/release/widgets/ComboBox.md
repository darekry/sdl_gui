# ComboBox

Rozwijana lista wyboru: pole z aktualną wartością i przycisk rozwijający listę opcji. Użyj go, gdy użytkownik ma wybrać jedną z kilku predefiniowanych wartości.

## Przeznaczenie

ComboBox przechowuje listę opcji tekstowych i pozwala wybrać jedną z nich. Kliknięcie rozwija/zamyka listę, kliknięcie opcji ustawia ją jako aktualną i wywołuje callback `on_selection_changed`. W przeciwieństwie do `Panel`-owych widgetów dziedziczy bezpośrednio po `GUIElement`, więc jest lekkim, samowystarczalnym elementem — idealnym do formularzy, filtrów i selektorów trybu.

## Tworzenie

```cpp
ComboBox(GUIManager& manager, int x, int y, int w, int h);
```

```cpp
auto combo = std::make_unique<ComboBox>(manager, 10, 10, 200, 30);
manager.addElement(std::move(combo));

// albo przez skrót:
ComboBox* combo = manager.create<ComboBox>(10, 10, 200, 30);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void addItem(std::string_view item)` | Dopisuje opcję |
| `void addItem(std::string&& item)` | Dopisuje opcję (wersja z przeniesieniem) |
| `void addItem(const char* item)` | Dopisuje opcję (wersja z literałem) |
| `void clearItems()` | Usuwa wszystkie opcje |
| `size_t getItemCount() const` | Liczba opcji |
| `std::string getItem(size_t index) const` | Tekst opcji o indeksie `index` |
| `std::string getSelectedItem() const` | Tekst aktualnie wybranej opcji |
| `int getSelectedIndex() const` | Indeks wybranej opcji (lub -1, gdy brak) |
| `void setSelectedIndex(int index)` | Ustawia wybraną opcję programowo |
| `bool isExpanded() const` | Czy lista jest rozwinięta |

## Callbacki / zdarzenia

W przeciwieństwie do innych widgetów, callback to **publiczne pole** przypisywane bezpośrednio:

```cpp
std::function<void(int, const std::string&)> on_selection_changed;
```

Wywoływane po wybraniu opcji z listy (lub przez `setSelectedIndex`), z indeksem i tekstem wybranej opcji:

```cpp
combo->on_selection_changed = [](int index, const std::string& text) {
    // index = wybrany indeks, text = wybrany tekst
};
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ComboBox", 800, 600);
        GUIManager manager(app.getRenderer(), Viewport{800, 600});
        manager.setTheme(ThemePresets::createDarkTheme());

        ComboBox* combo = manager.create<ComboBox>(10, 10, 200, 30);
        combo->addItem("Czerwony");
        combo->addItem("Zielony");
        combo->addItem("Niebieski");
        combo->setSelectedIndex(0);

        auto status = manager.create<Label>(10, 60, "Wybrano: Czerwony");
        auto statusRef = manager.makeRef(status);

        combo->on_selection_changed = [statusRef](int, const std::string& text) {
            if (statusRef) {
                statusRef->setText("Wybrano: " + text);
            }
        };

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

- `setSelectedIndex` ustawia wartość i wywołuje `on_selection_changed` — możesz dzięki temu centralnie obsługiwać zmiany (także te z kodu).
- Rozwinięta lista rysuje się nad innymi elementami — ComboBox nie jest przystosowany do bycia dzieckiem przycinającym zawartość (np. wewnątrz przewijanego obszaru może być obcinany).
- `getItem`/`getSelectedItem` zwracają `std::string` przez wartość — bezpieczne do przechowania.
- Kliknięcie poza listą zamyka ją (zachowanie domyślne).
