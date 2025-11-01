# SDL GUI for RTS Games

## Introduction

SDL GUI is a complete library for creating user interfaces in RTS games, built on SDL2. The library was designed with simplicity and performance in mind, providing all the necessary tools to build professional interfaces for strategy games.

### Key Features

**🎨 Animations and Visual Effects**
- Animation system with easing functions for smooth transitions
- AnimatedImage for unit sprite animations
- AnimationManager with automatic lifecycle management

**🖱️ Custom Cursor Support**
- Cursor class for managing cursors with multiple states
- Loading cursor textures with hotspot configuration
- Animated cursors for different actions (building, attack, movement)

**⚡ Performance Caching System**
- TextureManager - texture caching
- FontManager - font caching
- Per-element rendering cache for optimization
- Direct rendering for dynamic elements

## Animations

### AnimationManager

AnimationManager is the heart of the SDL GUI animation system. It manages all animations in the application and provides a simple API for creating smooth visual effects.

```cpp
// Example usage of AnimationManager
auto& animationManager = guiManager.getAnimationManager();

// Unit position animation
animationManager.createAnimation(
    &unit.position.x,  // pointer to animated property
    unit.position.x,   // initial value
    targetX,           // final value
    1000,              // duration in ms
    Easing::Type::Linear  // easing function
);

// Animation with completion callback
animationManager.createAnimation(
    &healthBar.width,
    currentWidth,
    newWidth,
    500,
    Easing::Type::EaseOutQuad,
    [this]() { // completion callback
        onHealthAnimationComplete();
    }
);
```

### AnimatedImage

AnimatedImage is a specialized widget for sprite animation, ideal for unit animations in RTS games.

```cpp
// Creating an animated unit
auto soldier = std::make_unique<AnimatedImage>(guiManager, 100, 100, 64, 64);

// Configure sprite sheet (9 frames horizontally)
soldier->setSpriteSheet("assets/units/soldier_walk.png", 9, 1);
soldier->setFPS(12.0f);  // 12 frames per second
soldier->setLoop(true);
soldier->play();

// Switch animations based on unit state
if (unitState == UnitState::WALKING) {
    soldier->setSpriteSheet("assets/units/soldier_walk.png", 6, 1);
    soldier->setFPS(8.0f);
} else if (unitState == UnitState::ATTACKING) {
    soldier->setSpriteSheet("assets/units/soldier_attack.png", 4, 1);
    soldier->setFPS(15.0f);
}
```

### Easing Functions

The library provides various easing functions for natural animations:

```cpp
namespace Easing {
    enum class Type {
        Linear,      // Uniform motion
        EaseInQuad,  // Acceleration (slow start, fast end)
        EaseOutQuad, // Deceleration (fast start, slow end)
        EaseInOutQuad // Combination of acceleration and deceleration
    };
    
    template<typename T>
    T interpolate(const T& start, const T& end, float progress, Type type);
}
```

**Practical applications in RTS:**
- Camera movement: `EaseInOutQuad` for smooth transitions
- Menu animations: `EaseOutQuad` for appearance effects
- Unit movements: `Linear` for precise pathfinding
- Building effects: `EaseInQuad` for progressive effects

## Cursor Management

### Custom Cursors

The `Cursor` class in [`src/cursor.hpp`](src/cursor.hpp:21) enables creation of advanced cursor systems tailored to different game states.

```cpp
// Initialize cursor system
auto cursor = std::make_unique<Cursor>(guiManager);
Cursor* cursorPtr = cursor.get();
guiManager.addElement(std::move(cursor));

// Configure cursors for different states
cursorPtr->setCursorTexture(CursorState::Normal, "assets/cursors/normal.png", 8, 8);
cursorPtr->setCursorTexture(CursorState::Hover, "assets/cursors/hover.png", 8, 8);
cursorPtr->setCursorTexture(CursorState::Pressed, "assets/cursors/pressed.png", 8, 8);

// Animated cursor for "busy" state
cursorPtr->setAnimatedCursor(
    CursorState::Busy, 
    "assets/cursors/busy.png",    // sprite sheet file
    4,                            // number of frames horizontally
    2,                            // number of frames vertically
    8.0f,                         // animation FPS
    16, 16                        // individual frame dimensions
);

// Callback for cursor state changes
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

Cursors can automatically change based on context:

```cpp
// Automatic cursor change based on interaction
void updateCursorState(int mouseX, int mouseY) {
    // Check if cursor is over a unit
    if (isOverUnit(mouseX, mouseY)) {
        cursor->setState(CursorState::Hover);
    }
    // Check if building is possible at this location
    else if (canBuildAt(mouseX, mouseY)) {
        cursor->setState(CursorState::Normal);
    }
    // Check if some action is in progress
    else if (isActionInProgress()) {
        cursor->setState(CursorState::Busy);
    }
    else {
        cursor->setState(CursorState::Normal);
    }
}

// Cursor scaling for different resolutions
cursor->setScale(0.5f);  // For high resolutions
cursor->setScale(1.0f);  // For standard resolutions
```

## RTS Widgets

### Panel (HUD Containers)

Panel is the basic container for all HUD elements in an RTS game.

```cpp
// Main HUD panel
auto hudPanel = std::make_unique<Panel>(guiManager, 0, 540, 800, 60);
hudPanel->setBackgroundColor(ElementState::Normal, SDL_Color{50, 50, 60, 200});
hudPanel->setBorder(ElementState::Normal, SDL_Color{100, 100, 120, 255}, 2);

// Resources panel (bottom left corner)
auto resourcesPanel = std::make_unique<Panel>(guiManager, 10, 550, 200, 40);
resourcesPanel->setBackgroundColor(ElementState::Normal, SDL_Color{80, 80, 40, 255});

// Add resource icons and counters
auto goldIcon = std::make_unique<Label>(guiManager, 10, 555, "💰", 24);
auto goldAmount = std::make_unique<Label>(guiManager, 40, 555, "1000", 16);
resourcesPanel->addChild(std::move(goldIcon));
resourcesPanel->addChild(std::move(goldAmount));
```

### ContextMenu (Unit Menus)

Contextual menus for units and buildings.

```cpp
// Create contextual menu for a unit
auto unitMenu = std::make_unique<ContextMenu>(guiManager);
ContextMenu* menuPtr = unitMenu.get();

// Add menu options with callbacks
menuPtr->addItem("Attack", [this]() {
    issueAttackCommand();
});

menuPtr->addItem("Move", [this]() {
    issueMoveCommand();
});

menuPtr->addSeparator();

menuPtr->addItem("Stop", [this]() {
    issueStopCommand();
});

menuPtr->addItem("Properties", [this]() {
    showUnitProperties();
}, true); // option enabled by default

// Display menu
void showContextMenu(int x, int y, Unit* unit) {
    menuPtr->showAt(x, y);
    selectedUnit = unit;
}
```

### TabControl (Multi-page Panels)

Tab controls for organizing complex interfaces.

```cpp
// Create tabbed panel for unit management
auto unitManagement = std::make_unique<TabControl>(guiManager, 100, 100, 600, 400);

// Add tabs
Panel* infantryTab = unitManagement->addTab("Infantry");
Panel* vehiclesTab = unitManagement->addTab("Vehicles");
Panel* buildingsTab = unitManagement->addTab("Buildings");

// Infantry tab content
auto soldierList = std::make_unique<Panel>(guiManager, 10, 30, 580, 360);
auto createSoldierBtn = std::make_unique<Button>(guiManager, 10, 10, 150, 30, "Create Soldier");
createSoldierBtn->setOnClickCallback([](GUIElement*) {
    createSoldier();
});
soldierList->addChild(std::move(createSoldierBtn));
infantryTab->addChild(std::move(soldierList));
```

### Button (Action Commands)

Action command buttons - the foundation of RTS interaction.

```cpp
// Unit production button
auto trainButton = std::make_unique<Button>(guiManager, 50, 50, 120, 40, "Soldier");
trainButton->setOnClickCallback([this](GUIElement*) {
    if (canTrainUnit(UnitType::SOLDIER)) {
        trainUnit(UnitType::SOLDIER);
    }
});

// Button styles in different states
trainButton->setBackgroundColor(ElementState::Normal, SDL_Color{60, 120, 60, 255});
trainButton->setBackgroundColor(ElementState::Hover, SDL_Color{80, 150, 80, 255});
trainButton->setBackgroundColor(ElementState::Pressed, SDL_Color{40, 100, 40, 255});
trainButton->setTextColor(ElementState::Normal, SDL_Color{255, 255, 255, 255});

// Icon on button
auto soldierIcon = std::make_unique<Label>(guiManager, 5, 10, "⚔️", 24);
trainButton->addChild(std::move(soldierIcon));
```

### TextInput (Player/Unit Names)

Text fields for entering player and unit names.

```cpp
// Player name field
auto playerNameInput = std::make_unique<TextInput>(guiManager, 200, 50, 200, 30);
playerNameInput->setPlaceholder("Enter player name...");
playerNameInput->setMaxLength(20);
playerNameInput->setOnTextChanged([](const std::string& text) {
    validatePlayerName(text);
});

// Unit name field
auto unitNameInput = std::make_unique<TextInput>(guiManager, 200, 100, 200, 30);
unitNameInput->setPlaceholder("Name the unit...");
unitNameInput->setMaxLength(15);
unitNameInput->setOnTextEntered([](const std::string& text) {
    renameSelectedUnit(text);
});
```

## Performance Optimization

### Cache System

The library uses a two-level caching system:

**1. Resource Cache (TextureManager and FontManager)**
```cpp
// TextureManager - caches textures
auto& textureManager = guiManager.getTextureManager();

// Load texture only once, then use from cache
auto unitTexture = textureManager.loadTexture("assets/units/soldier.png");
auto buildingTexture = textureManager.loadTexture("assets/buildings/barracks.png");

// FontManager - caches fonts
auto& fontManager = guiManager.getFontManager();
auto uiFont = fontManager.loadFont("assets/fonts/arial.ttf", 14);
auto titleFont = fontManager.loadFont("assets/fonts/arial.ttf", 24);
```

**2. Rendering Cache (m_cachedTexture)**
```cpp
// Each GUI element has its own rendering cache
// Cache is automatically refreshed when element changes
element->setPosition(newX, newY);  // Automatically invalidates cache
element->setSize(newWidth, newHeight);  // Recreates cache
element->markDirty();  // Manual cache invalidation
```

### Direct Rendering

For elements requiring frequent updates, use direct rendering:

```cpp
class Minimap : public GUIElement {
public:
    bool wantsDirectRender() const override { 
        return true;  // Render directly, don't cache
    }
    
    void drawDirect(SDL_Renderer* renderer) override {
        // Draw minimap directly to screen
        // Update every frame
        drawMinimapTiles(renderer);
        drawUnitMarkers(renderer);
        drawCameraView(renderer);
    }
};
```

### Best Practices

**1. Texture optimization**
- Use texture atlases for similar elements
- Maintain consistent sizes (powers of 2)
- Compress textures when possible

**2. Memory management**
- Use `std::unique_ptr` for element hierarchy
- Use `std::shared_ptr` for shared resources
- Mark elements for deletion with `markForDeletion()`

**3. Rendering**
- Cache static HUD elements
- Use direct rendering for dynamic elements
- Minimize renderer state changes

**4. Code organization**
```cpp
// Example of optimal structure for RTS game
class RTSGame {
private:
    GUIManager guiManager;
    
    // Game subsystems
    std::unique_ptr<UnitManager> unitManager;
    std::unique_ptr<BuildingManager> buildingManager;
    std::unique_ptr<ResourceManager> resourceManager;
    
    // GUI elements
    std::vector<std::unique_ptr<Panel>> hudPanels;
    std::unique_ptr<Cursor> gameCursor;
    std::unique_ptr<Minimap> minimap;
    
public:
    void update(float deltaTime);
    void render();
};
```

## Implementation Examples

### HUD Example

Complete HUD example for an RTS game:

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
        // Main HUD panel (bottom of screen)
        mainHudPanel = std::make_unique<Panel>(guiManager, 0, 540, 1024, 80);
        mainHudPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{40, 40, 50, 240});
        mainHudPanel->setBorder(ElementState::Normal, 
            SDL_Color{80, 80, 100, 255}, 2);
        
        // Resources panel (bottom left)
        createResourcesPanel();
        
        // Commands panel (bottom right)
        createCommandsPanel();
        
        // Minimap (top right corner)
        createMinimapPanel();
        
        guiManager.addElement(std::move(mainHudPanel));
    }
    
    void createResourcesPanel() {
        resourcesPanel = std::make_unique<Panel>(guiManager, 10, 550, 200, 60);
        resourcesPanel->setBackgroundColor(ElementState::Normal, 
            SDL_Color{60, 60, 40, 255});
        
        // Gold
        auto goldIcon = std::make_unique<Label>(guiManager, 10, 10, "💰", 20);
        auto goldText = std::make_unique<Label>(guiManager, 35, 10, "1000", 16);
        resourcesPanel->addChild(std::move(goldIcon));
        resourcesPanel->addChild(std::move(goldText));
        
        // Wood
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
        
        // Command buttons
        auto trainSoldierBtn = std::make_unique<Button>(guiManager, 10, 10, 80, 40, "Soldier");
        trainSoldierBtn->setOnClickCallback([](GUIElement*) {
            // Soldier training logic
            trainUnit(UnitType::SOLDIER);
        });
        
        auto trainArcherBtn = std::make_unique<Button>(guiManager, 100, 10, 80, 40, "Archer");
        trainArcherBtn->setOnClickCallback([](GUIElement*) {
            trainUnit(UnitType::ARCHER);
        });
        
        auto buildBarracksBtn = std::make_unique<Button>(guiManager, 190, 10, 80, 40, "Barracks");
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

Contextual menus for units:

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
        // Adapt menu to unit type
        setupMenuForUnitType(unit->getType());
        contextMenu->showAt(x, y);
    }
    
private:
    void createContextMenu() {
        contextMenu = std::make_unique<ContextMenu>(guiManager);
        
        // Move
        contextMenu->addItem("Move", [this]() {
            enterMoveMode();
        });
        
        // Attack
        contextMenu->addItem("Attack", [this]() {
            enterAttackMode();
        });
        
        contextMenu->addSeparator();
        
        // Special options (will be adapted dynamically)
        contextMenu->addItem("Properties", [this]() {
            showUnitProperties();
        });
    }
    
    void setupMenuForUnitType(UnitType type) {
        // Clear previous dynamic options
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
        contextMenu->addItem("Gather Resources", [this]() {
            assignResourceGathering();
        });
        
        contextMenu->addItem("Build", [this]() {
            enterBuildMode();
        });
        
        contextMenu->addItem("Repair", [this]() {
            enterRepairMode();
        });
    }
};
```

### Custom Cursor Example

Advanced cursor system:

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
        
        // Default cursor
        cursor->setCursorTexture(CursorState::Normal, "assets/cursors/arrow.png", 5, 5);
        
        // Cursor when hovering over unit
        cursor->setCursorTexture(CursorState::Hover, "assets/cursors/select.png", 5, 5);
        
        // Cursor when hovering over building
        cursor->setCursorTexture(CursorState::Pressed, "assets/cursors/building.png", 5, 5);
        
        // Animated cursor during action
        cursor->setAnimatedCursor(CursorState::Busy, 
            "assets/cursors/busy.png", 4, 1, 12.0f, 16, 16);
        
        // Text cursor
        cursor->setCursorTexture(CursorState::Text, "assets/cursors/text.png", 2, 10);
        
        guiManager.addElement(std::move(cursor));
    }
    
    CursorState determineCursorState(int x, int y) {
        // Check element under cursor
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
        
        // Check game state
        if (isGameActionInProgress()) {
            return CursorState::Busy;
        }
        
        return CursorState::Normal;
    }
};
```

### Animation Example

Animation system for units:

```cpp
class UnitAnimationSystem {
private:
    GUIManager& guiManager;
    std::map<Unit*, std::unique_ptr<AnimatedImage>> unitAnimations;
    
public:
    void addUnit(Unit* unit) {
        auto animation = std::make_unique<AnimatedImage>(guiManager, 
            unit->getX(), unit->getY(), 64, 64);
        
        // Default idle animation
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
            // After attack, return to idle animation
            animation->setSpriteSheet("assets/units/idle.png", 4, 1);
            animation->setFPS(2.0f);
            animation->play();
        });
        animation->play();
    }
};
```

## Best Practices

### 1. Code Organization

**Separation of logic and view**
```cpp
// Good - separated game logic from GUI
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

### 2. Resource Management

**Lazy loading and caching**
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

### 3. Performance Optimization

**Minimizing renderer state changes**
```cpp
// Good - group operations on the same texture
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

**Optimal event propagation**
```cpp
void GUIManager::processEvent(const SDL_Event& event) {
    // Stop propagation after handling event
    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        if ((*it)->processEvent(event)) {
            break;  // Event has been handled
        }
    }
}
```

### 5. Debugging and Testing

**State logging**
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

## Summary

The SDL GUI library provides a complete set of tools for creating professional interfaces in RTS games. Thanks to the animation system, advanced cursor handling, performance optimization, and rich implementation examples, developers can quickly create responsive and attractive user interfaces.

Key advantages of the library:
- **Performance** - two-level caching system
- **Flexibility** - modular design with extensibility options
- **Ease of use** - intuitive API with examples
- **Optimization** - automatic memory management
- **Completeness** - all necessary widgets for RTS

The documentation contains practical examples, best practices, and ready implementations that can be directly used in RTS game projects.