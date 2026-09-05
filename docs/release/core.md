# Core — fundamenty biblioteki

Ten dokument opisuje warstwę bazową, na której opierają się wszystkie widgety.
Po przeczytaniu go będziesz w stanie zrozumieć dokumentację każdego widgetu
(w `widgets/`), bo wszystkie dziedziczą po `GUIElement` i żyją w `GUIManager`.

## GUIElement

`GUIElement` jest abstrakcyjną bazą wszystkich widgetów. Zapewnia: pozycję
i rozmiar, hierarchię rodzic–dziecko, style per stan, tooltipy, anchor
(responsywny layout), rotację, fokus klawiatury i cykl życia (usuwanie).

### Pozycja i rozmiar

Pozycja i rozmiar elementu są dostępne jako **publiczne pola**:

```cpp
int m_x, m_y;           // pozycja (względem rodzica lub okna dla elementów top-level)
int m_width, m_height;  // rozmiar
bool m_enabled = true;  // czy element reaguje na zdarzenia
bool m_visible = true;  // czy element jest rysowany
```

Oraz przez metody:

| Metoda | Opis |
|--------|------|
| `int getX() const` / `int getY() const` | Pozycja względna (względem rodzica) |
| `int getWidth() const` / `int getHeight() const` | Rozmiar |
| `void getSize(int& width, int& height) const` | Rozmiar przez referencje |
| `void setPosition(int x, int y)` | Ustawia pozycję (względem rodzica) |
| `void setSize(int width, int height)` | Ustawia rozmiar |
| `SDL_Point getAbsolutePosition() const` | Pozycja absolutna w oknie (suma pozycji rodziców; cache'owana) |
| `SDL_Point getRelativePosition() const` | Pozycja względna `{m_x, m_y}` |
| `SDL_Point toLocalCoords(int globalX, int globalY) const` | Konwersja współrzędnych okna na lokalne |
| `virtual bool contains(int x, int y) const` / `bool contains(float x, float y) const` | Czy punkt (okno) leży wewnątrz elementu |

### Hierarchia

Każdy element może mieć dzieci — ich współrzędne są **względem rodzica**,
a renderowanie jest przycinane do obszaru rodzica.

```cpp
GUIElement* addChild(std::unique_ptr<GUIElement> child);  // zwraca surowy wskaźnik dziecka
void clearChildren();
GUIElement* getParent() const;
const std::vector<std::unique_ptr<GUIElement>>& getChildren() const;
size_t countDescendants() const;
GUIElement* findElementAt(int x, int y);   // najgłębszy element pod punktem (okno)
```

Wzorce tworzenia dzieci — patrz [patterns.md](patterns.md).

### Style per stan

Każdy element ma cztery stany wizualne (`ElementState`) i dla każdego z nich
osobny styl. Style ustawia się setterami (wygodnie) lub przez `Style`
(bezpośrednio):

| Metoda | Opis |
|--------|------|
| `void setStyle(ElementState state, Style style)` | Ustawia cały styl dla stanu |
| `void setBackgroundColor(ElementState state, SDL_Color color)` | Kolor tła |
| `void setTextColor(ElementState state, SDL_Color color)` | Kolor tekstu |
| `void setTexture(ElementState state, SharedTexture texture)` | Tekstura tła |
| `void setBorder(ElementState state, SDL_Color color, int width)` | Obramowanie |
| `void setBorderRadius(ElementState state, int radius)` | Zaokrąglenie rogów (0 = ostre) |

Efektywny styl elementu wyliczany jest kaskadowo (wewnętrznie przez
`getComposedStyle(state)`, metodę chronioną — dostępną w podklasach):

1. styl lokalny dla stanu (`m_localStyles[state]`),
2. motyw dla typu widgetu i stanu (`Theme`),
3. motyw dla typu widgetu i stanu `Normal`,
4. styl domyślny motywu.

Dzięki `std::optional` w `Style` niezdefiniowane pole jest dziedziczone z niższej
warstwy kaskady.

### Stany

```cpp
void setState(ElementState newState);
ElementState getState() const;
```

`ElementState` to `Normal`, `Hover`, `Pressed`, `Disabled`. Stan zmienia się
automatycznie (hover, kliknięcie, wyłączenie), ale można go też ustawić ręcznie.
Zmiana stanu ponownie składa style i oznacza element jako brudny.

### Tooltip

```cpp
void setTooltip(const std::string& text);
```

Tooltip pojawia się po najechaniu kursorem (wymaga `GUIManager::update()`
w pętli zdarzeń).

### ID

```cpp
void setID(std::string_view id);
std::string_view getID() const;
```

Identyfikator do identyfikacji elementów w hierarchii (np. przy parsowaniu
layoutów); nie ma wpływu na renderowanie.

### Widoczność i enabled

```cpp
void setEnabled(bool enabled);   bool isEnabled() const;
void setVisible(bool visible);   bool isVisible() const;
bool isHovered() const;
```

- `setEnabled(false)` — element nie reaguje na zdarzenia (i zwykle otrzymuje
  stan `Disabled` wizualnie).
- `setVisible(false)` — element nie jest rysowany ani nie odbiera zdarzeń.

### Anchor (responsive layout)

```cpp
void setAnchor(const Anchor& anchor);
const Anchor& getAnchor() const;
bool hasAnchor() const;
void applyAnchor(int parentWidth, int parentHeight);
void updateLayout(int parentWidth, int parentHeight);
virtual void onParentResize(int parentWidth, int parentHeight);
void storeOriginalSize();
int getOriginalWidth() const;
int getOriginalHeight() const;
```

`Anchor` definiuje pozycję/rozmiar elementu względem rodzica. Konwencja wartości:

- `< 0` — krawędź nieustawiona (stała pozycja),
- `0–1` — procent rozmiaru rodzica,
- `> 1` — piksele od krawędzi,
- `0.5` — środek rodzica (specjalny przypadek).

Presety statyczne: `Anchor::none()`, `topLeft(m)`, `topRight(m)`,
`bottomLeft(m)`, `bottomRight(m)`, `center()`, `fill(m)`,
`horizontalStretch(l, r)`, `verticalStretch(t, b)`, `topBar(h, l, r)`,
`bottomBar(h, l, r)`, `leftSidebar(w, t, b)`, `rightSidebar(w, t, b)`.
Pełny opis — [resources.md](resources.md).

### Rotacja

```cpp
void setRotation(double angleDegrees);
double getRotation() const;
void setRotationCenter(int cx, int cy);
SDL_Point getRotationCenter() const;
```

Obraca zawartość elementu wokół środka (`{-1, -1}` = środek elementu).

### Fokus klawiatury

```cpp
void setCanGetKeyboardFocus(bool canFocus);
bool canGetKeyboardFocus() const;
bool hasKeyboardFocus() const;
virtual void onFocusGained();
virtual void onFocusLost();
```

Element z fokusem rysuje obwódkę fokusu; `Tab`/`Shift+Tab` przesuwa fokus
między elementami (patrz `GUIManager`). `onFocusGained`/`onFocusLost` to hooki
do własnych reakcji (np. odświeżenie).

### Cykl życia — usuwanie

```cpp
void markForDeletion();
bool isMarkedForDeletion() const;
void cleanup();
```

Element nie jest usuwany od razu: `markForDeletion()` oznacza go do usunięcia,
a rzeczywiste usunięcie następuje w `GUIManager::cleanup()` (po zakończeniu
przetwarzania zdarzeń — bezpieczne dla callbacków). `cleanup()` na elemencie
wykonuje to samo dla pojedynczego elementu.

### Brudzenie (dirty) i cache renderowania

```cpp
void markDirty(bool cascadeToParents = true);
void markDirtyRecursively();
```

Elementy renderują się do cache'a tekstury; `markDirty()` wymusza ponowne
przerysowanie. Settery robią to automatycznie, ale **bezpośrednia modyfikacja
pól `Style` już nie** — patrz [patterns.md](patterns.md#4-style).

## GUIManager

`GUIManager` to kontekst, w którym żyją wszystkie elementy: przetwarza
zdarzenia, aktualizuje timery/animacje/tooltipy, usuwa oznaczone elementy
i renderuje hierarchię. Szczegółowy opis: [managers.md](managers.md).

### Cykl życia aplikacji

Kolejność wołania w pętli zdarzeń jest obowiązkowa:

1. `bool processEvent(const SDL_Event& e)` — dystrybucja zdarzeń do elementów
   (kliknięcia, klawiatura, hover, fokus).
2. `void update()` — timery, animacje, tooltipy, animowane elementy.
3. `void cleanup()` — usuwa elementy oznaczone `markForDeletion()`.
4. `void render()` — rysuje całą hierarchię (elementy top-level + overlay).

Zachowanie między `update()` a `cleanup()` nie jest przypadkowe: callbacki
i animacje mogą w trakcie `update()` oznaczyć elementy do usunięcia, a
`cleanup()` wykonuje się dopiero po ich zakończeniu.

### Tworzenie elementów

```cpp
GUIElement* addElement(std::unique_ptr<GUIElement> element);

template<typename T, typename... Args>
T* create(Args&&... args);                       // top-level

template<typename T, typename... Args>
T* create(GUIElement* parent, Args&&... args);   // dziecko parenta
```

`create<T>` tworzy widget przez `make_unique`, od razu dodaje do managera
(lub rodzica) i zwraca surowy wskaźnik `T*` — najwygodniejszy wzorzec.

### Fokus i przechwytywanie myszy

```cpp
void setKeyboardFocus(GUIElement* element);
GUIElement* getKeyboardFocus() const;
void focusNextElement(bool forward);   // Tab (true) / Shift+Tab (false)

void captureMouse(GUIElement* element);   // zdarzenia myszy trafiają do elementu aż do releaseMouse()
void releaseMouse();
```

`captureMouse` przydaje się w widgetach przeciąganych (np. suwaki, rozdzielacze
panelów) — mysz może wyjechać poza obszar elementu, a element nadal dostaje
zdarzenia.

### ElementRef — bezpieczne odniesienia

```cpp
template<typename T = GUIElement>
ElementRef<T> makeRef(T* element);
```

`ElementRef<T>` przechowuje wskaźnik i wskaźnik do managera; przy każdym
dostępie sprawdza `isElementAlive()`. Po usunięciu elementu `get()` zwraca
`nullptr` zamiast wiszącego wskaźnika:

```cpp
T* get() const;           // nullptr, gdy element usunięty
T* operator->() const;
T& operator*() const;
explicit operator bool() const;   // false, gdy element usunięty
```

Twórz referencję **przed** przekazaniem ownership (np. `std::move`) —
patrz [patterns.md](patterns.md#3-komunikacja-między-widgetami-callbacki-i-elementref).

### Pozostałe usługi

```cpp
void setTheme(Theme theme);   Theme& getTheme();
void setWindowSize(int width, int height);
void handleResize(int width, int height);   // po SDL_EVENT_WINDOW_RESIZED
void setResizeCallback(ResizeCallback callback);   // std::function<void(int, int)>
void getWindowSize(int& width, int& height) const;
void showTooltip(GUIElement* target, const std::string& text);
void hideTooltip();
GUIElement* getActiveTooltip() const;               // aktywny tooltip (panel) albo nullptr
GUIElement* findElementAt(int x, int y);
SDL_Renderer* getRenderer() const;
SDL_GPUDevice* getGPUDevice() const;
FontManager& getFontManager();
TextureManager& getTextureManager();
TimerManager* getTimerManager();
AnimationManager* getAnimationManager();
std::unique_ptr<GUIElement> detachElement(GUIElement* element);  // odłącza top-level bez usuwania
bool isElementAlive(GUIElement* element) const;
```

## Wspólne typy

| Typ | Definicja | Uwagi |
|-----|-----------|-------|
| `ElementState` | `enum class ElementState { Normal, Hover, Pressed, Disabled }` | Stany wizualne widgetów |
| `Style` | Struktura pól `std::optional<...>`: `backgroundColor`, `textColor`, `texture`, `borderColor`, `borderWidth`, `borderRadius`, `fontSize`, `fontName`, 4x bevel, `thumbColor`, `fillColor` | Brak wartości = dziedziczenie z motywu |
| `ComponentType` | `enum class ComponentType : uint8_t` | ID typu widgetu (klucz themu i render-cache; stringi tylko na granicy) |
| `SDL_Color` | Typ SDL3 `{r, g, b, a}` (0–255) | Porównywalny `operator==` |
| `SharedTexture` | `std::shared_ptr<SDL_Texture>` | Współdzielona tekstura z automatycznym usuwaniem |
| `SharedFont` | `std::shared_ptr<TTF_Font>` | Współdzielony font z automatycznym usuwaniem |
| `Orientation` | `enum class Orientation { Horizontal, Vertical }` | Dla suwaków, pasków postępu itd. |
| `Anchor` | Struktura `{float left, top, right, bottom}` + presety | Responsywny layout — pełny opis w [resources.md](resources.md) |

### Theme i ThemePresets

- `Theme::createDefaultTheme()` — domyślny motyw (klasyczny, w stylu Win9x).
- `Theme::createWindows95Theme()` — motyw z fazami 3D (bevel) w stylu Win95/98.
- `ThemePresets::createWin9xTheme()`, `createWindows95Theme()`, `createLightTheme()`,
  `createDarkTheme()`, `createHighContrastTheme()` — gotowe motywy (patrz [managers.md](managers.md)).

### SDLApp i GUIContext

- `SDLApp` — RAII: inicjalizacja SDL3, okno, renderer; dostęp przez
  `getRenderer()`, `getWindow()`, `getGPUDevice()`. Wariant GPU:
  `SDLApp(title, w, h, resizable, GPU_VULKAN)`.
- `GUIContext` — `SDLApp` + `GUIManager` + motyw w jednym; `run()` uruchamia
  pętlę zdarzeń (patrz [getting_started.md](getting_started.md)).

## Podsumowanie cyklu życia elementu

1. Utwórz: `make_unique<T>` + konfiguracja → `addElement(std::move(...))`
   albo `manager.create<T>(...)`.
2. Użyj: `processEvent` dostarcza zdarzenia, `update()` animuje,
   `cleanup()` usuwa oznaczone elementy, `render()` rysuje.
3. Usuń: `markForDeletion()` w dowolnym momencie (też z callbacka) — faktyczne
   usunięcie nastąpi w najbliższym `cleanup()`.
