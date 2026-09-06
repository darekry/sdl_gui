# Slider

Suwak wyboru wartości liczbowej z zakresu [min, max]. W przeciwieństwie do
`RangeSlider` pozwala wybrać jedną wartość — np. głośność, jasność, prędkość,
albo dowolny parametr liczbowy aplikacji.

## Przeznaczenie

`Slider` obsługuje trzy sposoby zmiany wartości: przeciąganie uchwytu,
kliknięcie na torze (skok do pozycji kursora) oraz dwa wbudowane przyciski
„<" i „>" zmieniające wartość o 1. Po najechaniu myszą działa też kółko —
zmiana o `wheelStep` (domyślnie 1). Widget dziedziczy po `Panel`, więc
można go stylizować (tło, obramowanie) i dodawać do niego dzieci, a na
wierzchu znajduje się narysowany tor i kwadratowy uchwyt.

## Tworzenie

```cpp
Slider(GUIManager& manager, int x, int y, int width, int height,
       int minValue, int maxValue, int initialValue, Orientation orientation);
```

- `orientation` — `Orientation::Horizontal` (poziomy, domyślny przypadek
  użycia) lub `Orientation::Vertical` (uchwyt przesuwany w pionie).
- `initialValue` jest klampowane do `[minValue, maxValue]` — można podać
  dowolną wartość, nie zepsuje to stanu suwaka.

```cpp
auto slider = std::make_unique<Slider>(manager, 40, 60, 260, 40,
                                       0, 100, 50, Orientation::Horizontal);
manager.addElement(std::move(slider));
// lub: Slider* slider = manager.create<Slider>(0, 100, 50, Orientation::Horizontal);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `int getValue() const` | Bieżąca wartość (zawsze w zakresie `[min, max]`) |
| `void setValue(int value)` | Ustawia wartość; klampuje do zakresu, woła callback jeśli wartość faktycznie się zmieniła |
| `int getMin() const` / `int getMax() const` | Granice zakresu |
| `void setMin(int min)` | Zmienia dolną granicę; bieżąca wartość jest re-klampowana |
| `void setMax(int max)` | Zmienia górną granicę; bieżąca wartość jest re-klampowana |
| `void setRange(int min, int max)` | Ustawia obie granice naraz (wartość re-klampowana) |
| `void setWheelStep(int step)` | Krok kółka myszy; wartości ≤ 0 są zamieniane na 1 |
| `int getWheelStep() const` | Aktualny krok kółka |
| `[[nodiscard]] Orientation getOrientation() const` | Orientacja suwaka |
| `Button* getDecrementButton()` | Wbudowany przycisk „<" (własność suwaka — nie usuwaj ręcznie) |
| `Button* getIncrementButton()` | Wbudowany przycisk „>" (własność suwaka — nie usuwaj ręcznie) |

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| onChange | `std::function<void(GUIElement*)>` ustawiany przez `void setOnChangeCallback(OnChangeCallback callback)` | Po każdej zmianie wartości — drag, klik na tor, przyciski, kółko, `setValue`/`setMin`/`setMax`/`setRange`. Wywoływany tylko gdy wartość faktycznie się zmieniła. |

W callbacku argument to `GUIElement*` — rzutuj na `Slider*`
(`static_cast<Slider*>(e)`). Jeśli w callbacku odwołujesz się do innego
widgetu (np. etykiety), trzymaj go przez `ElementRef` utworzony **przed**
`std::move`:

```cpp
auto label = std::make_unique<Label>(manager, 160, 80, "50", 18);
auto labelRef = manager.makeRef(label.get());   // PRZED std::move!
```

## Przykład

Suwak 0–100 z etykietą pokazującą bieżącą wartość, aktualizowaną przez
callback:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Slider", 420, 200);
        GUIManager manager(app.getRenderer(), Viewport{420, 200});
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE

        auto label = std::make_unique<Label>(manager, 170, 82, "50", 18);
        auto labelRef = manager.makeRef(label.get());        // PRZED std::move

        auto slider = std::make_unique<Slider>(manager, 40, 60, 260, 40,
                                               0, 100, 50, Orientation::Horizontal);
        slider->setWheelStep(5);
        slider->setOnChangeCallback([labelRef](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s && labelRef) {
                labelRef->setText(std::to_string(s->getValue()));
            }
        });

        manager.addElement(std::move(slider));
        manager.addElement(std::move(label));

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

- Wartość jest zawsze klampowana do `[min, max]` — dotyczy to też
  `initialValue` w konstruktorze. `setValue(1000)` przy maksimum 100 da 100.
- `onChange` odpala się tylko przy realnej zmianie — ustawienie tej samej
  wartości nie wywołuje callbacku.
- Kliknięcie na torze (poza uchwytem) przeskakuje wartość do pozycji
  kursora; przytrzymany lewy przycisk pozwala ciągnąć uchwyt.
- Kółko myszy działa tylko gdy kursor znajduje się nad suwakiem (`hover`).
  Zmiana o `wheelStep` w górę lub w dół; `setWheelStep(0)` i wartości ujemne
  są zamieniane na 1.
- Przyciski „<" i „>" zmieniają wartość zawsze o ±1 — przy dużym zakresie
  wygodniejsze są drag albo kółko. Są to zwykłe `Button` (dzieci suwaka);
  przez `getDecrementButton()`/`getIncrementButton()` możesz je dostylować,
  ale nie usuwaj ich ani nie przejmuj ownership — zarządza nimi `Slider`.
- Kursor myszy nie jest przechwytywany podczas dragu — przeciąganie poza
  obszar suwaka nie zmienia wartości (w przeciwieństwie do `RangeSlider`,
  który puszcza uchwyt po wyjściu poza obszar).
- Suwak przecina tylko tor i uchwyt — styl z `Panel` (tło, obramowanie)
  widoczny jest wokół nich; kolor uchwytu pochodzi z `thumbColor` stylu
  (`setThumbColor`), z fallbackiem do `borderColor` dla starszych motywów.
