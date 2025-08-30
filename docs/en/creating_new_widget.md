# How to Create a Custom Widget in SDL_GUI

This step‑by‑step guide explains how to create your own custom widget and integrate it with the SDL_GUI library architecture. We will use a simple widget named `MyWidget` as an example.

This page is also available in Polish: [`docs/pl/creating_new_widget.md`](../pl/creating_new_widget.md)

## Introduction

All user interface elements in this library (called widgets) derive from `GUIElement`. That base class provides the essentials such as position/size/visibility, the parent–child hierarchy, and — critically — access to the global application context via `GUIManager`.

To create a new widget, inherit from `GUIElement` and implement the two key virtual methods:
- `draw()`: Responsible for rendering the widget into its cache (or final output if using direct rendering).
- `handleEvent()`: Responsible for processing interactions (e.g., mouse or keyboard).

References:
- Base element API and rendering flow: [`src/gui.hpp`](../../src/gui.hpp:19), [`src/gui.cpp`](../../src/gui.cpp:135)
- Manager context and lifecycle: [`src/gui_manager.hpp`](../../src/gui_manager.hpp:19), [`src/gui_manager.cpp`](../../src/gui_manager.cpp:14)

## Step 1: Create Files

Following the project conventions, each widget should have a header (`.hpp`) and a source (`.cpp`) file.

1) Header file: `src/MyWidget.hpp`  
2) Implementation file: `src/MyWidget.cpp`

## Step 2: Declare the Class in the Header

In `src/MyWidget.hpp`, declare your `MyWidget` class deriving from `GUIElement`.

```cpp
// src/MyWidget.hpp

#ifndef MYWIDGET_HPP
#define MYWIDGET_HPP

#include "gui.hpp" // Core header for all GUI elements

// Declaration of our new widget
class MyWidget : public GUIElement {
public:
    // Constructor takes manager, position, and size
    MyWidget(GUIManager& manager, int x, int y, int width, int height);

    // Default destructor is sufficient thanks to smart pointers
    ~MyWidget() = default;

    // Override to handle events
    bool handleEvent(const SDL_Event& e) override;

    // Optional but useful for debugging and theming/type identification
    const char* getComponentType() const override;

protected:
    // Override to draw widget visuals
    void draw() override;
};

#endif // MYWIDGET_HPP
```

Notes:
- The base class provides the parent–child model and methods like `addChild(std::unique_ptr<GUIElement>)`.
- Style/theme, timers, and animations are provided via the manager and specialized managers:
  - Textures: [`src/texture_manager.hpp`](../../src/texture_manager.hpp:15)
  - Fonts: [`src/font_manager.hpp`](../../src/font_manager.hpp:30)
  - Timers: [`src/timer_manager.hpp`](../../src/timer_manager.hpp:18)
  - Animations: [`src/animation_manager.hpp`](../../src/animation_manager.hpp:24)
  - Theme/Style: [`src/theme.hpp`](../../src/theme.hpp:10), [`src/style.hpp`](../../src/style.hpp:17)

## Step 3: Implement the Widget in the `.cpp` File

In `src/MyWidget.cpp`, implement your widget logic.

### Constructor

Call the base `GUIElement` constructor with the manager and geometry.

```cpp
// src/MyWidget.cpp

#include "MyWidget.hpp"
#include "gui_manager.hpp" // Needed for renderer access and managers

// Constructor implementation
MyWidget::MyWidget(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
    // Additional widget-specific initialization can go here
}
```

### The `draw()` Method

This is the core of the widget’s visual representation. It is called when the element’s render cache is invalid and needs to be refreshed. Access the SDL renderer through `GUIManager`.

```cpp
void MyWidget::draw() {
    // 1) Optionally call the base behavior if it draws common background/border using resolved style
    GUIElement::draw();

    // 2) Add custom drawing on top (e.g., a diagonal line)
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Point abs_pos = getAbsolutePosition();

    // Set draw color to red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Draw a diagonal line across the widget
    SDL_RenderDrawLine(renderer, abs_pos.x, abs_pos.y, abs_pos.x + m_width, abs_pos.y + m_height);
}
```

Implementation notes:
- By default, widgets render into their own cached texture (`m_cachedTexture`) managed by `GUIElement`, and that image is then blitted to the main renderer. See: [`src/gui.cpp`](../../src/gui.cpp:135)
- For frequently changing visuals, consider a direct rendering path (see “Direct rendering” in [`src/gui.hpp`](../../src/gui.hpp:82) and widget overrides `wantsDirectRender()/drawDirect()`).

### The `handleEvent()` Method

Implements interaction logic. The base `GUIElement` already updates hover/pressed states and tracks whether the cursor is over the widget.

```cpp
bool MyWidget::handleEvent(const SDL_Event& e) {
    // Save state before handling
    auto previousState = m_currentState;

    // Let the base class update internal state (hover, pressed, etc.)
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        // Detect click: transition from Pressed to Hover is a common pattern
        if (previousState == ElementState::Pressed && m_currentState == ElementState::Hover) {
            printf("MyWidget clicked!\n");
            return true; // Event consumed
        }
    }

    return false; // Not handled here
}

const char* MyWidget::getComponentType() const {
    return "MyWidget";
}
```

Guidelines:
- Prefer returning `true` when you actively consume an event (e.g., finalize a click).
- Use `markDirty()` to invalidate the render cache after state changes that affect visuals.
- Consider keyboard focus, tab order, and IME/UTF‑8 concerns for text‑handling widgets.

## Step 4: Using the New Widget

Include the header, construct the widget, and add it to the `GUIManager`.

```cpp
// In your main or GUI initialization code

#include "gui_manager.hpp"
#include "MyWidget.hpp"
// ... other headers

// Initialize GUIManager
GUIManager guiManager(app.getRenderer());

// Create MyWidget using std::make_unique
auto myWidget = std::make_unique<MyWidget>(guiManager, 50, 50, 200, 80);

// Add the widget to the manager
guiManager.addElement(std::move(myWidget));

// Main loop pattern (simplified):
while (!quit) {
    while (SDL_PollEvent(&e)) {
        // handle quit...
        guiManager.processEvent(e);
    }

    // clean-up elements marked for deletion, run timers/animations
    guiManager.cleanup();

    // render everything
    guiManager.render();
}
```

## Best Practices and Next Steps

- Use styles: Prefer using theme/style to draw backgrounds, borders, and text, rather than manual primitives for static visuals. For example:
  ```cpp
  // In MyWidget constructor, configure style per state
  setBackgroundColor(ElementState::Normal, SDL_Color{200, 200, 200, 255});
  setBackgroundColor(ElementState::Hover,  SDL_Color{220, 220, 220, 255});
  setBackgroundColor(ElementState::Pressed, SDL_Color{180, 180, 180, 255});
  ```
- Use the `TextureManager` to draw images rather than raw primitives:
  - Access via `m_manager.getTextureManager()` and then set textures onto your widget as needed (e.g., in your draw logic).
- Leverage timers and animations for interactive or time‑based visuals:
  - Timers: [`src/timer_manager.hpp`](../../src/timer_manager.hpp:18)
  - Animations: [`src/animation_manager.hpp`](../../src/animation_manager.hpp:24)
- Remember to `markDirty()` after state/data changes that affect visuals.

## Example: ContextMenu (Reference Widget)

`ContextMenu` is a good example of a composite widget that extends `GUIElement` and composes other widgets (like `Panel` and `Button`) to provide a richer behavior.

- Inheritance base: [`src/context_menu.hpp`](../../src/context_menu.hpp), implementation: [`src/context_menu.cpp`](../../src/context_menu.cpp)
- Example usage: [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp:1)

### Architectural Highlights

- Composite architecture: a container `Panel` holds menu items.
- Lazy creation: actual item widgets are created on first `showAt()`.
- Auto‑positioning: the menu adjusts its position to remain within window bounds.
- Event handling: clicks outside the menu close it automatically.

## Direct Rendering vs. Cached Rendering

By default, `GUIElement` renders to an off‑screen cached texture, improving performance for complex but infrequently changing widgets. For continuously changing visuals, consider enabling a direct render path:

- Methods to look at: `wantsDirectRender()` and `drawDirect()` in your widget and how `GUIElement` integrates them. See [`src/gui.hpp`](../../src/gui.hpp:82) and [`src/gui.cpp`](../../src/gui.cpp:135)

## Links and References

- Base element: [`src/gui.hpp`](../../src/gui.hpp:19), implementation: [`src/gui.cpp`](../../src/gui.cpp:8)
- Manager: [`src/gui_manager.hpp`](../../src/gui_manager.hpp:19), implementation: [`src/gui_manager.cpp`](../../src/gui_manager.cpp:14)
- Texture manager: [`src/texture_manager.hpp`](../../src/texture_manager.hpp:15), implementation: [`src/texture_manager.cpp`](../../src/texture_manager.cpp:1)
- Font manager: [`src/font_manager.hpp`](../../src/font_manager.hpp:30), implementation: [`src/font_manager.cpp`](../../src/font_manager.cpp:7)
- Animations: [`src/animation_manager.hpp`](../../src/animation_manager.hpp:24)
- Timers: [`src/timer_manager.hpp`](../../src/timer_manager.hpp:18)
- Style & Theme: [`src/style.hpp`](../../src/style.hpp:17), [`src/theme.hpp`](../../src/theme.hpp:10)
- Example composite: [`src/context_menu.hpp`](../../src/context_menu.hpp), [`src/context_menu.cpp`](../../src/context_menu.cpp), example usage: [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp:1)

## Troubleshooting

- Widget not redrawing:
  - Ensure `markDirty()` is called after changes that affect visuals or size.
  - Verify cache recreation on resize: see [`src/gui.cpp`](../../src/gui.cpp:220)
- Resource issues:
  - Check asset paths and SDL initialization logs. Texture loading via `TextureManager` should log errors using `SDL_LogError`.
  - Verify `SDL_ttf` and `SDL_image` are properly initialized upstream.
- Input handling:
  - Confirm your hit‑testing logic allows base `handleEvent` to update state.
  - Return `true` only when consuming events to avoid blocking unrelated handlers.

## See Also

- Quick examples in [`examples/`](../../examples/:1), e.g. [`examples/example_button.cpp`](../../examples/example_button.cpp:1)
- Rendering flow deep‑dive: [`src/gui_manager.cpp`](../../src/gui_manager.cpp:51), [`src/gui.cpp`](../../src/gui.cpp:135)
- Testing strategy for the repo: [`docs/translation_guidelines.md`](../translation_guidelines.md)