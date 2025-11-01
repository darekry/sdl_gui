# SDL GUI dla Gier RTS

## Wprowadzenie

SDL GUI to kompletna biblioteka do tworzenia interfejsów użytkownika w grach RTS, oparta na SDL2. Biblioteka została zaprojektowana z myślą o prostocie i wydajności, dostarczając wszystkie niezbędne narzędzia do budowy profesjonalnych interfejsów gier strategicznych.

### Kluczowe Możliwości

**🎨 Animacje i efekty wizualne**
- System animacji z funkcjami easing dla płynnych przejść
- AnimatedImage dla animacji sprite'ów jednostek
- AnimationManager z automatycznym zarządzaniem cyklem życia

**🖱️ Obsługa custom kursorów**
- Cursor - klasa do zarządzania kursorami z wieloma stanami
- Ładowanie tekstur kursorów z ustawianiem hotspot
- Animowane kursory dla różnych akcji (budowanie, atak, ruch)

**⚡ System cache'owania dla wydajności**
- TextureManager - cache'owanie tekstur
- FontManager - cache'owanie czcionek
- Renderowanie cache per-element dla optymalizacji
- Direct rendering dla dynamicznych elementów

## Animacje

### AnimationManager

AnimationManager to serce systemu animacji biblioteki SDL GUI. Zarządza wszystkimi animacjami w aplikacji i udostępnia prosty API do tworzenia płynnych efektów wizualnych.

```cpp
// Przykład użycia AnimationManager
auto& animationManager = guiManager.getAnimationManager();

// Animacja pozycji jednostki
animationManager.createAnimation(
    &unit.position.x,  // wskaźnik do animowanej właściwości
    unit.position.x,   // wartość początkowa
    targetX,           // wartość końcowa
    1000,              // czas trwania w ms
    Easing::Type::Linear  // funkcja easing
);

// Animacja z callback'iem po zakończeniu
animationManager.createAnimation(
    &healthBar.width,
    currentWidth,
    newWidth,
    500,
    Easing::Type::EaseOutQuad,
    [this]() { // callback po zakończeniu
        onHealthAnimationComplete();
    }
);
```

### AnimatedImage

AnimatedImage to specjalizowany widget do animacji sprite'ów, idealny dla animacji jednostek w grach RTS.

```cpp
// Tworzenie animowanej jednostki
auto soldier = std::make_unique<AnimatedImage>(guiManager, 100, 100, 64, 64);

// Konfiguracja sprite sheet (9 klatek w poziomie)
soldier->setSpriteSheet("assets/units/soldier_walk.png", 9, 1);
soldier->setFPS(12.0f);  // 12 klatek na sekundę
soldier->setLoop(true);
soldier->play();

// Przełączanie animacji w zależności od stanu jednostki
if (unitState == UnitState::WALKING) {
    soldier->setSpriteSheet("assets/units/soldier_walk.png", 6, 1);
    soldier->setFPS(8.0f);
} else if (unitState == UnitState::ATTACKING) {
    soldier->setSpriteSheet("assets/units/soldier_attack.png", 4, 1);
    soldier->setFPS(15.0f);
}
```

### Funkcje Easing

Biblioteka dostarcza różne funkcje easing dla naturalnych animacji:

```cpp
namespace Easing {
    enum class Type {
        Linear,      // Równomierny ruch
        EaseInQuad,  // Przyspieszenie (początek wolny, koniec szybki)
        EaseOutQuad, // Hamowanie (początek szybki, koniec wolny)
        EaseInOutQuad // Kombinacja przyspieszenia i hamowania
    };
    
    template<typename T>
    T interpolate(const T& start, const T& end, float progress, Type type);
}
```

**Praktyczne zastosowania w RTS:**
- Ruch kamery: `EaseInOutQuad` dla płynnych przejść
- Animacje menu: `EaseOutQuad` dla pojawiania się
- Ruchy jednostek: `Linear` dla precyzyjnego pathfinding
- Efekty budowania: `EaseInQuad` dla narastających efektów

## Obsługa Kursorów

### Custom Cursors

Klasa `Cursor` w [`src/cursor.hpp`](src/cursor.hpp:21) umożliwia tworzenie zaawansowanych systemów kursorów dostosowanych do różnych stanów gry.

```cpp
// Inicjalizacja systemu kursorów
auto cursor = std::make_unique<Cursor>(guiManager);
Cursor* cursorPtr = cursor.get();
guiManager.addElement(std::move(cursor));

// Konfiguracja kursorów dla różnych stanów
cursorPtr->setCursorTexture(CursorState::Normal, "assets/cursors/normal.png", 8, 8);
cursorPtr->setCursorTexture(CursorState::Hover, "assets/cursors/hover.png", 8, 8);
cursorPtr->setCursorTexture(CursorState::Pressed, "assets/cursors/pressed.png", 8, 8);

// Animowany kursor dla stanu "zajęty"
cursorPtr->setAnimatedCursor(
    CursorState::Busy, 
    "assets/cursors/busy.png",    // plik sprite sheet
    4,                            // liczba klatek w poziomie
    2,                            // liczba klatek w pionie
    8.0f,                         // FPS animacji
    16, 16                        // wymiary pojedynczej klatki
);

// Callback dla zmiany stanu kursora
cursorPtr->setOnStateChanged([](CursorState state) {
    switch (state) {
        case CursorState::Normal: 
            setGameCursor(CursorType::DEFAULT); break;
        case CursorState::Hover:
            setGameCursor(CursorType::SELECT); break;
        case CursorState::Busy:
            setGameCursor(CursorType::BUILDING); break;
    }
});
```

### Dynamic Cursor States

Kursory mogą automatycznie zmieniać się w zależności od kontekstu:

```cpp
// Automatyczna zmiana kursora na podstawie interakcji
void updateCursorState(int mouseX, int mouseY) {
    // Sprawdź czy kursor znajduje się nad jednostką
    if (isOverUnit(mouseX, mouseY)) {
        cursor->setState(CursorState::Hover);
    }
    // Sprawdź czy można budować w tym miejscu
    else if (canBuildAt(mouseX, mouseY)) {
        cursor->setState(CursorState::Normal);
    }
    // Sprawdź czy jest jakaś akcja w toku
    else if (isActionInProgress()) {
        cursor->setState(CursorState::Busy);
    }
    else {
        cursor->setState(CursorState::Normal);
    }
}

// Skalowanie kursora dla różnych rozdzielczości
cursor->setScale(0.5f);  // Dla wysokich rozdzielczości
cursor->setScale(1.0f);  // Dla standardowych rozdzielczości
```

## Widgety dla RTS

### Panel (HUD Containers)

Panel to podstawowy kontener dla wszystkich elementów HUD w grze RTS.

```cpp
// Główny panel HUD
auto hudPanel = std::make_unique<Panel>(guiManager, 0, 540, 800, 60);
hudPanel->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 200});
hudPanel->setBorder(ElementState::Normal, SDL_Color{100, 100, 120, 255}, 2);

// Panel zasobów (lewy dolny róg)
auto resourcesPanel = std::make_unique<Panel>(guiManager, 10, 550, 200, 40);
resourcesPanel->setBackgroundColor(ElementState::Normal, SDL_Color{80, 80, 40, 255});

// Dodaj ikonki zasobów i liczniki
auto goldIcon = std::make_unique<Label>(guiManager, 10, 555, "💰", 24);
auto goldAmount = std::make_unique<Label>(guiManager, 40, 555, "1000", 16);
resourcesPanel->addChild(std::move(goldIcon));
resourcesPanel->addChild(std::move(goldAmount));
```

### ContextMenu (Unit Menus)

Menu kontekstowe dla jednostek i budynków.

```cpp
// Tworzenie menu kontekstowego dla jednostki
auto unitMenu = std::make_unique<ContextMenu>(guiManager);
ContextMenu* menuPtr = unitMenu.get();

// Dodaj opcje menu z callback'ami
menuPtr->addItem("Atak", [this]() {
    issueAttackCommand();
});

menuPtr->addItem("Ruch", [this]() {
    issueMoveCommand();
});

menuPtr->addSeparator();

menuPtr->addItem("Zatrzymaj", [this]() {
    issueStopCommand();
});

menuPtr->addItem("Właściwości", [this]() {
    showUnitProperties();
}, true); // opcja włączona domyślnie

// Wyświetlanie menu
void showContextMenu(int x, int y, Unit* unit) {
    menuPtr->showAt(x, y);
    selectedUnit = unit;
}
```

### TabControl (Multi-page Panels)

Kontrolki z zakładkami do organizacji złożonych interfejsów.

```cpp
// Tworzenie panelu z zakładkami dla zarządzania jednostkami
auto unitManagement = std::make_unique<TabControl>(guiManager, 100, 100, 600, 400);

// Dodaj zakładki
Panel* infantryTab = unitManagement->addTab("Piechota");
Panel* vehiclesTab = unitManagement->addTab("Pojazdy");
Panel* buildingsTab = unitManagement->addTab("Budynki");

// Zawartość zakładki piechoty
auto soldierList = std::make_unique<Panel>(guiManager, 10, 30, 580, 360);
auto createSoldierBtn = std::make_unique<Button>(guiManager, 10, 10, 150, 30, "Utwórz Żołnierza");
createSoldierBtn->setOnClickCallback([](GUIElement*) {
    createSoldier();
});
soldierList->addChild(std::move(createSoldierBtn));
infantryTab->addChild(std::move(soldierList));
```

### Button (Action Commands)

Przyciski dla komend akcji - podstawa interakcji w RTS.

```cpp
// Przycisk produkcji jednostki
auto trainButton = std::make_unique<Button>(guiManager, 50, 50, 120, 40, "Żołnierz");
trainButton->setOnClickCallback([this](GUIElement*) {
    if (canTrainUnit(UnitType::SOLDIER)) {
        trainUnit(UnitType::SOLDIER);
    }
});

// Style przycisku w różnych stanach
trainButton->setBackgroundColor(ElementState::Normal, SDL_Color{60, 120, 60, 255});
trainButton->setBackgroundColor(ElementState::Hover, SDL_Color{80, 150, 80, 255});
trainButton->setBackgroundColor(ElementState::Pressed, SDL_Color{40, 100, 40, 255});
trainButton->setTextColor(ElementState::Normal, SDL_Color{255, 255, 255, 255});

// Ikona na przycisku
auto soldierIcon = std::make_unique<Label>(guiManager, 5, 10, "⚔️", 24);
trainButton->addChild(std::move(soldierIcon));
```

### TextInput (Player/Unit Names)

Pola tekstowe do wprowadzania nazw graczy i jednostek.

```cpp
// Pole do nazwy gracza
auto playerNameInput = std::make_unique<TextInput>(guiManager, 200, 50, 200, 30);
playerNameInput->setPlaceholder("Wpisz nazwę gracza...");
playerNameInput->setMaxLength(20);
playerNameInput->setOnTextChanged([](const std::string& text) {
    validatePlayerName(text);
});

// Pole do nazwy jednostki
auto unitNameInput = std::make_unique<TextInput>(guiManager, 200, 100, 200, 30);
unitNameInput->setPlaceholder("Nazwij jednostkę...");
unitNameInput->setMaxLength(15);
unitNameInput->setOnTextEntered([](const std::string& text) {
    renameSelectedUnit(text);
});
```

## Optymalizacja Wydajności

### Cache System

Biblioteka wykorzystuje dwupoziomowy system cache'owania:

**1. Cache Zasobów (TextureManager i FontManager)**
```cpp
// TextureManager - cache'uje tekstury
auto& textureManager = guiManager.getTextureManager();

// Załaduj teksturę tylko raz, potem używaj z cache
auto unitTexture = textureManager.loadTexture("assets/units/soldier.png");
auto buildingTexture = textureManager.loadTexture("assets/buildings/barracks.png");

// FontManager - cache'uje czcionki
auto& fontManager = guiManager.getFontManager();
auto uiFont = fontManager.loadFont("assets/fonts/arial.ttf", 14);
auto titleFont = fontManager.loadFont("assets/fonts/arial.ttf", 24);
```

**2. Cache Renderowania (m_cachedTexture)**
```cpp
// Każdy element GUI ma własny cache renderowania
// Cache jest automatycznie odświeżany gdy element się zmienia
element->setPosition(newX, newY);  // Automatycznie unieważnia cache
element->setSize(newWidth, newHeight);  // Rekreacja cache
element->markDirty();  // Ręczne unieważnienie cache
```

### Direct Rendering

Dla elementów wymagających częstych aktualizacji używaj direct rendering:

```cpp
class Minimap : public GUIElement {
public:
    bool wantsDirectRender() const override { 
        return true;  // Renderuj bezpośrednio, nie cache'uj
    }
    
    void drawDirect(SDL_Renderer* renderer) override {
        // Rysuj minimapę bezpośrednio na ekran
        // Aktualizuj każdą klatkę
        drawMinimapTiles(renderer);
        drawUnitMarkers(renderer);
        drawCameraView(renderer);
    }
};
```

### Best Practices

**1. Optymalizacja tekstur**
- Używaj atlasów tekstur dla podobnych elementów
- Zachowaj spójne rozmiary (potęgi liczby 2)
- Kompresuj tekstury gdy to możliwe

**2. Zarządzanie pamięcią**
- Używaj `std::unique_ptr` dla hierarchii elementów
- Używaj `std::shared_ptr` dla współdzielonych zasobów
- Oznaczaj elementy do usunięcia przez `markForDeletion()`

**3. Renderowanie**
- Cache'uj statyczne elementy HUD
- Używaj direct rendering dla elementów dynamicznych
- Minimalizuj zmiany stanu renderera

**4. Organizacja kodu**
```cpp
// Przykład optymalnej struktury dla gry RTS
class RTSGame {
private:
    GUIManager guiManager;
    
    // Podsystemy gry
    std::unique_ptr<UnitManager> unitManager;
    std::unique_ptr<BuildingManager> buildingManager;
    std::unique_ptr<ResourceManager> resourceManager;
    
    // Elementy GUI
    std::vector<std::unique_ptr<Panel>> hudPanels;
    std::unique_ptr<Cursor> gameCursor;
    std::unique_ptr<Minimap> minimap;
    
public:
    void update(float deltaTime);
    void render();
};
```

## Przykłady Implementacji

### HUD Example

Kompletny przykład HUD dla gry RTS:

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "text_input.hpp"

class RTSHUD {
private:
    GUIManager& guiManager;
    std::unique_ptr<Panel> mainHudPanel;
    std::unique_ptr<Panel> resourcesPanel;
    std::unique_ptr<Panel> commandsPanel;
    std::unique_ptr<Panel> minimapPanel;
    
public:
    RTSHUD(GUIManager& manager) : guiManager(manager) {
        createHUD();
    }
    
private:
    void createHUD() {
        // Główny panel HUD (dół ekranu)
        mainHudPanel = std::make_unique<Panel>(guiManager, 0, 540, 1024, 80);
        mainHudPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{40, 40, 50, 240});
        mainHudPanel->setBorder(ElementState::Normal, 
            SDL_Color{80, 80, 100, 255}, 2);
        
        // Panel zasobów (lewy dolny)
        createResourcesPanel();
        
        // Panel komend (prawy dolny)
        createCommandsPanel();
        
        // Minimapa (prawy górny róg)
        createMinimapPanel();
        
        guiManager.addElement(std::move(mainHudPanel));
    }
    
    void createResourcesPanel() {
        resourcesPanel = std::make_unique<Panel>(guiManager, 10, 550, 200, 60);
        resourcesPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{60, 60, 40, 255});
        
        // Złoto
        auto goldIcon = std::make_unique<Label>(guiManager, 10, 10, "💰", 20);
        auto goldText = std::make_unique<Label>(guiManager, 35, 10, "1000", 16);
        resourcesPanel->addChild(std::move(goldIcon));
        resourcesPanel->addChild(std::move(goldText));
        
        // Drewno
        auto woodIcon = std::make_unique<Label>(guiManager, 80, 10, "🌲", 20);
        auto woodText = std::make_unique<Label>(guiManager, 105, 10, "500", 16);
        resourcesPanel->addChild(std::move(woodIcon));
        resourcesPanel->addChild(std::move(woodText));
        
        mainHudPanel->addChild(std::move(resourcesPanel));
    }
    
    void createCommandsPanel() {
        commandsPanel = std::make_unique<Panel>(guiManager, 700, 550, 300, 60);
        commandsPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{50, 50, 60, 255});
        
        // Przyciski komend
        auto trainSoldierBtn = std::make_unique<Button>(guiManager, 10, 10, 80, 40, "Żołnierz");
        trainSoldierBtn->setOnClickCallback([](GUIElement*) {
            // Logika trenowania żołnierza
            trainUnit(UnitType::SOLDIER);
        });
        
        auto trainArcherBtn = std::make_unique<Button>(guiManager, 100, 10, 80, 40, "Łucznik");
        trainArcherBtn->setOnClickCallback([](GUIElement*) {
            trainUnit(UnitType::ARCHER);
        });
        
        auto buildBarracksBtn = std::make_unique<Button>(guiManager, 190, 10, 80, 40, "Koszary");
        buildBarracksBtn->setOnClickCallback([](GUIElement*) {
            startBuilding(BuildingType::BARRACKS);
        });
        
        commandsPanel->addChild(std::move(trainSoldierBtn));
        commandsPanel->addChild(std::move(trainArcherBtn));
        commandsPanel->addChild(std::move(buildBarracksBtn));
        
        mainHudPanel->addChild(std::move(commandsPanel));
    }
    
    void createMinimapPanel() {
        minimapPanel = std::make_unique<Panel>(guiManager, 800, 10, 200, 150);
        minimapPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{20, 20, 30, 255});
        minimapPanel->setBorder(ElementState::Normal, 
            SDL_Color{100, 100, 100, 255}, 1);
        
        mainHudPanel->addChild(std::move(minimapPanel));
    }
};
```

### Context Menu Example

Menu kontekstowe dla jednostek:

```cpp
class UnitContextMenu {
private:
    GUIManager& guiManager;
    std::unique_ptr<ContextMenu> contextMenu;
    
public:
    UnitContextMenu(GUIManager& manager) : guiManager(manager) {
        createContextMenu();
    }
    
    void showForUnit(Unit* unit, int x, int y) {
        // Dostosuj menu do typu jednostki
        setupMenuForUnitType(unit->getType());
        contextMenu->showAt(x, y);
    }
    
private:
    void createContextMenu() {
        contextMenu = std::make_unique<ContextMenu>(guiManager);
        
        // Ruch
        contextMenu->addItem("Ruch", [this]() {
            enterMoveMode();
        });
        
        // Atak
        contextMenu->addItem("Atak", [this]() {
            enterAttackMode();
        });
        
        contextMenu->addSeparator();
        
        // Opcje specjalne (będą dostosowane dynamicznie)
        contextMenu->addItem("Właściwości", [this]() {
            showUnitProperties();
        });
    }
    
    void setupMenuForUnitType(UnitType type) {
        // Wyczyść poprzednie opcje dynamiczne
        contextMenu->clearDynamicItems();
        
        switch (type) {
            case UnitType::WORKER:
                addWorkerOptions();
                break;
            case UnitType::SOLDIER:
                addSoldierOptions();
                break;
            case UnitType::BUILDING:
                addBuildingOptions();
                break;
        }
    }
    
    void addWorkerOptions() {
        contextMenu->addItem("Zbieraj zasoby", [this]() {
            assignResourceGathering();
        });
        
        contextMenu->addItem("Buduj", [this]() {
            enterBuildMode();
        });
        
        contextMenu->addItem("Napraw", [this]() {
            enterRepairMode();
        });
    }
};
```

### Custom Cursor Example

Zaawansowany system kursorów:

```cpp
class GameCursor {
private:
    GUIManager& guiManager;
    std::unique_ptr<Cursor> cursor;
    
public:
    GameCursor(GUIManager& manager) : guiManager(manager) {
        createCursors();
    }
    
    void update(int mouseX, int mouseY) {
        auto newState = determineCursorState(mouseX, mouseY);
        cursor->setState(newState);
    }
    
private:
    void createCursors() {
        cursor = std::make_unique<Cursor>(guiManager);
        
        // Kursor domyślny
        cursor->setCursorTexture(CursorState::Normal, "assets/cursors/arrow.png", 5, 5);
        
        // Kursor przy najechaniu na jednostkę
        cursor->setCursorTexture(CursorState::Hover, "assets/cursors/select.png", 5, 5);
        
        // Kursor przy najechaniu na budynek
        cursor->setCursorTexture(CursorState::Pressed, "assets/cursors/building.png", 5, 5);
        
        // Animowany kursor podczas akcji
        cursor->setAnimatedCursor(CursorState::Busy, 
            "assets/cursors/busy.png", 4, 1, 12.0f, 16, 16);
        
        // Kursor tekstowy
        cursor->setCursorTexture(CursorState::Text, "assets/cursors/text.png", 2, 10);
        
        guiManager.addElement(std::move(cursor));
    }
    
    CursorState determineCursorState(int x, int y) {
        // Sprawdź element pod kursorem
        auto element = guiManager.getElementAt(x, y);
        
        if (element) {
            if (element->hasTag("unit")) {
                return CursorState::Hover;
            } else if (element->hasTag("building")) {
                return CursorState::Pressed;
            } else if (element->hasTag("text_input")) {
                return CursorState::Text;
            }
        }
        
        // Sprawdź stan gry
        if (isGameActionInProgress()) {
            return CursorState::Busy;
        }
        
        return CursorState::Normal;
    }
};
```

### Animation Example

System animacji dla jednostek:

```cpp
class UnitAnimationSystem {
private:
    GUIManager& guiManager;
    std::map<Unit*, std::unique_ptr<AnimatedImage>> unitAnimations;
    
public:
    void addUnit(Unit* unit) {
        auto animation = std::make_unique<AnimatedImage>(guiManager, 
            unit->getX(), unit->getY(), 64, 64);
        
        // Domyślna animacja bezczynności
        animation->setSpriteSheet("assets/units/idle.png", 1, 1);
        animation->setFPS(1.0f);
        
        unitAnimations[unit] = std::move(animation);
        guiManager.addElement(unitAnimations[unit].get());
    }
    
    void updateUnitAnimation(Unit* unit, UnitState newState) {
        auto it = unitAnimations.find(unit);
        if (it != unitAnimations.end()) {
            auto& animation = it->second;
            
            switch (newState) {
                case UnitState::IDLE:
                    playIdleAnimation(animation.get());
                    break;
                case UnitState::WALKING:
                    playWalkingAnimation(animation.get());
                    break;
                case UnitState::ATTACKING:
                    playAttackAnimation(animation.get());
                    break;
                case UnitState::DEAD:
                    playDeathAnimation(animation.get());
                    break;
            }
        }
    }
    
private:
    void playIdleAnimation(AnimatedImage* animation) {
        animation->setSpriteSheet("assets/units/idle.png", 4, 1);
        animation->setFPS(2.0f);
        animation->setLoop(true);
        animation->play();
    }
    
    void playWalkingAnimation(AnimatedImage* animation) {
        animation->setSpriteSheet("assets/units/walk.png", 8, 1);
        animation->setFPS(12.0f);
        animation->setLoop(true);
        animation->play();
    }
    
    void playAttackAnimation(AnimatedImage* animation) {
        animation->setSpriteSheet("assets/units/attack.png", 6, 1);
        animation->setFPS(15.0f);
        animation->setLoop(false);
        animation->setOnComplete([animation]() {
            // Po ataku wróć do animacji bezczynności
            animation->setSpriteSheet("assets/units/idle.png", 4, 1);
            animation->setFPS(2.0f);
            animation->play();
        });
        animation->play();
    }
};
```

## Najlepsze Praktyki

### 1. Organizacja Kodu

**Separacja logiki i widoku**
```cpp
// Dobrze - oddzielona logika gry od GUI
class Unit {
private:
    UnitState m_state;
    Vector2 m_position;
    
public:
    void update(float deltaTime);
    void setState(UnitState newState);
};

class UnitWidget : public GUIElement {
private:
    Unit* m_unit;
    std::unique_ptr<AnimatedImage> m_animation;
    
public:
    void render() override {
        m_animation->setPosition(m_unit->getPosition());
        updateAnimationBasedOnState(m_unit->getState());
    }
};
```

### 2. Zarządzanie Zasobami

**Lazy loading i cache'owanie**
```cpp
class ResourceManager {
private:
    TextureManager& textureManager;
    FontManager& fontManager;
    std::map<std::string, bool> loadedTextures;
    
public:
    SharedTexture getTexture(const std::string& path) {
        if (!loadedTextures[path]) {
            auto texture = textureManager.loadTexture(path);
            loadedTextures[path] = true;
            return texture;
        }
        return textureManager.getTexture(path);
    }
};
```

### 3. Optymalizacja Wydajności

**Minimalizowanie zmian stanu renderera**
```cpp
// Dobrze - grupuj operacje na tej samej teksturze
void renderUnits() {
    auto unitTexture = textureManager.getTexture("units.png");
    
    for (auto& unit : units) {
        SDL_Rect src = getUnitFrame(unit);
        SDL_Rect dst = {unit.x, unit.y, 64, 64};
        SDL_RenderCopy(renderer, unitTexture.get(), &src, &dst);
    }
}
```

### 4. Event Handling

**Optymalne przekazywanie zdarzeń**
```cpp
void GUIManager::processEvent(const SDL_Event& event) {
    // Przestań propagację po obsłużeniu zdarzenia
    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if ((*it)->processEvent(event)) {
            break;  // Zdarzenie zostało obsłużone
        }
    }
}
```

### 5. Debugowanie i Testowanie

**Logowanie stanów**
```cpp
class DebugGUI {
public:
    static void drawFPS(int fps) {
        auto fpsLabel = std::make_unique<Label>(guiManager, 10, 10, 
            "FPS: " + std::to_string(fps), 16);
        guiManager.addElement(std::move(fpsLabel));
    }
    
    static void drawMemoryUsage() {
        auto memoryInfo = getMemoryUsage();
        auto memoryLabel = std::make_unique<Label>(guiManager, 10, 30,
            "Memory: " + std::to_string(memoryInfo) + " MB", 16);
        guiManager.addElement(std::move(memoryLabel));
    }
};
```

## Podsumowanie

Biblioteka SDL GUI dostarcza kompletny zestaw narzędzi do tworzenia profesjonalnych interfejsów w grach RTS. Dzięki systemowi animacji, zaawansowanej obsłudze kursorów, optymalizacji wydajności i bogatym przykładom implementacji, programiści mogą szybko tworzyć responsywne i atrakcyjne interfejsy użytkownika.

Kluczowe zalety biblioteki:
- **Wydajność** - dwupoziomowy system cache'owania
- **Elastyczność** - modularny design z możliwością rozszerzania
- **Łatwość użycia** - intuicyjne API z przykładami
- **Optymalizacja** - automatyczne zarządzanie pamięcią
- **Kompletność** - wszystkie potrzebne widgety dla RTS

Dokumentacja zawiera praktyczne przykłady, najlepsze praktyki i gotowe implementacje, które można bezpośrednio wykorzystać w projektach gier RTS.