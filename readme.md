# SDL GUI Library

A lightweight, header-based GUI library built on SDL2 with C++23, designed for simplicity, safe memory management, and ease of use. Ideal for tools, prototypes, and lightweight desktop applications.

English is the authoritative language for this documentation.

## Overview

SDL GUI provides a simple and extensible GUI framework centered around:
- **GUIManager** - Central controller for rendering and event handling
- **GUIElement** - Base class for all widgets with texture caching
- **Resource Managers** - Automatic caching for textures and fonts
- **Composite Components** - Ready-to-use dialogs (DialogBox, MessageBox)
- **Screen Management** - ScreenManager for games, WindowManager for multi-window apps
- **Layout Parsers** - Define GUI from JSON or XML files

## Quick Start

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "label.hpp"

int main() {
    SDLApp app("My App", 800, 600);
    GUIManager guiManager(app.getRenderer());

    auto button = std::make_unique<Button>(guiManager, 100, 100, 150, 40, "Click Me");
    button->setOnClickCallback([](GUIElement*) {
        std::cout << "Button clicked!" << std::endl;
    });
    guiManager.addElement(std::move(button));

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            guiManager.processEvent(e);
        }
        guiManager.update();
        guiManager.cleanup();

        SDL_SetRenderDrawColor(app.getRenderer(), 50, 50, 50, 255);
        SDL_RenderClear(app.getRenderer());
        guiManager.render();
        SDL_RenderPresent(app.getRenderer());
    }
    return 0;
}
```

## Installation/Dependencies

### Required Libraries
- **SDL2** - Window, events, rendering
- **SDL2_image** - Image loading (PNG, etc.)
- **SDL2_ttf** - TrueType font rendering
- **SDL2_gfx** - Geometric primitives (rounded corners)

### Compiler Requirements
- C++23 compatible compiler (clang++ with libc++ recommended)
- CMake or nob.c build system

### Installing Dependencies (Debian/Ubuntu)
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev clang libc++-dev
```

## Building the Library

The project uses `nob.c` build system (Go Rebuild Urself technology).

```bash
# Bootstrap the build tool
cc -o nob nob.c

# Build examples (debug mode)
./nob examples

# Build and run tests
./nob test

# Build release artifacts (static/dynamic libraries in dist/)
./nob release

# Clean build artifacts
./nob clean

# Build in release mode with optimizations
./nob -r examples
```

### Build Outputs
- `output/` - Compiled examples (debug)
- `dist/` - Release artifacts (libsdl_gui.a, libsdl_gui.so, amalgamated headers)
- `compile_commands.json` - Generated for LSP tools (clangd)

## Using the Library in Your Project

### Static Linking
```bash
clang++ -std=c++23 your_app.cpp \
    -I/path/to/sdl_gui/dist \
    -L/path/to/sdl_gui/dist \
    -lsdl_gui \
    -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx \
    -o your_app
```

### Dynamic Linking
```bash
clang++ -std=c++23 your_app.cpp \
    -I/path/to/sdl_gui/dist \
    -L/path/to/sdl_gui/dist \
    -lsdl_gui \
    -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_gfx \
    -o your_app

# Ensure library path is set at runtime
export LD_LIBRARY_PATH=/path/to/sdl_gui/dist:$LD_LIBRARY_PATH
```

### Header-Only Usage
For simple projects, include the amalgamated header from `dist/sdl_gui.hpp`:
```cpp
#include "sdl_gui.hpp"  // All widgets and managers in one header
```

## Core Concepts

### GUIManager
Central controller that manages:
- Top-level GUI elements (`addElement`, `cleanup`)
- Event processing (`processEvent`)
- Rendering (`render`, `update`)
- Resource managers (FontManager, TextureManager, TimerManager, AnimationManager)
- Global theme (`setTheme`, `getTheme`)
- Window resize handling (`handleResize`, `setWindowSize`)

```cpp
GUIManager guiManager(renderer);
guiManager.setWindowSize(800, 600);  // Call once at initialization

// Add top-level elements
GUIElement* elem = guiManager.addElement(std::make_unique<Panel>(...));

// Access managers
guiManager.getFontManager().loadFont("path/to/font.ttf", 24);
guiManager.getTextureManager().loadTexture("path/to/image.png");
```

### GUIElement
Abstract base class for all widgets:
- **Position/Size**: `setPosition()`, `setSize()`, `getX()`, `getY()`, `getWidth()`, `getHeight()`
- **Visibility/State**: `setVisible()`, `setEnabled()`, `setState()`, `isHovered()`
- **Hierarchy**: `addChild()`, `getParent()`, `getChildren()`, `clearChildren()`
- **Styling**: `setStyle()`, `setBackgroundColor()`, `setTextColor()`, `setBorder()`
- **Anchor System**: `setAnchor()` for responsive layouts
- **Lifecycle**: `markForDeletion()`, `markDirty()`

```cpp
auto panel = std::make_unique<Panel>(manager, 50, 50, 300, 200);
panel->setBackgroundColor(ElementState::Normal, {200, 200, 200, 255});
panel->setBorder(ElementState::Normal, {100, 100, 100, 255}, 2);
panel->setDraggable(true);

// Add child elements
panel->addChild(std::make_unique<Button>(manager, 10, 10, 100, 30, "Child"));
```

### Element States
Widgets support four visual states:
- `ElementState::Normal` - Default appearance
- `ElementState::Hover` - Mouse over element
- `ElementState::Pressed` - Mouse button down
- `ElementState::Disabled` - Element not interactive

### Render Flow
1. `GUIManager::render()` iterates top-level elements
2. Each element checks visibility and clipping
3. **Cached path**: If dirty, renders to texture cache, then copies to renderer
4. **Direct path**: Some elements (StringGrid, Canvas) render directly for performance
5. Children rendered recursively with parent clipping

## Available Widgets

### Panel - Container Widget
```cpp
auto panel = std::make_unique<Panel>(manager, 50, 50, 300, 200);
panel->setBackgroundColor(ElementState::Normal, {240, 240, 240, 255});
panel->setBorder(ElementState::Normal, {100, 100, 100, 255}, 2);
panel->setDraggable(true);  // Allow user to drag

// Add children
panel->addChild(std::make_unique<Label>(manager, 10, 10, "Title", 24));
panel->addChild(std::make_unique<Button>(manager, 10, 50, 100, 30, "OK"));
```

### Button - Clickable Button
```cpp
auto button = std::make_unique<Button>(manager, 100, 100, 150, 40, "Click Me");
button->setOnClickCallback([](GUIElement* elem) {
    std::cout << "Clicked!" << std::endl;
});
button->setOnMouseOverCallback([](GUIElement* elem) {
    std::cout << "Mouse over" << std::endl;
});

// Custom styling
button->setBackgroundColor(ElementState::Normal, {0, 120, 0, 255});
button->setBackgroundColor(ElementState::Hover, {0, 180, 0, 255});
button->setTextColor(ElementState::Normal, {255, 255, 255, 255});
```

### Label - Static Text
```cpp
auto label = std::make_unique<Label>(manager, 100, 50, "Hello, World!", 24);
label->setText("Updated text");  // Change text dynamically
```

### Checkbox - Toggle Control
```cpp
auto checkbox = std::make_unique<Checkbox>(manager, 100, 100, 20, 20);
checkbox->setChecked(true);
checkbox->setOnChange([](Checkbox* cb, bool isChecked) {
    std::cout << "Checkbox: " << (isChecked ? "ON" : "OFF") << std::endl;
});

bool state = checkbox->isChecked();  // Query state
```

### RadioButton & RadioGroup - Mutually Exclusive Options
```cpp
auto radioGroup = std::make_unique<RadioGroup>(manager, 100, 100, 200, 150);
radioGroup->addOption("Option A", true);   // true = initially selected
radioGroup->addOption("Option B");
radioGroup->addOption("Option C");

RadioButton* selected = radioGroup->getSelectedButton();
```

### Slider - Value Range Control
```cpp
auto slider = std::make_unique<Slider>(
    manager, 100, 200, 200, 30,
    0, 100, 50,              // min, max, initial
    Orientation::Horizontal  // or Vertical
);
slider->setOnChangeCallback([](GUIElement* elem) {
    Slider* s = static_cast<Slider*>(elem);
    std::cout << "Value: " << s->getValue() << std::endl;
});
slider->setWheelStep(5);  // Mouse wheel increment

int value = slider->getValue();
slider->setValue(75);
```

### TextInput - Single-Line Text Entry
```cpp
auto input = std::make_unique<TextInput>(manager, 100, 100, 200, 30);
input->setText("Initial value");
input->setOnTextChanged([](TextInput* ti) {
    std::cout << "Text: " << ti->getText() << std::endl;
});
input->setOnEnterPressed([](TextInput* ti) {
    std::cout << "Entered: " << ti->getText() << std::endl;
});
input->setLocked(true);  // Disable editing
```

### TextArea - Multi-Line Text Entry
```cpp
auto area = std::make_unique<TextArea>(
    manager, 100, 100, 300, 200,
    "assets/font.ttf", 16
);
area->setText("Line 1\nLine 2\nLine 3");
area->setWordWrap(true);
area->setOnTextChanged([](TextArea* ta) {
    std::cout << "Changed" << std::endl;
});
```

### ComboBox - Dropdown Selection
```cpp
auto combo = std::make_unique<ComboBox>(manager, 100, 100, 150, 30);
combo->addItem("Option A");
combo->addItem("Option B");
combo->addItem("Option C");
combo->on_selection_changed = [](int index, const std::string& item) {
    std::cout << "Selected: " << item << " at index " << index << std::endl;
};

combo->setSelectedIndex(1);  // Select "Option B"
std::string selected = combo->getSelectedItem();
int idx = combo->getSelectedIndex();
```

### TabControl - Tabbed Container
```cpp
auto tabs = std::make_unique<TabControl>(manager, 50, 50, 400, 300, 30);  // tabHeight=30

Panel* tab1 = tabs->addTab("General");
tab1->addChild(std::make_unique<Label>(manager, 10, 10, "Settings", 20));

Panel* tab2 = tabs->addTab("Advanced");
tab2->addChild(std::make_unique<Button>(manager, 10, 10, 100, 30, "Apply"));

// Tabs automatically switch when clicked
```

### StringGrid - Data Table
```cpp
auto grid = std::make_unique<StringGrid>(manager, 50, 50, 400, 300, 5, 3);  // 5 rows, 3 cols

// Set headers
grid->setColumnHeader(0, "Name");
grid->setColumnHeader(1, "Value");
grid->setColumnHeader(2, "Status");

// Set cell data
grid->setCellText(0, 0, "Item 1");
grid->setCellText(0, 1, "100");
grid->setCellText(0, 2, "OK");

// Configure
grid->setEditable(true);
grid->setColumnWidth(0, 150);
grid->setRowHeight(24);
grid->setHorizontalScrollEnabled(true);
grid->setVerticalScrollEnabled(true);

// Sorting
grid->sortByColumn(1, SortDirection::Ascending);

// Callbacks
grid->setOnCellClick([](StringGrid* g, CellCoord cell) {
    std::cout << "Clicked: row=" << cell.row << " col=" << cell.col << std::endl;
});
grid->setOnCellEdit([](StringGrid* g, CellCoord cell, std::string newText) {
    std::cout << "Edited: " << newText << std::endl;
});
grid->setOnSelectionChange([](StringGrid* g, SelectionRange range) {
    std::cout << "Selection changed" << std::endl;
});
```

### ListView - Simple List
```cpp
auto list = std::make_unique<ListView>(manager, 50, 50, 200, 300);
list->addItem("First item");
list->addItem("Second item");
list->addItem("Third item");

list->setOnRowClick([](ListView* lv, size_t row) {
    std::cout << "Clicked row " << row << ": " << lv->getItem(row) << std::endl;
});
list->setOnRowDoubleClick([](ListView* lv, size_t row) {
    std::cout << "Double-clicked row " << row << std::endl;
});

auto selected = list->getSelectedRow();
list->setSelectedRow(1);
list->removeItem(0);
```

### Canvas - Drawing Surface
```cpp
auto canvas = std::make_unique<Canvas>(manager, 50, 50, 400, 300);
canvas->clear();  // Clear all drawings
// User can draw with mouse (brush size = 4)
```

### AnimatedImage - Sprite Animation
```cpp
auto anim = std::make_unique<AnimatedImage>(manager, 100, 100, 200, 200);
anim->setSpriteSheet("assets/sprite.png", 8, 1);  // 8 frames, 1 row
anim->setFPS(12);
anim->setLoop(true);
anim->play();

// Scale modes
anim->setScaleMode(AnimatedImage::ScaleMode::Fit);     // Fit to widget size
anim->setScaleMode(AnimatedImage::ScaleMode::Center);  // Center without scaling
anim->setScaleMode(AnimatedImage::ScaleMode::None);    // Top-left, no scaling

anim->setOnAnimationEnd([]() { std::cout << "Animation ended" << std::endl; });
anim->setOnFrameChanged([](int frame) { std::cout << "Frame: " << frame << std::endl; });
```

### ContextMenu - Right-Click Menu
```cpp
auto menu = std::make_unique<ContextMenu>(manager);
menu->addItem("Copy", []() { std::cout << "Copy" << std::endl; });
menu->addItem("Paste", []() { std::cout << "Paste" << std::endl; });
menu->addSeparator();
menu->addItem("Delete", []() { std::cout << "Delete" << std::endl; }, false);  // disabled

// Show at position (typically on right-click)
menu->showAt(mouseX, mouseY);
menu->hide();
```

## Composite Components

### DialogBox - Modal Dialog
```cpp
// Confirm dialog (Yes/No)
DialogBox::createConfirm(manager, "Are you sure?", "Yes", "No",
    [](bool confirmed) {
        if (confirmed) { std::cout << "User confirmed" << std::endl; }
    });

// Alert dialog (OK)
DialogBox::createAlert(manager, "Operation completed.", "OK",
    [](int) { std::cout << "Closed" << std::endl; });

// Dialog with title
DialogBox::createWithTitle(manager, "Save?", "Save changes?",
    {"Save", "Don't Save", "Cancel"},
    [](int idx) {
        switch (idx) {
            case 0: /* Save */ break;
            case 1: /* Don't Save */ break;
            case 2: /* Cancel */ break;
        }
    });

// Custom dialog
DialogBox::createCustom(manager, "Choose an option",
    {"Option A", "Option B", "Option C"},
    [](int idx) { std::cout << "Chose option " << idx << std::endl; });
```

### MessageBox - Quick Alerts
```cpp
MessageBox::showInfo(manager, "File saved successfully.");
MessageBox::showError(manager, "Error: Cannot open file.");
MessageBox::showWarning(manager, "Low memory warning.");

MessageBox::showQuestion(manager, "Continue installation?",
    []() { std::cout << "Yes" << std::endl; },  // onYes
    []() { std::cout << "No" << std::endl; }    // onNo
);

MessageBox::showCustom(manager, "Custom Title", "Custom message",
    "Understood", MessageBox::IconType::Info,
    []() { std::cout << "Closed" << std::endl; });
```

## Screen Management

### ScreenManager - Single Window, Multiple Screens (Games)
```cpp
class MenuScreen : public Screen {
public:
    void onEnter(GUIManager& manager) override {
        // Add menu elements
        manager.addElement(std::make_unique<Button>(...));
    }
    void onExit(GUIManager& manager) override {
        // Cleanup (elements marked for deletion)
    }
    bool handleEvent(GUIManager& manager, const SDL_Event& e) override {
        return manager.processEvent(e);  // or custom handling
    }
    void update(GUIManager& manager) override { manager.update(); }
    void render(GUIManager& manager, SDL_Renderer* r) override { manager.render(); }
    std::string getName() const override { return "MenuScreen"; }
};

// Usage
ScreenManager screenManager(guiManager);
screenManager.addScreen("menu", std::make_unique<MenuScreen>());
screenManager.addScreen("game", std::make_unique<GameScreen>());
screenManager.changeScreen("menu");

// Overlay (pause menu on top of game)
screenManager.pushScreen("pause");
screenManager.popScreen();
```

### WindowManager - Multiple System Windows
```cpp
WindowManager windowManager;  // Initializes SDL

Window* main = windowManager.createWindow("Main", 800, 600);
main->getGUIManager().addElement(std::make_unique<Panel>(...));

Window* settings = windowManager.createWindow("Settings", 400, 300, true);  // resizable
settings->getGUIManager().addElement(std::make_unique<TextInput>(...));

settings->setOnCloseCallback([](Window* w) {
    std::cout << "Settings window closed" << std::endl;
    w->markForClose();
});

// Main loop
while (!windowManager.shouldQuit()) {
    windowManager.processEvents();
    windowManager.updateAll();
    windowManager.renderAll();
    windowManager.cleanupAll();
}
```

## Layout Parsers

### JSON Layout
Define GUI structure in JSON files:

```json
{
  "type": "Panel",
  "x": 50, "y": 50, "width": 400, "height": 300,
  "background": "#F0F0F0",
  "border": "#808080",
  "borderWidth": 2,
  "children": [
    {
      "type": "Label",
      "x": 20, "y": 20,
      "text": "Welcome",
      "fontSize": 24
    },
    {
      "type": "Button",
      "x": 20, "y": 60,
      "width": 100, "height": 30,
      "text": "OK"
    }
  ]
}
```

```cpp
JsonParser parser(guiManager);
auto root = parser.loadLayout("layout.json");
if (root) guiManager.addElement(std::move(root));
```

### XML/SGML Layout
```xml
<Panel x="50" y="50" width="400" height="300" background="#F0F0F0">
  <Label x="20" y="20" text="Welcome" fontSize="24"/>
  <Button x="20" y="60" width="100" height="30" text="OK"/>
</Panel>
```

```cpp
SGMLParser parser(guiManager);
auto root = parser.loadLayout("layout.xml");
if (root) guiManager.addElement(std::move(root));
```

## Styling and Themes

### Style Struct
```cpp
Style style;
style.backgroundColor = {240, 240, 240, 255};  // RGBA
style.textColor = {0, 0, 0, 255};
style.borderColor = {100, 100, 100, 255};
style.borderWidth = 2;
style.borderRadius = 8;  // Rounded corners
style.fontSize = 16;
style.fontName = "path/to/font.ttf";
style.texture = textureManager.loadTexture("bg.png");

element->setStyle(ElementState::Normal, style);
```

### Theme System
```cpp
Theme theme = Theme::createDefaultTheme();  // Windows 95/98 style
theme.setStyle("Button", buttonStyle);
theme.setStyle("Panel", panelStyle);
guiManager.setTheme(theme);

// Elements inherit theme styles automatically
```

### Color Formats (Layout Parsers)
- RGBA tuple: `[240, 240, 240, 255]`
- Hex: `"#F0F0F0"` or `"#F0F0F0FF"`

## Examples Index

| Example | Description |
|---------|-------------|
| `example_animation.cpp` | AnimationManager usage with easing functions |
| `example_animated_image.cpp` | AnimatedImage sprite animation |
| `example_button.cpp` | Button styling and callbacks |
| `example_canvas.cpp` | Canvas drawing surface |
| `example_checkbox.cpp` | Checkbox toggle control |
| `example_combobox.cpp` | ComboBox dropdown selection |
| `example_context_menu.cpp` | ContextMenu right-click menus |
| `example_dialog.cpp` | DialogBox and MessageBox composite components |
| `example_json_parser.cpp` | JSON layout file parsing |
| `example_list_view.cpp` | ListView simple list widget |
| `example_mouse_cursor.cpp` | Cursor management |
| `example_paint.cpp` | Full painting application with Canvas |
| `example_panel.cpp` | Panel container with draggable feature |
| `example_performance.cpp` | Performance benchmarking |
| `example_radio_button.cpp` | RadioButton and RadioGroup |
| `example_resize.cpp` | Window resize handling with anchors |
| `example_rounded_corners.cpp` | Rounded corners using SDL2_gfx |
| `example_screen_manager.cpp` | ScreenManager for game screens |
| `example_slider.cpp` | Slider value control |
| `example_sprite_animator.cpp` | Sprite animation system |
| `example_string_grid.cpp` | StringGrid data table widget |
| `example_tabs.cpp` | TabControl tabbed interface |
| `example_text_area.cpp` | TextArea multi-line input |
| `example_text_input.cpp` | TextInput single-line input |
| `example_texture_font_previewer.cpp` | Texture and font preview tool |
| `example_theme_playground.cpp` | Theme customization playground |
| `example_themes.cpp` | Theme system usage |
| `example_tooltip.cpp` | Tooltip on GUI elements |
| `example_window.cpp` | Window class usage |
| `example_window_manager.cpp` | WindowManager multi-window app |
| `example_xml_parser.cpp` | XML/SGML layout parsing |

Run examples after building:
```bash
./output/example_button
./output/example_dialog
./output/example_json_parser assets/test_layout.json
```

## API Documentation

Detailed API documentation is available in `docs/api/`:
- Widget class references
- Manager API details
- Style/Theme configuration
- Layout parser schema

## Project Structure

```
sdl_gui/
├── src/                    # Source implementation
│   ├── *.hpp               # Widget headers
│   ├── *.cpp               # Widget implementations
│   ├── composite/          # DialogBox, MessageBox
│   ├── gui.hpp             # GUIElement base class
│   ├── gui_manager.hpp     # GUIManager controller
│   └── sdl_app.hpp         # SDL initialization helper
├── examples/               # 31 usage examples
├── tests/                  # Unit tests (Catch2)
├── lib/                    # Third-party libraries
├── docs/                   # Documentation
├── nob.c                   # Build system
├── nob.h                   # Build library (v3.8.0)
└── readme.md               # This file
```

## Memory Management

- **GUI Elements**: `std::unique_ptr` with parent-child hierarchy
- **SDL Resources**: `SharedTexture`, `SharedFont` - `std::shared_ptr` with custom deleters
- **Resource Cache**: TextureManager and FontManager cache loaded resources
- **Render Cache**: Each element has texture cache invalidated by `markDirty()`

## License

See LICENSE file for licensing information.