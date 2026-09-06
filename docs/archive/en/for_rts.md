# Using SDL GUI for RTS Games

This document provides an overview of how to use the SDL GUI library to create user interfaces for Real-Time Strategy (RTS) games. It focuses on the public API of the most relevant components.

## Key Components for RTS Interfaces

### AnimationManager
The `AnimationManager` is used for creating smooth visual effects, such as unit movements, UI transitions, or health bar changes.

```cpp
// Animate a property from a start value to an end value over a duration
animationManager.createAnimation(
    &property,
    startValue,
    endValue,
    duration_ms,
    Easing::Type::Linear,
    []() { /* on-completion callback */ }
);
```

### AnimatedImage
Ideal for unit animations. It displays animations from a sprite sheet.

```cpp
auto unitSprite = std::make_unique<AnimatedImage>(guiManager, x, y, w, h);
unitSprite->setSpriteSheet("path/to/sprites.png", totalFrames, rows);
unitSprite->setFPS(12.0f);
unitSprite->setLoop(true);
unitSprite->play();
```

### MouseCursor
Allows for custom, context-aware cursors, which are essential in RTS games.

```cpp
// Enable and configure a custom cursor
guiManager.setCustomCursorEnabled(true);
MouseCursor* cursor = guiManager.getMouseCursor();

cursor->setCursorTexture(CursorState::Normal, "assets/cursor_normal.png");
cursor->setCursorTexture(CursorState::Hover, "assets/cursor_hover.png");
cursor->setAnimatedCursor(CursorState::Busy, "assets/cursor_busy.png", 8, 1, 12.0f);
```

### Panel
The base container for building complex UI layouts, such as the main HUD.

```cpp
auto hudPanel = std::make_unique<Panel>(guiManager, 0, 500, 800, 100);
hudPanel->setBackgroundColor(ElementState::Normal, {30, 30, 30, 200});
```

### Button
Used for action commands, unit training, and other interactive elements.

```cpp
auto trainButton = std::make_unique<Button>(guiManager, 10, 10, 100, 30, "Train Unit");
trainButton->setOnClickCallback([](GUIElement*) {
    // Logic to train a unit
});
```

### ContextMenu
Provides pop-up menus for unit commands (e.g., move, attack, build).

```cpp
auto unitMenu = std::make_unique<ContextMenu>(guiManager);
unitMenu->addItem("Attack", []() { /* ... */ });
unitMenu->addItem("Move", []() { /* ... */ });
unitMenu->showAt(mouseX, mouseY);
```

### TabControl
Useful for organizing complex panels, such as build menus or technology trees.

```cpp
auto buildMenu = std::make_unique<TabControl>(guiManager, 100, 100, 400, 300);
Panel* buildingsTab = buildMenu->addTab("Buildings");
Panel* unitsTab = buildMenu->addTab("Units");
```

## Building a Basic RTS HUD

A typical RTS HUD can be constructed by combining `Panel`, `Button`, and `Label` elements.

1.  **Main HUD Panel**: Create a main `Panel` docked to the bottom of the screen to serve as the primary container.
2.  **Minimap**: Use a `Panel` or a custom-drawn widget for the minimap, typically placed in a corner.
3.  **Resource Display**: Use `Label` elements within a `Panel` to show resources like gold, wood, or supply.
4.  **Command Panel**: Group `Button` elements in a `Panel` to display available actions for a selected unit or building.

```cpp
// 1. Create the main HUD panel
auto mainHud = std::make_unique<Panel>(guiManager, 0, 500, 800, 100);

// 2. Create a panel for the command buttons
auto commandGrid = std::make_unique<Panel>(guiManager, 600, 10, 190, 80);

// 3. Add command buttons to the command grid
auto attackButton = std::make_unique<Button>(guiManager, 0, 0, 40, 40, "A");
commandGrid->addChild(std::move(attackButton));

// 4. Add the command grid to the main HUD
mainHud->addChild(std::move(commandGrid));

guiManager.addElement(std::move(mainHud));
```

This structure provides a clean and organized way to build a flexible and responsive RTS user interface.