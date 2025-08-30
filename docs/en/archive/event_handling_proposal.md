# Event Handling Refactor Proposal
[This page is also available in Polish](../../pl/archive/event_handling_proposal.md)
[Back to Archive](./README.md)

Date: 2025-07-06

## Introduction

The current event handling in the library swallows unhandled events inside the GUI layer. Historically, logic like `GUIManager::handleEvents()` would poll SDL events internally and only signal application shutdown (`SDL_QUIT`) via a boolean flag. This prevents the host application from reacting to events that no GUI element consumed (e.g., global keyboard shortcuts, custom system events).

This document proposes a refactor where the application owns the SDL event polling loop and forwards events to the GUI one-by-one. The GUI reports whether the event was consumed.

References:
- [GUIManager](../../src/gui_manager.hpp)
- Event API suggestion: [GUIManager::processEvent()](../../src/gui_manager.hpp:19)
- GUI elements: [GUIElement::handleEvent()](../../src/gui.hpp:19)

## 1. Problem: Swallowing unhandled events

In the legacy model, `GUIManager::handleEvents()` loops `SDL_PollEvent`, distributes to children, and discards anything unhandled. The application never sees those events unless they are `SDL_QUIT`. This design couples the GUI event loop with the app event loop and blocks global behaviors at the app level.

## 2. Proposed Solution

Shift event-loop ownership to the application. The GUI receives one `SDL_Event` at a time and returns whether it consumed the event. The app decides what to do with unhandled events.

### 2.1. Method signature

- Introduce `bool GUIManager::processEvent(const SDL_Event& e)` that returns `true` if any GUI element consumed the event, `false` otherwise.
- This aligns with the current naming used across docs and architecture.

Sketch:

```cpp
// gui_manager.hpp
// bool GUIManager::processEvent(const SDL_Event& e);

// gui_manager.cpp
bool GUIManager::processEvent(const SDL_Event& e) {
    // Iterate top-level elements; let them try to handle the event
    for (const auto& element : m_elements) {
        if (element && element->handleEvent(e)) { // GUIElement::handleEvent(...)
            return true; // Event consumed by GUI
        }
    }
    return false; // Not consumed by GUI
}
```

Notes:
- This relies on each element implementing `bool GUIElement::handleEvent(const SDL_Event&)` and returning whether the event was handled.
- See: [GUIElement::handleEvent()](../../src/gui.hpp:19)

### 2.2. Main application loop owns SDL_PollEvent

Move the `SDL_PollEvent` loop to the host application. First forward the event to the GUI, then handle the remainder at the app level.

Example:

```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    while (SDL_PollEvent(&e)) {
        // 1) Let GUI try to consume the event
        const bool handledByGUI = guiManager.processEvent(e);

        // 2) If GUI did not consume it, app handles it
        if (!handledByGUI) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // Example: global shortcuts, application-level commands, etc.
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F5) {
                // Reload data / refresh configuration
            }
        }
    }

    // Update/cleanup frame
    guiManager.cleanup();

    // Render frame
    SDL_RenderClear(renderer);
    guiManager.render();
    SDL_RenderPresent(renderer);
}
```

Benefits:
- Clear separation of concerns: GUI processes GUI-related events; the app manages everything else.
- Better extensibility: add application-wide features without changing GUI internals.
- Matches common patterns in other frameworks.

## 3. Migration and Compatibility

To minimize churn:

- Keep a deprecated wrapper for one release:
  - `bool GUIManager::handleEvents()` (deprecated) can internally poll events and forward each to [GUIManager::processEvent()](../../src/gui_manager.hpp:19). Mark it deprecated and document that applications should own the poll loop.
- Update examples to the new pattern:
  - Replace internal polling with an application-owned `SDL_PollEvent` loop.
  - This also simplifies testing and integration with non-GUI subsystems.
- Tests:
  - Unit/integration tests that previously relied on `handleEvents()` should switch to the single-event flow via `processEvent(...)`.

## 4. Summary

- New canonical API: [GUIManager::processEvent()](../../src/gui_manager.hpp:19) receives a single `SDL_Event` and returns whether it was consumed.
- The application owns the `SDL_PollEvent` loop and decides what to do with unhandled events.
- Improves flexibility, separation of concerns, and aligns with established practices.