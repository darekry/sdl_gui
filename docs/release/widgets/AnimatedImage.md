# AnimatedImage

Widget wyświetlający animację z arkusza sprite'ów (sprite sheet) — jednej
tekstury zawierającej kolejne klatki w siatce wierszy i kolumn. Służy do
animacji postaci, efektów (eksplozje, iskry), ikon ładowania i wszystkiego,
co da się zapisać jako sekwencję klatek.

## Przeznaczenie

`AnimatedImage` odtwarza klatki automatycznie (timer wewnętrzny) lub steruje
nimi ręcznie przez `setFrame()`. Obsługuje pętlę odtwarzania, zmianę
prędkości, skalowanie do rozmiaru widgetu oraz płynną animację między
klatkami przez `AnimationManager`. Klatki są numerowane wierszami: klatka 0
to lewy górny róg tekstury, a liczba kolumn jest wyliczana jako
`ceil(totalFrames / rows)`.

## Tworzenie

```cpp
AnimatedImage(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto anim = std::make_unique<AnimatedImage>(manager, 40, 40, 160, 160);
anim->setSpriteSheet("assets/explosion.png", 16, 4);  // 16 klatek w 4 wierszach
anim->setFPS(12);
anim->play();
manager.addElement(std::move(anim));
// lub: AnimatedImage* anim = manager.create<AnimatedImage>(40, 40, 160, 160);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0)` | Ładuje teksturę i konfiguruje animację. `frameW`/`frameH` równe 0 są wyliczane z rozmiaru tekstury (`width/cols`, `height/rows`) i klampowane do jej wymiarów |
| `void setFPS(float fps)` | Prędkość w klatkach na sekundę (domyślnie 12); wartości ≤ 0 są ignorowane |
| `void setFrameDuration(float secondsPerFrame)` | Alternatywa dla `setFPS` — czas trwania pojedynczej klatki w sekundach; przy odtwarzaniu restartuje timer |
| `void setLoop(bool loop)` | Czy animacja ma się powtarzać po dojściu do ostatniej klatki (domyślnie `true`) |
| `void setUseCache(bool useCache)` | `true` (domyślnie) = render do cache tekstury; `false` = rysowanie bezpośrednie co klatkę |
| `void setScaleMode(ScaleMode mode)` | Sposób dopasowania klatki do obszaru: `ScaleMode::Fit` (domyślny), `ScaleMode::Center` (bez skalowania, wycentrowany), `ScaleMode::None` (lewy górny róg) |
| `void setPreserveAspect(bool preserve)` | Czy przy `ScaleMode::Fit` zachować proporcje klatki (domyślnie `true`) |
| `void play()` | Startuje odtwarzanie od bieżącej klatki |
| `void pause()` | Wstrzymuje odtwarzanie, zachowując bieżącą klatkę |
| `void stop()` | Zatrzymuje odtwarzanie i resetuje do klatki 0 |
| `void setFrame(int frameIndex)` | Natychmiastowa zmiana klatki (klampowana do zakresu) |
| `int getCurrentFrame() const` | Bieżący indeks klatki |
| `int getTotalFrames() const` | Liczba klatek z `setSpriteSheet` |
| `bool isPlaying() const` | Czy animacja jest odtwarzana |
| `void animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false)` | Płynna animacja od bieżącej do docelowej klatki (liniowe przejście przez `AnimationManager`); po zakończeniu woła `setOnAnimationEnd` |

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| onAnimationEnd | `std::function<void()>` ustawiany przez `void setOnAnimationEnd(std::function<void()> cb)` | Po dojściu do ostatniej klatki przy `setLoop(false)` oraz po zakończeniu `animateToFrame` |
| onFrameChanged | `std::function<void(int)>` ustawiany przez `void setOnFrameChanged(std::function<void(int)> cb)` | Po każdej zmianie klatki (odtwarzanie, `setFrame`); argument to nowy indeks klatki |

## Przykład

Animacja z arkusza 16 klatek (4 wiersze), odtwarzana w pętli, z etykietą
pokazującą bieżącą klatkę:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("AnimatedImage", 420, 240);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE
        manager.setWindowSize(420, 240);                     // dla anchorów

        auto frameLabel = std::make_unique<Label>(manager, 160, 220, "klatka: 0", 16);
        auto frameRef = manager.makeRef(frameLabel.get());   // PRZED std::move

        auto anim = std::make_unique<AnimatedImage>(manager, 40, 40, 160, 160);
        anim->setSpriteSheet("assets/explosion.png", 16, 4);
        anim->setFPS(12);
        anim->setLoop(true);
        anim->setOnFrameChanged([frameRef](int frame) {
            if (frameRef) frameRef->setText("klatka: " + std::to_string(frame));
        });
        anim->play();

        manager.addElement(std::move(anim));
        manager.addElement(std::move(frameLabel));

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

- `setSpriteSheet` resetuje animację do klatki 0 i przelicza geometrię klatek.
  Po zmianie arkusza trzeba ponownie wywołać `play()`, jeśli ma być odtwarzany.
- Liczba kolumn to `ceil(totalFrames / rows)` — ostatni wiersz może być
  niepełny; nieużywane pola arkusza są po prostu pomijane.
- Przy `setLoop(false)` odtwarzanie zatrzymuje się na ostatniej klatce i
  wywoływany jest `onAnimationEnd` — po nim `isPlaying()` zwraca `false`.
- `animateToFrame` ustawia `setLoop` na wartość z argumentu `loop` — jeśli
  animacja pętlowa była odtwarzana, jej zachowanie może się zmienić.
- `stop()` resetuje klatkę do 0, ale nie zmienia `onFrameChanged` —
  przejście do klatki 0 jest zgłaszane callbackiem.
- Domyślnie widget renderuje się przez cache (`setUseCache(true)`). Przy
  bardzo częstych zmianach klatki można włączyć `setUseCache(false)` — wtedy
  każda klatka jest rysowana bezpośrednio, bez pośredniej tekstury.
- `ScaleMode::Center` i `ScaleMode::None` nie skalują klatki; przy
  `ScaleMode::Center` klatka większa od obszaru jest przycinana do jego
  rozmiaru.
