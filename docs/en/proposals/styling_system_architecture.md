# GUI Styling System Architecture

[This page is also available in Polish](../../pl/proposals/styling_system_architecture.md)

[Back to Feature Proposals](../feature_proposals.md)

## 1. Introduction

The current styling system in the library is inconsistent and limited. This document proposes a new, extensible architecture that unifies the look and behavior of components, introduces a theme system, and simplifies customization of UI elements.

## 2. Key Data Structures

### 2.1. ElementState

We introduce an enum class to explicitly define possible states of an element. This is foundational to managing styles dependent on user interaction.

```cpp
enum class ElementState {
    Normal,
    Hover,
    Pressed,
    Disabled
};
```

### 2.2. Style

The Style struct is a container for all visual attributes of a single element state. Using std::optional enables inheritance of attributes from the default theme—if an attribute in the element style is std::nullopt, the value from the theme will be used.

```cpp
#include "texture_manager.hpp" // For SharedTexture
#include <optional>
#include <SDL_pixels.h>

struct Style {
    std::optional<SDL_Color> backgroundColor;
    std::optional<SDL_Color> textColor;
    std::optional<SharedTexture> texture;
    std::optional<SDL_Color> borderColor;
    std::optional<int> borderWidth;
};
```

- SharedTexture is defined by the texture manager: [texture_manager.hpp](../../src/texture_manager.hpp:15)

### 2.3. Theme

The Theme class is the central store of default styles for all component types. It allows global look-and-feel customization. The default theme will mimic Windows 95/98.

Using std::string as a key is crucial; it makes it easy to add styles for newly created component classes without modifying Theme itself.

```cpp
#include <map>
#include <string>

class Theme {
public:
    // Sets the default style for a given component type and state
    void setStyle(const std::string& componentType, ElementState state, Style style);

    // Retrieves the default style. Returns the fallback style if none is defined.
    const Style& getStyle(const std::string& componentType, ElementState state) const;
    
    // Factory for the default "Windows 95" theme
    static Theme createDefaultTheme();

private:
    // Map: Component type -> Map: State -> Style
    std::map<std::string, std::map<ElementState, Style>> styles;
    Style defaultStyle; // Used when a specific style is missing
};
```

## 3. API Changes

### 3.1. GUIManager

[GUIManager](../../src/gui_manager.hpp:19) will own the Theme object and share it with all elements.

```cpp
class GUIManager {
public:
    // ... existing methods ...

    // Sets a new theme
    void setTheme(Theme theme);

    // Returns a reference to the current theme
    Theme& getTheme();

private:
    // ... existing fields ...
    Theme m_theme = Theme::createDefaultTheme(); // Default theme
};
```

### 3.2. GUIElement

[GUIElement](../../src/gui.hpp:19) will be extended to manage its own styles and cooperate with the theme system.

```cpp
class GUIElement {
public:
    // ... existing methods ...

    // Sets the style for a specific state
    void setStyle(ElementState state, Style style);

    // Gets the style for a given state (may be empty)
    const std::optional<Style>& getStyle(ElementState state) const;

    // Resolves the final style by merging element style with the theme
    Style getResolvedStyle() const;

    // Helpers for per-state updates
    void setBackgroundColor(ElementState state, SDL_Color color);
    void setTextColor(ElementState state, SDL_Color color);
    void setTexture(ElementState state, SharedTexture texture);
    void setBorder(ElementState state, SDL_Color color, int width);

    // Returns the component type as a string (to be implemented in derived classes)
    virtual const char* getComponentType() const { return "GUIElement"; }

protected:
    // ... existing fields ...
    
    // Stores element-specific styles
    std::map<ElementState, Style> m_styles;

    // Current element state
    ElementState m_currentState = ElementState::Normal;

private:
    // Helper to merge styles
    Style resolveStyle(const Style& base, const std::optional<Style>& override) const;
};
```

Important note: Each class deriving from GUIElement (e.g., Button, Checkbox) must override getComponentType() so the theme system can correctly identify components.

```cpp
// In Button
const char* getComponentType() const override { return "Button"; }
```

- Button is a typical widget within the library; see rendering and base mechanisms in [gui.cpp](../../src/gui.cpp:8)

### 3.3. Removal of the old API

Methods such as setNormalTexture, setHoverTexture in RadioButton will be removed and replaced by the new unified API setStyle or by helpers (e.g., setTexture(ElementState::Hover, ...)).

## 4. Rendering Logic and Style Resolution

The decision process in an element’s render() method will be:

```mermaid
graph TD
    A[Begin element rendering] --> B{Get current state<br>(m_currentState)};
    B --> C[Call getResolvedStyle()];
    C --> I[Use returned style to render<br>background, border, text];
    I --> J[Finish rendering];
```

Implementation of getResolvedStyle:

```cpp
Style GUIElement::getResolvedStyle() const {
    // 1. Fetch the default style from theme
    const auto& themeStyle = m_manager.getTheme().getStyle(getComponentType(), m_currentState);
    
    // 2. Check if there is an element-specific style
    auto it = m_styles.find(m_currentState);
    if (it != m_styles.end()) {
        // 3. Merge styles: element style overrides theme
        return resolveStyle(themeStyle, it->second);
    } else {
        // No local override, return theme style
        return themeStyle;
    }
}
```

- m_manager references the owning manager; see [gui_manager.hpp](../../src/gui_manager.hpp:19)
- Theme and default theme creation live in [theme.hpp](../../src/theme.hpp:10) and [theme.cpp](../../src/theme.cpp:1)

## 5. Design Rationale and Alternatives

The proposed architecture aims to balance flexibility, performance, and simplicity.

- std::optional in Style: Provides a clean inheritance mechanism from the theme. Without it, we’d need explicit flags/pointers, complicating code. This is the foundation for overrides.
- Component identification (getComponentType): Using a virtual function and std::string keys in Theme is crucial for extensibility. It lets users define custom components and their default styles without modifying the library. An enum would be faster but would close the system to new component types.
- Dynamic style resolution (getResolvedStyle): Calculating the style at render time allows dynamic theme switching while the app is running—every element immediately adopts the new look.

### 5.1. Potential Simplification and Its Consequences

We could simplify the system at the cost of flexibility.

Simplification idea: Instead of dynamic resolution, copy the theme’s style into the element at construction time.

- How it would work: In GUIElement’s constructor, copy styles from the current theme into m_styles. Then getResolvedStyle becomes trivial—it just returns from the local map.
- Pros:
  - Performance: No merging or virtual calls in the rendering loop.
  - Simpler code: getResolvedStyle is straightforward.
- Cons:
  - Loss of dynamic theme switching: After creation, elements won’t react to later theme changes—this is a significant UX loss.
  - Higher memory usage: Each element stores full sets of styles, not only overrides.

Given the library’s goals (flexibility and extensibility), the current dynamic model is preferred.

## 6. API Usage Example

The following code shows how a developer can customize a button by overriding default theme settings.

```cpp
// Manager initialization
GUIManager guiManager(renderer);

// Create a button
auto myButton = std::make_unique<Button>(guiManager, 50, 50, 150, 40);

// --- Customization ---

// 1. Change background for Normal
myButton->setBackgroundColor(ElementState::Normal, SDL_Color{200, 200, 255, 255}); // Light blue

// 2. Set texture and different text color for Hover
Style hoverStyle;
hoverStyle.texture = textureManager.load("assets/button_hover.png");
hoverStyle.textColor = SDL_Color{255, 255, 0, 255}; // Yellow text
myButton->setStyle(ElementState::Hover, hoverStyle);

// 3. Red border for Pressed
myButton->setBorder(ElementState::Pressed, SDL_Color{255, 0, 0, 255}, 2);

// Add the button to the manager
guiManager.addElement(std::move(myButton));
```

- GUIManager owns global resources like the texture manager and theme; see [gui_manager.cpp](../../src/gui_manager.cpp:14)
- Texture creation, caching and SharedTexture are provided by [texture_manager.cpp](../../src/texture_manager.cpp:1)

## 7. Implementation Notes

- Where to store Theme: Owned by GUIManager ([gui_manager.hpp](../../src/gui_manager.hpp:19)), accessible to elements through a reference.
- Interaction with caching: When a style changes, ensure the element is marked dirty so its cached texture is refreshed; see the render-to-cache path in [gui.cpp](../../src/gui.cpp:174).
- Default theme content: Place sensible defaults for common components (e.g., Button, Checkbox) in [theme.cpp](../../src/theme.cpp:1).
- Rendering consistency: Rendering honors clipping from parents; see [gui.cpp](../../src/gui.cpp:133) for clip rect behavior.

## 8. Summary

- Introduce a Theme-driven styling system with per-state Style and dynamic resolution.
- Keep component identification open via getComponentType strings for extensibility.
- Allow element-level overrides through std::optional fields merged atop theme defaults.
- Preserve dynamic theme switching and rendering consistency within the existing architecture, centered around [GUIManager](../../src/gui_manager.hpp:19).