# WorldView

Kamera 2D na świat gry (pan-only, bez zoomu): dzieci trzymają współrzędne świata,
a widok pokazuje wycinek o rozmiarze widgetu. Użyj go zamiast ręcznego przesuwania
widgetów, gdy mapa jest większa niż okno.

## Przeznaczenie

`WorldView` dziedziczy po `Panel` i wewnętrznie składa się z przycinającego viewportu
oraz panelu zawartości w rozmiarze świata. `setCamera` przesuwa zawartość o `(-camX, -camY)`
(jak `ScrollArea`, ale programowo — bez sliderów). Renderowanie jest przycinane do
viewportu, więc dzieci poza kamerą są odcinane. Współrzędne dzieci (`addWorldChild`)
to współrzędne świata — nigdy nie trzeba ich przeliczać przy panowaniu.

## Tworzenie

```cpp
WorldView(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto view = std::make_unique<WorldView>(manager, 0, 30, 1024, 645);
view->setWorldSize(2048, 2048);   // mapa większa niż widok
view->setCamera(100, 150);        // lewy górny róg widoku w świecie
manager.addElement(std::move(view));

// albo przez skrót:
WorldView* view = manager.create<WorldView>(0, 30, 1024, 645);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setWorldSize(int width, int height)` | Rozmiar mapy w pikselach; clampuje kamerę |
| `void setCamera(int x, int y)` | Ustawia kamerę (clamp do `[0, world - view]`; świat mniejszy niż widok → `0,0`) |
| `void panBy(int dx, int dy)` | Przesuwa kamerę relatywnie (z clampem) |
| `void centerOn(int worldX, int worldY)` | Centruje widok na punkcie świata |
| `int getCamX() const` / `int getCamY() const` | Aktualna kamera |
| `int getWorldWidth() const` / `int getWorldHeight() const` | Rozmiar świata |
| `GUIElement* addWorldChild(std::unique_ptr<GUIElement> child)` | Dodaje dziecko w koordynatach świata |
| `GUIElement* getContent() const` | Wewnętrzny panel zawartości |
| `SDL_Point worldToScreen(int wx, int wy) const` | `wx - camX` (rysowanie outline'ów, HUD) |
| `SDL_Point screenToWorld(int sx, int sy) const` | `sx + camX` (mysz → logika gry) |

## Przykład

```cpp
WorldView* view = manager.create<WorldView>(0, 30, 1024, 645);
view->setWorldSize(2048, 2048);
view->addWorldChild(std::make_unique<Button>(manager, 500, 400, 60, 30, "U"));

// input: mysz (okno) -> świat (logika)
int wx = e.button.x + view->getCamX();
int wy = e.button.y + view->getCamY();
env.handleWorldClick(wx, wy);

// pan strzałkami
if (key == SDLK_LEFT) view->panBy(-32, 0);
```

## Uwagi

- Kamera jest clampowana — nie da się wyjechać poza mapę.
- `setSize` widoku (resize) przelicza viewport i re-clampuje kamerę.
- Dzieci rysują się normalnie (Z-order hierarchii); sortowanie po depth (izometria)
  nie jest wspierane — to punkt zaczepienia na później.
- Brak zoomu z zasady (mnożyłby hit-test i geometrię klatek `AnimatedImage`).
- Culling: gra może chować dzieci daleko poza kamerą przez `setVisible(false)`.
