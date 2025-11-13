# AnimatedImage — animowany widget obrazka

`AnimatedImage` to widget pozwalający wyświetlać animacje klatkowe z pliku typu sprite-sheet.

**Kluczowe funkcje:**
- Odtwarzanie klatek w stałym tempie (`play`/`pause`/`stop`).
- Płynne animowanie przejścia między klatkami.
- Konfiguracja skalowania i zachowania proporcji obrazu.

## Konstrukcja i podstawowe użycie

Aby użyć `AnimatedImage`, należy go utworzyć, załadować arkusz sprite'ów i dodać do `GUIManager`.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"

int main() {
    SDLApp app("Animowany obraz", 640, 480);
    GUIManager gui(app.getRenderer());

    // Tworzenie widgetu
    auto anim = std::make_unique<AnimatedImage>(gui, 50, 50, 256, 128);

    // Załaduj sprite-sheet, podając ścieżkę, liczbę klatek i wierszy
    anim->setSpriteSheet("assets/my_sprite.png", 12, 3);

    // Konfiguracja i start
    anim->setFPS(12.0f);
    anim->setLoop(true);
    anim->play();

    gui.addElement(std::move(anim));

    // Pętla główna aplikacji
    while (app.isRunning()) {
        app.handleEvents();
        gui.processEvent(app.getEvent());
        
        app.clearScreen();
        gui.render();
        app.present();
    }

    return 0;
}
```

## Kontrola odtwarzania

Do sterowania animacją służą metody:
- `play()`: Uruchamia odtwarzanie w pętli (jeśli `setLoop(true)`).
- `pause()`: Zatrzymuje animację na bieżącej klatce.
- `stop()`: Zatrzymuje animację i resetuje ją do pierwszej klatki.

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->play();
anim_ptr->pause();
anim_ptr->stop();
```

## Animacja do konkretnej klatki

Metoda `animateToFrame` pozwala na płynne przejście do wybranej klatki w określonym czasie.

```cpp
// Płynne przejście do klatki 8 w czasie 500 ms
anim_ptr->animateToFrame(8, 500, false);
```

## Tryby skalowania

Widget `AnimatedImage` oferuje kilka trybów skalowania obrazu, zdefiniowanych w `ScaleMode`:

- `Fit`: Dopasowuje obraz do rozmiaru widgetu (domyślny).
- `Center`: Wyświetla obraz w oryginalnym rozmiarze, wyśrodkowany w widgecie.
- `None`: Wyświetla obraz w oryginalnym rozmiarze w lewym górnym rogu widgetu.

Można również zachować oryginalne proporcje obrazu podczas skalowania.

```cpp
anim_ptr->setScaleMode(AnimatedImage::ScaleMode::Fit);
anim_ptr->setPreserveAspect(true); // Zachowaj proporcje
```

## Referencje API

### Konfiguracja
- `AnimatedImage(GUIManager& manager, int x, int y, int w, int h)`: Konstruktor.
- `setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0)`: Ładuje arkusz sprite'ów.
- `setFPS(float fps)`: Ustawia prędkość odtwarzania w klatkach na sekundę.
- `setFrameDuration(float secondsPerFrame)`: Ustawia czas trwania pojedynczej klatki.
- `setLoop(bool loop)`: Włącza lub wyłącza zapętlanie animacji.
- `setScaleMode(ScaleMode mode)`: Ustawia tryb skalowania.
- `setPreserveAspect(bool preserve)`: Włącza lub wyłącza zachowanie proporcji.

### Kontrola
- `play()`: Rozpoczyna odtwarzanie.
- `pause()`: Pauzuje odtwarzanie.
- `stop()`: Zatrzymuje i resetuje animację.
- `setFrame(int frameIndex)`: Ustawia animację na konkretną klatkę.
- `animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false)`: Animuje przejście do klatki.

### Gettery
- `getCurrentFrame() const`: Zwraca indeks bieżącej klatki.
- `getTotalFrames() const`: Zwraca całkowitą liczbę klatek.
- `isPlaying() const`: Sprawdza, czy animacja jest odtwarzana.

### Callbacki
- `setOnAnimationEnd(std::function<void()> cb)`: Ustawia funkcję zwrotną wywoływaną po zakończeniu animacji (gdy `loop` jest `false`).
- `setOnFrameChanged(std::function<void(int)> cb)`: Ustawia funkcję zwrotną wywoływaną przy każdej zmianie klatki.

## Przykład

Kompletny, działający przykład użycia `AnimatedImage` znajduje się w pliku:
- [`examples/example_animated_image.cpp`](../../examples/example_animated_image.cpp)