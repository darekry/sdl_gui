# Zasoby i typy wspólne

Typy i klasy współdzielone przez całą bibliotekę: style i motywy, responsywny
layout (Anchor), inicjalizacja aplikacji (SDLApp, GUIContext), parsery layoutu
oraz logowanie.

## Style i ElementState

`ElementState` definiuje stany, w jakich może być element interfejsu:

```cpp
enum class ElementState {
    Normal,
    Hover,
    Pressed,
    Disabled
};
```

`Style` przechowuje atrybuty wizualne dla pojedynczego stanu. Wszystkie pola są
typu `std::optional` — brak wartości oznacza dziedziczenie z motywu (kaskada,
patrz niżej):

```cpp
struct Style {
    std::optional<SDL_Color> backgroundColor;
    std::optional<SDL_Color> textColor;
    std::optional<SharedTexture> texture;
    std::optional<SDL_Color> borderColor;
    std::optional<int> borderWidth;
    std::optional<int> borderRadius;   // promień zaokrąglenia rogów (0 = ostre)
    std::optional<int> fontSize;
    std::optional<std::string> fontName;
    std::optional<SDL_Color> borderColorOuterTopLeft;      // bevel Win95
    std::optional<SDL_Color> borderColorOuterBottomRight;
    std::optional<SDL_Color> borderColorInnerTopLeft;
    std::optional<SDL_Color> borderColorInnerBottomRight;
    std::optional<SDL_Color> thumbColor;  // uchwyt Slider/RangeSlider (fallback: borderColor)
    std::optional<SDL_Color> fillColor;   // wypełnienie ProgressBar (fallback: borderColor)

    void mergeWith(const Style& base);   // uzupełnia puste pola z base
    bool operator==(const Style& other) const;
    bool operator!=(const Style& other) const;
    bool hasBevel() const;
};
```

Typ widgetu to `ComponentType` (enum, `uint8_t`) zamiast hasha stringa
w ścieżce renderu — `componentTypeFromString()` mapuje `"Button"…` na ID raz
(np. w parserze), a `getComposedStyle()` cache'uje scalony styl per stan
i przelicza go tylko przy zmianie themu (`Theme::epoch()`) lub stylu
lokalnego. Globalny współdzielony cache tekstur (`TextureManager::renderCache`)
jest zachowany: identyczne widgety (ten sam typ, rozmiar, stan i styl)
współdzielą JEDEN wpis / JEDNĄ teksturę (np. 100 kart jednostek w RTS → 1 wpis).

`SDL_Color` to `{r, g, b, a}` (0-255), np. `{45, 48, 58, 255}`.

### Ustawianie stylu elementu

`GUIElement` oferuje skróty, które ustawiają styl dla konkretnego stanu
(automatycznie oznaczają element do przerysowania):

```cpp
void setStyle(ElementState state, Style style);
void setBackgroundColor(ElementState state, SDL_Color color);
void setTextColor(ElementState state, SDL_Color color);
void setTexture(ElementState state, SharedTexture texture);
void setBorder(ElementState state, SDL_Color color, int width);
void setBorderRadius(ElementState state, int radius);
void setBevel(ElementState state, BevelType type);
void setThumbColor(ElementState state, SDL_Color color);  // uchwyt suwaka
void setFillColor(ElementState state, SDL_Color color);   // wypełnienie paska
void setState(ElementState newState);   // wymuszenie stanu
ElementState getState() const;
```

```cpp
Style s;
s.backgroundColor = {45, 48, 58, 255};
s.borderColor     = {98, 114, 164, 255};
s.borderWidth     = 2;
s.borderRadius    = 10;
btn->setStyle(ElementState::Normal, s);

// skróty robią to samo:
btn->setBackgroundColor(ElementState::Hover, {60, 63, 73, 255});
btn->setBorder(ElementState::Pressed, {200, 100, 100, 255}, 3);
btn->setTextColor(ElementState::Disabled, {128, 128, 128, 255});
```

### Kaskada stylów

Rzeczywisty styl elementu powstaje przez połączenie (`mergeWith`) w kolejności
od najbardziej lokalnego:

1. styl lokalny elementu dla danego stanu (`setStyle`/skróty),
2. styl motywu dla typu elementu i danego stanu,
3. styl motywu dla typu elementu i stanu `Normal`,
4. domyślny styl motywu (`setDefaultStyle`).

Uwaga: bezpośrednia modyfikacja pola `Style` (np. przez `getStyle()`) nie
wymusza przerysowania — wywołaj wtedy `markDirty()`.

## Theme i ThemePresets

`Theme` to tablica stylów `O(1)`: dla każdego typu widgetu (`ComponentType`,
np. `ComponentType::Button`, `::Panel`, `::Label`) i każdego stanu
można ustawić styl. Klasy `Style` nie są tu wymagane w całości — brakujące
pola i tak dziedziczą w kaskadzie. Nazwy string (`"Button"`…) istnieją tylko
na granicy: pliki layoutu (`componentTypeFromString`) i C-API
(`componentTypeToString`).

```cpp
void setStyle(ComponentType type, ElementState state, Style style);
Style getStyle(ComponentType type, ElementState state) const;
void setStyle(ComponentType type, Style style);          // wszystkie stany
Style getStyle(ComponentType type) const;
void setDefaultStyle(Style style);
const Style& getDefaultStyle() const;
static Theme createDefaultTheme();   // deleguje do ThemePresets::createWin9xTheme()
static Theme createWindows95Theme(); // deleguje do ThemePresets::createWindows95Theme()
```

### ThemePresets

Pięć gotowych motywów (funkcje `inline` w namespace `ThemePresets`):

| Funkcja | Opis |
|---------|------|
| `ThemePresets::createWin9xTheme()` | Klasyczny Windows 95/98: szare tło, ostre krawędzie, białe inputy |
| `ThemePresets::createWindows95Theme()` | Autentyczny Win95 z fazami 3D (bevel): przyciski Raised, pola edycji Sunken |
| `ThemePresets::createLightTheme()` | Jasny, nowoczesny, z niebieskim akcentem |
| `ThemePresets::createDarkTheme()` | Ciemny (dark mode) |
| `ThemePresets::createHighContrastTheme()` | Wysoki kontrast: czarne tło, żółte akcenty, większe fonty |

```cpp
manager.setTheme(ThemePresets::createDarkTheme());

// Dostrojenie pojedynczego typu:
Theme& theme = manager.getTheme();
Style s;
s.backgroundColor = {200, 80, 80, 255};
theme.setStyle("Button", ElementState::Hover, s);

Style btnStyle = theme.getStyle("Button", ElementState::Hover);
```

## Anchor

System responsywnego pozycjonowania względem rodzica/okna. Każdy `GUIElement`
ma anchor ustawiany przez `setAnchor(const Anchor&)`; przeliczany przez
`applyAnchor()` podczas `handleResize()`.

```cpp
struct Anchor {
    float left = -1.0f;
    float top = -1.0f;
    float right = -1.0f;
    float bottom = -1.0f;

    bool hasLeft() const;          // left >= 0
    bool hasTop() const;
    bool hasRight() const;
    bool hasBottom() const;
    bool stretchesHorizontal() const;   // left i right ustawione
    bool stretchesVertical() const;     // top i bottom ustawione
    bool isStretched() const;
    bool hasAnyAnchor() const;
};
```

Konwencja kodowania wartości:

- `< 0` — krawędź nieustawiona (element ma stałą pozycję/rozmiar),
- `0-1` — procent rozmiaru rodzica,
- `> 1` — piksele od krawędzi,
- `0.5` — **specjalny przypadek: centrum** (środek elementu w środku rodzica).

Gdy `left` i `right` są ustawione, element rozciąga się poziomo; gdy `top`
i `bottom` — pionowo.

### Presety

| Preset | Znaczenie |
|--------|-----------|
| `static Anchor none()` | Brak anchora — stała pozycja i rozmiar |
| `static Anchor topLeft(float margin = 0)` | Lewy górny róg z marginesem |
| `static Anchor topRight(float margin = 0)` | Prawy górny róg |
| `static Anchor bottomLeft(float margin = 0)` | Lewy dolny róg |
| `static Anchor bottomRight(float margin = 0)` | Prawy dolny róg |
| `static Anchor center()` | Wyśrodkowanie (zachowuje rozmiar) |
| `static Anchor fill(float margin = 0)` | Wypełnienie rodzica z marginesem |
| `static Anchor horizontalStretch(float leftMargin = 0, float rightMargin = 0)` | Pełna szerokość (wysokość bez zmian) |
| `static Anchor verticalStretch(float topMargin = 0, float bottomMargin = 0)` | Pełna wysokość (szerokość bez zmian) |
| `static Anchor bottomBar(float height, float leftMargin = 0, float rightMargin = 0)` | Pasek na dole: pełna szerokość, stała wysokość |
| `static Anchor topBar(float height, float leftMargin = 0, float rightMargin = 0)` | Pasek na górze |
| `static Anchor leftSidebar(float width, float topMargin = 0, float bottomMargin = 0)` | Pasek po lewej: pełna wysokość, stała szerokość |
| `static Anchor rightSidebar(float width, float topMargin = 0, float bottomMargin = 0)` | Pasek po prawej |

Dodatkowo `enum class AnchorMode { Pixels, Percentage, Hybrid };` — tryb
interpretacji wartości (domyślnie `Hybrid`, czyli konwencja powyżej).

### Użycie

```cpp
panel->setAnchor(Anchor::center());
toolbar->setAnchor(Anchor::topBar(50, 10, 10));      // pełna szerokość, 50 px wysokości
statusBar->setAnchor(Anchor::bottomBar(30, 10, 10));
sidebar->setAnchor(Anchor::leftSidebar(200, 60, 70));
content->setAnchor(Anchor::fill(10));

// okno resizable + w pętli:
if (e.type == SDL_EVENT_WINDOW_RESIZED) {
    manager.handleResize(e.window.data1, e.window.data2);
}
```

Dla poprawnego działania anchorów wywołaj `manager.setWindowSize(width, height)`
raz przy inicjalizacji (robi to też `GUIContext`).

## SDLApp

Klasa RAII inicjalizująca SDL3 (wideo, TTF, obrazki) i tworząca okno
+ renderer. Destruktor automatycznie sprząta wszystko — nie trzeba ręcznie
wywoływać `SDL_Quit()`.

```cpp
// Renderer CPU (standardowy):
SDLApp(const char* title, int width, int height, bool resizable = false);

// Renderer GPU (dla shaderów / Vulkan):
SDLApp(const char* title, int width, int height,
       bool resizable, GPUBackend /*backend*/, bool gpuDebug = false);
// GPU_VULKAN to jedyny dostępny backend (inline constexpr GPUBackend GPU_VULKAN{})
```

Konstruktory rzucają `std::runtime_error` przy niepowodzeniu (SDL, TTF, okno,
renderer).

```cpp
SDLApp app("Aplikacja", 800, 600);              // CPU
SDLApp gpuApp("GPU", 800, 600, false, GPU_VULKAN);  // GPU (Vulkan)

SDL_Renderer* renderer = app.getRenderer();     // działa z obu
```

### Metody

| Metoda | Opis |
|--------|------|
| `SDL_Renderer* getRenderer() const;` | Renderer (CPU lub GPU) |
| `SDL_Window* getWindow() const;` | Okno SDL |
| `SDL_GPUDevice* getGPUDevice() const;` | Urządzenie GPU (tylko wariant GPU; `nullptr` dla CPU) |
| `void getWindowSize(int& width, int& height) const;` | Bieżący rozmiar okna |

### Gotowa pętla: `run()`

```cpp
template<typename F = std::nullptr_t>
void run(GUIManager& guiManager, SDL_Color clearColor = {40, 42, 54, 255}, F onEvent = nullptr);
```

Obsługuje cały cykl: PollEvent → `processEvent` → `update` → `cleanup` →
clear (kolorem `clearColor`) → `render` → present. Opcjonalny `onEvent`
wołany po `processEvent` — np. do obsługi klawiszy lub resize.

```cpp
SDLApp app("Aplikacja", 800, 600, true);
GUIManager manager(app.getRenderer());
manager.setTheme(ThemePresets::createDarkTheme());
manager.setWindowSize(800, 600);

app.run(manager, {40, 42, 54, 255}, [&](SDL_Event& e) {
    if (e.type == SDL_EVENT_WINDOW_RESIZED) {
        manager.handleResize(e.window.data1, e.window.data2);
    }
});
```

## GUIContext

Wygodne opakowanie łączące `SDLApp` + `GUIManager` + motyw w jednym obiekcie
RAII. Konstruktor automatycznie ustawia motyw i rozmiar okna w menedżerze —
nie trzeba wołać `setTheme` ani `setWindowSize`.

```cpp
GUIContext(const char* title, int width, int height, bool resizable = false);
GUIContext(const char* title, int width, int height, Theme theme, bool resizable = false);
```

| Metoda | Opis |
|--------|------|
| `SDL_Renderer* getRenderer() const;` | Renderer |
| `SDL_Window* getWindow() const;` | Okno |
| `SDLApp& getApp();` | Bazowy `SDLApp` |
| `GUIManager& getGUIManager();` | Menedżer GUI |
| `void setTheme(Theme theme);` | Zmiana motywu |
| `Theme& getTheme();` | Bieżący motyw |
| `void handleResize(int width, int height);` | Delegat do `GUIManager::handleResize` |
| `template<typename F = std::nullptr_t> void run(SDL_Color clearColor = {40, 42, 54, 255}, F onEvent = nullptr);` | Gotowa pętla (jak `SDLApp::run`) |

```cpp
GUIContext ctx("Aplikacja", 800, 600, ThemePresets::createDarkTheme());
GUIManager& manager = ctx.getGUIManager();

auto btn = manager.create<Button>(10, 10, 120, 40, "Kliknij");
btn->setOnClickCallback([](GUIElement*) { LOG_INFO("App", "klik"); });

ctx.run();
```

## Parsery layoutu

Definiowanie GUI w plikach zewnętrznych (JSON lub XML). `LayoutParser` to
klasa bazowa; `JsonParser` i `SGMLParser` to konkretne implementacje.

```cpp
class LayoutParser {
public:
    LayoutParser(GUIManager& guiManager);
    virtual ~LayoutParser() = default;
    std::unique_ptr<GUIElement> loadLayout(const std::string& file_path);
};

class JsonParser : public LayoutParser {
public:
    JsonParser(GUIManager& guiManager);
};

class SGMLParser : public LayoutParser {
public:
    SGMLParser(GUIManager& guiManager);
};
```

`loadLayout()` zwraca `unique_ptr<GUIElement>` z pełną hierarchią zdefiniowaną
w pliku (wraz z zasobami i stylami) — przekaż go do `addElement()`.

```cpp
JsonParser parser(guiManager);
auto layout = parser.loadLayout("ui/main.json");
if (layout) guiManager.addElement(std::move(layout));

// odpowiednik XML:
SGMLParser sgml(guiManager);
auto layout2 = sgml.loadLayout("ui/main.xml");
```

## Logowanie

Prosty logger z poziomami i formatowaniem jak `std::format` (`{}` placeholdery).
Poziomy w namespace `sdlgui`:

```cpp
enum class LogLevel : int {
    Trace = 0, Debug = 1, Info = 2, Warning = 3, Error = 4, Fatal = 5, Off = 6
};

void setLogLevel(LogLevel level);   // próg: logowane są poziomy >= progu
LogLevel getLogLevel();
```

Makra (przyjmują tag i format `{}` jak `std::format`):

```cpp
LOG_TRACE(tag, ...)
LOG_DEBUG(tag, ...)
LOG_INFO(tag, ...)
LOG_WARNING(tag, ...)
LOG_ERROR(tag, ...)
LOG_FATAL(tag, ...)
```

`LOG_ACTIVE_LEVEL` (domyślnie `LOG_LEVEL_INFO`) wycina na etapie kompilacji
makra poniżej progu — możesz je nadpisać w definicji przed includem nagłówka.

```cpp
sdlgui::setLogLevel(sdlgui::LogLevel::Debug);
LOG_INFO("App", "Start: {}x{}", width, height);
LOG_ERROR("App", "Nie udało się: {}", SDL_GetError());
```

## Easing

Funkcje przejścia (easing) dla `AnimationManager::createAnimation` — wszystkie
o sygnaturze `float(float)` (t od 0.0 do 1.0, zwracają sprogressowany postęp):

```cpp
namespace Easing {
    inline float linear(float t);
    inline float easeInQuad(float t);
    inline float easeOutQuad(float t);
    inline float easeInOutQuad(float t);
}
```

| Funkcja | Charakter |
|---------|-----------|
| `Easing::linear` | Jednostajne tempo (domyślne) |
| `Easing::easeInQuad` | Wolny start, przyspieszanie |
| `Easing::easeOutQuad` | Szybki start, spowalnianie |
| `Easing::easeInOutQuad` | Wolno → szybko → wolno |

```cpp
auto* anims = manager.getAnimationManager();
float x = 0.0f;
anims->createAnimation<float>(&x, 0.0f, 300.0f, 1000, Easing::easeOutQuad);
```
