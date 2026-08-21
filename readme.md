# SDL GUI Library

A lightweight, header-based GUI library built on SDL3 with C++23, designed for simplicity, safe memory management, and ease of use. Ideal for tools, prototypes, and lightweight desktop applications.

English is the authoritative language for this documentation.

## Overview

SDL GUI provides a simple and extensible GUI framework centered around:
- **GUIManager** - Central controller for rendering and event handling
- **GUIElement** - Base class for all widgets with texture caching
- **Resource Managers** - Automatic caching for textures and fonts
- **Composite Components** - Ready-to-use dialogs (DialogBox, MessageBox, FileDialog)
- **Screen Management** - ScreenManager for games, WindowManager for multi-window apps
- **Layout Parsers** - Define GUI from JSON or XML files
- **WYSIWYG Editor** - Visual GUI editor for creating layouts interactively

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
            if (e.type == SDL_EVENT_QUIT) quit = true;
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
- **SDL3** - Window, events, rendering (GPU)
- **SDL3_image** - Image loading (PNG, etc.) — on-demand, no IMG_Init required
- **SDL3_ttf** - TrueType font rendering

### Compiler Requirements
- C++23 compatible compiler (clang++-22 with libc++ recommended)
- CMake or nob.c build system

### Installing Dependencies (Debian/Ubuntu)
```bash
sudo apt-get install clang-22 libc++-dev libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev
```

SDL3 packages may need to be installed from source or third-party repositories. Ensure `pkg-config sdl3 sdl3-image sdl3-ttf` resolves correctly.

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
clang++-22 -std=c++23 your_app.cpp \
    -I/path/to/sdl_gui/dist \
    -L/path/to/sdl_gui/dist \
    -lsdl_gui \
    -lSDL3 -lSDL3_image -lSDL3_ttf \
    -o your_app
```

### Dynamic Linking
```bash
clang++-22 -std=c++23 your_app.cpp \
    -I/path/to/sdl_gui/dist \
    -L/path/to/sdl_gui/dist \
    -lsdl_gui \
    -lSDL3 -lSDL3_image -lSDL3_ttf \
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
area->setLocked(true);  // Disable editing
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

### ProgressBar - Progress Indicator
```cpp
auto bar = std::make_unique<ProgressBar>(manager, 100, 100, 200, 30, 0.0f, 1.0f, 0.5f);
bar->setValue(0.75f);
bar->setShowPercentage(true);

float progress = bar->getValue();
```

### ScrollArea - Scrollable Container
```cpp
auto scroll = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);

// Add content (larger than the visible area)
auto content = std::make_unique<Panel>(manager, 0, 0, 500, 400);
content->addChild(std::make_unique<Label>(manager, 10, 10, "Scrolled content", 20));
scroll->setContent(std::move(content));
```

### ArcContainer - Arced Layout Container
```cpp
auto arc = std::make_unique<ArcContainer>(manager, 200, 200, 150, 0.0f, 360.0f);

arc->addChild(std::make_unique<Button>(manager, 0, 0, 60, 30, "A"));
arc->addChild(std::make_unique<Button>(manager, 0, 0, 60, 30, "B"));
arc->addChild(std::make_unique<Button>(manager, 0, 0, 60, 30, "C"));
// Children are arranged along the arc automatically
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

### ShaderPanel - GPU Shader Widget
```cpp
auto shaderPanel = std::make_unique<ShaderPanel>(manager, 50, 50, 400, 300,
    // Vertex shader
    R"(#version 450
       layout(location=0) in vec2 pos;
       layout(location=1) in vec2 uv;
       layout(location=0) out vec2 v_uv;
       void main() {
           gl_Position = vec4(pos, 0.0, 1.0);
           v_uv = uv;
       })",
    // Fragment shader
    R"(#version 450
       layout(location=0) in vec2 v_uv;
       layout(location=0) out vec4 fragColor;
       void main() {
           fragColor = vec4(v_uv, 0.5, 1.0);
       })"
);
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

### FileDialog - File Picker
```cpp
FileDialog::createOpen(manager, "Open File",
    [](const std::string& path) {
        std::cout << "Selected: " << path << std::endl;
    });

FileDialog::createSave(manager, "Save File", "default.txt",
    [](const std::string& path) {
        std::cout << "Save to: " << path << std::endl;
    });
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

## WYSIWYG Editor

The library includes a visual GUI editor for building layouts interactively:

- **EditorWindow** - Main editor UI with widget palette and properties panel
- **EditorState** - State management for undo/redo, selection, and clipboard
- **PreviewWindow** - Live preview of the edited layout
- **LayoutImporter** - Load existing JSON/XML layouts for editing
- **LayoutExporter** - Save edited layouts to JSON or XML

```cpp
#include "editor/editor_window.hpp"

EditorWindow editor(guiManager);
// GUI editor with drag-and-drop widget placement, property editing,
// and live preview — all built with the library's own widgets
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

Examples are numbered by complexity — simplest first, full applications last.
Follow this order when learning the library.

### Getting Started (00–02)
| # | Example | Description |
|---|---------|-------------|
| 00 | `00_hello_world.cpp` | Minimal window with a label |
| 01 | `01_run_basic.cpp` | Shortest possible app via `SDLApp::run()` |
| 02 | `02_run_callback.cpp` | `SDLApp::run()` with custom event handling |

### Basic Widgets (03–10)
| 03 | `03_panel.cpp` | Panel container with border styling |
| 04 | `04_text_input.cpp` | TextInput single-line input |
| 05 | `05_slider.cpp` | Slider value control with callback |
| 06 | `06_checkbox.cpp` | Checkbox toggle control |
| 07 | `07_radio_button.cpp` | RadioButton and RadioGroup |
| 08 | `08_combobox.cpp` | ComboBox dropdown selection |
| 09 | `09_text_area.cpp` | TextArea multi-line input, font loading, clipboard |
| 10 | `10_range_slider.cpp` | RangeSlider dual-handle range control |

### Interaction (11–14)
| 11 | `11_tooltip.cpp` | Tooltips on multiple widget types |
| 12 | `12_context_menu.cpp` | ContextMenu right-click menus |
| 13 | `13_tabs.cpp` | TabControl tabbed interface |
| 14 | `14_paint.cpp` | Canvas freehand drawing |

### Data & Containers (15–21)
| 15 | `15_progress_bar.cpp` | ProgressBar + animation-driven loading |
| 16 | `16_list_view.cpp` | ListView data list with selection |
| 17 | `17_string_grid.cpp` | StringGrid data table with inline editing |
| 18 | `18_scroll_area.cpp` | ScrollArea scrollable container |
| 19 | `19_arc_container.cpp` | ArcContainer radial layout |
| 20 | `20_widgets_combo.cpp` | Multiple widgets working together |
| 21 | `21_window.cpp` | Composite draggable window widget |

### Animation (22–24)
| 22 | `22_animation.cpp` | AnimationManager with easing functions |
| 23 | `23_animated_image.cpp` | AnimatedImage sprite sheet setup |
| 24 | `24_hover_animation.cpp` | Custom Panel subclasses with hover animations |

### Styling & Themes (25–28)
| 25 | `25_rounded_corners.cpp` | BorderRadius on multiple widget types |
| 26 | `26_button_styling.cpp` | Button styling deep dive (5 variants) |
| 27 | `27_themes.cpp` | Theme system (light/dark) |
| 28 | `28_theme_playground.cpp` | Real-time theme editor with RGB sliders |

### Layout & Assets (29–32)
| 29 | `29_resize.cpp` | Anchor system: responsive layout with SDL resize |
| 30 | `30_json_parser.cpp` | Loading GUI layouts from JSON files |
| 31 | `31_xml_parser.cpp` | Loading GUI layouts from XML/SGML files |
| 32 | `32_embedded_assets.cpp` | Embedded binary assets via ld |

### Resources & Tools (33–34)
| 33 | `33_mouse_cursor.cpp` | Custom cursor system with state management |
| 34 | `34_texture_font_previewer.cpp` | Texture and font preview tool |

### Dialogs (35–36)
| 35 | `35_dialog.cpp` | DialogBox and MessageBox (8 variants) |
| 36 | `36_file_dialog.cpp` | FileDialog open/save with path filtering |

### Window Systems (37–38)
| 37 | `37_screen_manager.cpp` | ScreenManager: game screen lifecycle |
| 38 | `38_window_manager.cpp` | WindowManager multi-window SDL application |

### Performance & GPU (39–41)
| 39 | `39_performance.cpp` | Performance stress test with FPS counter |
| 40 | `40_gpu_shader.cpp` | ShaderPanel GPU fragment shader on GUI |
| 41 | `41_gpu_shader_animation.cpp` | Animated GPU shaders reacting to time and mouse |

### Full Applications & Special (42–47)
| 42 | `42_mobile_touch.cpp` | Touch-optimized mobile UI patterns |
| 43 | `43_gamepad_controller.cpp` | Gamepad navigation UI patterns |
| 44 | `44_tv_remote.cpp` | TV / 10-foot UI patterns |
| 45 | `45_wysiwyg_editor.cpp` | Full WYSIWYG GUI editor |
| 46 | `46_c_api_demo.cpp` | C API used side-by-side with C++ API |
| 47 | `47_standalone.cpp` | Standalone app using combined header + static library |

Run examples after building:
```bash
./output/00_hello_world
./output/35_dialog
./output/30_json_parser examples/comprehensive_layout.json
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
│   ├── composite/          # DialogBox, MessageBox, FileDialog
│   ├── editor/             # WYSIWYG editor (EditorWindow, EditorState, PreviewWindow, import/export)
│   ├── gui.hpp             # GUIElement base class
│   ├── gui_manager.hpp     # GUIManager controller
│   └── sdl_app.hpp         # SDL3 initialization helper
├── examples/               # 39 usage examples
├── tests/                  # Unit tests (Catch2)
├── lib/                    # Third-party libraries (tinyxml2, Catch2)
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

## Key SDL3 Differences

This library targets SDL3. Key API differences from SDL2:

| SDL2 | SDL3 |
|------|------|
| `SDL_QUIT` | `SDL_EVENT_QUIT` |
| `SDL_QueryTexture` | `SDL_GetTextureSize` (float\*) |
| `SDL_RenderFillRect` (SDL_Rect\*) | `SDL_RenderFillRect` (SDL_FRect\*) |
| `SDL_RenderCopy` | `SDL_RenderTexture` (SDL_FRect\*) |
| `e.key.keysym.sym` | `e.key.key` (flat struct) |
| `e.button.x/y` (int) | `e.button.x/y` (float) |
| `SDL_WINDOWEVENT` | Individual `SDL_EVENT_WINDOW_*` events |
| `SDL_CreateRenderer(-1, flags)` | `SDL_CreateRenderer(NULL)` |
| `SDL_ShowCursor(0/1)` | `SDL_HideCursor()` / `SDL_ShowCursor()` |
| `TTF_RenderUTF8_Blended` | `TTF_RenderText_Blended` (+length param) |
| `TTF_SizeUTF8` | `TTF_GetStringSize` (+length param, returns bool) |
| `TTF_GetError()` | `SDL_GetError()` (removed from SDL3_ttf) |
| `IMG_Init` / `IMG_Quit` | Removed (on-demand loading) |
| `SDL2_gfx` (roundedBoxRGBA) | `SDL_RenderGeometry` (built-in) |
| `SDL_PRESSED` / `SDL_RELEASED` | Removed (implicit per event type) |
| `SDL_TEXTINPUTEVENT_TEXT_SIZE` | Removed (text is const char\*) |
| `SDL_RENDERER_PRESENTVSYNC` | Removed (default behavior) |
| `SDL_Init()` returns `< 0` on error | `SDL_Init()` returns `bool` |

## License

See LICENSE file for licensing information.
