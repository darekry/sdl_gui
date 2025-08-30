# Feature Proposals for the GUI Library

This page is also available in Polish: [`docs/pl/feature_proposals.md`](../pl/feature_proposals.md)

The list below gathers proposed new features and improvements for the library, including user requests and additional ideas.

## User Proposals

1.  Clipboard support (paste text): enable pasting into `TextInput` / `TextArea` using SDL clipboard APIs (`SDL_SetClipboardText()`, `SDL_GetClipboardText()`).
2.  Context menu: a widget that appears on right‑click over an element that has a defined menu. It should be fully configurable and execute actions attached to entries. [IMPLEMENTED]
3.  “Open‑up” Combobox: extend `Combobox` so it automatically opens upward if there’s no room below within its parent container.

## Additional Proposals

1.  Layout Managers:
    - Stack Layout — vertically or horizontally stacks children and auto‑manages their positions.
    - Grid Layout — arranges elements in a configurable grid of rows and columns.
    - Simplifies building complex UIs without manual coordinate calculations.

2.  Tooltips: small informational popups when the mouse hovers and pauses over an element. [IMPLEMENTED]

3.  File/Directory chooser dialog: a standard, built‑in widget to browse/select files or folders from the filesystem.

4.  Data Binding: bind widget properties (e.g., text in `TextInput`, `Checkbox` state) directly to variables in application code. Changes in the UI automatically update the variable and vice versa.

5.  Simple animations/transitions: support for smooth transitions such as fade‑in/fade‑out of windows or subtle color changes on hover to make the UI feel more “alive”.

### Detailed proposal documents

- Animation System Proposal — architecture and API: [docs/en/proposals/animation_system_proposal.md](proposals/animation_system_proposal.md)
- GUI Styling System Architecture — theme-driven styling: [docs/en/proposals/styling_system_architecture.md](proposals/styling_system_architecture.md)

---

## 2. Recommended Alternative: Internal Time Event Manager [IMPLEMENTED — as TimerManager]

- Description: introduce a dedicated class (e.g., TimeEventManager) integrated with `GUIManager`. The manager keeps a queue of scheduled events. In each frame of the main loop, `GUIManager` calls `TimeEventManager::update(deltaTime)`. The manager checks which events should fire and executes their callbacks directly on the main thread.
- Benefits:
  - Full control and safety: everything runs on the main (UI) thread; no multithreading pitfalls.
  - Lifecycle safety: events can be tied to GUI elements owned by `GUIManager`. If an element is removed, associated time events can be safely canceled.
  - Flexibility: easy to extend with recurring events, frame‑based animations, grouping/cancelation.
  - Low overhead: straightforward implementation without heavy synchronization.

### Comparison and Recommendation

Recommendation: implement the internal Time Event Manager. It is safer, simpler to maintain, and more flexible, aligning well with the `GUIManager`‑centric design and avoiding typical multithreading pitfalls.

```mermaid
sequenceDiagram
    participant App as Application
    participant GM as GUIManager
    participant TEM as TimeEventManager
    participant Element as GUIElement

    Note over App: Main application loop
    App->>App: Compute deltaTime
    App->>GM: update(deltaTime)
    GM->>TEM: update(deltaTime)

    Note over TEM: Check if any event is due
    alt Event to execute
        TEM->>Element: execute_action()
        Note over Element: e.g., show tooltip, change color
    end

    Note over App: End update, start rendering
    App->>GM: render()