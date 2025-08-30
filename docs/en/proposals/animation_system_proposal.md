# Animation System Proposal for the GUI Library

[This page is also available in Polish](../../pl/proposals/animation_system_proposal.md)

[Back to Feature Proposals](../feature_proposals.md)

## Introduction

To implement smooth, time-driven animations (e.g., color transitions, movement, resizing), the library needs a dedicated system that updates properties frame-by-frame in sync with the main loop. The current [TimerManager](../../src/timer_manager.hpp:18) targets one-shot or periodic “every N ms” callbacks, not continuous per-frame updates.

This document proposes the architecture of such a system.

## Proposed Changes

### 1) Easing functions

Natural, non-linear animations require standard easing curves.

- Action: add a new header [easing.hpp](../../src/easing.hpp:1).
- Content: a set of functions such as `linear`, `easeInQuad`, `easeOutQuad`, `easeInOutQuad`, each taking a normalized progress t in [0.0, 1.0] and returning a transformed progress.

### 2) AnimationManager

The core of the system that manages the lifecycle of all active animations.

- Action: create class `AnimationManager` in [animation_manager.hpp](../../src/animation_manager.hpp:24) and [animation_manager.cpp](../../src/animation_manager.cpp:1).
- Internal struct Animation (example):
  - `std::function<void(float)> update_callback` — called every frame with eased progress.
  - `Uint32 start_time`, `Uint32 duration_ms` — start time and total duration.
  - `std::function<float(float)> easing_function` — easing curve.
  - `bool is_finished` — completion flag.
- Logic of `AnimationManager::update()`:
  1. Iterate all active animations.
  2. Compute raw progress `raw = clamp((now - start_time) / duration_ms, 0.0f, 1.0f)`.
  3. Apply `easing_function(raw)`.
  4. Invoke `update_callback(progress)`.
  5. Mark finished animations and remove them.

Additionally provide:
- Add-by-params API that returns a handle or id.
- Cancel by id.
- Cancel all animations bound to a given owner (e.g., GUIElement*).

### 3) Integration with GUIManager

The [GUIManager](../../src/gui_manager.hpp:19) owns the animation manager and updates it in the main loop.

- Changes:
  - Add `std::unique_ptr<AnimationManager> m_animationManager;`
  - Initialize in the constructor.
  - Call `m_animationManager->update()` in [GUIManager::cleanup()](../../src/gui_manager.cpp:14) so animations advance each tick.
  - Add `AnimationManager* getAnimationManager()` (or a reference) for external access.

## Usage Example

Creating a color transition animation in application code:

```cpp
// Assume we have a Panel-derived element and a guiManager reference
SDL_Color start_color = { 40, 40, 40, 255 };
SDL_Color end_color   = { 200, 200, 255, 255 };

Panel& panel_ref = *myPanel;

guiManager.getAnimationManager()->addAnimation(
    // update_callback: progress in [0..1] after easing
    [=, &panel_ref](float progress) {
        Uint8 r = static_cast<Uint8>(start_color.r + (end_color.r - start_color.r) * progress);
        Uint8 g = static_cast<Uint8>(start_color.g + (end_color.g - start_color.g) * progress);
        Uint8 b = static_cast<Uint8>(start_color.b + (end_color.b - start_color.b) * progress);
        panel_ref.setBackgroundColor({r, g, b, 255});
        panel_ref.markDirty(); // request cache refresh
    },
    2000,                   // duration_ms
    Easing::easeInOutQuad   // easing function
);
```

API notes:
- Consider `addAnimationFor(GUIElement*, ...)` to bind lifecycle to an owner. When the element is removed, its animations are automatically cancelled.
- Alternatively, accept an “owner id” or weak_ptr to validate ownership.

## Implementation Notes

- Safety and lifecycle:
  - Prefer binding animations to an owner (e.g., `GUIElement*`) so they can be cancelled on destruction.
  - Avoid storing long-lived raw pointers in callbacks; validate ownership (e.g., weak_ptr) where applicable.
- Performance:
  - Keep `update()` lightweight; iterate a short list and erase finished in one pass.
  - Avoid allocations inside the update loop.
- Separation of concerns:
  - `AnimationManager` must remain rendering-agnostic. It only manages time and progress and invokes user callbacks.

## Summary

- Introduce `AnimationManager` for smooth, time-based animations with easing.
- Integrate it into the [GUIManager](../../src/gui_manager.hpp:19) lifecycle via [GUIManager::cleanup()](../../src/gui_manager.cpp:14).
- Provide a simple API to animate arbitrary properties (color, position, size, etc.) with safe lifecycle management and extensible easing curves.