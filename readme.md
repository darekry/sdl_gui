# SDL2 GUI Library

English is the authoritative language for this repository (Profile A). The Polish mirror of this page is available at: [`README.pl.md`](README.pl.md)

## Overview

A simple and extensible GUI library built on SDL2 with C++23, designed for clarity, safe memory management, and ease of use. It provides a lightweight set of widgets, resource managers, and a consistent architecture centered around a context manager.

## How to Use

### Initialization

To get started, include the main header `gui_manager.hpp` and other necessary widget headers. The recommended entry-point helper is provided in `sdl_app.hpp`.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
// ... other widgets

// Initialize SDL and a window
SDLApp app("My Application", 800, 600);

// Create the GUI manager
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

## Linking the Library

To use the library, you need to link it against your application. Below are examples for both static and dynamic linking.

### Static Linking (`.a`)

When linking with the static library (`libsdl_gui.a`), you need to provide the path to the header files and the library file, along with the necessary SDL2 dependencies.

```bash
g++ your_app.cpp -o your_app -I/path/to/headers -L/path/to/library -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

### Dynamic Linking (`.so`)

When linking with the shared library (`libsdl_gui.so`), ensure the linker can find it at runtime.

```bash
g++ your_app.cpp -o your_app -I/path/to/headers -L/path/to/library -lsdl_gui -lSDL2 -lSDL2_image -lSDL2_ttf
```

Make sure the `.so` file is in a directory known to the dynamic linker (e.g., `/usr/local/lib`) or set the `LD_LIBRARY_PATH` environment variable.

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

For more details on design and coding guidelines, see the translation and terminology guide: [`docs/translation_guidelines.md`](docs/translation_guidelines.md)

## Documentation Indexes
- Archive (EN): [docs/en/archive/README.md](docs/en/archive/README.md)
- Archiwum (PL): [docs/pl/archive/README.md](docs/pl/archive/README.md)