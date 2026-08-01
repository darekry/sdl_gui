# RangeSlider

Podwójny suwak do wyboru przedziału wartości `[lower, upper]` wewnątrz
zakresu `[min, max]` — np. zakres cen, zakres dat, zakres jasności.
W przeciwieństwie do `Slider` ma dwa uchwyty, z których każdy przeciąga się
niezależnie.

## Przeznaczenie

`RangeSlider` przydaje się wszędzie tam, gdzie filtr ma dolną i górną
granicę: „od–do" w UI filtrów, zakresy liczbowe, selekcja przedziału czasu.
Dwa uchwyty nie mogą się minąć — wartość dolna jest zawsze
`≤` wartości górnej. Po najechaniu myszą działa kółko (zmienia uchwyt,
nad którym stoi kursor; bez kursora nad uchwytem — dolny). Widget dziedziczy
po `Panel`, więc dziedziczy też stylowanie tła i obramowania. Nie ma
wbudowanych przycisków „+/-" (w przeciwieństwie do `Slider`).

## Tworzenie

```cpp
RangeSlider(GUIManager& manager, int x, int y, int width, int height,
            int minValue, int maxValue, int lowerValue, int upperValue,
            Orientation orientation);
```

- `orientation` — `Orientation::Horizontal` lub `Orientation::Vertical`.
- Obie wartości początkowe są klampowane do `[minValue, maxValue]`;
  jeśli `lowerValue > upperValue`, są automatycznie zamieniane miejscami.

```cpp
auto range = std::make_unique<RangeSlider>(manager, 40, 60, 320, 40,
                                           0, 100, 25, 75, Orientation::Horizontal);
manager.addElement(std::move(range));
// lub: RangeSlider* range = manager.create<RangeSlider>(0, 100, 25, 75, Orientation::Horizontal);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `int getLowerValue() const` | Bieżąca wartość dolna (zawsze `≥ min` i `≤ upper`) |
| `int getUpperValue() const` | Bieżąca wartość górna (zawsze `≥ lower` i `≤ max`) |
| `void setLowerValue(int value)` | Ustawia dolną wartość; klampuje do `[min, upper]`, woła callback przy realnej zmianie |
| `void setUpperValue(int value)` | Ustawia górną wartość; klampuje do `[lower, max]`, woła callback przy realnej zmianie |
| `int getMin() const` / `int getMax() const` | Granice zakresu |
| `void setMin(int min)` / `void setMax(int max)` | Zmiana granicy; obie wartości re-klampowane |
| `void setRange(int min, int max)` | Ustawia obie granice naraz; obie wartości re-klampowane |
| `void setWheelStep(int step)` | Krok kółka myszy; wartości ≤ 0 są zamieniane na 1 |
| `int getWheelStep() const` | Aktualny krok kółka |

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| onChange | `std::function<void(GUIElement*)>` ustawiany przez `void setOnChangeCallback(OnChangeCallback callback)` | Po każdej zmianie którejkolwiek z wartości — drag uchwytu, klik na torze, kółko, `setLowerValue`/`setUpperValue`/`setMin`/`setMax`/`setRange`. Wywoływany tylko gdy wartość faktycznie się zmieniła. |

W callbacku argument to `GUIElement*` — rzutuj na `RangeSlider*`
(`static_cast<RangeSlider*>(e)`) i odczytaj `getLowerValue()` /
`getUpperValue()`. Do odwołań do innych widgetów używaj `ElementRef`
utworzonego **przed** `std::move`:

```cpp
auto lowerLabel = std::make_unique<Label>(manager, 40, 110, "25", 16);
auto lowerRef = manager.makeRef(lowerLabel.get());   // PRZED std::move!
```

## Przykład

Suwak zakresu 0–100 (początkowo 25–75) z dwiema etykietami pokazującymi
obie wartości, aktualizowanymi w jednym callbacku:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("RangeSlider", 420, 220);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE
        manager.setWindowSize(420, 220);                     // dla anchorów

        auto lowerLabel = std::make_unique<Label>(manager, 60, 110, "25", 16);
        auto upperLabel = std::make_unique<Label>(manager, 300, 110, "75", 16);
        auto lowerRef = manager.makeRef(lowerLabel.get());   // PRZED std::move
        auto upperRef = manager.makeRef(upperLabel.get());

        auto range = std::make_unique<RangeSlider>(manager, 40, 60, 340, 40,
                                                   0, 100, 25, 75, Orientation::Horizontal);
        range->setWheelStep(5);
        range->setOnChangeCallback([lowerRef, upperRef](GUIElement* e) {
            auto* r = static_cast<RangeSlider*>(e);
            if (r) {
                if (lowerRef) lowerRef->setText(std::to_string(r->getLowerValue()));
                if (upperRef) upperRef->setText(std::to_string(r->getUpperValue()));
            }
        });

        manager.addElement(std::move(range));
        manager.addElement(std::move(lowerLabel));
        manager.addElement(std::move(upperLabel));

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

- Niezmiennik `min ≤ lower ≤ upper ≤ max` jest egzekwowany zawsze:
  `setLowerValue` klampuje do `[min, upper]`, `setUpperValue` do
  `[lower, max]`, a w konstruktorze wartości początkowe są klampowane,
  a ewentualna zamiana (lower > upper) naprawiana.
- `onChange` odpala się tylko przy realnej zmianie — ustawienie tej samej
  wartości nie wywołuje callbacku.
- Kliknięcie na torze (poza uchwytami) przyciąga **bliższy** uchwyt do
  pozycji kursora. Trzymając lewy przycisk można potem ciągnąć.
- Kółko myszy działa tylko nad suwakiem: zmienia uchwyt, nad którym stoi
  kursor (hover), a gdy kursor nie jest nad żadnym — dolny.
- Podczas dragu wyjście kursora poza obszar suwaka **puszcza uchwyt**
  (drag nie jest kontynuowany poza widgetem) — w przeciwieństwie do
  `Slider`, który trzyma drag również po wyjściu poza obszar.
- Uchwyty nie mogą się minąć: gdy dociągniesz dolny do górnego, dolny
  zatrzyma się na wartości górnej (i odwrotnie).
- Suwak nie ma wbudowanych przycisków „<"/„>" — do precyzyjnej zmiany
  używaj kółka myszy (`setWheelStep`) albo metod `setLowerValue`/
  `setUpperValue`.
