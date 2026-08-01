# RadioGroup

Kontener wzajemnie wykluczających się opcji (dziedziczy po `Panel`). Użyj go, gdy użytkownik ma wybrać dokładnie jedną opcję z listy.

## Przeznaczenie

`RadioGroup` tworzy wewnętrznie przyciski `RadioButton` z etykietami tekstowymi (metoda `addOption`), układa je pionowo i gwarantuje, że zaznaczony jest zawsze co najwyżej jeden. Wybór można odczytać przez `getSelectedButton()` albo śledzić callbackiem `setOnSelectionChange` (z indeksem i tekstem opcji). Jako kontener dziedziczy po `Panel` — opcje są jego dziećmi i są usuwane razem z nim.

## Tworzenie

```cpp
RadioGroup(GUIManager& manager, int x, int y, int w, int h);
```

```cpp
auto group = std::make_unique<RadioGroup>(manager, 20, 20, 240, 140);
guiManager.addElement(std::move(group));

// lub krócej:
RadioGroup* group = manager.create<RadioGroup>(20, 20, 240, 140);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `RadioButton* addOption(std::string_view text, bool selected = false)` | Dodaje opcję z etykietą; zwraca wskaźnik do przycisku |
| `void setOptionSpacing(int spacing)` | Odstęp pionowy między opcjami (domyślnie 40) |
| `void setOptionMargins(int buttonX, int labelX, int startY)` | Pozycja przycisku (buttonX), etykiety (labelX) i pierwszej opcji (startY) |
| `void setOptionSizes(int buttonSize, int labelFontSize)` | Rozmiar przycisku i fontu etykiety |
| `void onButtonSelected(RadioButton* selectedButton)` | Zaznacza dany przycisk w grupie (używane wewnętrznie, dostępne publicznie) |
| `RadioButton* getSelectedButton() const` | Aktualnie zaznaczony przycisk lub `nullptr` |
| `void setOnSelectionChange(SelectionChangeCallback callback)` | Callback przy zmianie wyboru |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Tło kontenera (dziedziczone z `Panel`) |

Typ callbacka (publiczny alias klasy):

```cpp
using SelectionChangeCallback = std::function<void(int index, const std::string& text)>;
```

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| Zmiana wyboru | `void(int index, const std::string& text)` | Po kliknięciu opcji lub `setSelected()` na przycisku grupy; `index` to pozycja opcji (0-based), `text` jej etykieta |

Callback dostaje indeks i tekst — nie musisz mapować wskaźników:

```cpp
group->setOnSelectionChange([](int index, const std::string& text) {
    std::printf("Wybrano [%d]: %s\n", index, text.c_str());
});
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("RadioGroup", 400, 240);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto group = manager.create<RadioGroup>(20, 20, 240, 160);
        group->addOption("Zielony", true);          // domyślny wybór
        group->addOption("Niebieski");
        group->addOption("Czerwony");
        group->addOption("Czarny");

        auto status = manager.create<Label>(280, 24, "Zielony");
        auto ref = manager.makeRef(status);

        group->setOnSelectionChange([ref](int, const std::string& text) {
            if (ref) ref->setText(text);
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

- `addOption(text, true)` ustawia domyślny wybór; jeśli żadna opcja nie jest zaznaczona, `getSelectedButton()` zwraca `nullptr`.
- Rozmiar grupy dobieraj do liczby opcji: `startY + n * spacing` (domyślnie 20 + n×40).
- `onButtonSelected()` to metoda publiczna wywoływana przez wewnętrzne przyciski — nie musisz jej wołać ręcznie; `setOnSelectionChange` wystarczy do śledzenia wyboru.
- Opcje są dziećmi grupy (jak `Panel::addChild`) — nie dodawaj ich też do `GUIManager` na top-level.
