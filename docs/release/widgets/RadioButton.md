# RadioButton

Pojedynczy przycisk opcji (kółko wyboru). Użyj go samodzielnie do przełącznika, a do wzajemnie wykluczających się opcji — w połączeniu z `RadioGroup`.

## Przeznaczenie

RadioButton przechowuje stan wyboru (bool) i wywołuje callback przy każdej zmianie. **Samodzielnie nie odznacza innych przycisków** — jeśli potrzebujesz wyboru jeden-z-wielu, umieść przyciski w `RadioGroup` (który automatycznie zarządza wzajemną ekskluzywnością) albo odznaczaj pozostałe ręcznie w callbacku.

## Tworzenie

```cpp
RadioButton(GUIManager& manager, int x, int y, int w, int h);
```

```cpp
auto rb = std::make_unique<RadioButton>(manager, 20, 20, 20, 20);
guiManager.addElement(std::move(rb));

// lub krócej:
RadioButton* rb = manager.create<RadioButton>(20, 20, 20, 20);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `bool isSelected() const` | Aktualny stan zaznaczenia |
| `void setSelected(bool selected)` | Ustawia stan (wywołuje callback) |
| `void setOnChange(OnChangeCallback callback)` | Callback przy każdej zmianie stanu |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Tło per stan (dziedziczone z GUIElement) |
| `void setBorder(ElementState state, SDL_Color color, int width)` | Obramowanie per stan (dziedziczone) |
| `void setEnabled(bool enabled)` / `bool isEnabled()` | Włączanie/wyłączanie (dziedziczone) |
| `void setTooltip(const std::string& text)` | Tooltip (dziedziczone) |

Typ callbacka (publiczny alias klasy):

```cpp
using OnChangeCallback = std::function<void(RadioButton*, bool)>;
```

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| Zmiana stanu | `void(RadioButton*, bool)` | Po kliknięciu i po `setSelected()` |

Parametr `bool` to nowa wartość zaznaczenia:

```cpp
RadioButton* rb = manager.create<RadioButton>(20, 20, 20, 20);
auto ref = manager.makeRef(rb);

rb->setOnChange([ref](RadioButton*, bool selected) {
    if (ref) { /* bezpieczny dostęp do widgetu po std::move */ }
    std::printf("Wybrano: %s\n", selected ? "tak" : "nie");
});
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("RadioButton", 400, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto status = manager.create<Label>(50, 24, "Nic nie wybrano");

        // Samodzielne przyciski — zaznaczanie wzajemnie się nie wyklucza
        auto rbA = manager.create<RadioButton>(20, 20, 20, 20);
        auto rbB = manager.create<RadioButton>(20, 60, 20, 20);
        auto ref = manager.makeRef(status);

        auto onChange = [ref](const char* name) {
            return [ref, name](RadioButton*, bool selected) {
                if (selected && ref) ref->setText("Wybrano: " + std::string(name));
            };
        };
        rbA->setOnChange(onChange("A"));
        rbB->setOnChange(onChange("B"));

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

- Do wyboru jeden-z-wielu użyj `RadioGroup` — on zarządza wykluczaniem i daje callback z indeksem opcji.
- `setSelected()` wywołuje callback — nie wołaj go z wnętrza tego samego callbacka bez zabezpieczenia przed rekurencją.
- Wymiary `w`/`h` powinny być kwadratowe (np. 20×20) dla poprawnego wyglądu kółka.
