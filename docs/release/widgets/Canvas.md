# Canvas

Płótno do rysowania odręcznego — użytkownik maluje lewym przyciskiem myszy
bezpośrednio na obszarze widgetu. Służy do podpisów, adnotacji, szkicowania
i prostych narzędzi „whiteboard".

## Przeznaczenie

`Canvas` utrzymuje własną teksturę renderowania (RGBA8888), na której rysuje
pędzlem kwadratowym o stałym rozmiarze 4×4 piksele. Rysowanie odbywa się
podczas przeciągania myszy z wciśniętym lewym przyciskiem; współrzędne są
klampowane do granic tekstury, więc nie można wyjechać poza obszar. Widget
renderuje się bez cache (`wantsDirectRender()` zwraca `true`) — zawartość
jest blitowana na ekran bezpośrednio z tekstury płótna.

## Tworzenie

```cpp
Canvas(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto canvas = std::make_unique<Canvas>(manager, 20, 20, 460, 320);
canvas->setPenColor(SDL_Color{20, 120, 220, 255});
manager.addElement(std::move(canvas));
// lub: Canvas* canvas = manager.create<Canvas>(20, 20, 460, 320);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void clear()` | Czyści całe płótno (wypełnia białym kolorem) |
| `void setPenColor(SDL_Color color)` | Ustawia kolor pędzla dla kolejnych pociągnięć |

Poza tym widget dziedziczy po `GUIElement` — działa z nim stylizacja (tło,
obramowanie), tooltipy i callbacki z klasy bazowej.

## Callbacki / zdarzenia

Widget nie ma dedykowanych callbacków. Interakcja odbywa się przez zdarzenia
myszy: `SDL_EVENT_MOUSE_BUTTON_DOWN` (lewy przycisk) rozpoczyna rysowanie,
`SDL_EVENT_MOUSE_MOTION` rysuje odcinek od ostatniego punktu, a
`SDL_EVENT_MOUSE_BUTTON_UP` kończy pociągnięcie. Podczas rysowania mysz jest
przechwytywana przez `GUIManager::captureMouse` — pociągnięcie można
kontynuować poza obszarem widgetu (punkty i tak są klampowane do płótna).

## Przykład

Płótno z trzema przyciskami zmieniającymi kolor pędzla i przyciskiem
czyszczącym:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Canvas", 520, 420);
        GUIManager manager(app.getRenderer(), Viewport{520, 420});
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE

        Canvas* canvas = manager.create<Canvas>(20, 20, 480, 320);

        auto redBtn = std::make_unique<Button>(manager, 20, 360, 100, 36, "Czerwony");
        redBtn->setOnClickCallback([canvas](GUIElement*) {
            if (canvas) canvas->setPenColor(SDL_Color{220, 40, 40, 255});
        });

        auto blueBtn = std::make_unique<Button>(manager, 130, 360, 100, 36, "Niebieski");
        blueBtn->setOnClickCallback([canvas](GUIElement*) {
            if (canvas) canvas->setPenColor(SDL_Color{40, 120, 220, 255});
        });

        auto clearBtn = std::make_unique<Button>(manager, 240, 360, 100, 36, "Wyczyść");
        clearBtn->setOnClickCallback([canvas](GUIElement*) {
            if (canvas) canvas->clear();
        });

        manager.addElement(std::move(redBtn));
        manager.addElement(std::move(blueBtn));
        manager.addElement(std::move(clearBtn));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);    // 1. zdarzenia
            }
            manager.update();               // 2. timery, animacje, tooltipy
            manager.cleanup();              // 3. usuwanie elementów
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();               // 4. rysowanie
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

- Rozmiar pędzla jest stały (4×4 px) — nie ma API do jego zmiany. Grubsze
  linie można uzyskać skalując płótno albo rysując wielokrotnie.
- Płótno startuje białe; `clear()` zawsze wraca do bieli. Rysowanie kolorem
  o alpha < 255 nakłada się na biały spód.
- Tekstura płótna jest odtwarzana przy zmianie rozmiaru widgetu — zmiana
  `setSize`/`setWidth`/`setHeight` **wymazuje** narysowaną zawartość.
- Nie ma undo/redo ani zapisu stanu — zawartość to wewnętrzna tekstura.
- Rysowanie działa tylko lewym przyciskiem myszy; prawy przycisk jest
  ignorowany (i nie zamyka np. menu kontekstowego).
- Ze względu na render bez cache, duże płótna rysowane co klatkę są
  kosztowne — trzymaj rozmiar w rozsądnych granicach.
