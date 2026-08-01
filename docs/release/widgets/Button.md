# Button

Przycisk akcji — podstawowy, klikalny element interfejsu z etykietą tekstową. Użyj go do wywoływania akcji, potwierdzania operacji lub nawigacji.

## Przeznaczenie

Przycisk reaguje na kliknięcie lewym przyciskiem myszy (wywołanie callbacka po zwolnieniu przycisku na elemencie) oraz na aktywację klawiaturą (Enter/Space), gdy ma ustawione `setCanGetKeyboardFocus(true)`. Obsługuje stany wizualne Normal/Hover/Pressed/Disabled z cascading theme. Do komunikacji z innymi widgetami w callbacku używaj `ElementRef<T>`.

## Tworzenie

```cpp
Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label = "");
```

```cpp
auto btn = std::make_unique<Button>(manager, 10, 10, 120, 40, "Kliknij");
guiManager.addElement(std::move(btn));

// lub krócej — GUIManager::create<T> zwraca surowy wskaźnik:
Button* btn = manager.create<Button>(10, 10, 120, 40, "Kliknij");
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setOnClickCallback(OnClickCallback callback)` | Ustawia callback wywoływany po kliknięciu |
| `void setOnMouseOverCallback(OnMouseOverCallback callback)` | Ustawia callback najechania myszą (patrz Uwagi) |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Kolor tła per stan (dziedziczone z GUIElement) |
| `void setTextColor(ElementState state, SDL_Color color)` | Kolor tekstu per stan (dziedziczone) |
| `void setBorder(ElementState state, SDL_Color color, int width)` | Obramowanie per stan (dziedziczone) |
| `void setBorderRadius(ElementState state, int radius)` | Zaokrąglenie rogów per stan (dziedziczone) |
| `void setEnabled(bool enabled)` / `bool isEnabled()` | Włączanie/wyłączanie (dziedziczone) |
| `void setTooltip(const std::string& text)` | Tooltip przy najechaniu (dziedziczone) |
| `void setID(std::string_view id)` | Identyfikator elementu (dziedziczone) |
| `void setCanGetKeyboardFocus(bool canFocus)` | Pozwala na aktywację Enter/Space po Tab (dziedziczone) |

Typy callbacków (publiczne aliasy klasy):

```cpp
using OnClickCallback      = std::function<void(GUIElement*)>;
using OnMouseOverCallback  = std::function<void(GUIElement*)>;
```

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| Kliknięcie | `void(GUIElement*)` | Po zwolnieniu lewego przycisku myszy na przycisku (też z klawiatury Enter/Space) |
| Najechanie | `void(GUIElement*)` | Najechanie kursorem — w tej wersji zdefiniowany, ale nie jest wywoływany (patrz Uwagi) |

Callback kliknięcia otrzymuje `GUIElement*` — rzutuj na `Button*`, a jeśli przycisk został przekazany przez `std::move`, trzymaj `ElementRef<Button>` utworzoną **przed** przeniesieniem:

```cpp
Button* btn = manager.create<Button>(10, 10, 120, 40, "Kliknij");
auto ref = manager.makeRef<Button>(btn);

auto counter = manager.create<Label>(150, 20, "0");

btn->setOnClickCallback([ref, counter](GUIElement* e) {
    if (auto* b = static_cast<Button*>(e)) {
        if (ref) ref->setEnabled(false);            // bezpieczny dostęp po std::move
        // ...
    }
});
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Przycisk", 400, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto label = manager.create<Label>(150, 80, "Kliknięć: 0");
        auto btn = manager.create<Button>(120, 120, 160, 40, "Kliknij mnie");
        auto ref = manager.makeRef(label);

        btn->setOnClickCallback([ref](GUIElement*) {
            static int count = 0;
            if (ref) ref->setText("Kliknięć: " + std::to_string(++count));
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

- `setOnMouseOverCallback` jest wywoływany przy wejściu myszy na przycisk (przejście stanu do `Hover`); do wizualnej reakcji na hover używaj stylów stanu `ElementState::Hover` (`setBackgroundColor(ElementState::Hover, ...)` itp.).
- Callback kliknięcia dostaje `GUIElement*` — rzutowanie `static_cast<Button*>(e)` jest bezpieczne w kontekście tego callbacka.
- `manager.create<T>(...)` zwraca surowy wskaźnik ważny, dopóki element żyje w `GUIManager`; do użycia w callbackach po przeniesieniu ownership zawsze twórz `ElementRef` przed `std::move`.
- Bez wywołania `manager.cleanup()` w pętli elementy z `markForDeletion()` nigdy nie zostaną usunięte.
