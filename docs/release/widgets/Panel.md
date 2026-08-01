# Panel

Kontener na inne widgety — prostokątny obszar z tłem, obramowaniem i opcjonalnym przeciąganiem. Użyj go do grupowania elementów i budowania layoutu.

## Przeznaczenie

Panel to najprostszy kontener: przyjmuje dzieci przez `addChild()` (współrzędne dzieci są względne do panelu), rysuje tło/obramowanie z cascading theme i może być przeciągany myszą. Jest też bazą dla widgetów złożonych (`RadioGroup`, `ProgressBar`, `Slider` itd. dziedziczą po Panel). Nie ma własnych callbacków — interakcję zapewnia `setDraggable()`.

## Tworzenie

```cpp
Panel(GUIManager& manager, int x, int y, int width, int height);
Panel(GUIManager& manager, SDL_Rect rect);
```

```cpp
auto panel = std::make_unique<Panel>(manager, 20, 20, 300, 200);
panel->setBackgroundColor(ElementState::Normal, {45, 48, 58, 255});
guiManager.addElement(std::move(panel));

// lub przez GUIManager::create<T>:
Panel* panel = manager.create<Panel>(20, 20, 300, 200);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setDraggable(bool draggable)` | Włącza przeciąganie panelu (i jego dzieci) myszą |
| `GUIElement* addChild(std::unique_ptr<GUIElement> child)` | Dodaje dziecko; zwraca surowy wskaźnik do niego (dziedziczone z GUIElement) |
| `void clearChildren()` | Usuwa wszystkie dzieci (dziedziczone) |
| `const std::vector<std::unique_ptr<GUIElement>>& getChildren()` | Lista dzieci (dziedziczone) |
| `GUIElement* getParent()` / `GUIElement* findElementAt(int x, int y)` | Nawigacja w hierarchii (dziedziczone) |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Tło per stan (dziedziczone) |
| `void setBorder(ElementState state, SDL_Color color, int width)` | Obramowanie per stan (dziedziczone) |
| `void setBorderRadius(ElementState state, int radius)` | Zaokrąglenie rogów per stan (dziedziczone) |
| `void setAnchor(const Anchor& anchor)` | Responsywne pozycjonowanie (dziedziczone) |
| `void setEnabled(bool)` / `void setVisible(bool)` / `void setTooltip(...)` | Standardowe właściwości (dziedziczone) |

## Callbacki / zdarzenia

Panel nie definiuje własnych callbacków. Interakcje:

- **Przeciąganie** — po `setDraggable(true)` panel przesuwa się za kursorem (capture myszy przez `GUIManager::captureMouse()`); zwolnienie przycisku kończy drag. Brak callbacka zmiany pozycji.
- **Dzieci** — zdarzenia trafiają najpierw do dzieci; panel dostaje je tylko, gdy żadne dziecko nie obsłużyło kliknięcia.

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Panel", 500, 400);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        // Kontener nadrzędny — dzieci mają współrzędne względem niego
        auto panel = manager.create<Panel>(20, 20, 300, 200);
        panel->setBackgroundColor(ElementState::Normal, {45, 48, 58, 255});
        panel->setBorder(ElementState::Normal, {98, 114, 164, 255}, 2);
        panel->setDraggable(true);

        // Dzieci (współrzędne względem panelu!)
        panel->addChild(std::make_unique<Button>(manager, 10, 10, 120, 36, "OK"));
        panel->addChild(std::make_unique<Button>(manager, 140, 10, 120, 36, "Anuluj"));
        panel->addChild(std::make_unique<Label>(manager, 10, 60, "Przeciągnij panel"));

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

- Dzieci mają współrzędne **względem rodzica**, nie okna. Podobnie anchor dzieci działa względem rozmiaru panelu.
- Panel przeciągany myszą trzyma capture do zwolnienia przycisku — w tym czasie inne widgety nie otrzymają zdarzeń myszy.
- Dzieci dodawane przez `addChild()` są usuwane razem z panelem; nie musisz ich czyścić ręcznie.
- `manager.create<T>(parent, args...)` tworzy widget i od razu dodaje go jako dziecko — wygodne dla struktur panelowych.
