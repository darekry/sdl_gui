# AnimatedImage — animowany widget obrazka

Krótkie wprowadzenie
---------------------
`AnimatedImage` to widget pozwalający wyświetlać animacje z pliku typu sprite-sheet (kilka klatek w jednej teksturze). Pozwala na:
- odtwarzanie klatek w stałym tempie (play/pause/stop),
- animowanie przejścia między klatkami przez `AnimationManager`,
- konfigurację skalowania i zachowania proporcji,
- pracę z cache'em renderowania lub bezpośrednie rysowanie do renderer'a.

Wymagania i zależności
----------------------
- Dostęp do menedżerów przez `GUIManager`:
  - `TextureManager` (ładowanie tekstur) — używany w implementacji: [`src/animated_image.cpp`](src/animated_image.cpp:26).
  - `AnimationManager` (do płynnych animacji właściwości) — sprawdzany w [`src/animated_image.cpp`](src/animated_image.cpp:334).
  - `TimerManager` (timery dla trybu odtwarzania) — używane przez metody `startTimer`/`stopTimer` dostępne w [`src/gui.hpp`](src/gui.hpp:76).
- Widget dziedziczy po `GUIElement` — konstruktor: [`src/animated_image.hpp`](src/animated_image.hpp:22).

Konstrukcja i podstawowe użycie
-------------------------------
Przykład minimalnego użycia (kompilowalny fragment):

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"

int main() {
    SDLApp app("Animowany obraz", 640, 480);
    GUIManager gui(app.getRenderer());

    // Tworzenie widgetu: zobacz konstruktor
    auto anim = std::make_unique<AnimatedImage>(gui, 50, 50, 256, 128); // konstruktor: [`src/animated_image.hpp`](src/animated_image.hpp:22)

    // Załaduj sprite-sheet (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:19))
    anim->setSpriteSheet("assets/my_sprite.png", 12, 3); // totalFrames = 12, rows = 3

    // Konfiguracja i start
    anim->setFPS(12.0f);
    anim->setLoop(true);
    gui.addElement(std::move(anim));

    // -> reszta pętli SDL: processEvent / render (jak w przykładzie)
    return 0;
}
```

Wywołanie `setSpriteSheet` w implementacji znajduje się w [`src/animated_image.cpp`](src/animated_image.cpp:19) i automatycznie:
- ładuje teksturę przez `TextureManager` (`m_manager.getTextureManager().loadTexture`),
- oblicza geometrię klatek wywołując `recalcFrameGeometry()` ([`src/animated_image.cpp`](src/animated_image.cpp:40)),
- ustawia bieżącą klatkę na 0.

Odtwarzanie (play/pause/stop)
-----------------------------
Kontrola odtwarzania:
- `play()` — uruchamia timer, który co interwał przesuwa klatkę dalej (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:268)).
- `pause()` — zatrzymuje timer (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:303)).
- `stop()` — zatrzymuje timer i resetuje klatkę do 0 (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:312)).

Przykład użycia:

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->play();   // uruchom odtwarzanie (timer-driven)
anim_ptr->pause();  // zatrzymaj (zachowaj aktualną klatkę)
anim_ptr->stop();   // zatrzymaj i ustaw klatkę 0
```

Animacja do konkretnej klatki (animateToFrame)
----------------------------------------------
`animateToFrame(target, duration_ms, loop)` używa `AnimationManager` do animowania wewnętrznej właściwości float `m_animFrame`. Implementacja znajduje się w [`src/animated_image.cpp`](src/animated_image.cpp:333).

Przykład:

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->animateToFrame(8, 500 /*ms*/, false); // płynne przejście do klatki 8 w 500ms
```

Jeśli `AnimationManager` nie jest dostępny, metoda ustawi klatkę natychmiast i wywoła callback zakończenia (zachowanie opisane w implementacji: [`src/animated_image.cpp`](src/animated_image.cpp:335-340)).

Tryby skalowania i zachowanie proporcji
---------------------------------------
Typ skalowania definiowany jest przez enum `ScaleMode` w [`src/animated_image.hpp`](src/animated_image.hpp:15):
- Fit — dopasuj do rozmiaru widgetu (domyślnie),
- Center — wycentruj bez skalowania,
- None — brak skalowania, rysuj w lewym górnym rogu.

Konfiguracja:

```cpp
anim_ptr->setScaleMode(AnimatedImage::ScaleMode::Fit);      // enum: [`src/animated_image.hpp`](src/animated_image.hpp:15)
anim_ptr->setPreserveAspect(true);                         // zachowaj proporcje przy Fit
```

Pełna lista publicznych metod
----------------------------
Poniżej lista publicznych metod wraz z sygnaturami i krótkim opisem. Sygnatury pochodzą z nagłówka: [`src/animated_image.hpp`](src/animated_image.hpp:13).

- setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0) — załaduj sprite-sheet i skonfiguruj liczbę klatek/wierszy. ([`src/animated_image.hpp`](src/animated_image.hpp:27), implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:19))
- setFPS(float fps) — ustaw prędkość w klatkach na sekundę; wrapper do setFrameDuration. ([`src/animated_image.hpp`](src/animated_image.hpp:30))
- setFrameDuration(float secondsPerFrame) — ustaw czas trwania pojedynczej klatki w sekundach; restartuje timer jeśli jest odtwarzanie. ([`src/animated_image.hpp`](src/animated_image.hpp:31), implem: [`src/animated_image.cpp`](src/animated_image.cpp:219))
- setLoop(bool loop) — ustaw, czy animacja ma się zapętlać. ([`src/animated_image.hpp`](src/animated_image.hpp:32))
- setUseCache(bool useCache) — włącz/wyłącz renderowanie do cache (`m_useCache`, domyślnie true); jeśli wyłączone, widget może rysować bezpośrednio przez `drawDirect`. ([`src/animated_image.hpp`](src/animated_image.hpp:33), implem: [`src/animated_image.cpp`](src/animated_image.cpp:234))
- setScaleMode(ScaleMode mode) — ustaw tryb skalowania. ([`src/animated_image.hpp`](src/animated_image.hpp:34))
- setPreserveAspect(bool preserve) — czy zachowywać proporcje przy Fit. ([`src/animated_image.hpp`](src/animated_image.hpp:35))

Kontrola odtwarzania:
- play() — uruchom odtwarzanie (timer-driven). ([`src/animated_image.hpp`](src/animated_image.hpp:38), implem: [`src/animated_image.cpp`](src/animated_image.cpp:268))
- pause() — wstrzymaj odtwarzanie, zatrzymaj timer. ([`src/animated_image.hpp`](src/animated_image.hpp:39), implem: [`src/animated_image.cpp`](src/animated_image.cpp:303))
- stop() — zatrzymaj i zresetuj do klatki 0. ([`src/animated_image.hpp`](src/animated_image.hpp:40), implem: [`src/animated_image.cpp`](src/animated_image.cpp:312))
- setFrame(int frameIndex) — natychmiast ustaw konkretną klatkę. ([`src/animated_image.hpp`](src/animated_image.hpp:41), implem: [`src/animated_image.cpp`](src/animated_image.cpp:200))

Gettery:
- getCurrentFrame() const — zwraca indeks aktualnej klatki. ([`src/animated_image.hpp`](src/animated_image.hpp:44), implem: [`src/animated_image.cpp`](src/animated_image.cpp:256))
- getTotalFrames() const — zwraca łączną liczbę klatek. ([`src/animated_image.hpp`](src/animated_image.hpp:45), implem: [`src/animated_image.cpp`](src/animated_image.cpp:260))
- isPlaying() const — czy aktualnie odtwarzane (timer). ([`src/animated_image.hpp`](src/animated_image.hpp:46), implem: [`src/animated_image.cpp`](src/animated_image.cpp:264))

Callbacki:
- setOnAnimationEnd(std::function<void()> cb) — callback wywoływany po zakończeniu animacji (np. po osiągnięciu końca przy loop=false). ([`src/animated_image.hpp`](src/animated_image.hpp:49), implem: [`src/animated_image.cpp`](src/animated_image.cpp:325))
- setOnFrameChanged(std::function<void(int)> cb) — callback wywoływany po zmianie klatki. ([`src/animated_image.hpp`](src/animated_image.hpp:50), implem: [`src/animated_image.cpp`](src/animated_image.cpp:329))

Animacja właściwości:
- animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false) — płynna animacja wartości klatki przez `AnimationManager` (wartość float `m_animFrame`), implementacja w [`src/animated_image.cpp`](src/animated_image.cpp:333).

Mechanizmy wewnętrzne istotne dla użytkownika
-------------------------------------------
- Obliczanie kolumn/wierszy i wymiarów klatki:
  - `recalcFrameGeometry()` oblicza liczbę kolumn (`m_cols`) oraz szerokość/wysokość klatki (`m_frameW`, `m_frameH`) na podstawie rozmiaru tekstury i ustawień (`totalFrames`, `rows`, ewentualnych podanych `frameW/frameH`). Implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:40).
  - `updateSrcRect()` buduje `SDL_Rect m_srcRect` dla aktualnej klatki (kolumna/wiersz) i znajduje się w [`src/animated_image.cpp`](src/animated_image.cpp:70).
  - Kolumny liczone są jako ceil(totalFrames / rows) (patrz [`src/animated_image.cpp`](src/animated_image.cpp:52)).

- Dwa tryby animacji:
  1. Timer-driven: wywołanie `play()` tworzy timer przez `GUIElement::startTimer` (deklaracja: [`src/gui.hpp`](src/gui.hpp:76)) i co interwał przesuwa klatkę. To tryb klasyczny "frame stepping" (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:268)).
  2. Property animation via `AnimationManager`: `animateToFrame(...)` animuje wewnętrzną właściwość `m_animFrame` (float). `draw()`/`drawDirect()` przeliczają `m_animFrame` na int przez round() i aktualizują `m_currentFrame`. Implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:90) i [`src/animated_image.cpp`](src/animated_image.cpp:151).

- Rola m_useCache:
  - Domyślnie `m_useCache = true` (pole zadeklarowane w [`src/animated_image.hpp`](src/animated_image.hpp:88)).
  - Jeśli `m_useCache` jest false, widget będzie chciał rysować bezpośrednio — metoda `wantsDirectRender()` zwraca false/true (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:140)) i `drawDirect()` jest używane do rysowania bezpośredniego (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:144)).
  - Mechanizm "direct render" został dodany do `GUIElement` i opisany w [`src/gui.hpp`](src/gui.hpp:82). Uwaga: wyłączenie cache powoduje usunięcie ewentualnego zbuforowanego obrazu (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:238-242)).

- Callbacki:
  - `m_onAnimationEnd` — wywoływany po zakończeniu animacji (np. gdy bez zapętlania osiągnięto koniec) — ustawiany przez `setOnAnimationEnd` ([`src/animated_image.hpp`](src/animated_image.hpp:49)).
  - `m_onFrameChanged` — wywoływany po każdej zmianie klatki (zarówno w trybie timer jak i podczas animacji właściwości) — ustawiany przez `setOnFrameChanged` ([`src/animated_image.hpp`](src/animated_image.hpp:50)).

- Timery i czyszczenie:
  - `play()` zapisuje otrzymany identyfikator timera w `m_playTimerId` (pole w nagłówku: [`src/animated_image.hpp`](src/animated_image.hpp:105)) aby móc go zatrzymać w `pause()`/`stop()` (implementacje: [`src/animated_image.cpp`](src/animated_image.cpp:279), [`src/animated_image.cpp`](src/animated_image.cpp:303), [`src/animated_image.cpp`](src/animated_image.cpp:312)).
  - `animateToFrame()` uruchamia dodatkowy "tick timer" (`m_animTickTimerId`) aby regularnie wywoływać `markDirty()` podczas trwania animacji (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:374-380)). Timery są zatrzymywane w odpowiednich miejscach (np. `stopFrameAnimation` w [`src/animated_image.cpp`](src/animated_image.cpp:387)).

Tips & Gotchas (najczęściej przydatne wskazówki)
-----------------------------------------------
- Tekstura się nie ładuje:
  - Sprawdź wartość `m_texturePath` i wynik `m_texture` — jeśli `m_texture` jest null, `ensureTextureLoaded()` próbuje ponownie załadować z `m_texturePath` (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:34-38)). Upewnij się, że ścieżka jest poprawna i plik istnieje.
- Ręczne ustawienie rozmiaru klatki:
  - Możesz podać `frameW` / `frameH` w `setSpriteSheet(...)`. Jeśli pozostawisz je jako 0, widget spróbuje automatycznie obliczyć wymiary klatki dzieląc rozmiar tekstury przez kolumny/wiersze (zob. [`src/animated_image.cpp`](src/animated_image.cpp:55-61)).
- `animateToFrame` vs brak `AnimationManager`:
  - `animateToFrame` używa `AnimationManager` jeśli jest dostępny; gdy go nie ma, metoda ustawi klatkę natychmiast i wywoła `m_onAnimationEnd` (implementacja: [`src/animated_image.cpp`](src/animated_image.cpp:334-340)).
- Direct render a cache:
  - Jeśli chcesz, żeby element był rysowany bezpośrednio (np. aby uniknąć kosztu aktualizacji cache dla często zmieniającej się animacji), wyłącz cache przez `setUseCache(false)` i upewnij się, że `GUIManager`/renderer obsługują `drawDirect()` (patrz [`src/gui.hpp`](src/gui.hpp:82) i implementacja `drawDirect` w [`src/animated_image.cpp`](src/animated_image.cpp:144)).

Dodatkowe odwołania / przykłady
-------------------------------
- Zobacz kompletny przykład w katalogu examples: [`examples/example_animated_image.cpp`](examples/example_animated_image.cpp:1) (pewny, minimalny przykład pokazujący dodanie widgetu do `GUIManager` i toggle cache).
- Konstruktor widgetu: [`src/animated_image.hpp`](src/animated_image.hpp:22).
- Implementacja `setSpriteSheet`: [`src/animated_image.cpp`](src/animated_image.cpp:19).
- Implementacja `animateToFrame`: [`src/animated_image.cpp`](src/animated_image.cpp:333).
- Deklaracja `ScaleMode` (enum): [`src/animated_image.hpp`](src/animated_image.hpp:15).
- Mechanizm drawDirect/wantsDirectRender: [`src/gui.hpp`](src/gui.hpp:82).

Częste linie kodu do przeglądnięcia (ważne miejsca implementacji)
-----------------------------------------------------------------
- Konstruktor / destruktor: [`src/animated_image.cpp`](src/animated_image.cpp:9)
- setSpriteSheet: [`src/animated_image.cpp`](src/animated_image.cpp:19)
- recalcFrameGeometry: [`src/animated_image.cpp`](src/animated_image.cpp:40)
- updateSrcRect: [`src/animated_image.cpp`](src/animated_image.cpp:70)
- draw(): [`src/animated_image.cpp`](src/animated_image.cpp:84)
- drawDirect(): [`src/animated_image.cpp`](src/animated_image.cpp:144)
- play(): [`src/animated_image.cpp`](src/animated_image.cpp:268)
- pause(): [`src/animated_image.cpp`](src/animated_image.cpp:303)
- stop(): [`src/animated_image.cpp`](src/animated_image.cpp:312)
- animateToFrame(): [`src/animated_image.cpp`](src/animated_image.cpp:333)

Zakończenie
-----------
Ten dokument ma pomóc w szybkim zrozumieniu jak używać `AnimatedImage`, jak działa w środku (krótko) oraz jakie problemy można napotkać. Jeśli potrzebujesz fragmentów kodu rozbudowanych o dodatkowe callbacki (np. synchronizacja z innymi widgetami) mogę dodać przykłady pokazujące integrację z `AnimationManager` i `TimerManager`.