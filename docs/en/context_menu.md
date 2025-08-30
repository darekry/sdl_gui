# ContextMenu — contextual menu widget

This page is also available in Polish: [`docs/pl/context_menu.md`](../pl/context_menu.md)

Short introduction
------------------
`ContextMenu` is a widget that implements a contextual (right‑click) menu which appears on demand in response to mouse or other events. It supports:
- dynamic creation of menu items with actions,
- separators between groups of options,
- enabling/disabling individual items,
- automatic positioning and closing behavior,
- full integration with SDL event handling.

Requirements and dependencies
-----------------------------
- Inherits from `GUIElement` — core functionality: [`src/gui.hpp`](../../src/gui.hpp:19)
- Uses `Panel` as the container for menu items: [`src/panel.hpp`](../../src/panel.hpp:13)
- Creates a `Button` for each clickable item: [`src/button.hpp`](../../src/button.hpp:13)
- Managed by `GUIManager`: [`src/gui_manager.hpp`](../../src/gui_manager.hpp:19)

Construction and basic usage
----------------------------
Minimal usage example (compilable fragment):

```cpp
#include "gui_manager.hpp"
#include "context_menu.hpp"

int main() {
    // Initialize SDL and GUIManager
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
    }, false); // disabled

    menuPtr->addSeparator();

    menuPtr->addItem("Delete", []() {
        std::cout << "Delete action executed!" << std::endl;
    });

    // Attach to the manager
    manager.addElement(std::move(contextMenu));

    // Show the menu at cursor position in your app
    menuPtr->showAt(mouseX, mouseY);

    return 0;
}
```

Adding menu items
-----------------
The context menu supports different item types:

### Standard items
```cpp
// Add an item with an action
menuPtr->addItem("Open File", []() {
    openFileDialog();
});

// Add a disabled item
menuPtr->addItem("Save", []() {
    saveCurrentFile();
}, false); // disabled

// Add a label-only item (no action)
menuPtr->addItem("Version 1.0");
```

### Separators
```cpp
menuPtr->addSeparator(); // Adds a visual separator
```

### Clearing the menu
```cpp
menuPtr->clearItems(); // Removes all items
```

Showing and hiding the menu
---------------------------
You can show the menu at any position on screen:

```cpp
// Show the menu
menuPtr->showAt(x, y);

// Check visibility
if (menuPtr->isVisible()) {
    std::cout << "Menu is currently shown" << std::endl;
}

// Hide the menu
menuPtr->hide();
```

The menu automatically:
- positions itself so it does not overflow the window bounds,
- closes after an option is selected,
- closes when clicking outside the menu area.

Event handling
--------------
ContextMenu automatically handles mouse events:

```cpp
// In your main event loop
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_RIGHT) {

        // Show the menu at the cursor
        menuPtr->showAt(event.button.x, event.button.y);
    }

    manager.processEvent(event);
}
```

The menu automatically:
- captures clicks on its items,
- executes the assigned actions,
- closes after action execution,
- closes when clicking outside its area.

Public method list
------------------
Below is the list of public methods with signatures and short descriptions. Signatures are from the header: [`src/context_menu.hpp`](../../src/context_menu.hpp:25).

### Item management
- `void addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)` — adds an item with an optional action and enabled state. ([`src/context_menu.hpp`](../../src/context_menu.hpp:30))
- `void addSeparator()` — adds a separator between item groups. ([`src/context_menu.hpp`](../../src/context_menu.hpp:31))
- `void clearItems()` — removes all items from the menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:32))

### Display control
- `void showAt(int x, int y)` — shows the menu at a given position, automatically keeping it within window bounds. ([`src/context_menu.hpp`](../../src/context_menu.hpp:34))
- `void hide()` — hides the menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:35))
- `bool isVisible() const` — returns true if the menu is currently visible. ([`src/context_menu.hpp`](../../src/context_menu.hpp:36))

### Inherited from GUIElement
- `bool handleEvent(const SDL_Event& event) override` — handles mouse events for the menu. ([`src/context_menu.hpp`](../../src/context_menu.hpp:38))
- `const char* getComponentType() const override` — returns "ContextMenu". ([`src/context_menu.hpp`](../../src/context_menu.hpp:39))

Internal mechanisms important to users
--------------------------------------
### ContextMenuItem structure
```cpp
struct ContextMenuItem {
    std::string text;              // Display text
    std::function<void()> action;  // Action to execute
    bool enabled = true;           // Whether the item is enabled
    bool separator = false;        // Whether this is a separator
};
```

### Composite architecture
ContextMenu uses a composite pattern:
- The root `ContextMenu` inherits from `GUIElement`,
- An internal `Panel` (`m_panel`) contains all menu items,
- Each item is either a `Button` (for actions) or a `Panel` (for separators).

### Deferred button creation
Buttons are created on-demand in `showAt()`:
- `createMenuButtons()` is called only when needed,
- sets a `m_needsUpdate` flag for synchronization,
- creates an appropriate widget per item.

### Automatic positioning
The `positionMenu()` method:
- checks window bounds (defaults to 800×600),
- shifts the menu left/up if it would overflow the right/bottom edges,
- ensures the menu is always fully visible.

### Clicking outside the menu
The `shouldCloseOnClick()` method:
- detects clicks outside the menu rectangle,
- uses `SDL_PointInRect()` to test position,
- returns true when the menu should be closed.

Tips & Gotchas (most common)
------------------------------
### Building dynamic menus
```cpp
// Context-sensitive menu
void showFileMenu(int fileType) {
    menuPtr->clearItems();

    menuPtr->addItem("Open");
    menuPtr->addItem("Edit");

    if (fileType == IMAGE_FILE) {
        menuPtr->addSeparator();
        menuPtr->addItem("View Image");
        menuPtr->addItem("Resize");
    }

    menuPtr->showAt(mouseX, mouseY);
}
```

### Managing item state
```cpp
// Enable/disable items depending on application state
bool canSave = hasUnsavedChanges();
menuPtr->addItem("Save", []() { saveFile(); }, canSave);

bool canUndo = !undoStack.empty();
menuPtr->addItem("Undo", []() { undoLastAction(); }, canUndo);
```

### Right mouse button handling
```cpp
// In the event loop
if (event.type == SDL_MOUSEBUTTONDOWN &&
    event.button.button == SDL_BUTTON_RIGHT) {

    // Check which element was clicked
    GUIElement* clickedElement = getElementAt(event.button.x, event.button.y);

    if (clickedElement != nullptr) {
        showContextMenuFor(clickedElement, event.button.x, event.button.y);
    }
}
```

### Positioning issues
- The menu assumes an 800×600 window by default — adjust inside `positionMenu()` if needed.
- Ensure `GUIManager` is properly configured before constructing the menu.
- You may need to call `markDirty()` manually after structural changes.

### Visibility issues
- The menu is hidden initially — always call `showAt()` to display it.
- The menu automatically closes after an option is selected.
- Ensure all callbacks are connected correctly.

Additional references / examples
--------------------------------
- Full example in the examples directory: [`examples/example_context_menu.cpp`](../../examples/example_context_menu.cpp:1)
- Widget constructor: [`src/context_menu.hpp`](../../src/context_menu.hpp:27)
- `addItem` implementation: [`src/context_menu.cpp`](../../src/context_menu.cpp:18)
- `showAt` implementation: [`src/context_menu.cpp`](../../src/context_menu.cpp:37)
- `handleEvent` implementation: [`src/context_menu.cpp`](../../src/context_menu.cpp:56)

Frequently reviewed code lines (important implementation spots)
--------------------------------------------------------------
- Constructor / destructor: [`src/context_menu.cpp`](../../src/context_menu.cpp:5)
- addItem: [`src/context_menu.cpp`](../../src/context_menu.cpp:18)
- addSeparator: [`src/context_menu.cpp`](../../src/context_menu.cpp:24)
- showAt: [`src/context_menu.cpp`](../../src/context_menu.cpp:37)
- hide: [`src/context_menu.cpp`](../../src/context_menu.cpp:50)
- handleEvent: [`src/context_menu.cpp`](../../src/context_menu.cpp:56)
- createMenuButtons: [`src/context_menu.cpp`](../../src/context_menu.cpp:88)
- positionMenu: [`src/context_menu.cpp`](../../src/context_menu.cpp:125)
- shouldCloseOnClick: [`src/context_menu.cpp`](../../src/context_menu.cpp:153)

Conclusion
----------
ContextMenu is a versatile widget that can be adapted to many scenarios. Its composite architecture makes it easy to extend, and its automatic lifecycle behavior makes it reliable for everyday use. It works well for simple context menus as well as more complex contextual systems.