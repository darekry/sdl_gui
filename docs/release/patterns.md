# Patterns — zalecane wzorce użycia

Ten dokument zbiera wzorce, których warto się trzymać, oraz pułapki, których
należy unikać. Fundamenty znajdziesz w [core.md](core.md).

## 1. Cykl życia i kolejność wywołań

Kolejność w pętli zdarzeń jest obowiązkowa:

```cpp
manager.processEvent(e);   // 1. dystrybucja zdarzeń
manager.update();          // 2. timery, animacje, tooltipy
manager.cleanup();         // 3. usuwanie elementów z markForDeletion()
manager.render();          // 4. rysowanie
```

Dlaczego taka kolejność:

- `update()` napędza tooltipy (pokazywanie/ukrywanie), animacje i timery —
  bez niego tooltipy wiszą w nieskończoność, a animowane widgety stoją.
- Callbacki wywoływane w trakcie `processEvent`/`update` mogą wołać
  `markForDeletion()`; elementy są faktycznie usuwane dopiero w `cleanup()`,
  po zakończeniu wszystkich callbacków — dzięki temu wskaźniki i `ElementRef`
  są bezpieczne do końca klatki.
- `render()` rysuje hierarchię dopiero po przetworzeniu wszystkich zmian.

Jeśli nie chcesz pisać pętli ręcznie, użyj `SDLApp::run()` lub `GUIContext::run()` —
obie implementują ten sam cykl (patrz [getting_started.md](getting_started.md)).

## 2. Tworzenie widgetów

### Wzorzec A: make_unique → konfiguracja → addElement

```cpp
auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
panel->setBackgroundColor(ElementState::Normal, {45, 48, 58, 255});

auto btn = std::make_unique<Button>(manager, 10, 10, 120, 40, "Kliknij");
btn->setOnClickCallback(...);
panel->addChild(std::move(btn));          // dziecko — pozycja względem panelu

manager.addElement(std::move(panel));     // top-level — pozycja względem okna
```

### Wzorzec B: GUIManager::create<T>

```cpp
// top-level
Button* btn = manager.create<Button>(10, 10, 120, 40, "Kliknij");

// dziecko — drugi argument to rodzic
Button* btn2 = manager.create<Button>(panel, 10, 60, 120, 40, "W środku");
```

`create<T>` tworzy widget, dodaje go od razu i zwraca surowy wskaźnik.
Oba wzorce są równoważne — wybierz wg czytelności.

**Kluczowa zasada**: dzieci mają współrzędne względem rodzica, elementy
top-level względem okna. Mieszanie tych układów daje „dziwne" pozycje.

## 3. Komunikacja między widgetami: callbacki i ElementRef

Callbacki dostają `GUIElement*` — nie zamykaj w nich surowych wskaźników do
innych widgetów, bo po `std::move`/usunięciu staną się wiszące. Twórz
`ElementRef<T>` **przed** przekazaniem ownership:

```cpp
auto label = manager.create<Label>(250, 20, "0");
auto ref = manager.makeRef(label);              // PRZED jakimkolwiek std::move

auto slider = manager.create<Slider>(10, 50, 200, 30, 0, 100, 50,
                                     Orientation::Horizontal);
slider->setOnChangeCallback([ref](GUIElement* e) {
    auto* s = static_cast<Slider*>(e);
    if (s && ref) {                              // ref == nullptr, gdy label usunięty
        ref->setText(std::to_string(s->getValue()));
    }
});
```

`ElementRef::get()` zwraca `nullptr`, jeśli element został usunięty —
wzorzec „sprawdź i użyj" jest bezpieczny nawet po `markForDeletion()`.

### Gotowe wiązania: `ui_helpers` (`src/ui_helpers.hpp`)

Najczęstsze wiązanie slider→label nie wymaga już ręcznego callbacka —
`linkLabel` robi to za Ciebie (odświeżenie natychmiast + przy każdej zmianie,
etykieta trzymana przez `ElementRef`):

```cpp
#include "ui_helpers.hpp"

linkLabel(volumeSlider, volumeLabel, "vol=", "%");  // "vol=80%"
linkLabel(speedSlider, speedLabel, [](int v) { return std::to_string(v / 60) + " min"; });
linkRangeLabel(range, windowLabel);                 // "[20, 80]"
```

Uwaga: jedno wiązanie na widget — ponowne `linkLabel` tego samego slidera
zastępuje poprzednie (jak `setOnChangeCallback`). Plik `ui_helpers` to baza
pod kolejne wolne funkcje (generatory, buildery) — dopisuj deklarację + definicję
w `ui_helpers.cpp`, bez zależności od konkretnych aplikacji.

### Generatory plansz i pasków (`ui_helpers`)

Gry planszowe powtarzają trzy schematy — generatory budują je w 1–3 linijkach
(przykłady 59_snake, 60_minesweeper, 62_memory):

```cpp
#include "ui_helpers.hpp"

// Siatka paneli (dzieci rodzica; wskaźniki należą do rodzica):
auto cell = gridPanels(*board, 20, 14, 26, 26);          // [y][x], gap/ox/oy opcjonalne
auto top  = gridPanels(guiManager, 20, 14, 26, 26, 0, 30, 120);  // top-level

// Siatka klikalnych komórek (Button nie ma setText — caption niesie Label-face):
auto g = faceGrid(*field, 9, 9, 40, 40, 0, 0, 0, 20);    // + gap/ox/oy/faceSize
g[y][x].button->setOnClickCallback(...);
g[y][x].face->setText("3");                              // faces startują puste

// Pasek n widgetów (kanały equalizera, rzędy kontrolek):
auto sliders = makeStrip<Slider>(panel, 5, 90, 80, 40, 340, 125, 0,
    [](GUIManager& m, int x, int y, int w, int h, int i) {
        return std::make_unique<Slider>(m, x, y, w, h, 0, 100, 50, Orientation::Horizontal);
    });

// Potasowana talia par do Memory:
std::vector<int> deck = shuffledPairs(8);                // {0,0,1,1,…,7,7} losowo
```

Faces są centrowane dla początkowego (pustego) rozmiaru — jednoznakowe
captiony (`"3"`, `"F"`, `"*"`) zostają ~na środku; szerokie dynamiczne teksty
wycentruj ręcznie po `setText`.

### Tracked ownership, buildery i style (`ui_helpers`)

Trzy najczęściej powtarzane wzorce z ~65 przykładów, zamknięte w helperach
(przykłady 04/05/06/10/36/42/44/49/51/55):

```cpp
#include "ui_helpers.hpp"

// makeRef PRZED move, w jednym kroku — koniec crashy z martwym refem:
auto [ball, ballRef] = addTracked(guiManager, std::make_unique<Panel>(...));
auto [child, childRef] = addTracked(*parent, std::make_unique<Label>(...));

// Kanoniczny ciemny panel + captiony w jednej linijce:
styleCard(*panel);                                            // same defaulty co dema
Panel* card = addDarkPanel(guiManager, 200, 80, 400, 250);    // + overload z parentem
Label* t = addLabel(*panel, 20, 20, "Tytuł", 22, SDL_Color{255,255,255,255});
Button* b = addButton(*panel, 100, 100, 200, 40, "Open...", [&](GUIElement*) { ... });

// Checkbox z captionem po prawej (kolor tylko gdy podany):
LabeledCheckbox cb = addLabeledCheckbox(*panel, 20, 70, "Check me!", 24, 16, white);
cb.box->setOnChange(...);

// Siatka 2D z fabryką (uogólnia gridPanels/makeStrip na klawiatury, talie, tile'e):
makeGrid<Button>(*screen, 3, 4, x0, y0, w, h, gapX, gapY,
    [&](GUIManager& m, int c, int r, int x, int y, int w, int h) {
        return std::make_unique<Button>(m, x, y, w, h, keys[r * 3 + c]);
    });

// Jeden refresh() na N sliderów (odpala się też natychmiast):
onAnyChange({sliderR.get(), sliderG.get(), sliderB.get()}, refresh);

// Slider → zmienna (rodzeństwo linkLabel dla celów nie-labelkowych):
bindSliderValue(*gravSlider, gravity);          // int
bindSliderValue(*dampSlider, damping, 0.01f);   // float ze skalą

// Paski statusu (dodatkowe labele dopinaj jako dzieci bar):
StatusBar top = makeTopBar(guiManager, screenW, 28, "carrier", 14);
StatusBar bottom = makeBottomBar(guiManager, screenW, screenH, 70, "Movies", 20);
// + overloady z parentem (szerokość/offset z rozmiaru rodzica)
```

Wszystkie zwrócone surowe wskaźniki należą do rodzica/managera (lifetime
jak przy `make_unique` + `addChild`). W stored-callbackach trzymaj `ElementRef`,
nigdy surowe wskaźniki (wyjątek: wartości kopiowane jak `key` powyżej).

## 4. Style

Setterami (automatycznie oznaczają element jako brudny):

```cpp
btn->setBackgroundColor(ElementState::Hover, {60, 63, 73, 255});
btn->setBorder(ElementState::Pressed, {200, 100, 100, 255}, 3);
```

Bezpośrednią modyfikacją `Style` (wymaga ręcznego `markDirty()`):

```cpp
Style s;
s.backgroundColor = {45, 48, 58, 255};
s.borderRadius = 10;
btn->setStyle(ElementState::Normal, s);
// albo: modyfikacja po setStyle + wymuszenie przerysowania
btn->markDirty();
```

`Style` używa `std::optional` — niezdefiniowane pola dziedziczą z motywu
(kaskada: lokalny → motyw[typ][stan] → motyw[typ][Normal] → domyślny).
Dzięki temu wystarczy ustawić tylko to, co się różni od motywu.

Motywy: `ThemePresets::createWindows95Theme()`,
`createLightTheme()`, `createDarkTheme()`, `createHighContrastTheme()` oraz
`Theme::createDefaultTheme()` (= Windows95) i `Theme::createWindows95Theme()`.
(`ThemePresets::createWin9xTheme()` to deprecated alias dla Windows95.)
Ustawiaj przez `manager.setTheme(ThemePresets::createDarkTheme())`.

## 5. Responsive layout: Anchor + resize

1. Wstrzyknij viewport w konstruktorze: `GUIManager manager(renderer, Viewport{w, h})`.
2. Ustaw anchor elementom, które mają reagować na zmianę rozmiaru.
3. Przy `SDL_EVENT_WINDOW_RESIZED` wołaj `manager.handleResize(w, h)`.

```cpp
SDLApp app("Responsive", 800, 600, true);   // resizable
GUIManager manager(app.getRenderer(), Viewport{800, 600});
manager.setTheme(ThemePresets::createDarkTheme());

// pasek na górze — pełna szerokość, 50 px wysokości, 10 px marginesy
auto topBar = manager.create<Panel>(0, 0, 0, 0);
topBar->setAnchor(Anchor::topBar(50, 10, 10));

// treść wycentrowana w pozostałej przestrzeni
auto content = manager.create<Label>(0, 0, "Treść");
content->setAnchor(Anchor::center());

// lewa kolumna — 200 px szerokości, cała wysokość
auto sidebar = manager.create<Panel>(0, 0, 0, 0);
sidebar->setAnchor(Anchor::leftSidebar(60, 10));

app.run(manager, {40, 42, 54, 255}, [&manager](SDL_Event& e) {
    if (e.type == SDL_EVENT_WINDOW_RESIZED) {
        manager.handleResize(e.window.data1, e.window.data2);
    }
});
```

Kotwice (`resources.md`): enum per oś (`HAnchor`/`VAnchor`) + marginesy
w pikselach (int) — np. `Anchor::center()`, `Anchor::fill(10)`,
`Anchor::bottomRightAt(12, 34)`, `Anchor::topCenter(40)`.

## 6. Okna dialogowe (overlay)

`DialogBox` i `MessageBox` to widgety overlay — renderują się nad resztą
interfejsu i przechwytują fokus. Najprościej przez statyczne fabryki:

```cpp
// komunikat informacyjny
MessageBox::showInfo(manager, "Operacja zakończona sukcesem");

// pytanie z odpowiedzią Tak/Nie
MessageBox::showQuestion(manager, "Zapisać zmiany?", [] {
    /* zapisz */
}, [] {
    /* anuluj */
});

// pełny dialog z przyciskami (callback dostaje indeks klikniętego przycisku)
auto dlg = DialogBox::createWithTitle(manager, "Tytuł", "Treść",
                                      {"OK", "Anuluj"},
                                      [](int index) { /* 0 = OK, 1 = Anuluj */ });
manager.addElement(std::move(dlg));
```

Szczegóły: [composites.md](composites.md).

## 7. Renderer GPU (ShaderPanel)

Do shaderów użyj wariantu GPU `SDLApp` — tworzy device Vulkan i renderer GPU:

```cpp
SDLApp app("Shadery", 800, 600, false, GPU_VULKAN);
GUIManager manager(app.getRenderer());
manager.setTheme(ThemePresets::createDarkTheme());

auto panel = manager.create<ShaderPanel>(0, 0, 800, 600);
// panele shaderowe rysują się bezpośrednio (drawDirect), poza cache'em

// dane per-klatka: czas i pozycja myszy jako uniformy
panel->setUniformTime(static_cast<float>(SDL_GetTicks()) / 1000.0f);
panel->setUniformMouse(mouseX, mouseY);
```

`ShaderPanel` wymaga renderera GPU (`SDLApp(title, w, h, false, GPU_VULKAN)`)
— na zwykłym rendererze CPU nie zadziała.

## 8. C API dla integracji z C

Pełna warstwa C (`sdlgui_*`) jest dostępna w `sdl_gui.h` — patrz
[c_api.md](c_api.md). Typowy schemat:

```c
sdlgui_ctx_t* ctx = sdlgui_create("Aplikacja C", 800, 600, 0);
sdlgui_ctx_set_theme(ctx, SDLGUI_THEME_DARK);

sdlgui_element_t* btn = sdlgui_button_create(ctx, NULL, 320, 300, 160, 40, "Kliknij");
sdlgui_button_set_on_click(btn, my_callback, NULL);

while (sdlgui_poll_events(ctx)) {
    sdlgui_update(ctx);
    sdlgui_cleanup(ctx);
    sdlgui_render(ctx);
}
sdlgui_destroy(ctx);
```

Kompilacja: `gcc -std=c11 -pedantic-errors -I dist -c app.c`, linkowanie
kompilatorem C++ z `-lsdl_gui` (patrz [getting_started.md](getting_started.md)).

## 9. Typowe pułapki

| Problem | Przyczyna i rozwiązanie |
|---------|--------------------------|
| Widget się nie rysuje / jest niewidoczny | Brak motywu: wywołaj `manager.setTheme(...)` (bez kolorów z motywu element nie ma czym się narysować) |
| Tooltip nie znika, animacje stoją | Brak `manager.update()` w pętli zdarzeń |
| Elementy z `markForDeletion()` nigdy nie znikają (wyciek) | Brak `manager.cleanup()` w pętli zdarzeń |
| Crash / UB w callbacku | Surowy wskaźnik przechowywany w callbacku po `std::move` — użyj `ElementRef` utworzonej przed przeniesieniem |
| Dzieci w dziwnych miejscach | Dzieci mają współrzędne względem rodzica, top-level względem okna |
| Zmiana `Style` nie daje efektu | Bezpośrednia modyfikacja pól `Style` nie oznacza elementu jako brudnego — wywołaj `markDirty()` (settery robią to automatycznie) |
| Fokus nie działa po Tab | Zapomniano `setCanGetKeyboardFocus(true)` na elemencie |
| Element nie reaguje na zdarzenia | `setVisible(false)` lub `setEnabled(false)` — sprawdź oba |
| Anchor nie działa przy resize | Brak `handleResize` na zdarzeniu `SDL_EVENT_WINDOW_RESIZED` (viewport startowy daje konstruktor — kotwice działają od razu) |
| Dialog nie łapie fokusu | Dialogi są overlayami — elementy spoza aktywnego overlayu nie dostają fokusu Tab; nie twórz dialogów w środku innego overlayu |
