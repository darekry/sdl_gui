# ContextMenu — Contextual Menu Widget

This page is also available in Polish: [`docs/pl/context_menu.md`](../pl/context_menu.md)

`ContextMenu` is a widget that implements a context menu, which appears in response to user actions, typically a right mouse click.

**Key Features:**
- Dynamically create menu items with assigned actions.
- Add separators between groups of options.
- Enable and disable individual items.
- Automatic positioning and closing.

## Construction and Basic Usage

To use `ContextMenu`, create an instance, add menu items, and then display it at the appropriate time.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "context_menu.hpp"

int main() {
    SDLApp app("ContextMenu Example", 800, 600);
    GUIManager manager(app.getRenderer());

    // Create the context menu
    auto contextMenu = std::make_unique<ContextMenu>(manager);
    ContextMenu* menuPtr = contextMenu.get();

    // Add menu items
    menuPtr->addItem("Copy", []() {
        std::cout << "Copy action executed!" << std::endl;
    });
    menuPtr->addItem("Paste", []() {
        std::cout << "Paste action executed!" << std::endl;
    }, false); // Disabled item
    menuPtr->addSeparator();
    menuPtr->addItem("Delete", []() {
        std::cout << "Delete action executed!" << std::endl;
    });

    manager.addElement(std::move(contextMenu));

    // Main application loop
    while (app.isRunning()) {
        app.handleEvents();
        const SDL_Event& event = app.getEvent();

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
            menuPtr->showAt(event.button.x, event.button.y);
        }

        manager.processEvent(event);
        
        app.clearScreen();
        manager.render();
        app.present();
    }

    return 0;
}
```

## Managing Menu Items

The context menu allows for flexible addition and removal of items.

```cpp
// Add an item with an action
menuPtr->addItem("Open File", []() { /* ... */ });

// Add a disabled item
menuPtr->addItem("Save", []() { /* ... */ }, false);

// Add a visual separator
menuPtr->addSeparator();

// Remove all items
menuPtr->clearItems();
```

## Displaying and Hiding

The menu can be displayed anywhere on the screen. It will automatically adjust its position to stay within the window bounds.

```cpp
// Display the menu at the given coordinates
menuPtr->showAt(x, y);

// Check if the menu is visible
if (menuPtr->isVisible()) {
    // ...
}

// Hide the menu
menuPtr->hide();
```

The menu closes automatically when an option is selected or when a click occurs outside its area.

## API Reference

### Item Management
- `ContextMenu(GUIManager& manager)`: Constructor.
- `addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)`: Adds a menu item.
- `addSeparator()`: Adds a separator.
- `clearItems()`: Removes all items.

### Display Control
- `showAt(int x, int y)`: Displays the menu at the specified position.
- `hide()`: Hides the menu.
- `isVisible() const`: Returns `true` if the menu is visible.

## Example

A complete, working example of `ContextMenu` usage can be found in:
- [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp)