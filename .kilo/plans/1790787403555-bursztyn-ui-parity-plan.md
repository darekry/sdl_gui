# BursztynUI parity — plan iteracyjny (1 kontrolka / iteracja)

## Cel
Przenieść do `sdl_gui` sprawdzone idee z BursztynUI małymi, bezpiecznymi krokami, bez regresji wydajności (gorąca ścieżka `frame_loop` / `hover` nietknięta).

## Uzgodnione zasady
- 1 kontrolka (lub 1 menedżer layoutu) na iterację. Po każdej: implementacja → `./nob test` + build przykładów → feedback użytkownika → checkbox poniżej → dopiero następna.
- Małe diffy. Zero nowych zależności. C++23, `std.hpp`, clang-format.
- Wydajność: layout i popup działają tylko w `layoutChildren()/handleResize()/addChild/showAt`, nigdy per-frame. `arrange()` = O(n), zero alokacji w pętli, `setPosition/setSize` mają early-out (istniejący). Invisible dzieci pomijane.

## Kolejka (status)
- [ ] #1 `DockLayout` (ta iteracja, zakres full z parserem — decyzja użytkownika)
- [ ] #2 `GridLayout`
- [ ] #3 `FlowLayout`
- [ ] #4 per-widget menu + dziedziczenie + `Shift+F10`/`Menu`
- [ ] #5 `Toast` (powiadomienia) + `InfoBar`
- [ ] #6 `Spinner` + `ProgressBar::indeterminate`
- [ ] #7 `UICommand` + `CommandList`
- [ ] #8 `ModalDialog` baza (refactor `DialogBox/FileDialog` bez zmiany API)
- [ ] #9 `ColorDialog` (composite)
- [ ] #10 `VirtualListView` (potem `VirtualGrid`)
- [ ] #11 `ChartPanel`

---

## Iteracja #1: DockLayout — spec implementacyjna

### Decyzje (zamknięte)
1. Pierwsza: `DockLayout`, bo najmniejsze ryzyko (tylko layout, zero `ComponentType`, zero event/render-loop).
2. API: enum na elemencie (nie mapa w menedżerze, nie reuse `Anchor`).
3. Zakres: full z parserem (layout + test + example + `LayoutParser`/`WidgetFactory` + C-API).

### API (docelowe)
```cpp
// gui.hpp — obok Anchor, domyślnie None
enum class Dock : uint8_t { None, Top, Bottom, Left, Right, Fill };
void setDock(Dock d); Dock getDock() const;

// layout.hpp
class DockLayout : public ILayoutManager {
public:
  explicit DockLayout(int spacing = 0, int padL=0, int padT=0, int padR=0, int padB=0);
  LayoutSize measure(GUIElement& c, LayoutConstraints) override;
  void arrange(GUIElement& c) override;
};
```

### Semantyka (pinowana testami)
- Kolejność = kolejność dzieci. `Top/Bottom` zabierają pełną bieżącą szerokość pasa i wysokość ze swojego `getHeight()`; `Left/Right` biorą pełną wysokość pasa i szerokość z `getWidth()`; `Fill` (zwykle ostatni, może być kilka — dzielą resztę po równo albo pierwszy wygrywa — wybrać 1 i spinać testem; rekomendacja: ostatnie `Fill` wygrywa, wcześniejsze traktowane jak `None`? NIE — prościej: każdy kolejny `Fill` dostaje resztę po poprzednich, czyli standardowy chain) zajmuje resztę.
- Cross-axis stretch: `Top/Bottom` → `w = pozostała szerokość`; `Left/Right` → `h = pozostała wysokość`. Główna oś = rozmiar własny (z konstruktora).
- `None` = pomijany przez `DockLayout` (zostaje gdzie był — kompatybilność mieszana z `Anchor`).
- Niewidoczne (`!isVisible()`) pomijane, nie zjadają przestrzeni.
- Padding kontenera + `spacing` między pasami. Ujemna reszta clampowana do 0.
- Gdy kontener ma `DockLayout`, `Anchor` dzieci dokowanych jest ignorowany (udokumentować). Rekurencja: po ustawieniu rectu wołać `child->layoutChildren()` jak w `StackLayout::arrange`.
- `measure()` = content size (jak `StackLayout::measure`), potrzebne dla `ScrollArea`.

### Pliki do ruszenia (tylko te)
1. `src/gui.hpp` + `src/gui.cpp` — pole `Dock m_dock`, `setDock/getDock` (setter bez `markDirty` treści, tylko `layoutChildren` rodzica? sprawdzić jak `setAnchor` — ma być tanio).
2. `src/layout.hpp` + `src/layout.cpp` — klasa `DockLayout` (~80 linii, wzorowana na `StackLayout`).
3. `src/layout_parser.cpp` (+ `layout_parser.hpp:parseDock` jeśli potrzeba) — atrybut `dock="top|bottom|left|right|fill|none"` per node + `layout="dock"` na kontenerze (z `spacing/padding` jeśli parser już wspiera dla stack — użyć tego samego schematu).
4. `src/widget_factory.cpp` — nic dla samego docka (dock nie jest typem); dodać `dock` do `WidgetProps` tylko jeśli parser idzie przez `WidgetProps` (sprawdzić `fillPropsFromNode` — jeśli tak, dodać `std::string dock` + aplikacja w `create()`).
5. `src/sdl_gui.h` + `src/sdl_gui_c_api.cpp` — `sdlgui_dock_t` + `sdlgui_element_set_dock` (wzorem `sdlgui_anchor_t`), bez łamania ABI.
6. `tests/test_dock_layout.cpp` (nowy, wzór `test_anchor.cpp`) — min. 6 sekcji: top/bottom/left/right/fill chain, fill-reszta, hidden pomijany, spacing/padding, resize rodzica, `None` nietknięty.
7. `examples/66_dock_layout.cpp` (nowy) — okno 800x600: top-bar, bottom-bar, left-sidebar, fill-center, resize przez `handleResize`. Wzorzec pętli z AGENTS.md (`processEvent→update→cleanup→render` + `app.endFrame`).
8. `nob.c` — dopisać test/example do list (sprawdzić czy auto-wykrywa; jeśli tak, pominąć).

### Czego NIE ruszać w tej iteracji
- `OverlayStack`, `ContextMenu`, `DialogBox/FileDialog`, `Theme`, `TextureManager`, gorąca ścieżka eventów. Brak nowego `ComponentType`.

### Walidacja
- `./nob test` zielone (w tym nowy `test_dock_layout`), `./nob examples` zielone.
- Smoke: `./output/66_dock_layout` headless (`SDL_GUI_HIDDEN=1`, timeout = pętla chodzi).
- Perf: `arrange()` nie alokuje (brak `vector`, tylko iteracja po `getChildren()`); porównanie `bench` opcjonalne — scena bench nie używa docka, wynik ma być w szumie (±5%).

### Ryzyka / failure modes
- Mieszanie `Anchor` + `Dock` na tym samym dziecku → rozstrzygnięte: dock wygrywa, anchor ignorowany.
- Kilka `Fill` → chain (każdy bierze resztę po poprzednim), nie overlap.
- `setSize(0)` rodzica → clamp, brak crasha (niezmiennik NonZero viewportu).

---

## Kolejne iteracje F/G/H — spec (po DockLayout następny jest ModalDialog)

### #8 ModalDialog — cienka baza (NASTĘPNY po #1)
Decyzja: cienka baza, zero zmian w `GUIManager` (focus-trap/Tab i `getActiveOverlay` już istnieją).
```cpp
// src/composite/modal_dialog.hpp — nowe
class ModalDialog : public Panel {
public:
  ModalDialog(GUIManager& m, int x, int y, int w, int h);
  virtual ~ModalDialog() = default;
  bool isOverlay() const override { return true; }
  void close(); bool isOpen() const;
  void centerInViewport();              // Anchor::center() + viewport z managera
protected:
  bool handleEvent(const SDL_Event& e) override; // Panel::handleEvent + Esc→close + Enter→onConfirm
  virtual void onConfirm() {}           // FileDialog potwierdza, DialogBox ignoruje lub mapuje na OK
  virtual void onCancel() { close(); }
  bool m_isOpen = true;
};
```
- `DialogBox : ModalDialog`, `FileDialog : ModalDialog` — tylko zmiana rodzica + usunięcie własnych `close/isOpen/isOverlay/Esc` (Enter w `FileDialog` → `onConfirm=confirmSelection`, w `DialogBox` brak Enter = no-op żeby nie zmieniać semantyki przycisków).
- Publiczne `create*` bez zmian sygnatur. Brak nowego `ComponentType` (baza abstrakcyjna, `getComponentTypeId` zostaje w podklasach).
- Pliki: nowe `modal_dialog.{hpp,cpp}` + edycja `dialog_box.{hpp,cpp}` + `file_dialog.{hpp,cpp}` + test `tests/test_modal_dialog.cpp` (Esc zamyka, Enter w FileDialog potwierdza, `isOverlay`, `close→markForDeletion`, `createConfirm` API bez zmian).
- Walidacja: `./nob test`, przykłady `35_dialog/36_file_dialog` smoke. Perf: brak (tylko event-key path).

### #9 ColorDialog — RGB+HSV, 6 sliderów z synchronizacją (decyzja użytkownika)
```cpp
// src/composite/color_dialog.hpp — nowe, dziedziczy ModalDialog
class ColorDialog : public ModalDialog {
public:
  using ColorCallback = std::function<void(SDL_Color)>;
  static ColorDialog* create(GUIManager& m, std::string_view title, SDL_Color initial, ColorCallback cb);
  void setColor(SDL_Color c); SDL_Color getColor() const; SDL_Color getInitial() const;
protected:
  void onConfirm() override; // woła callback + close
};
```
- Wnętrze (kompozycja istniejących widgetów, jak `FileDialog`): 3x `Slider 0-255` (R,G,B) + 3x `Slider` (H 0-360, S/V 0-100) + `TextInput HEX` (`#RRGGBB`) + 2x `Panel` preview (old/new, `setBackgroundColor`) + siatka presetów (8-12x `Button` ze stałej palety, klik → `setColor`) + `OK/Anuluj` (`onConfirm`/`close`).
- Synchronizacja dwukierunkowa z guardem `m_syncing`: zmiana dowolnego slidera/HEX → konwersja RGB↔HSV (helper w cpp, int math, clamp) → `setValue` pozostałych bez rekurencji (guard) + odświeżenie preview. Konwersje tylko w event path, zero kosztu per-frame. `Slider::setValue` już woła callback — guard obowiązkowy.
- `layoutChildren()` proporcjonalny jak w `FileDialog`. Brak `ComponentType`? Dodać `ColorDialog` do `ComponentType` + `WidgetFactory::createBare` dopiero jeśli potrzebny w parserze — domyślnie NIE (dialog tworzony z kodu, jak `DialogBox::create*`).
- Pliki: `color_dialog.{hpp,cpp}`, `tests/test_color_dialog.cpp` (RGB→HSV→RGB roundtrip, guard anti-loop, HEX parse, preset click, OK callback), `examples/67_color_dialog.cpp`.
- Failure modes: błędny HEX → ignorowany (bez crasha); `setValue` spoza zakresu → clamp w sliderze.

### #10 VirtualListView — nowy widget z providerem (ListView/StringGrid nietknięte)
```cpp
// src/virtual_list_view.hpp — nowe
class VirtualListView : public Panel {
public:
  using Provider = std::function<std::string_view(size_t index)>;
  VirtualListView(GUIManager& m, int x, int y, int w, int h, int rowHeight = 24);
  void setItemCount(size_t n); size_t getItemCount() const;
  void setProvider(Provider p); void setRowHeight(int h);
  void setSelectedIndex(std::optional<size_t>); std::optional<size_t> getSelectedIndex() const;
  using RowCallback = std::function<void(VirtualListView*, size_t)>;
  void setOnRowClick(RowCallback); void setOnRowActivate(RowCallback);
protected:
  bool wantsDirectRender() const override { return true; }
  void drawDirect(SDL_Renderer*) override;   // tylko visible rows, tekstury z m_textCache
  bool handleEvent(const SDL_Event&) override; // klik/selekcja, wheel, klawiatura góra/dół/Enter
  void layoutChildren() override;            // pozycja wewnętrznego Slidera
};
```
- Offset: `firstVisible = scrollOffset / rowHeight` (stała wysokość = O(1) matematyka). Wewnętrzny pionowy `Slider` jak w `StringGrid::setupSliders/updateSliderRanges` (kopiować wzorzec, nie dziedziczyć). Selekcja single. Tekst per wiersz przez provider (caller trzyma dane — zero kopiowania 10k stringów do widgetu). Rysowanie jak `StringGrid::drawCells` (krok po `calculateVisibleRange`-analog, `createTextureFromText` z TextShaper — hit bez alokacji).
- `VirtualGrid` (wirtualna tabela) świadomie ODROCZONY: dopiero po feedbacku z listy (kolumny/sort/edycja to osobna iteracja).
- Pliki: `virtual_list_view.{hpp,cpp}`, `ComponentType::VirtualListView` + factory bare, `tests/test_virtual_list_view.cpp` (10k count, provider wołany tylko dla widocznych — licznik wywołań, scroll offset math, klik→selekcja, klawiatura), `examples/68_virtual_list.cpp` (10k wierszy, label z wybranym).
- Perf: `drawDirect` O(visible), provider bez `std::string` kopii (`string_view`), brak dzieci-widgetów. Bench: porównać `frame_loop` sceny z 10k `ListView` (kontrola: nie budować — za ciężka) vs `VirtualListView` — asercja testowa na liczbie wywołań providera, nie fps.

### #11 ChartPanel — Line+Bar (+Pie jeśli trywialny), bez legendy/interakcji
```cpp
// src/chart_panel.hpp — nowe
class ChartPanel : public Panel {
public:
  enum class Mode { Line, Bar, Pie };
  ChartPanel(GUIManager& m, int x, int y, int w, int h);
  void setData(std::vector<float>); void setMode(Mode); void setShowGrid(bool);
protected:
  bool wantsDirectRender() const override { return true; }
  void drawDirect(SDL_Renderer*) override;
};
```
- Normalizacja `min/max` z danych (pusta/stała seria → flat line, brak dzielenia przez 0). Kolory: `effectiveFillColor` (słupki/wypełnienie linii) + `effectiveThumbColor` (linia) — spójne z unifikacją Win95. Siatka 4 linie (cienkie, `RenderLine`). Pie: wycinki koła przez `SDL_RenderGeometry` fan-triangulację tylko jeśli <30 linii — inaczej wypada z iteracji (decyzja w implementacji, test ma nie wymagać Pie).
- Pliki: `chart_panel.{hpp,cpp}`, `ComponentType::ChartPanel`, `tests/test_chart_panel.cpp` (puste dane, stałe dane, setMode nie crashuje, Line/Bar piksele smoke), `examples/69_chart.cpp` (3 panele: Line/Bar/Pie lub 2 jeśli Pie wypadnie).
- Perf: `setData` kopiuje wektor + `markDirty`; `drawDirect` O(n) punktów tylko przy redraw (cache? direct jak `Canvas` — scena bez zmianOK).

### Odroczone / poza zakresem F/G/H
- `VirtualGrid` (kolumny, sort, edycja), legenda/zoom/tooltip Chart, alpha w ColorDialog, palety systemowe — osobne iteracje po feedbacku.

## Pytania otwarte (na później, nie blokują #1)
- Dokładny podział reszty między wiele `Fill` (chain vs równy podział) — do decyzji przy #1 jeśli test wykaże niejasność.
- Czy `Grid` potrzebuje auto-rows, czy sztywne `rows/cols` wystarczą.
