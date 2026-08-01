# ProgressBar

Pasek postępu (dziedziczy po `Panel`). Użyj go do wizualizacji postępu operacji, poziomów, procentów itp.

## Przeznaczenie

`ProgressBar` renderuje wypełnienie od 0 do `getMax()` (domyślnie 0..100), opcjonalnie z tekstem procentowym. Wartość ustawia się przez `setValue()` (clamp do zakresu + automatyczne odświeżenie) albo bezpośrednio przez `getValuePtr()` — wskaźnik przeznaczony do animacji przez `AnimationManager`. Orientację (poziomą/pionową) i format tekstu można zmieniać w locie. Jako kontener dziedziczy po `Panel`, ale zwykle używa się go bez dzieci.

## Tworzenie

```cpp
ProgressBar(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto bar = std::make_unique<ProgressBar>(manager, 20, 20, 300, 24);
guiManager.addElement(std::move(bar));

// lub krócej:
ProgressBar* bar = manager.create<ProgressBar>(20, 20, 300, 24);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `float getValue() const` / `void setValue(float value)` | Aktualna wartość / ustawienie (clamp do min..max, odświeża widok) |
| `float getMin() const` / `float getMax() const` | Zakres (domyślnie 0 i 100) |
| `void setMin(float min)` / `void setMax(float max)` | Zmiana granic zakresu |
| `void setRange(float min, float max)` | Ustawienie obu granic naraz |
| `void setOrientation(Orientation orientation)` | `Orientation::Horizontal` (domyślnie) lub `Orientation::Vertical` |
| `Orientation getOrientation() const` | Aktualna orientacja |
| `void setShowText(bool show)` / `bool getShowText() const` | Włącza/wyłącza tekst procentowy (domyślnie włączony) |
| `void setTextFormat(const std::string& format)` | Format tekstu w stylu printf (domyślnie `"%.0f%%"`) |
| `float* getValuePtr()` | Surowy wskaźnik do wartości — do animacji przez `AnimationManager` |

`Orientation` to wspólny typ enumeracji biblioteki: `Orientation::Horizontal`, `Orientation::Vertical`.

## Callbacki / zdarzenia

`ProgressBar` nie definiuje własnych callbacków — to wskaźnik informacyjny, nie interaktywny. Zmiany wartości z zewnątrz:

```cpp
bar->setValue(50.0f);   // skok; setValue sam odświeża (markDirty)
```

Animacja przez `AnimationManager::createAnimation(..., on_frame)`: animacja modyfikuje `*getValuePtr()` w każdym kroku `update()`, ale **nie odświeża cache'u tekstury** — przekaż `on_frame` wołający `markDirty()`:

```cpp
manager.getAnimationManager()->createAnimation(
    bar->getValuePtr(), 0.0f, 100.0f, 3000,
    Easing::easeOutQuad,
    nullptr,
    [bar]() { bar->markDirty(); });
```

Prostsza alternatywa bez wskaźnika — pętla z `setValue()` (odświeżanie wbudowane):

```cpp
manager.getAnimationManager()->addAnimation(16, [bar](...) {
    static float v = 0.0f;
    bar->setValue(v);
    v += 1.0f;
    if (v > 100.0f) v = 0.0f;
});
```

## Przykład

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ProgressBar", 500, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());

        auto bar = manager.create<ProgressBar>(20, 20, 300, 24);
        bar->setValue(0.0f);

        // Płynna animacja 0 -> 100% w 3 sekundy (on_frame odświeża widok)
        manager.getAnimationManager()->createAnimation(
            bar->getValuePtr(), 0.0f, 100.0f, 3000,
            Easing::easeOutQuad, nullptr,
            [bar]() { bar->markDirty(); });

        // Wariant pionowy, bez tekstu
        auto vbar = manager.create<ProgressBar>(350, 20, 24, 120);
        vbar->setOrientation(Orientation::Vertical);
        vbar->setShowText(false);
        vbar->setValue(35.0f);

        // Własny format tekstu
        auto fbar = manager.create<ProgressBar>(20, 60, 300, 24);
        fbar->setTextFormat("Postęp: %.1f%%");
        fbar->setValue(25.0f);

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);
            }
            manager.update();       // animacje
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

- `getValuePtr()` zwraca surowy `float*` — nie trzymaj go dłużej niż żyje pasek. `AnimationManager` ostrzega: animacja na usuniętym obiekcie to undefined behavior.
- `setValue()` clampuje do zakresu i sam wywołuje `markDirty()`. Po bezpośrednim zapisie przez `*getValuePtr()` odświeżenie widoku wymaga ręcznego `markDirty()` — stąd `on_frame` w animacji.
- Format tekstu to format printf — `%.0f%%` pokazuje liczby całkowite z procentem, `%.1f%%` z jedną cyfrą po przecinku. Format jest stosowany do znormalizowanego procentu (0..100). Tekst znika po `setShowText(false)`.
- Dla orientacji pionowej ustaw rozmiar `height > width` (np. 24×120), dla poziomej odwrotnie.
- Kolor wypełnienia pochodzi z `borderColor` stylu (`Theme`/`setBorder`), a kolor tekstu z `textColor`.
