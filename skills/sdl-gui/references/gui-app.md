# GUI Application Architecture with SDL GUI

Reference for building desktop tools, editors, dashboards and form
applications. Read together with `dist/docs/getting_started.md`,
`dist/docs/core.md`, `dist/docs/patterns.md` and `dist/docs/composites.md`.
All code below uses only `sdl_gui.hpp`.

## Standard layout: resizable single window

The typical tool shell: menu bar on top, navigation sidebar on the left,
content area in the middle, status bar at the bottom. Anchors keep it
responsive:

```cpp
SDLApp app("Tool", 1024, 768, /*resizable=*/true);
GUIManager manager(app.getRenderer());
manager.setTheme(ThemePresets::createDarkTheme());
manager.setWindowSize(1024, 768);

auto toolbar = manager.create<Panel>(0, 0, 0, 0);
toolbar->setAnchor(Anchor::topBar(48, 0, 0));            // full width, 48 px

auto sidebar = manager.create<Panel>(0, 0, 0, 0);
sidebar->setAnchor(Anchor::leftSidebar(200, 48, 24));    // below toolbar, above status

auto content = manager.create<Panel>(0, 0, 0, 0);
content->setAnchor(Anchor::fill(0));                     // whatever remains

auto statusBar = manager.create<Panel>(0, 0, 0, 0);
statusBar->setAnchor(Anchor::bottomBar(24, 0, 0));

app.run(manager, {40, 42, 54, 255}, [&manager](SDL_Event& e) {
    if (e.type == SDL_EVENT_WINDOW_RESIZED) {
        manager.handleResize(e.window.data1, e.window.data2);
    }
});
```

Anchor presets: `center()`, `fill(m)`, `topBar(h, l, r)`, `bottomBar(h, l, r)`,
`leftSidebar(w, t, b)`, `rightSidebar(w, t, b)`, `horizontalStretch(l, r)`,
`verticalStretch(t, b)`, corners with margins. Value convention: `0–1` =
fraction of parent, `>1` = pixels, `0.5` = center. Create children of a panel
with `manager.create<Label>(parent, x, y, ...)` — coordinates are relative to
the parent.

## Widget selection by use case

| Use case | Widget | Doc |
|----------|--------|-----|
| Actions | `Button` | widgets/Button.md |
| Containers / grouping | `Panel` (children clipped, optionally draggable) | widgets/Panel.md |
| Static text / titles | `Label` | widgets/Label.md |
| One-line input | `TextInput` (Enter, input lock, focus) | widgets/TextInput.md |
| Multi-line text / logs | `TextArea` | widgets/TextArea.md |
| On/off options | `Checkbox`, `RadioButton` + `RadioGroup` | widgets/Checkbox.md |
| Numeric options | `Slider`, `RangeSlider` | widgets/Slider.md |
| Single choice from a list | `ComboBox` (callback: public field `on_selection_changed`) | widgets/ComboBox.md |
| Item lists (files, logs) | `ListView` (`addItem`, `setOnRowClick`, `setOnRowActivate`) | widgets/ListView.md |
| Tables | `StringGrid` (sorting, selection, cell editing) | widgets/StringGrid.md |
| Tabs (settings pages) | `TabControl` | widgets/TabControl.md |
| Scrollable content | `ScrollArea` (hosts any widget as content) | widgets/ScrollArea.md |
| Progress | `ProgressBar` | widgets/ProgressBar.md |
| Right-click menus | `ContextMenu` | widgets/ContextMenu.md |
| Radial menus / dashboards | `ArcContainer` (children arranged on an arc) | widgets/ArcContainer.md |
| Sprites / loading animations | `AnimatedImage` (sprite sheets) | widgets/AnimatedImage.md |
| Drawing / annotation | `Canvas` (pen color, clear) | widgets/Canvas.md |

## Form pattern

Labels + inputs + a submit button; validate on click:

```cpp
auto nameInput = manager.create<TextInput>(300, 60, 200, 30);

auto okBtn = manager.create<Button>(300, 100, 120, 32, "Save");
okBtn->setOnClickCallback([nameInput](GUIElement*) {
    std::string name = nameInput->getText();     // raw pointer is fine:
    // nameInput is top-level and never deleted before the app exits
});
```

Rules of thumb:

- Capture raw pointers only for widgets whose lifetime you control; otherwise
  create `ElementRef` before `std::move` and check `if (ref)`.
- `ComboBox::setSelectedIndex` fires `on_selection_changed` — centralize
  handling there.
- A `ComboBox` inside a clipped/scrollable parent can be cut off; keep it
  top-level or inside a plain panel.

## Dialogs and file access

```cpp
MessageBox::showInfo(manager, "Saved successfully.");
MessageBox::showError(manager, "Cannot open the file.");

MessageBox::showQuestion(manager, "Save changes?", [] { /* yes */ }, [] { /* no */ });

auto dlg = DialogBox::createWithTitle(manager, "Export", "Choose format",
                                      {"CSV", "JSON", "Cancel"},
                                      [](int idx) { /* 0, 1 or 2 */ });
manager.addElement(std::move(dlg));

FileDialog* fd = FileDialog::createOpen(manager, "Open layout",
    [](const std::string& path) { /* load path */ },
    ".", "*.json");
```

- `DialogBox`/`FileDialog` must be added via `manager.addElement(...)`;
  `MessageBox` adds itself. All are overlays — GUI behind them is inert.
- `FileDialog` callback receives the full path; the dialog closes itself.
  Do not store the returned pointer beyond the dialog's lifetime.
- Dialog buttons mark the dialog for deletion — use `ElementRef` if the
  callback must touch other widgets.

## Theming

```cpp
manager.setTheme(ThemePresets::createDarkTheme());   // or Win9x/Light/HighContrast

// per-type tweaks on top of the preset:
Theme& theme = manager.getTheme();
Style s;
s.backgroundColor = {200, 80, 80, 255};
theme.setStyle("Button", ElementState::Hover, s);
```

Widget styles cascade: local style → theme[type][state] → theme[type][Normal]
→ default. Use the convenience setters (`setBackgroundColor`,
`setBorder`, `setBorderRadius`, `setTextColor`) — they mark the widget dirty
automatically. Direct `Style` field mutation requires `markDirty()`.

## Data-centric tools

- Show file lists in `ListView` with double-click activation
  (`setOnRowActivate`); populate/refresh with `clearItems()` + `addItem()`.
- Tables with `StringGrid` — per-cell text, sorting, editable cells.
- Event logs: `TextArea` (append) or `ListView` with auto-scroll.
- Long-running tasks: `ProgressBar` driven by `TimerManager::addTimer(...,
  singleShot=false, ...)` or `AnimationManager` cycling callbacks.
- Layouts defined in files: `JsonParser`/`SGMLParser` return a widget tree
  from a JSON/XML file — `manager.addElement(std::move(layout))`.
  See `dist/docs/resources.md`.

## Multiple windows

`WindowManager` runs several independent windows, each with its own
`GUIManager`:

```cpp
WindowManager wm;
Window* mainWin = wm.createWindow("Main", 1024, 768, /*resizable=*/true);
mainWin->getGUIManager().setTheme(ThemePresets::createDarkTheme());
auto btn = mainWin->getGUIManager().create<Button>(10, 10, 120, 40, "OK");

while (wm.hasOpenWindows()) {
    wm.processEvents();   // routes events by windowID
    wm.updateAll();
    wm.renderAll();
    wm.cleanupAll();
}
```

Use `Window::setOnResizeCallback` to call `handleResize` on the window's
`GUIManager`. Prefer a single resizable window with anchors for most tools;
`WindowManager` is for genuinely separate windows (main + inspector).

## C API

If the application mixes C: the whole API is exposed in `sdl_gui.h`
(`sdlgui_ctx_t`, `sdlgui_*_create`, ...) — compile with
`gcc -std=c11 -pedantic-errors -I dist -c`, link with `clang++ -lsdl_gui`.
See `dist/docs/c_api.md`.
