# Architectural Review of the GUI Library
[This page is also available in Polish](../../pl/archive/architectural_review.md)
[Back to Archive](./README.md)

Date: 2025-07-06
## Introduction

This document summarizes the results of the library's architectural review. The objectives were to identify inconsistencies against the intended design, point out scalability risks, and suggest fixes.

## 1. Identified Architectural Inconsistencies

### 1.1. Using Raw Pointers to Manage Children

**Problem:**
Components like ComboBox and TabControl store raw pointers to their child elements (e.g., `Button* m_main_button`, `std::vector<Panel*> m_tabPanels`). Ownership is already managed by `std::unique_ptr` in the base class GUIElement’s `m_children` container.

**Risks:**
- Dangling pointers: when a child is removed or replaced, the raw pointer may dangle, causing undefined behavior.
- Violates single-ownership principle: the architecture is designed around smart pointers; additional raw pointers complicate lifetime management.

**Recommendation:**
Remove raw pointers from those classes. Access children by iterating `m_children` and, if needed, use `dynamic_cast` to obtain the specific derived type.

```cpp
// Accessing the main button inside ComboBox
// Instead of: m_main_button->setLabel(...);
// Use:
if (!m_children.empty()) {
    if (auto main_button = dynamic_cast<Button*>(m_children[0].get())) {
        main_button->setLabel(...);
    }
}
```

### 1.2. Redundant SDL_Renderer* parameter in render methods

**Problem:**
All render() methods across the hierarchy accept `SDL_Renderer* renderer`, which contradicts the idea that [GUIManager](../../src/gui_manager.hpp) centrally provides context, and every [GUIElement](../../src/gui.hpp) can access it through `m_manager`.

**Recommendation:**
Remove the `SDL_Renderer*` parameter from all render signatures. Implementations should query the renderer directly from the manager:

```cpp
// Inside any derived GUIElement::render()
void MyElement::render() { // No parameter
    SDL_Renderer* renderer = m_manager.getRenderer();
    // ... rendering logic ...
}
```

### 1.3. Hard-coded resource paths

**Problem:**
In several places (e.g., `TextInput::updateTextTexture`, `GUIElement::setLabel`) the font path is hard-coded ("assets/fonts/font.ttf").

**Risk:**
- Inflexibility: users cannot easily change the default font or folder structure without modifying the library.

**Recommendation:**
Use the default font loaded by [FontManager](../../src/font_manager.hpp) during initialization. Methods such as `setLabel` should call `m_manager.getFontManager().getDefaultFont()`. The asset path should be configurable in one place—when constructing [GUIManager](../../src/gui_manager.hpp).

### 1.4. Bypassing the TextureManager in examples

**Problem:**
Example code (e.g., [examples/example_window.cpp](../../examples/example_window.cpp)) creates textures via a helper like `createColorTexture` and manually manages their lifetime, bypassing [TextureManager](../../src/texture_manager.hpp).

**Recommendation:**
Extend [TextureManager](../../src/texture_manager.hpp) with `createColorTexture(SDL_Color color, int width, int height)`. Update examples to use it for consistency.

## 2. Scalability Issues and Code Duplication

### 2.1. Repeated SDL init/cleanup boilerplate in examples

**Problem:**
Each file in [examples/](../../examples) repeats lengthy initialization for SDL, SDL_image, SDL_ttf, error handling, and final cleanup.

**Recommendation:**
Create an RAII helper (e.g., `SDLApp`) encapsulating init/teardown and holding window/renderer. This reduces duplication and centralizes error handling.

**Sketch:**
```cpp
// examples/helpers/sdl_app.hpp
class SDLApp {
public:
    SDLApp(const char* title, int w, int h);
    ~SDLApp();
    SDL_Renderer* getRenderer() { return renderer; }
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

int main(int, char**) {
    SDLApp app("Example", 800, 600);
    GUIManager gui(app.getRenderer());
    // ... GUI logic ...
    // main loop
    return 0;
}