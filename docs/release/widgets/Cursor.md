# Cursor

Własny, teksturowany kursor myszy rysowany jako overlay na wierzchu całego
GUI. Zastępuje systemowy kursor własnymi grafikami (również animowanymi) —
przydatny do gier, stylizowanych interfejsów i aplikacji z własnym
językiem wizualnym.

## Przeznaczenie

`Cursor` pozwala przypisać osobną teksturę do każdego z 9 stanów
(`CursorState`): `Normal`, `Hover`, `Pressed`, `Disabled`, `Busy`, `Text`
oraz trzy sloty `Custom1`–`Custom3`. Każdy stan może być statyczną teksturą
lub animacją ze sprite sheetu. Kursor jest rysowany w `renderOverlay` — na
wierzchu wszystkich elementów — a jego pozycja podąża za myszą. Obiekt
przejmuje `GUIManager` przez `setCursor`; nie dodaje się go przez
`addElement`. Stan zmienia aplikacja przez `setState` (np. w callbackach
hover lub w pętli zdarzeń) — widget nie przełącza stanów automatycznie.

## Tworzenie

```cpp
explicit Cursor(GUIManager& manager);
```

Konstruktor ukrywa systemowy kursor (`SDL_HideCursor`); destruktor przywraca
go. Zamiast `addElement` użyj `setCursor`:

```cpp
auto cursor = std::make_unique<Cursor>(manager);
cursor->setCursorTexture(CursorState::Normal, "assets/cursor.png", 4, 2);
cursor->setCursorTexture(CursorState::Hover,  "assets/cursor_hover.png", 4, 2);
manager.setCursor(std::move(cursor));
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setCursorTexture(CursorState state, const std::string& path, int hotspotX = 0, int hotspotY = 0)` | Przypisuje statyczną teksturę do stanu; `hotspot` to punkt „celownika” w pikselach tekstury |
| `void setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows = 1, float fps = 12.0f, int hotspotX = 0, int hotspotY = 0)` | Animowany kursor ze sprite sheetu (klatki numerowane wierszami); `totalFrames <= 0` = zwykła tekstura; animacja działa przez `AnimationManager` |
| `void setState(CursorState state)` | Przełącza bieżący stan; jeśli stan faktycznie się zmienił, woła `onStateChanged` |
| `CursorState getState() const` | Bieżący stan |
| `void setOffset(int offsetX, int offsetY)` | Przesunięcie rysowania względem hotspotu (w pikselach) |
| `void getOffset(int& offsetX, int& offsetY) const` | Odczyt przesunięcia |
| `void setScale(float scale)` | Skala tekstury (klampowana do minimum 0.1) |
| `float getScale() const` | Bieżąca skala |
| `void setVisible(bool visible)` | Pokazuje/ukrywa kursor; ukrycie przywraca systemowy kursor |

## Callbacki / zdarzenia

| Callback | Sygnatura | Kiedy wywoływany |
|----------|-----------|------------------|
| onStateChanged | `std::function<void(CursorState)>` ustawiany przez `void setOnStateChanged(std::function<void(CursorState)> callback)` | Po zmianie stanu przez `setState` — argument to nowy stan |

`handleEvent` nie robi nic — kursor sam nie reaguje na zdarzenia; `GUIManager`
woła `renderOverlay` podczas renderowania.

## Przykład

Kursor z dwoma stanami (normalny i hover) oraz animowanym stanem „busy"
przełączanym checkboxem:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Cursor", 420, 240);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE
        manager.setWindowSize(420, 240);                     // dla anchorów

        auto cursor = std::make_unique<Cursor>(manager);
        cursor->setCursorTexture(CursorState::Normal, "assets/cursor.png", 4, 2);
        cursor->setCursorTexture(CursorState::Hover,  "assets/cursor_hover.png", 4, 2);
        cursor->setAnimatedCursor(CursorState::Busy, "assets/spinner.png", 8, 2, 12.0f);
        cursor->setScale(1.5f);

        auto busyBox = std::make_unique<Checkbox>(manager, 40, 60, 200, 30);
        busyBox->setOnChange([cursor = manager.makeRef(cursor.get())](Checkbox* cb, bool checked) {
            if (cb && cursor) {
                cursor->setState(checked ? CursorState::Busy : CursorState::Normal);
            }
        });

        manager.setCursor(std::move(cursor));
        manager.addElement(std::move(busyBox));

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

Przełączanie stanu na hover robi się samodzielnie, np. w pętli zdarzeń na
podstawie pozycji myszy (wskaźnik do kursora trzymaj przez `ElementRef`
utworzony przed `setCursor`):

```cpp
auto cursorRef = manager.makeRef(cursor.get());   // PRZED manager.setCursor!
// ...
if (e.type == SDL_EVENT_MOUSE_MOTION) {
    bool overButton = SDL_PointInRect(&SDL_Point{(int)e.motion.x, (int)e.motion.y},
                                      &SDL_Rect{40, 60, 200, 30});
    if (cursorRef)
        cursorRef->setState(overButton ? CursorState::Hover : CursorState::Normal);
}
```

## Uwagi

- Kursor przejmuje `GUIManager` przez `setCursor` — nie dodawaj go przez
  `addElement`; `setCursor` zastępuje poprzedni kursor (ownership przechodzi
  do managera).
- Pozycja rysowania: `mouse − hotspot × scale + offset`. Hotspot skalowany
  razem z teksturą, offset nie.
- Dla stanu bez przypisanej tekstury (lub nieudanego loadu) kursor nie
  rysuje nic — przydziel teksturę dla wszystkich stanów, których używasz.
- Animowane stany odtwarzają się zawsze, niezależnie od tego, czy są
  aktualnie aktywne (animacja zarejestrowana w `AnimationManager` od razu po
  `setAnimatedCursor`) — dotyczy to tylko kosztu, nie wyglądu.
- Zmiana stanu nie jest automatyczna: sam decydujesz (hover, naciśnięcie,
  tryb aplikacji), kiedy wywołać `setState`.
- `setVisible(false)` pokazuje z powrotem systemowy kursor; usunięcie
  obiektu `Cursor` (lub koniec aplikacji) również go przywraca.
- Animowany kursor to sprite sheet — liczba kolumn to `ceil(totalFrames / rows)`,
  klatki numerowane wierszami, tak jak w `AnimatedImage`.
