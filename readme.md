# SDL2 GUI Library

English is the authoritative language for this repository (Profile A). The Polish mirror of this page is available at: [`README.pl.md`](README.pl.md)

## Overview

A simple and extensible GUI library built on SDL2 with C++23, designed for clarity, safe memory management, and ease of use. It provides a lightweight set of widgets, resource managers, and a consistent architecture centered around a context manager.

Key source files:
- Core element base: [`src/gui.hpp`](src/gui.hpp:19), implementation: [`src/gui.cpp`](src/gui.cpp:8)
- Manager: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), implementation: [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- Textures: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), implementation: [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- Fonts: [`src/font_manager.hpp`](src/font_manager.hpp:30), implementation: [`src/font_manager.cpp`](src/font_manager.cpp:7)
- Theme and style: [`src/theme.hpp`](src/theme.hpp:10), [`src/style.hpp`](src/style.hpp:17)
- Timers and animations: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24)

## Architecture

The library architecture focuses on reusability, central resource management, and efficient rendering (including caching).

### [`GUIElement`](src/gui.hpp:19) — Base Class

An abstract base class that defines a common interface and behavior for all widgets.

- Hierarchy and ownership: parent–child managed via `std::vector<std::unique_ptr<GUIElement>>` (RAII, safe destruction).
- Context access: each element references the global manager [`GUIManager`](src/gui_manager.hpp:19) for access to renderer and resource managers.
- Core properties: position, size, visibility, enabled state, hover/press state tracking.
- Event handling: a virtual `handleEvent(const SDL_Event&)` propagates events down the widget tree; widgets can consume events to stop propagation.
- Rendering: `render()` draws the element and its children. The widget-specific drawing logic lives in `draw()` or (optionally) in a direct path `drawDirect()` when bypassing cache.
- Deferred deletion: elements can be safely removed via a mark-and-cleanup phase.
- Tooltips: built-in tooltip support using [`TimerManager`](src/timer_manager.hpp:18).

### [`GUIManager`](src/gui_manager.hpp:19) — Central Context

The central controller and provider of shared resources.

- Life-cycle management: owns top-level GUI elements and coordinates event processing, rendering, and cleanup.
- Global services: constructs and owns [`FontManager`](src/font_manager.hpp:30), [`TextureManager`](src/texture_manager.hpp:15), and [`TimerManager`](src/timer_manager.hpp:18); stores a pointer to `SDL_Renderer`.
- Event propagation: receives SDL events from the main loop and forwards them to elements.
- Global utilities: e.g., dynamic tooltips.

### Resource Managers

Optimized memory and resource management to avoid redundant loads.

- [`FontManager`](src/font_manager.hpp:30): caches TTF fonts as `std::shared_ptr<TTF_Font>` and provides precise text measurement.
- [`TextureManager`](src/texture_manager.hpp:15): loads and caches `SDL_Texture` as `std::shared_ptr<SDL_Texture>`; can accept user-provided textures (e.g., via add methods).
- [`TimerManager`](src/timer_manager.hpp:18): safe timer handling on the main thread (single-shot and interval), used by tooltips, animations, etc.

## Available Widgets

A set of ready-to-use, configurable widgets.

### Panel
- Purpose: container for grouping other elements (windows, panels, sections).
- Example:
```cpp
auto panel = std::make_unique<Panel>(50, 50, 300, 200);
panel->setBackgroundColor(SDL_Color{200, 200, 200, 255});
panel->setBorder(2, SDL_Color{100, 100, 100, 255});
panel->setDraggable(true);
guiManager.addElement(std::move(panel));
```

### Label
- Purpose: static text label.
- Example:
```cpp
auto label = std::make_unique<Label>(100, 100, "Hello, world!", 24, SDL_Color{0, 0, 0, 255});
guiManager.addElement(std::move(label));
```

### Button
- Purpose: clickable button.
- Example:
```cpp
auto button = std::make_unique<Button>(100, 150, 120, 40, "Click me");
button->setOnClickCallback([](GUIElement*) {
    // Click handler
});
guiManager.addElement(std::move(button));
```

### Checkbox
- Purpose: on/off toggle.
- Example:
```cpp
auto checkbox = std::make_unique<Checkbox>(100, 200, "I agree");
checkbox->setOnChange([](Checkbox* cb, bool isChecked){
    // State changed
});
guiManager.addElement(std::move(checkbox));
```

### RadioButton and RadioGroup
- Purpose: mutually-exclusive options.
- Example:
```cpp
auto radioGroup = std::make_unique<RadioGroup>(100, 250, 200, 100);
radioGroup->addChild(std::make_unique<RadioButton>(10, 10, "Option 1", true)); // default selected
radioGroup->addChild(std::make_unique<RadioButton>(10, 40, "Option 2"));
guiManager.addElement(std::move(radioGroup));
```

### Slider
- Purpose: value selection from a range.
- Example:
```cpp
auto slider = std::make_unique<Slider>(100, 380, 200, 20, 0, 100);
slider->setOnChangeCallback([](int value){
    // Value changed
});
guiManager.addElement(std::move(slider));
```

### TextInput
- Purpose: single-line text input.
- Example:
```cpp
auto textInput = std::make_unique<TextInput>(100, 420, 200, 30);
textInput->setOnEnterPressed([](const std::string& text){
    // Enter pressed
});
guiManager.addElement(std::move(textInput));
```

### TextArea
- Purpose: multi-line text input with word wrapping.
- Example:
```cpp
auto textArea = std::make_unique<TextArea>(400, 50, 300, 200);
textArea->setText("This is a multi-line\ntext with\nword-wrapping.");
textArea->setWordWrap(true);
guiManager.addElement(std::move(textArea));
```

### ComboBox
- Purpose: dropdown list of options.
- Example:
```cpp
auto comboBox = std::make_unique<ComboBox>(400, 280, 150, 30);
comboBox->addItem("Option A");
comboBox->addItem("Option B");
comboBox->addItem("Option C");
comboBox->on_selection_changed = [](int index, const std::string& item){
    // Selection changed
};
guiManager.addElement(std::move(comboBox));
```

### TabControl
- Purpose: tabbed container to switch views.
- Example:
```cpp
auto tabControl = std::make_unique<TabControl>(400, 330, 300, 150);
Panel* tab1 = tabControl->addTab("Tab 1");
tab1->addChild(std::make_unique<Label>(10, 10, "First tab content."));
Panel* tab2 = tabControl->addTab("Tab 2");
tab2->addChild(std::make_unique<Button>(10, 10, 100, 30, "Button"));
guiManager.addElement(std::move(tabControl));
```

## How to Use

### Initialization

The recommended entry-point helper is provided in [`src/sdl_app.hpp`](src/sdl_app.hpp:1).

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
// ... other widgets

SDLApp app("My Application", 800, 600);
GUIManager guiManager(app.getRenderer());
```

### Main Application Loop

Your application owns the SDL event loop, which provides maximum flexibility.

```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        // Forward events to the GUI
        guiManager.processEvent(e);
    }

    // Safe removal of elements and upkeep
    guiManager.cleanup();

    SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
    SDL_RenderClear(app.getRenderer());
    guiManager.render();
    SDL_RenderPresent(app.getRenderer());
}
```

## Build

The project uses a Makefile and a unity build to speed up compilation.

### Requirements

- SDL2, SDL2_image, SDL2_ttf
- C++23 compiler (Clang++ recommended)
- `make`

### Commands

- `make all`: builds examples and tests
- `make examples`: builds only examples
- `make test`: builds and runs tests
- `make clean`: removes build outputs

Run an example:
```bash
make examples
./output/example_button
```

## Testing

The project uses **Catch2** for unit testing. Tests reside in [`tests/`](tests/:1) and can be run with:
```bash
make test
```

## Notes on Rendering and Caching

- Render path: the manager iterates over top-level elements and calls their render entry point (see [`src/gui_manager.cpp`](src/gui_manager.cpp:51), [`src/gui.cpp`](src/gui.cpp:135)).
- Cached rendering: when the widget does not opt into direct rendering, its content is drawn into a cached texture and then blitted to the main renderer; cache is invalidated on changes.
- Direct rendering: for rapidly changing widgets, `drawDirect()` may be used by returning true from the direct-render path; this bypasses the off-screen cache and draws straight to the renderer.

For more details on design and coding guidelines, see the translation and terminology guide: [`docs/translation_guidelines.md`](docs/translation_guidelines.md)

## Documentation Indexes
- Archive (EN): [docs/en/archive/README.md](docs/en/archive/README.md)
- Archiwum (PL): [docs/pl/archive/README.md](docs/pl/archive/README.md)