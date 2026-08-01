# Label

Nieinteraktywny tekst statyczny. Użyj go do tytułów, opisów, wartości i komunikatów w interfejsie.

## Przeznaczenie

Label renderuje tekst z cache'owaną teksturą (dirty-flag) — rozmiar elementu wylicza się automatycznie z tekstu (`recalculateSize()`), więc konstruktor nie przyjmuje szerokości/wysokości. Tekst można zmieniać w trakcie działania przez `setText()`; najczęściej robi się to z callbacka innego widgetu przez `ElementRef`.

## Tworzenie

```cpp
Label(GUIManager& manager, int x, int y, std::string_view text, int font_size = -1);
```

`font_size = -1` oznacza domyślny rozmiar czcionki motywu.

```cpp
auto label = std::make_unique<Label>(manager, 10, 10, "Witaj, świecie!", 18);
guiManager.addElement(std::move(label));

// lub krócej:
Label* label = manager.create<Label>(10, 10, "Witaj, świecie!", 18);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setText(std::string_view text)` | Ustawia nową treść i przelicza rozmiar elementu |
| `const std::string& getText() const` | Zwraca aktualną treść |
| `void setTextColor(ElementState state, SDL_Color color)` | Kolor tekstu per stan (dziedziczone z GUIElement) |
| `void setPosition(int x, int y)` / `void setSize(int w, int h)` | Pozycja/rozmiar (dziedziczone; rozmiar zwykle nadpisze `setText`) |
| `void setTooltip(const std::string& text)` | Tooltip (dziedziczone) |
| `void setID(std::string_view id)` | Identyfikator (dziedziczone) |
| `void setAnchor(const Anchor& anchor)` | Responsywne pozycjonowanie (dziedziczone) |

Typ `getComponentType()`: `"Label"`.

## Callbacki / zdarzenia

Label nie ma własnych callbacków i nie przyjmuje zdarzeń (nie jest klikalny). Zmieniaj jego treść z zewnątrz — z callbacka innego widgetu:

```cpp
Label* label = manager.create<Label>(10, 10, "0");
auto ref = manager.makeRef(label);

// np. w callbacku przycisku:
btn->setOnClickCallback([ref](GUIElement*) {
    if (ref) ref->setText("Wartość zmieniona");
});
```

`ElementRef` musi być utworzony **przed** `std::move` i sprawdza żywotność elementu przy każdym dostępie — bezpieczny po ewentualnym usunięciu widgetu.

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Etykieta", 400, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto title = manager.create<Label>(20, 20, "Licznik", 24);
        title->setTextColor(ElementState::Normal, {98, 114, 164, 255});

        auto counter = manager.create<Label>(20, 60, "0");
        auto ref = manager.makeRef(counter);

        auto btn = manager.create<Button>(20, 100, 140, 40, "Dodaj");
        btn->setOnClickCallback([ref](GUIElement*) {
            static int n = 0;
            if (ref) ref->setText(std::to_string(++n));
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

- Rozmiar labela wynika z tekstu — `setSize()` ma sens tylko wtedy, gdy tekst się nie zmienia lub gdy element ma anchor.
- Tekst jest cache'owany jako tekstura i odświeżany tylko przy zmianie (treść/kolor/rozmiar fontu) — zmiana stylu przez bezpośrednią modyfikację `Style` wymaga `markDirty()`.
- `font_size` podaje się tylko w konstruktorze; do zmiany rozmiaru fontu w trakcie działania utwórz nowy Label.
