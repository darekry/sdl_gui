# RTS Game Architecture with SDL GUI

Reference for building real-time strategy games. Read together with
`dist/docs/managers.md` (ScreenManager), `dist/docs/patterns.md` and the
widget docs you use. All code below uses only `sdl_gui.hpp`; the game world is
drawn with plain SDL3 calls on the same renderer.

## Division of labor: game world vs. GUI

The library renders **widgets only**. The game world (map, units, particles,
selection box) must be drawn by hand with SDL3 API on the same
`SDL_Renderer*`. The render order decides what is on top:

1. `SDL_RenderClear(renderer)` — clear background,
2. raw SDL3 drawing of the world (`SDL_RenderTexture`, `SDL_RenderGeometry`,
   `SDL_RenderRect`, ...),
3. `manager.render()` — GUI (HUD, menus) on top,
4. `SDL_RenderPresent(renderer)`.

Because widgets render into cached textures, an unchanged HUD costs almost
nothing per frame; only world drawing runs every frame.

## Screen flow (menu / gameplay / pause)

Use `ScreenManager` with one `Screen` per phase. Create all GUI elements in
`Screen::onEnter`, remove them in `onExit` via `markForDeletion()`
(actual removal happens in `manager.cleanup()`).

```cpp
class GameScreen : public Screen {
public:
    std::string getName() const override { return "Game"; }

    void onEnter(GUIManager& manager) override {
        // HUD: top bar with resources
        m_bar = manager.create<Panel>(0, 0, 0, 40);
        m_bar->setAnchor(Anchor::topBar(0, 0, 0));
        // Label has no rect constructor, so it becomes a child via addChild;
        // create the ElementRef BEFORE transferring ownership:
        auto gold = std::make_unique<Label>(manager, 10, 10, "Gold: 100");
        m_gold = gold.get();
        m_goldRef = manager.makeRef(m_gold);
        m_bar->addChild(std::move(gold));
        // ... more widgets ...
    }

    void onExit(GUIManager&) override {
        // remove everything this screen created (actual removal happens
        // in manager.cleanup() later in the frame):
        if (m_bar) m_bar->markForDeletion();
        m_bar = nullptr;
        m_gold = nullptr;
    }

    bool handleEvent(GUIManager&, const SDL_Event& e) override {
        // game input: camera pan, selection box, keyboard shortcuts
        // return true when the event is consumed by the game
        return m_world.handleEvent(e);
    }

    bool wantsPreProcessEvent() const override { return true; }
    // ^ consume events BEFORE GUI: clicking the map must not also click HUD

    void update(GUIManager& manager) override {
        m_world.update();   // fixed timestep logic, see below
    }

    void render(GUIManager&, SDL_Renderer* renderer) override {
        m_world.render(renderer);   // raw SDL3 drawing (map, units)
    }

private:
    World m_world;
    Panel* m_bar = nullptr;
    Label* m_gold = nullptr;
    ElementRef<Label> m_goldRef;
};
```

Wire-up:

```cpp
GUIContext ctx("RTS", 1280, 720);
GUIManager& manager = ctx.getGUIManager();
ScreenManager screens(manager);
screens.addScreen("menu", std::make_unique<MenuScreen>());
screens.addScreen("game", std::make_unique<GameScreen>());
screens.addScreen("settings", std::make_unique<SettingsScreen>());
screens.changeScreen("menu");
```

Main loop (manual — the game needs control over rendering):

```cpp
bool quit = false;
SDL_Event e;
while (!quit) {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) quit = true;
        screens.handleEvent(e);    // game input first (wantsPreProcessEvent)
        manager.processEvent(e);   // then GUI input
    }
    screens.update();              // game logic (Screen::update)
    manager.update();              // timers, animations, tooltips
    manager.cleanup();
    SDL_SetRenderDrawColor(ctx.getRenderer(), 0, 0, 0, 255);
    SDL_RenderClear(ctx.getRenderer());
    screens.render(ctx.getRenderer());   // world (raw SDL) — or draw before manager.render()
    manager.render();                    // GUI on top
    SDL_RenderPresent(ctx.getRenderer());
}
```

If world drawing lives in `Screen::render()`, call `screens.render()` BEFORE
`manager.render()` so the GUI stays on top.

### Pause and settings overlays

`pushScreen("pause")` overlays the pause screen while the game screen stays in
the background (it receives no events — no need to freeze it manually).
`popScreen()` returns to the game. The pause screen renders on top because
`ScreenManager::render()` draws the stack bottom-up; give pause/settings
screens a semi-transparent `Panel` background (`{0, 0, 0, 180}`).

## Game loop: fixed timestep

Drive simulation with a fixed step and an accumulator; keep GUI updates in
sync:

```cpp
class World {
    static constexpr double kStepMs = 16.666;  // 60 Hz
    double m_accumulator = 0.0;
    uint64_t m_lastTick = SDL_GetPerformanceCounter();

public:
    void update() {
        uint64_t now = SDL_GetPerformanceCounter();
        double elapsed = (now - m_lastTick) * 1000.0 / SDL_GetPerformanceFrequency();
        m_lastTick = now;
        m_accumulator += std::min(elapsed, 250.0);  // clamp: no spiral of death
        while (m_accumulator >= kStepMs) {
            tick();                 // one simulation step (units, economy, AI)
            m_accumulator -= kStepMs;
        }
    }
};
```

For one-shot delayed game events (unit production, cooldowns) use
`TimerManager::addTimer(element, delayMs, /*singleShot=*/true, cb)` — it
fires during `manager.update()`. For repeated ticks use `singleShot=false`.
`AnimationManager::createAnimation` animates `int*`/`float*` properties with
easing — note it holds a RAW pointer to the property: ensure the owning
object outlives the animation.

## Input

- `Screen::handleEvent` receives every `SDL_Event` when the screen is
  active. Handle `SDL_EVENT_MOUSE_MOTION`, `SDL_EVENT_MOUSE_BUTTON_DOWN/UP`
  for camera pan and rubber-band selection (draw the box in `World::render`).
- Override `wantsPreProcessEvent()` returning `true` when the game must see
  clicks before GUI widgets (map clicks vs. HUD buttons).
- Keyboard: `SDL_EVENT_KEY_DOWN` (arrows/WASD for camera, hotkeys for
  building); GUI keyboard focus is separate — buttons handle their own keys.
- Gamepad: open with `SDL_OpenGamepad(deviceID)` on
  `SDL_EVENT_GAMEPAD_ADDED`, read axes/buttons in `Screen::handleEvent`;
  see `examples/43_gamepad_controller.cpp` for a D-pad grid + focus pattern.

- On `SDL_EVENT_WINDOW_RESIZED` call `manager.handleResize(w, h)` so anchors
  (HUD) reposition.

## HUD patterns

- Resource bar: `Panel` with `Anchor::topBar(h, margin, margin)`, children
  `Label`s positioned relative to the panel. Update values via `ElementRef`
  captured before `std::move`.
- Minimap: a small `Panel` or a custom widget — the simplest approach is a
  raw SDL texture/drawing updated when the world changes, blitted in
  `World::render` under the GUI layer.
- Selection info: `Label` under the top bar, updated on selection change —
  do not update every frame.
- Build menu / unit buttons: `Button` grid inside a `Panel`; each button's
  callback triggers a build order. Use `setTooltip` on buttons for hints.
- Unit health bars: draw with `SDL_RenderRect` in `World::render` (world
  layer) — not with `ProgressBar` (a widget).
- Production queue / event log: `ListView` (append-only rows) or `TextArea`.

## Dialogs

```cpp
MessageBox::showQuestion(manager, "Quit to menu?",
                         [] { /* yes: screens.changeScreen("menu") */ });
// full custom dialog:
auto dlg = DialogBox::createWithTitle(manager, "Settings", "Pick speed",
                                      {"1x", "2x", "4x"}, [](int idx) { /* ... */ });
manager.addElement(std::move(dlg));
```

Dialogs are overlays: GUI outside them is inert while open. Do not keep the
dialog pointer in callbacks — it is marked for deletion on button click.

## Performance checklist

- Keep the HUD small and static; update labels only when values change
  (`setText` marks dirty automatically).
- The world is drawn raw every frame — batch geometry with
  `SDL_RenderGeometry` for many units; avoid per-unit `SDL_RenderTexture`
  when possible.
- Do not create/destroy widgets per frame; create in `onEnter`, reuse.
- `AnimationManager` for UI effects; the game sim runs on the fixed step.
- Full-screen shader effects (fog of war, bloom): `ShaderPanel` requires the
  GPU variant `SDLApp(title, w, h, false, GPU_VULKAN)` — set per-frame
  uniforms with `setUniformTime(float)` / `setUniformMouse(x, y)`.
