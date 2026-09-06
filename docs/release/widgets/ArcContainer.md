# ArcContainer

Kontener układający dzieci na łuku okręgu — każdy element dostaje pozycję na
obwodzie pod zadanym kątem. Służy do interfejsów radialnych: menu
pierścieniowe, tarcze, panele wokół centralnego punktu, wskaźniki.

## Przeznaczenie

`ArcContainer` rozmieszcza dzieci wokół środka o współrzędnych
`(centerX, centerY)` i promieniu `radius`. Dziecko dodaje się przez
`addChildAtAngle` z kątem w stopniach (0° = prawa strona, 90° = dół, kąty
dodatnie zgodnie z ruchem wskazówek zegara — oś Y ekranu skierowana jest w
dół). Pozycja dziecka jest wycentrowana na punkcie łuku. Sam kontener nic
nie rysuje — rysują się tylko jego dzieci.

## Tworzenie

```cpp
ArcContainer(GUIManager& manager, int centerX, int centerY, int radius,
             float startAngleDeg = 0.0f, float endAngleDeg = 360.0f);
```

```cpp
auto arc = std::make_unique<ArcContainer>(manager, 200, 200, 150);
manager.addElement(std::move(arc));
// lub: ArcContainer* arc = manager.create<ArcContainer>(200, 200, 150);
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void addChildAtAngle(std::unique_ptr<GUIElement> child, float angleDeg, bool rotateChild = true, int offset = 0)` | Dodaje dziecko i pozycjonuje je na łuku pod kątem `angleDeg`; `rotateChild` = obróć dziecko o `angleDeg + 90°`; `offset` = przesunięcie po promieniu (dodawane do `radius`) |
| `void setRadius(int radius)` | Zmienia promień oraz rozmiar i pozycję kontenera |
| `void setArcRange(float startAngleDeg, float endAngleDeg)` | Zakres kątowy (w stopniach) używany przez `contains` — od jakiego do jakiego kąta kontener „łapie" kliknięcia |
| `bool contains(int x, int y) const` | Czy punkt (współrzędne okna) leży w pierścieniu kontenera (`radius ± 10`) i w zakresie kątowym; dostępny też przeciążony `bool contains(float x, float y) const` |
| `ComponentType getComponentTypeId() const` | Zwraca `ComponentType::ArcContainer` |

## Callbacki / zdarzenia

Brak dedykowanych callbacków — zachowanie kontenera (sprawdzanie `contains`
dla kliknięć) i zdarzenia dzieci działają standardowo. Dzieci to zwykłe
elementy GUI: mają własne callbacki, hover i tooltipy.

## Przykład

Osiem przycisków rozmieszczonych równomiernie na okręgu (co 45°), bez
rotacji, z klikalnym pierścieniem ograniczonym do pierwszych 180°:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("ArcContainer", 500, 500);
        GUIManager manager(app.getRenderer(), Viewport{500, 500});
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE

        auto arc = std::make_unique<ArcContainer>(manager, 250, 250, 180);
        arc->setArcRange(0.0f, 180.0f);

        for (int i = 0; i < 8; ++i) {
            auto btn = std::make_unique<Button>(manager, 0, 0, 90, 34, "Opcja " + std::to_string(i + 1));
            btn->setOnClickCallback([i](GUIElement*) {
                std::fprintf(stderr, "Klik: opcja %d\n", i + 1);
            });
            arc->addChildAtAngle(std::move(btn), i * 45.0f, false);
        }

        manager.addElement(std::move(arc));

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

- `addChildAtAngle` wycentrowuje dziecko na punkcie łuku — środek dziecka
  leży na promieniu, nie jego lewy górny róg. Ustaw rozmiar dziecka
  (`setSize`) **przed** dodaniem; pozycja liczona jest z aktualnych
  wymiarów.
- `rotateChild` obraca dziecko o `angleDeg + 90°` (rotation wokół środka
  dziecka). Dla tekstu i przycisków zwykle chcesz `false`.
- `setRadius` zmienia położenie kontenera, ale nie przelicza pozycji już
  dodanych dzieci — kąt i odległość od środka zostają takie, jak w momencie
  `addChildAtAngle`. Dlatego ustaw promień i zakres łuku **przed** dodaniem
  dzieci.
- `contains` działa na pierścień o szerokości ok. 60 px (od `radius − 50`
  do `radius + 10`): kliknięcia w środek koła ani poza pierścień nie są
  łapane przez kontener (ale dzieci obsługują zdarzenia normalnie).
- `setArcRange` wpływa tylko na `contains` — nie zmienia pozycji dzieci.
  Przekroczenie zakresu 0–360° jest normalizowane (kąty ujemne i ≥ 360° są
  zawijane).
- Kontener nie rysuje tła ani łuku — jeśli potrzebujesz wizualizacji łuku,
  dodaj własny element (np. `Canvas` lub teksturę) jako dziecko.
- Kąty: 0° = prawa strona, 90° = dół (Oś Y skierowana w dół), 180° = lewa
  strona, 270° = góra.
