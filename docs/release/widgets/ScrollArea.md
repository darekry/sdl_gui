# ScrollArea

Przewijany obszar (viewport) z paskami przewijania: ustawiasz jeden widget jako zawartość, definiujesz jego rozmiar, a ScrollArea pokazuje wycinek z możliwością przewijania. Użyj go do dużych paneli, obrazów lub list, które nie mieszczą się w oknie.

## Przeznaczenie

ScrollArea dziedziczy po `Panel` i wewnętrznie składa się z przycinającego viewportu, panelu zawartości oraz opcjonalnych pasków (`Slider` — pionowy domyślnie włączony, poziomy domyślnie wyłączony). `setContent` przejmuje własność wskazanego widgetu (widget staje się dzieckiem wewnętrznego panelu; jego współrzędne są względem ScrollArea i są przesuwane o aktualny offset). Kółko myszy przewija zawartość (pionowo, gdy aktywny pasek pionowy — w przeciwnym razie poziomo). Idealny do podglądów, dużych formularzy i niestandardowych list.

## Tworzenie

```cpp
ScrollArea(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto scroll = std::make_unique<ScrollArea>(manager, 10, 10, 400, 300);
manager.addElement(std::move(scroll));

// albo przez skrót:
ScrollArea* scroll = manager.create<ScrollArea>(10, 10, 400, 300);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setContent(std::unique_ptr<GUIElement> content)` | Ustawia zawartość; ScrollArea przejmuje własność widgetu (będzie jego rodzicem) |
| `GUIElement* getContent() const` | Wskaźnik do zawartości (lub `nullptr`, gdy nie ustawiono) |
| `void setContentSize(int width, int height)` | Rozmiar zawartości w pikselach; większy od obszaru widocznego = pojawia się przewijanie |
| `void setScrollEnabled(bool vertical, bool horizontal)` | Włącza/wyłącza oba paski naraz |
| `void setVerticalScroll(bool enabled)` | Włącza/wyłącza pasek pionowy |
| `void setHorizontalScroll(bool enabled)` | Włącza/wyłącza pasek poziomy |
| `int getScrollOffsetX() const` | Aktualne przesunięcie poziome w pikselach |
| `int getScrollOffsetY() const` | Aktualne przesunięcie pionowe w pikselach |
| `void setScrollOffset(int x, int y)` | Ustawia przesunięcie programowo (synchronizuje paski) |

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ScrollArea", 800, 600);
        GUIManager manager(app.getRenderer(), Viewport{800, 600});
        manager.setTheme(ThemePresets::createDarkTheme());

        ScrollArea* scroll = manager.create<ScrollArea>(10, 10, 400, 300);
        scroll->setHorizontalScroll(true);

        auto content = std::make_unique<Panel>(manager, 0, 0, 700, 500);
        for (int i = 0; i < 15; ++i) {
            int col = i % 3;
            int row = i / 3;
            auto btn = std::make_unique<Button>(manager, 10 + col * 220, 10 + row * 60, 200, 40,
                                                "Przycisk " + std::to_string(i + 1));
            content->addChild(std::move(btn));
        }
        scroll->setContent(std::move(content));
        scroll->setContentSize(700, 500);

        auto pos = manager.create<Label>(10, 330, "Offset: 0, 0");
        auto posRef = manager.makeRef(pos);

        manager.create<Button>(420, 10, 150, 30, "Skocz do 300, 200")->setOnClickCallback([scroll, posRef](GUIElement*) {
            scroll->setScrollOffset(300, 200);
            if (posRef) posRef->setText("Offset: 300, 200");
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

- Zawartość ustawia się raz przez `setContent` (własność przechodzi na ScrollArea) — późniejszy dostęp do niej masz przez `getContent()`, a rozmiar zmieniasz przez `setContentSize`.
- Jeśli nie ustawisz `setContentSize`, ScrollArea przyjmuje rozmiar własny — wtedy nic się nie przewija.
- Współrzędne widgetów wewnątrz zawartości są względem ScrollArea i nie muszą być dostosowywane do offsetu — robi to widget automatycznie.
- Kółko myszy przewija pionowo, gdy widoczny jest pasek pionowy; po wyłączeniu go (i włączeniu poziomego) kółko przewija poziomo.
- Paski przewijania to wewnętrzne `Slider`y — nie dodawaj własnych pasków do ScrollArea.
