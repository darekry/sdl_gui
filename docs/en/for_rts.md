# SDL GUI Guide for RTS Game Development

## Brief Introduction and Document Purpose

This document aims to provide a complete, technical, and accessible introduction for a team of developers who want to build a simple RTS game using the SDL GUI library contained in this repository. It includes quick start instructions, architecture description, rendering flow, resource management, animations, timers, event handling, and a sample RTS project plan.

The library and key source files are located in the [`src`](src/:1) directory. The most important references used in this document are:
- GUI Manager: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- GUI Element Base: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- TextureManager: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- FontManager: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)
- AnimationManager: [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
- TimerManager: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1)
- Style and Theme: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10)
- Examples: [`examples/`](examples/:1) directory (includes, among others, [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1))

## Quick Start: Minimal Step-by-Step

Environmental Requirements:
- SDL2 (development library)
- SDL_image
- SDL_ttf

The fastest way to run examples:
1. Install system dependencies (e.g., on Debian/Ubuntu):
   sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
2. Go to the project directory and build:
   make
   (Compilation and target details are in [`Makefile`](Makefile:1))
3. Run an example, e.g.:
   ./output/example_button

Example files can be viewed in [`examples/`](examples/:1). Useful: [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1), [`examples/example_panel.cpp`](examples/example_panel.cpp:1).

Note: If CI or the environment is headless, see the testing strategy in [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1) and use Xvfb to run rendering in headless mode.

## Library Architecture — Key Components and Files

1) GUIManager
- Central application point — initializes SDL renderer, resource managers, and manages top-level GUI elements.
- Main files: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- Methods: initialization, `render()`, `pollEvents()`, and `addElement()` — widget registration in the manager.

2) GUIElement
- Abstract base for widgets: position, size, visibility, parent-child hierarchy.
- Files: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- Key concepts: markDirty, markForDeletion, wantsDirectRender(), draw()/drawDirect(), renderToCache(), `m_cachedTexture`.

3) TextureManager
- Loading images and creating `SharedTexture` with cache.
- Files: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- Provides functions: loadTexture(path), getTexture(key), createDefaultTexture().

4) FontManager
- Font cache and helper API for measuring text size (`getTextSize`).
- Files: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)

5) AnimationManager
- Animation system operating on int/float/variant fields and easing.
- File: [`src/animation_manager.hpp`](src/animation_manager.hpp:24)

6) TimerManager
- Scheduler for timed events (single-shot, interval) associated with GUI elements.
- Files: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1)

7) Style and Theme
- Centralization of styles and themes: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10)

8) Example Widgets and Usage
- Example of widget creation: [`examples/example_button.cpp`](examples/example_button.cpp:1)
- Animations: [`examples/example_animation.cpp`](examples/example_animation.cpp:1)
- More examples: [`examples/`](examples/:1)

## Rendering Flow and Element Lifecycle

Outline:
- `GUIManager::render()` iterates through top-level elements and calls `GUIElement::render()` — see [`src/gui_manager.cpp`](src/gui_manager.cpp:51) and [`src/gui.cpp`](src/gui.cpp:135).
- `GUIElement::render()` checks `wantsDirectRender()`:
  - If true: sets clipping and calls `drawDirect(renderer)`, renders children without cache.
  - If false: if `m_isDirty` -> `renderToCache()` (sets render target to `m_cachedTexture`), calls `draw(renderer)` and resets the target. Implementation is in [`src/gui.cpp`](src/gui.cpp:135).
- After rendering, `m_cachedTexture` is copied to the main renderer (SDL_RenderCopy) and then the element renders children with appropriate clipping.
- Cleanup: `GUIElement::cleanup()` and `GUIManager::cleanup()` remove elements marked for deletion (`markForDeletion`) and free timers/animations — see [`src/gui.cpp`](src/gui.cpp:267) and [`src/gui_manager.cpp`](src/gui_manager.cpp:64).

Practical Conclusions:
- Use caching (`m_cachedTexture`) for complex, infrequently changing widgets (maps, HUD) to reduce the cost of `draw()` every frame.
- Use `drawDirect()`/`wantsDirectRender()` for dynamic interactive elements that require drawing children in the frame context (e.g., cursors, particle effects).
- Ensure cache recreation when a widget's size changes — implementation in [`src/gui.cpp`](src/gui.cpp:220).

## Resource Management: TextureManager and FontManager

TextureManager:
- API: loading via `loadTexture(path)` and querying `getTexture(key)` — definitions in [`src/texture_manager.hpp`](src/texture_manager.hpp:15).
- Returns `SharedTexture` (std::shared_ptr<SDL_Texture>) and maintains a cache map.
- Best Practices:
  - Use consistent resource keys/paths (e.g., "units/soldier.png").
  - Create texture atlases where possible to limit renderer texture switching.
  - Register a default texture for GUI in `GUIManager` — see [`src/gui_manager.cpp`](src/gui_manager.cpp:14).
- Asset Debugging:
  - Check if the file exists and the path is correct.
  - Log `SDL_LogError` output from TextureManager (the manager should log errors).
  - If the texture is not created, check `SDL_image` initialization.
  - Default font/asset path: e.g., `assets/fonts/font.ttf` — TODO: verification of exact asset publishing policy (noted below).

FontManager:
- API: `loadFont(path, size)` and `getTextSize(font, text)` — definitions in [`src/font_manager.hpp`](src/font_manager.hpp:30).
- Cache: `SharedFont` in a map, synchronization with `SDL_ttf`.
- Debug: `TTF_OpenFont` initialization errors are common — check logs and if the font file is accessible.

## Animation System

- AnimationManager handles animations of simple properties (int/float/variant) and easing. Main API in [`src/animation_manager.hpp`](src/animation_manager.hpp:24).
- Supported properties:
  - Integer and floating-point numbers
  - `std::variant` to represent heterogeneous values
  - Easing: basic easing functions (see [`src/easing.hpp`](src/easing.hpp:1))
- Animation Lifecycle:
  - Animation creation: register target, start value, end value, duration, easing, and optional `on_complete` callback.
  - Update: `AnimationManager::update(dt)` is called daily (Frame tick) by `GUIManager`.
  - Removal: animations finish and are automatically removed after completion or can be interrupted.
- A practical example can be found in [`examples/example_animation.cpp`](examples/example_animation.cpp:1).

Design Tips:
- Use animations for unit state transitions (idle -> walk -> attack) and for smooth camera movements.
- Synchronize animation frames with textures via `TextureManager` (e.g., animation atlas) and update the frame index in the animation callback.

## TimerManager: Using Timers in Game Logic

- TimerManager provides single-shot and interval (cyclic) timers. See [`src/timer_manager.hpp`](src/timer_manager.hpp:18).
- Typical API: `startTimer(ownerId, intervalMs, singleShot, callback)` and `stopTimer(id)`.
- Integration with GUI Elements:
  - GUI elements can start timers via `startTimer`, and TimerManager notifies the element's associated callback.
  - Timers are used for: unit movement (ticks), attack cooldowns, AI ticks, time-controlled animations.

Usage Example: A patrolling unit can start a timer with a 50ms interval to update its position and trigger appropriate animations.

## Event Handling and Clickability

- The base `GUIElement` type in [`src/gui.hpp`](src/gui.hpp:19) provides event detection mechanisms and default hooks.
- Integration with SDL event loop: `GUIManager` integrates the SDL event loop and passes events to elements (see [`src/gui_manager.hpp`](src/gui_manager.hpp:19)).
- Examples:
  - Button: [`examples/example_button.cpp`](examples/example_button.cpp:1) shows how to register a click callback.
  - Checkbox, slider, and other widgets are in [`examples/`](examples/:1).

Implementing Clickable Units:
- Inherit from `GUIElement` and override event handling methods (e.g., onMouseDown/onMouseUp/onMouseMove) — the code snippet below provides an example.
- Drag selection: implement simple rectangle selection in global world coordinates, draw an overlay in `drawDirect()`, and during mouse up, calculate which units are within the rectangle.
- Coordinate mapping: remap screen coordinates (pixel window) to world-coordinates using camera transformation (offset + scale) — see example pseudocode.

Reference snippets: [`examples/example_button.cpp`](examples/example_button.cpp:1) and [`docs/creating_new_widget.md`](docs/creating_new_widget.md:1) contain practical tips.

## Sample RTS Project — Architecture and Pseudocode

Proposed Class Structure:
- UnitWidget: inherits from [`src/gui.hpp`](src/gui.hpp:19)
- BuildingWidget: inherits from [`src/gui.hpp`](src/gui.hpp:19)
- HUD: panel with GUI elements (resource bar, buttons) — extends [`src/panel.hpp`](src/panel.hpp:1)
- Minimap: lightweight panel drawing the world map directly (can use `drawDirect()`)

Element Registration:
- Add elements to the manager via `GUIManager::addElement(element)` — implementation in [`src/gui_manager.hpp`](src/gui_manager.hpp:19) / [`src/gui_manager.cpp`](src/gui_manager.cpp:14).

Resource Initialization (example):
- Load unit textures:
  auto soldierTex = guiManager.textureManager.loadTexture("assets/units/soldier.png");
- Load animation atlas and initialize animations via AnimationManager.
- Example reference files: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`examples/example_animation.cpp`](examples/example_animation.cpp:1).

Clickability Logic and Issue Commands:
- Selection: clicking `onMouseDown` on a single unit sets selected = true.
- Grouping: shift+click adds to selection, drag-selection selects multiple units in a rectangle.
- Issue command (move/attack): click on empty terrain -> calculate target point in world-coordinates and set move target for each selected unit.
- Mapping click -> world:
  worldX = (screenX / zoom) + cameraOffsetX
  worldY = (screenY / zoom) + cameraOffsetY

Unit Movement — Simplified Simulation:
- Use `TimerManager` for movement ticks (e.g., 30-60ms) or movement animations with `AnimationManager`.
- Simple algorithm:
  - compute direction = normalize(target - pos)
  - pos += direction * (speed * dt)
  - if dist(pos, target) < threshold => stop, change state to idle
- Synchronize state with `AnimationManager` (walk->idle).

Animation Synchronization with Atlas:
- Keep the animation frame index in UnitWidget and change it in the animation callback or in the TimerManager tick.
- TextureManager can store the atlas and return subrects for each frame.

Timers used in AI / pathfinding / attack cooldown:
- AI tick: interval timer (e.g., 200ms) to update decisions.
- Pathfinding tick: timer initialized when a unit has a target, updates position every n ms.
- Attack cooldown: single-shot timer with a callback resetting attack capability.

Minimal, Compilable C++ Pseudocode (schematic)

#include "src/gui.hpp"
#include "src/gui_manager.hpp"

class UnitWidget : public GUIElement {
public:
  UnitWidget(GUIManager* mgr) : GUIElement(), manager(mgr) {
    tex = manager->textureManager.loadTexture("assets/units/soldier.png");
  }
  void draw(SDL_Renderer* r) override {
    // draw current frame from tex
    SDL_Rect dst{static_cast<int>(pos.x), static_cast<int>(pos.y), w, h};
    // assuming tex contains atlas and getFrameRect method returns SDL_Rect
    auto frame = manager->textureManager.getFrameRect("soldier_walk", frameIndex);
    SDL_RenderCopy(r, tex.get(), &frame, &dst);
  }
  void onMouseDown(int sx, int sy) override {
    selected = true;
  }
  void moveTo(float x, float y) {
    target = {x,y};
    // start movement timer
    manager->timerManager.startTimer(this, 50, false, [this](int){ tickMove(); });
    // start walk animation
    manager->animationManager.animate(&frameIndex, frameIndex, 10, 300, easing::linear);
  }
private:
  void tickMove() {
    // simple movement step
  }
  GUIManager* manager;
  SharedTexture tex;
  int frameIndex = 0;
  bool selected = false;
  Vec2 pos, target;
};

// Registration:
auto unit = std::make_shared<UnitWidget>(&guiManager);
guiManager.addElement(unit);

## Practical Code Snippets and Usage Patterns

1) Creating a unit class inheriting from GUIElement
See example above and base [`src/gui.hpp`](src/gui.hpp:19).

2) Registering and using TextureManager and FontManager
auto& texMgr = guiManager.textureManager; // reference
auto tex = texMgr.loadTexture("assets/units/soldier.png");
auto& fontMgr = guiManager.fontManager;
auto font = fontMgr.loadFont("assets/fonts/font.ttf", 14);

3) Starting an animation via AnimationManager
manager->animationManager.animate(&pos.x, pos.x, dest.x, durationMs, easing::linear, on_complete);

4) Setting up and handling a timer via TimerManager
manager->timerManager.startTimer(this, 100, false, [this](int id){
  // tick logic
});

5) Handling click / drag selection and coordinate mapping
// on mouse down
int sx, sy; // screen coords
float worldX = (sx / camera.zoom) + camera.offsetX;
float worldY = (sy / camera.zoom) + camera.offsetY;
// then check if point belongs to element: element->contains(worldX, worldY)

## Optimizations and Limitations

- Caching (`m_cachedTexture`) and when to use `drawDirect`:
  - Use cache for static or infrequently changing widgets (map, HUD). Implementation in [`src/gui.cpp`](src/gui.cpp:135).
  - `drawDirect` when an element needs immediate drawing without copying to a texture.
- Texture Recommendations:
  - Texture sizes: keep them tightly constrained (mipmaping/texture size depends on platform).
  - Use atlases to minimize texture switching and draw calls.
  - Batch drawings when possible.
- Minimizing renderer target changes: creating/recreating target textures is costly — avoid this in the hot path.
- Tests and CI (headless): check [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1) — use Xvfb in CI.

## Debugging and Common Problems

- Failed texture creation:
  - Check SDL_LogError logs; check for returned nullptr from `SDL_CreateTexture`.
  - Ensure the renderer was created correctly before calling TextureManager.
- Missing fonts:
  - Check the path to `assets/fonts/font.ttf`. If an error occurs, FontManager should log the issue.
- SDL_image/SDL_ttf initialization errors:
  - Ensure `IMG_Init(...)` and `TTF_Init()` returned success, log appropriately.
- Caching problems:
  - If a widget does not refresh after data changes, check if `markDirty()` is called and if the size of `m_cachedTexture` matches the new size (see [`src/gui.cpp`](src/gui.cpp:220)).

## Checklists and Best Practices for the RTS Team

- Resource Naming:
  - assets/units/<unit_name>_walk_0.png, ... _n.png or atlas: assets/units/<unit_name>.atlas.png
- Separation of Game Logic from View:
  - Keep AI/pathfinding logic in independent classes/services; GUIElement should only visualize the state.
- Lifecycle: always mark an element for deletion via `markForDeletion()` instead of immediate delete; GUIManager will perform cleanup ([`src/gui.cpp`](src/gui.cpp:267) and [`src/gui_manager.cpp`](src/gui_manager.cpp:64)).
- Unit Tests: add tests for managers (TextureManager, FontManager, AnimationManager, TimerManager).

## Links to Reference Files in the Repository
- [`Makefile`](Makefile:1)
- [`src/gui_manager.hpp`](src/gui_manager.hpp:19)
- [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- [`src/gui.hpp`](src/gui.hpp:19)
- [`src/gui.cpp`](src/gui.cpp:135)
- [`src/texture_manager.hpp`](src/texture_manager.hpp:15)
- [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- [`src/font_manager.hpp`](src/font_manager.hpp:30)
- [`src/font_manager.cpp`](src/font_manager.cpp:7)
- [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
- [`src/timer_manager.hpp`](src/timer_manager.hpp:18)
- [`src/style.hpp`](src/style.hpp:17)
- [`src/theme.hpp`](src/theme.hpp:10)
- Examples: [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1), [`examples/example_panel.cpp`](examples/example_panel.cpp:1)
- Memory bank test strategy: [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1)

## Ready Example — Action Plan (Immediate Start Steps)

Quick plan (first sprint, 2 weeks, team of 3-5 people):
1. Resource Loader
   - Task: Implement asset loader and directory structure; provide fallbacks in TextureManager/FontManager.
   - References: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/font_manager.hpp`](src/font_manager.hpp:30)
2. Unit System (position, render, selection)
   - Task: UnitWidget, basic selection and rendering logic.
   - References: [`src/gui.hpp`](src/gui.hpp:19), [`examples/example_button.cpp`](examples/example_button.cpp:1)
3. Selection and Grouping
   - Task: Drag selection + shift+click.
4. Unit Movement and Simple Simulation (timer/animation)
   - Task: TimerManager tick for position updates, AnimationManager for state changes.
   - References: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
5. HUD and Basic Interface
   - Task: Resource panel, production buttons.
6. Minimap and Camera
   - Task: Minimap with direct drawing and camera zoom/pan.

Minimal Backlog Tasks (concrete stories):
- [ ] Prepare asset structure and example textures (soldier, building).
- [ ] Add UnitWidget example in `examples/` and an integration test for basic selection.
- [ ] Implement TimerManager-driven movement for units.
- [ ] Basic HUD with a few buttons and a resource counter.

## TODOs and Verification Points

- TODO: verification of exact asset publishing policy and example path `assets/fonts/font.ttf`.
- TODO: check exact cache implementation lines in [`src/gui.cpp`](src/gui.cpp:135) and update this document if the implementation changes.

------------------------------------------------------------

Document prepared as a complete startup guide for the RTS team using SDL GUI. All references to source files and examples are provided as links to the repository.

End of document.