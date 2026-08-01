# ContextMenu

Menu kontekstowe (prawy przycisk myszy) — pionowa lista elementów z
opcjonalnymi separatorami, pojawiająca się na żądanie w zadanej pozycji.
Służy do akcji kontekstowych: „Wytnij/Kopiuj/Wklej", operacji na liście,
opcji widoku itp.

## Przeznaczenie

`ContextMenu` buduje wewnętrznie `Panel` z przyciskami (po jednym na
element) i rysuje się tylko po wywołaniu `showAt`. Kliknięcie w element
wykonuje jego akcję i zamyka menu; kliknięcie poza menu również je zamyka.
Menu startuje ukryte. Elementy nieaktywne (`enabled = false`) są widoczne,
ale nieklikalne.

## Tworzenie

```cpp
ContextMenu(GUIManager& manager);
```

```cpp
auto menu = std::make_unique<ContextMenu>(manager);
menu->addItem("Kopiuj", []() { /* ... */ });
menu->addItem("Wklej", []() { /* ... */ }, false);  // nieaktywne
menu->addSeparator();
menu->addItem("Usuń", []() { /* ... */ });
manager.addElement(std::move(menu));
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)` | Dodaje element menu; `action` wołana po kliknięciu (menu zamyka się automatycznie), `enabled = false` blokuje klik |
| `void addSeparator()` | Dodaje poziomy separator wizualny |
| `void clearItems()` | Usuwa wszystkie elementy (i separator) |
| `void showAt(int x, int y)` | Pozycjonuje menu (przycinając do krawędzi okna) i pokazuje je |
| `void hide()` | Ukrywa menu |
| `bool isVisible() const` | Czy menu jest widoczne |

Wewnętrzna struktura elementu:

```cpp
struct ContextMenuItem {
    std::string text;
    std::function<void()> action;
    bool enabled;
    bool separator;
};
```

## Callbacki / zdarzenia

Callbacki przekazuje się w `addItem` jako `action` — są wywoływane po
kliknięciu w element, po czym menu jest zamykane. Menu samo obsługuje
zdarzenia: `handleEvent` zamyka menu po kliknięciu poza jego obszarem.
Jeśli w akcji odwołujesz się do widgetów przeniesionych do managera,
używaj `ElementRef` utworzonego przed `std::move`.

## Przykład

Menu kontekstowe otwierane prawym przyciskiem myszy, z akcją zmieniającą
etykietę:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ContextMenu", 500, 320);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE
        manager.setWindowSize(500, 320);                     // dla anchorów

        auto info = std::make_unique<Label>(manager, 40, 200, "Kliknij prawym przyciskiem", 16);
        auto infoRef = manager.makeRef(info.get());          // PRZED std::move

        auto menu = std::make_unique<ContextMenu>(manager);
        menu->addItem("Opcja A", [infoRef]() {
            if (infoRef) infoRef->setText("Wybrano: A");
        });
        menu->addItem("Opcja B", [infoRef]() {
            if (infoRef) infoRef->setText("Wybrano: B");
        });
        menu->addItem("Zablokowana", nullptr, false);
        menu->addSeparator();
        menu->addItem("Zamknij", []() { std::fprintf(stderr, "Zamykanie\n"); });

        ContextMenu* menuPtr = menu.get();   // ważny po std::move (własność: manager)
        manager.addElement(std::move(menu));
        manager.addElement(std::move(info));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_RIGHT) {
                    menuPtr->showAt((int)e.button.x, (int)e.button.y);
                }
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

- Menu otwiera się tylko przez `showAt(x, y)` — współrzędne są w
  przestrzeni okna (tak jak `e.button.x/y` z eventu SDL3).
- `showAt` przycina pozycję do krawędzi okna, ale wewnętrznie używa stałego
  rozmiaru okna 800×600 — przy oknie innego rozmiaru menu może nie być
  idealnie dopasowane do krawędzi.
- Szerokość menu jest stała (200 px), wysokość elementu 25 px, separatora
  8 px — dopasuj liczbę elementów do okna.
- Kliknięcie w element z `action == nullptr` (i `enabled == true`) zamyka
  menu bez akcji. Kliknięcie poza menu zawsze zamyka menu.
- `addSeparator` to zwykły element wizualny (szary `Panel`) — nie ma akcji.
- Po `clearItems()` menu pozostaje ukryte aż do następnego `showAt`.
- Elementy są budowane (jako przyciski wewnątrz `Panel`) dopiero przy
  pierwszym `showAt` — zmiana `addItem` po pokazaniu menu wymaga ponownego
  `showAt`, aby odświeżyć listę.
