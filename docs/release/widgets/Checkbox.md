# Checkbox

Przełącznik dwustanowy (zaznaczone/odznaczone). Użyj go do opcji włącz/wyłącz i filtrów.

## Przeznaczenie

Checkbox przechowuje stan boolowski i wywołuje callback przy każdej zmianie (kliknięcie lub Space, gdy ma focus klawiaturowy). Callback dostaje wskaźnik do checkboxa oraz nową wartość — to inny wzorzec niż w `Button` (gdzie callback dostaje `GUIElement*`). Stan można też ustawiać programowo przez `setChecked()`.

## Tworzenie

```cpp
Checkbox(GUIManager& manager, int x, int y, int w, int h);
```

```cpp
auto cb = std::make_unique<Checkbox>(manager, 20, 20, 20, 20);
guiManager.addElement(std::move(cb));

// lub krócej:
Checkbox* cb = manager.create<Checkbox>(20, 20, 20, 20);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `bool isChecked() const` | Aktualny stan |
| `void setChecked(bool checked)` | Ustawia stan (wywołuje też callback) |
| `void setOnChange(OnChangeCallback callback)` | Callback przy każdej zmianie stanu |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Tło per stan (dziedziczone z GUIElement) |
| `void setBorder(ElementState state, SDL_Color color, int width)` | Obramowanie per stan (dziedziczone) |
| `void setEnabled(bool enabled)` / `bool isEnabled()` | Włączanie/wyłączanie (dziedziczone) |
| `void setCanGetKeyboardFocus(bool canFocus)` | Przełączanie Space po Tab (dziedziczone) |
| `void setTooltip(const std::string& text)` | Tooltip (dziedziczone) |

Typ callbacka (publiczny alias klasy):

```cpp
using OnChangeCallback = std::function<void(Checkbox*, bool)>;
```

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| Zmiana stanu | `void(Checkbox*, bool)` | Po kliknięciu, po `setChecked()` oraz przy przełączeniu klawiaturą (Space) |

Parametr `bool` to nowa wartość (`checked`). Dostęp do innych widgetów po `std::move` przez `ElementRef`:

```cpp
Checkbox* cb = manager.create<Checkbox>(20, 20, 20, 20);
auto label = manager.create<Label>(50, 22, "Wycisz dźwięk");
auto ref = manager.makeRef(label);

cb->setOnChange([ref](Checkbox*, bool checked) {
    if (ref) ref->setText(checked ? "Dźwięk wyciszony" : "Dźwięk włączony");
});
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Checkbox", 400, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto status = manager.create<Label>(20, 24, "Opcja: OFF");

        auto cb = manager.create<Checkbox>(20, 20, 20, 20);
        auto ref = manager.makeRef(status);
        cb->setOnChange([ref](Checkbox*, bool checked) {
            if (ref) ref->setText(checked ? "Opcja: ON" : "Opcja: OFF");
        });

        auto cb2 = manager.create<Checkbox>(20, 60, 20, 20);
        cb2->setChecked(true);   // stan początkowy (wywoła callback)

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

- `setChecked()` wywołuje callback — uważaj na rekurencję, jeśli callback z powrotem ustawia stan.
- W przeciwieństwie do `Button`, callback dostaje `Checkbox*` i `bool` wprost — rzutowanie nie jest potrzebne.
- Wymiary `w`/`h` powinny być kwadratowe (np. 20×20) dla poprawnego wyglądu znacznika.
