# TabControl

Kontener z zakładkami: pasek przycisków-zakładek u góry i obszar treści pod spodem. Użyj go do grupowania zawartości w kilka widoków przełączanych jednym kliknięciem.

## Przeznaczenie

TabControl dziedziczy po `Panel` i składa się z paska zakładek (rząd `Button`ów, układanych poziomo z odstępem 5 px) oraz panelu treści aktywnej zakładki. Każda zakładka ma własny panel zawartości — do niego dodaje się dziećmi właściwe widgety (współrzędne względem tego panelu). Pierwsza dodana zakładka staje się aktywna automatycznie. Idealny do formularzy wielosekcyjnych, ustawień i podglądów trybów.

## Tworzenie

```cpp
TabControl(GUIManager& manager, int x, int y, int width, int height,
           int tabButtonHeight = 30);
```

```cpp
auto tabs = std::make_unique<TabControl>(manager, 10, 10, 500, 300, 32);
manager.addElement(std::move(tabs));

// albo przez skrót:
TabControl* tabs = manager.create<TabControl>(10, 10, 500, 300, 32);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `Panel* addTab(std::string_view title, int width = 100, int height = -1)` | Dodaje zakładkę; `width` to szerokość jej przycisku, `height` — wysokość przycisku (`-1` = `tabButtonHeight` z konstruktora). Zwraca panel zawartości zakładki |
| `void setActiveTab(Button* tabButton)` | Uaktywnia zakładkę na podstawie wskaźnika na jej przycisk |
| `void setActiveTab(int index)` | Uaktywnia zakładkę po indeksie (0-based; błędny indeks jest ignorowany) |

Kliknięcie przycisku zakładki przełącza ją automatycznie — ręczne `setActiveTab` potrzebne jest tylko do sterowania z kodu.

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("TabControl", 800, 600);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());
        manager.setWindowSize(800, 600);

        TabControl* tabs = manager.create<TabControl>(10, 10, 500, 300, 32);

        Panel* tab1 = tabs->addTab("Informacje", 100);
        tab1->addChild(std::make_unique<Label>(manager, 10, 10, "Wersja 1.0"));
        tab1->addChild(std::make_unique<Label>(manager, 10, 30, "Autor: SDL GUI"));

        Panel* tab2 = tabs->addTab("Ustawienia", 110);
        tab2->addChild(std::make_unique<Checkbox>(manager, 10, 10, 22, 22));

        Panel* tab3 = tabs->addTab("Pomoc", 90);
        tab3->addChild(std::make_unique<Label>(manager, 10, 10, "Klikaj zakładki u góry."));

        manager.create<Button>(530, 10, 160, 30, "Przełącz na Ustawienia")->setOnClickCallback([tabs](GUIElement*) {
            tabs->setActiveTab(1);
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

- `addTab` zwraca panel zawartości; widgety dodawaj przez `panel->addChild(...)` — mają wtedy współrzędne względem panelu zakładki, nie całego TabControl.
- Pierwsza dodana zakładka jest uaktywniana automatycznie; przełączanie przez `setActiveTab` nie wymaga zapamiętywania przycisków — indeksy są w kolejności `addTab`.
- Panel treści zajmuje obszar pod paskiem zakładek: `y = tabButtonHeight`, `height = height - tabButtonHeight`.
- Zawartość nieaktywnych zakładek jest ukrywana (`setVisible(false)`), więc nie przechwytują zdarzeń ani nie są renderowane.
- Wskaźniki `Panel*` zwrócone przez `addTab` są ważne, dopóki żyje TabControl — nie usuwaj ich ręcznie.
