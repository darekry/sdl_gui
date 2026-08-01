# Menedżery

Menedżery to klasy sterujące całością biblioteki: hierarchią elementów, zasobami
(czcionki, tekstury), czasem (timery, animacje) oraz całymi ekranami i oknami.
Wszystkie są tworzone i żyją wewnątrz `GUIManager` (poza `ScreenManager`
i `WindowManager`, które działają obok niego).

## GUIManager

Centralny obiekt aplikacji. Posiada renderer SDL, hierarchię elementów,
tooltipy, focus klawiaturowy, przechwytywanie myszy oraz pod-menedżery
czcionek, tekstur, timerów i animacji. Jeden `GUIManager` = jeden renderer.

### Konstrukcja

```cpp
GUIManager(SDL_Renderer* renderer);
```

```cpp
SDLApp app("Aplikacja", 800, 600);
GUIManager manager(app.getRenderer());
manager.setTheme(ThemePresets::createDarkTheme());  // KONIECZNE przed renderowaniem
manager.setWindowSize(800, 600);                    // dla anchorów
```

### Dodawanie elementów

| Metoda | Opis |
|--------|------|
| `GUIElement* addElement(std::unique_ptr<GUIElement> element);` | Dodaje element najwyższego poziomu; przejmuje ownership, zwraca surowy wskaźnik |
| `template<typename T, typename... Args> T* create(Args&&... args);` | Tworzy widget przez `make_unique<T>(*this, args...)` i auto-dodaje do managera; zwraca `T*` |
| `template<typename T, typename... Args> T* create(GUIElement* parent, Args&&... args);` | Jak wyżej, ale auto-dodaje jako dziecko `parent` |
| `std::unique_ptr<GUIElement> detachElement(GUIElement* element);` | Wyciąga element top-level bez usuwania (element pozostaje zarejestrowany jako żywy); `nullptr` gdy element nie jest top-level |

```cpp
auto btn = manager.create<Button>(10, 10, 120, 40, "Kliknij");      // top-level

auto panel = manager.create<Panel>(0, 0, 300, 200);
auto label = manager.create<Label>(panel, 10, 50, 100, 30, "0");    // dziecko panelu
```

### Pętla główna

Kolejność jest krytyczna: `processEvent` → `update` → `cleanup` → `render`.

| Metoda | Opis |
|--------|------|
| `bool processEvent(const SDL_Event& e);` | Przekazuje zdarzenie SDL do elementów (hover, klik, focus, tooltipy) |
| `void update();` | Aktualizuje timery, animacje, tooltipy i stany hover |
| `void cleanup();` | Usuwa elementy oznaczone `markForDeletion()` |
| `void render();` | Renderuje całą hierarchię do renderera |

### Resize i rozmiar okna

| Metoda | Opis |
|--------|------|
| `void handleResize(int width, int height);` | Przelicza wszystkie elementy z anchorami; wołaj przy `SDL_EVENT_WINDOW_RESIZED` |
| `void setResizeCallback(ResizeCallback callback);` | Dodatkowy callback wołany po przeliczeniu anchorów (`ResizeCallback = std::function<void(int, int)>` — nowa szerokość, wysokość) |
| `void getWindowSize(int& width, int& height) const;` | Bieżący zapisany rozmiar okna |
| `void setWindowSize(int width, int height);` | Ustawia rozmiar okna; wywołaj raz przy inicjalizacji |

### Dostęp do zasobów i pod-menedżerów

| Metoda | Opis |
|--------|------|
| `SDL_Renderer* getRenderer() const;` | Renderer SDL |
| `SDL_GPUDevice* getGPUDevice() const;` | Urządzenie GPU (dla renderera GPU) lub `nullptr` dla CPU |
| `FontManager& getFontManager();` | Menedżer czcionek (patrz niżej) |
| `TextureManager& getTextureManager();` | Menedżer tekstur (patrz niżej) |
| `TimerManager* getTimerManager();` | Menedżer timerów (patrz niżej) |
| `AnimationManager* getAnimationManager();` | Menedżer animacji (patrz niżej) |

### Tooltipy, motyw, wskaźnik

| Metoda | Opis |
|--------|------|
| `void showTooltip(GUIElement* target, const std::string& text);` | Pokazuje tooltip nad elementem |
| `void hideTooltip();` | Ukrywa aktywny tooltip |
| `void setTheme(Theme theme);` | Ustawia motyw (kopiowany) |
| `Theme& getTheme();` | Bieżący motyw |
| `void setCursor(std::unique_ptr<Cursor> new_cursor);` | Ustawia własny kursor (widget `Cursor`); przekazanie `nullptr` przywraca domyślny |

### Focus, przechwytywanie myszy, żywotność

| Metoda | Opis |
|--------|------|
| `void captureMouse(GUIElement* element);` | Przechwytuje wszystkie zdarzenia myszy do elementu (używane m.in. przez Slider przy drag) |
| `void releaseMouse();` | Zwalnia przechwycenie |
| `void setKeyboardFocus(GUIElement* element);` | Ustawia element z focusem klawiaturowym |
| `GUIElement* getKeyboardFocus() const;` | Element z focusem lub `nullptr` |
| `void focusNextElement(bool forward);` | Przesuwa focus do następnego/poprzedniego elementu z `setCanGetKeyboardFocus(true)` (DFS z zawijaniem) |
| `bool isElementAlive(GUIElement* element) const;` | Czy element nadal istnieje w hierarchii |
| `GUIElement* findElementAt(int x, int y);` | Najgłębszy element pod punktem (współrzędne okna) |

Nawigacja klawiaturą: **Tab** = `focusNextElement(true)`, **Shift+Tab** =
`focusNextElement(false)`. Focus otrzymują tylko elementy z
`setCanGetKeyboardFocus(true)` (domyślnie m.in. Button, Checkbox, pola tekstowe).

### ElementRef — bezpieczne wskaźniki w callbackach

`ElementRef<T>` trzyma wskaźnik do elementu i sprawdza `isElementAlive()` przy
każdym dostępie — zwraca `nullptr`, gdy element został usunięty. Twórz go
**przed** `std::move` elementu do managera.

```cpp
ElementRef<Label> ref = manager.makeRef<Label>(label.get());

auto slider = manager.create<Slider>(10, 50, 200, 30, 0, 100, 50);
slider->setOnChangeCallback([ref](GUIElement* e) {
    auto* s = static_cast<Slider*>(e);
    if (s && ref) ref->setText(std::to_string(s->getValue()));
});
```

API `ElementRef<T>`: `ElementRef()` (pusty), `ElementRef(GUIManager& manager, T* ptr)`,
`T* get() const`, `T* operator->() const`, `T& operator*() const`,
`explicit operator bool() const`, `bool operator==(std::nullptr_t) const`.

## FontManager

Ładuje i cache'uje czcionki TTF. `SharedFont = std::shared_ptr<TTF_Font>` —
zasoby są współdzielone i niszczone automatycznie. Dostęp przez
`guiManager.getFontManager()`.

```cpp
FontManager();   // konstruktor (inicjalizuje SDL_ttf)
bool isInitialized() const;   // czy inicjalizacja SDL_ttf się powiodła
```

### Metody

| Metoda | Opis |
|--------|------|
| `SharedFont loadFont(std::string_view path, int size);` | Ładuje czcionkę; cache'owana po (ścieżka, rozmiar) — drugie wywołanie zwraca istniejący `SharedFont` |
| `SharedFont loadFontFromMemory(const uint8_t* data, size_t size, int fontSize, std::string_view key);` | Ładuje z pamięci (dla zasobów wkompilowanych); `key` to klucz cache'u |
| `void loadDefaultFont(std::string_view path, int size);` | Ustawia domyślną czcionkę |
| `SharedFont getDefaultFont() const;` | Domyślna czcionka |
| `TTF_Font* getFont(std::string_view path, int size);` | Surowe `TTF_Font*` (nie zarządza pamięcią — do odczytu rozmiarów) |
| `void getTextSize(std::string_view text, std::string_view fontPath, int fontSize, int* width, int* height);` | Rozmiar tekstu w pikselach |
| `int getTextWidth(TTF_Font* font, std::string_view text);` | Szerokość tekstu (cache'owana; cache do 1000 wpisów) |
| `void clearTextWidthCache();` | Czyści cache szerokości tekstu |

```cpp
FontManager& fonts = manager.getFontManager();
auto font = fonts.loadFont("assets/font.ttf", 16);   // drugi raz: z cache
fonts.loadDefaultFont("assets/font.ttf", 16);
auto def = fonts.getDefaultFont();
```

Uwaga: klasa **nie jest thread-safe** — wszystkie wywołania z wątku
posiadającego `GUIManager`.

## TextureManager

Ładuje i cache'uje tekstury SDL (`SharedTexture = std::shared_ptr<SDL_Texture>`).
Tworzony wewnętrznie przez `GUIManager` — dostęp przez
`guiManager.getTextureManager()`.

```cpp
explicit TextureManager(SDL_Renderer* renderer);
bool isInitialized() const;   // czy inicjalizacja SDL_image się powiodła
```

### Metody

| Metoda | Opis |
|--------|------|
| `SharedTexture loadTexture(std::string_view path);` | Ładuje plik obrazu (PNG/JPG/...); cache'owana po ścieżce |
| `SharedTexture createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color);` | Renderuje tekst do tekstury |
| `SharedTexture createTextureFromText(std::string_view text, std::string_view fontPath, int fontSize, const SDL_Color& color);` | Jak wyżej, z cache'owaniem po (ścieżka czcionki, rozmiar) |
| `SharedTexture loadTextureFromMemory(const uint8_t* data, size_t size, std::string_view key);` | Ładuje z pamięci (dla zasobów wkompilowanych) |
| `SharedTexture addTexture(std::string_view key, SDL_Texture* texture);` | Rejestruje istniejącą teksturę pod kluczem (przejmuje ownership) |
| `SharedTexture addTexture(std::string_view key, SharedTexture texture);` | Jak wyżej (wersja ze `shared_ptr`) |
| `SharedTexture getTexture(std::string_view key) const;` | Tekstura po kluczu (pusta, gdy brak) |
| `bool hasTexture(std::string_view key) const;` | Czy tekstura o kluczu istnieje |
| `bool queryTexture(std::string_view key, int& width, int& height);` | Rozmiar tekstury (ładuje, jeśli nie jest załadowana); `true` gdy dostępna |
| `void createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text);` | Tworzy domyślną teksturę z tekstu |
| `SharedTexture getDefaultTexture() const;` | Domyślna tekstura |
| `void pruneUnused();` | Usuwa tekstury bez aktywnych referencji |
| `void clearCache();` | Czyści cały cache |
| `size_t getCacheSize() const;` | Liczba tekstur w cache'u |

### Zasoby wkompilowane (embedded assets)

Zasoby wkompilowane w bibliotekę są ładowane automatycznie: `loadTexture("assets/...")`
zamiast szukać pliku na dysku zwraca wkompilowany asset. Działa to
transparentnie — nie musisz znać klucza, wystarczy normalna ścieżka.

```cpp
TextureManager& textures = manager.getTextureManager();
auto tex = textures.loadTexture("assets/button1.png");   // dysk LUB asset wkompilowany
int w = 0, h = 0;
if (textures.queryTexture("assets/button1.png", w, h)) {
    // w, h dostępne
}
```

Uwaga: klasa **nie jest thread-safe**.

## TimerManager

Jednorazowe i cykliczne timery powiązane z elementem GUI. Dostęp przez
`guiManager.getTimerManager()` (wskaźnik); `GUIManager::update()` wywołuje
`update()` timerów automatycznie.

```cpp
uint32_t addTimer(GUIElement* target, uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
void removeTimer(uint32_t timerId);
void update();
```

- `target` — element GUI, do którego timer jest przypisany (otrzyma go callback);
- `delay` — opóźnienie w milisekundach;
- `singleShot` — `true` = wykonaj raz, `false` = powtarzaj co `delay` ms;
- zwraca identyfikator timera (do `removeTimer`).

```cpp
auto* timers = manager.getTimerManager();
uint32_t id = timers->addTimer(btn, 1000, true, [](GUIElement* target) {
    if (target) static_cast<Button*>(target)->setText("Czas minął");
});
// timers->removeTimer(id);  // anulowanie przed odpaleniem
```

Uwaga: klasa **nie jest thread-safe** (ostrzeżenie z nagłówka).

## AnimationManager

Dwa mechanizmy: cykliczne callbacki (`addAnimation`) i animacje właściwości
(`createAnimation<T>`). Dostęp przez `guiManager.getAnimationManager()`
(wskaźnik); aktualizowany automatycznie z `GUIManager::update()`.

| Metoda | Opis |
|--------|------|
| `uint32_t addAnimation(uint32_t interval_ms, std::function<void()> callback);` | Rejestruje callback wykonywany co `interval_ms`; zwraca id (do `removeAnimation`) |
| `void removeAnimation(uint32_t id);` | Usuwa cykliczną animację |
| `template<typename T> void createAnimation(T* target_property, float start_value, float end_value, uint32_t duration, std::function<float(float)> easing = Easing::linear, Animation::CompleteCallback on_complete = nullptr, Animation::FrameCallback on_frame = nullptr);` | Animuje właściwość `T*` (działa dla `int*` i `float*`) od `start_value` do `end_value` w `duration` ms z funkcją easing |

Callbacki animacji: `Animation::CompleteCallback = std::function<void()>`
(po zakończeniu) i `Animation::FrameCallback = std::function<void()>`
(co klatkę po aktualizacji wartości). `on_complete` wywoływane po ostatniej
klatce.

```cpp
auto* anims = manager.getAnimationManager();

// Cykliczne "oddychanie" panelu — zmiana koloru co 500 ms
anims->addAnimation(500, []() {
    LOG_INFO("App", "tick");
});

// Animacja właściwości int* od 0 do 100 w 2 s z łagodnym startem/końcem
int progress = 0;
anims->createAnimation<int>(&progress, 0, 100, 2000, Easing::easeInOutQuad,
    []() { LOG_INFO("App", "koniec animacji"); },
    []() { LOG_INFO("App", "klatka"); });
```

Uwaga: `createAnimation` przechowuje **surowy wskaźnik** do animowanej
właściwości. Jeśli obiekt posiadający właściwość zostanie zniszczony przed
zakończeniem animacji, zachowanie jest niezdefiniowane — zapewnij, że animacje
kończą się przed destrukcją obiektu. Klasa **nie jest thread-safe**.

## ScreenManager

Dzieli aplikację na ekrany (menu, gra, ustawienia...) w jednym oknie i jednym
rendererze. Każdy ekran ma własne elementy GUI i cykl życia
(`onEnter`/`onExit`). Działa obok `GUIManager` — wymaga jego referencji.

```cpp
explicit ScreenManager(GUIManager& manager);
```

### Zarządzanie ekranami

| Metoda | Opis |
|--------|------|
| `bool addScreen(const std::string& name, std::unique_ptr<Screen> screen);` | Dodaje ekran (przejmuje ownership); `false` gdy nazwa już istnieje |
| `bool removeScreen(const std::string& name);` | Usuwa ekran; `false` gdy nie istnieje lub jest aktywny |
| `bool hasScreen(const std::string& name) const;` | Czy ekran istnieje |
| `Screen* getScreen(const std::string& name) const;` | Ekran po nazwie lub `nullptr` |

### Przełączanie

| Metoda | Opis |
|--------|------|
| `bool changeScreen(const std::string& name);` | Woła `onExit()` bieżącego i `onEnter()` nowego ekranu; `false` gdy brak ekranu |
| `std::string getCurrentScreenName() const;` | Nazwa aktywnego ekranu (puste, gdy brak) |
| `Screen* getCurrentScreen() const;` | Aktywny ekran lub `nullptr` |
| `bool pushScreen(const std::string& name);` | Nakłada ekran na stos (overlay — pauza, ustawienia); poprzedni ekran zostaje w tle, ale nie dostaje zdarzeń |
| `std::string popScreen();` | Zdejmuje górę stosu; zwraca nazwę zdjętego ekranu (puste, gdy stos pusty) |
| `size_t getStackDepth() const;` | Głębokość stosu |
| `void clearStack();` | Czyści cały stos |

### Integracja z pętlą

| Metoda | Opis |
|--------|------|
| `bool handleEvent(const SDL_Event& e);` | Zdarzenia do aktywnego ekranu (lub wierzchu stosu); `true` gdy zużyto |
| `void update();` | Aktualizacja aktywnego ekranu |
| `void render(SDL_Renderer* renderer);` | Renderuje ekrany od dołu stosu do wierzchu |
| `void cleanup();` | Czyści oznaczone elementy w `GUIManager` |

### Screen — klasa bazowa ekranu

```cpp
class Screen {
    virtual ~Screen() = default;
    virtual void onEnter(GUIManager& manager) = 0;            // dodaj elementy
    virtual void onExit(GUIManager& manager) = 0;             // sprzątanie
    virtual bool handleEvent(GUIManager& manager, const SDL_Event& e) = 0;  // true = zużyto
    virtual void update(GUIManager& manager) = 0;
    virtual void render(GUIManager& manager, SDL_Renderer* renderer) = 0;
    virtual std::string getName() const = 0;                  // do logowania
    virtual bool wantsPreProcessEvent() const { return false; }  // przechwyć zdarzenie przed GUIManager
};
```

`wantsPreProcessEvent()` zwraca domyślnie `false`; zwróć `true`, gdy ekran ma
przechwytywać zdarzenia przed elementami GUI.

```cpp
class MenuScreen : public Screen {
public:
    void onEnter(GUIManager& manager) override {
        auto btn = manager.create<Button>(10, 10, 120, 40, "Start");
        btn->setOnClickCallback([](GUIElement*) { /* ... */ });
    }
    void onExit(GUIManager& manager) override { /* usuń elementy */ }
    bool handleEvent(GUIManager&, const SDL_Event&) override { return false; }
    void update(GUIManager&) override {}
    void render(GUIManager&, SDL_Renderer*) override {}
    std::string getName() const override { return "Menu"; }
};

ScreenManager screens(manager);
screens.addScreen("menu", std::make_unique<MenuScreen>());
screens.changeScreen("menu");

// w pętli: screens.handleEvent(e); screens.update(); screens.render(renderer); screens.cleanup();
```

## WindowManager

Wiele niezależnych okien systemowych, każde z własnym `SDL_Window`,
rendererem i `GUIManager`. Inicjalizuje SDL, SDL_image i SDL_ttf w konstruktorze.

```cpp
WindowManager();   // throw std::runtime_error, gdy inicjalizacja SDL zawiedzie
```

### Tworzenie i dostęp do okien

| Metoda | Opis |
|--------|------|
| `Window* createWindow(const std::string& title, int width, int height, bool resizable = false);` | Tworzy okno; `nullptr` przy błędzie |
| `Window* createWindow(const std::string& title, int width, int height, const char* name, bool resizable);` | Jak wyżej, z nazwą renderera SDL (`NULL` = domyślny) |
| `Window* getWindowByID(Uint32 windowID) const;` | Okno po identyfikatorze z `event.window.windowID` |
| `Window* getWindow(size_t index) const;` | Okno po indeksie (0-based) |
| `size_t getWindowCount() const;` | Liczba okien (razem z ukrytymi) |
| `Window* getFocusedWindow() const;` | Okno z focusem lub `nullptr` |
| `bool hasOpenWindows() const;` | Czy istnieje okno nieoznaczone do zamknięcia |

### Zamykanie

| Metoda | Opis |
|--------|------|
| `bool closeWindow(Uint32 windowID);` | Oznacza okno do zamknięcia (usuwane w `cleanupAll`); `false` gdy nie znaleziono |
| `void closeSecondaryWindows();` | Zamyka wszystkie okna poza głównym |
| `void closeAllWindows();` | Zamyka wszystkie okna (zakończenie aplikacji) |

### Pętla główna

| Metoda | Opis |
|--------|------|
| `bool processEvents();` | Przetwarza zdarzenia SDL i routuje je do właściwego okna po `windowID`; `false` gdy wszystkie okna zamknięte (sygnał wyjścia) |
| `void updateAll();` | Aktualizuje menedżery wszystkich okien |
| `void renderAll();` | Renderuje wszystkie widoczne okna |
| `void cleanupAll();` | Usuwa oznaczone elementy i okna oznaczone do zamknięcia |
| `bool shouldQuit() const;` | `true`, gdy wszystkie okna zamknięte lub odebrano `SDL_EVENT_QUIT` |
| `void requestQuit();` | Prośba o zakończenie (zamyka wszystkie okna) |

```cpp
WindowManager wm;
Window* mainWindow = wm.createWindow("Aplikacja", 800, 600, true);
Window* form = wm.createWindow("Formularz", 400, 300);
form->setOnCloseCallback([](Window* w) { w->markForClose(); });

while (wm.hasOpenWindows()) {
    wm.processEvents();
    wm.updateAll();
    wm.renderAll();
    wm.cleanupAll();
}
```

### Window — pojedyncze okno

```cpp
Window(const std::string& title, int width, int height,
       const char* name = NULL, bool resizable = false);
// throw std::runtime_error, gdy SDL_CreateWindow/SDL_CreateRenderer zawiedzie
```

| Metoda | Opis |
|--------|------|
| `SDL_Window* getSDLWindow() const;` | Niskopoziomowy `SDL_Window` |
| `SDL_Renderer* getRenderer() const;` | Renderer okna |
| `GUIManager& getGUIManager();` | Menedżer GUI okna (dodawaj tu elementy) |
| `Uint32 getWindowID() const;` | Identyfikator okna SDL |
| `std::string getTitle() const;` | Tytuł okna |
| `void getSize(int& width, int& height) const;` | Bieżący rozmiar okna |
| `bool isVisible() const;` / `void show();` / `void hide();` | Widoczność okna |
| `bool isMarkedForClose() const;` / `void markForClose();` | Oznaczanie do zamknięcia |
| `bool isFocused() const;` | Czy okno ma focus |
| `bool processEvent(const SDL_Event& e);` | Zdarzenie dla tego okna (przez `WindowManager`) |
| `void update();` / `void render();` / `void cleanup();` | Cykl życia menedżera okna |
| `using CloseCallback = std::function<void(Window*)>;` `void setOnCloseCallback(CloseCallback callback);` | Callback przy prośbie zamknięcia (przycisk X, Alt+F4) |
| `using ResizeCallback = std::function<void(Window*, int, int)>;` `void setOnResizeCallback(ResizeCallback callback);` | Callback przy zmianie rozmiaru |

```cpp
Window* win = wm.createWindow("Aplikacja", 800, 600, true);
win->getGUIManager().setTheme(ThemePresets::createDarkTheme());
auto btn = win->getGUIManager().create<Button>(10, 10, 120, 40, "OK");
win->setOnResizeCallback([](Window* w, int width, int height) {
    w->getGUIManager().handleResize(width, height);
});
```
