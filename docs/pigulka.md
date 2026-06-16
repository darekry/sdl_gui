# SDL GUI — ściąga

**SDL3 + C++23 (clang++-22, libc++)** • [examples/](../examples/)

## Start

```bash
cc -o nob nob.c && ./nob examples
```

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
int main() {
    SDLApp app("Tytuł", 800, 600);
    GUIManager gui(app.getRenderer()); gui.setWindowSize(800, 600);
    bool quit = false; SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) { if (e.type == SDL_EVENT_QUIT) quit = true; gui.processEvent(e); }
        gui.update(); gui.cleanup();
        SDL_SetRenderDrawColor(app.getRenderer(), 40,40,40,255); SDL_RenderClear(app.getRenderer());
        gui.render(); SDL_RenderPresent(app.getRenderer());
    }
}
```

Widgety: `std::make_unique<Widget>(gui, x, y, w, h, ...)` → `gui.addElement(std::move(...))`.

## Widgety

| Widget | Sygnatura / kluczowe API | Przykład |
|--------|--------------------------|----------|
| **Panel** | `(gui,x,y,w,h)` `.setDraggable()`, `.addChild()` | `example_panel` |
| **Label** | `(gui,x,y,"tekst",fontSize)` `.setText()` | `example_button` |
| **Button** | `(gui,x,y,w,h,"tekst")` `.setOnClickCallback([](GUIElement*){})` | `example_button` |
| **Checkbox** | `(gui,x,y,w,h)` `.setChecked()`, `.setOnChange()` | `example_checkbox` |
| **RadioGroup** | `(gui,x,y,w,h)` `.addOption()`, `.getSelectedButton()` | `example_radio_button` |
| **Slider** | `(gui,x,y,w,h,min,max,val,Orient)` `.getValue()/.setValue()`, `setWheelStep()` | `example_slider` |
| **TextInput** | `(gui,x,y,w,h)` `.getText()/.setText()`, `setLocked()`, `setOnEnterPressed()` | `example_text_input` |
| **TextArea** | `(gui,x,y,w,h,font,size)` `.setWordWrap()`, `setLocked()` | `example_text_area` |
| **ComboBox** | `(gui,x,y,w,h)` `.addItem()`, `.setSelectedIndex()`, `.on_selection_changed` | `example_combobox` |
| **TabControl** | `(gui,x,y,w,h,tabH)` `.addTab("nazwa")` → zwraca `Panel*` | `example_tabs` |
| **StringGrid** | `(gui,x,y,w,h,rows,cols)` `.setCellText()`, `.sortByColumn()`, `.setEditable()` | `example_string_grid` |
| **ListView** | `(gui,x,y,w,h)` `.addItem()`, `.setOnRowClick()`, `.getSelectedRow()` | `example_list_view` |
| **ProgressBar** | `(gui,x,y,w,h,min,max,val)` `.setValue()`, `.setShowPercentage()` | `example_progress_bar` |
| **ScrollArea** | `(gui,x,y,w,h)` `.setContent(std::move(elem))` | `example_scroll_area` |
| **ArcContainer** | `(gui,x,y,r,startAngle,endAngle)` dzieci po łuku | `example_arc_container` |
| **Canvas** | `(gui,x,y,w,h)` `.clear()` — rysowanie | `example_paint` |
| **AnimatedImage** | `(gui,x,y,w,h)` `.setSpriteSheet()`, `.play()`, `.setFPS()` | `example_animated_image` |
| **ShaderPanel** | `(gui,x,y,w,h,vertSrc,fragSrc)` GPU | `example_gpu_shader` |
| **ContextMenu** | `(gui)` `.addItem()`, `.showAt(mx,my)` | `example_context_menu` |

## Stylowanie

```cpp
elem->setBackgroundColor(ElementState::Normal, {200,200,200,255});
elem->setBorder(ElementState::Normal, {100,100,100,255}, 2);
```

Stany: `Normal`, `Hover`, `Pressed`, `Disabled`. Pełny `Style` → `.setStyle(state, style)`. Motyw: `gui.setTheme(Theme::createDefaultTheme())` — `example_themes`, `example_theme_playground`.

## Anchor — responsywny layout

```cpp
Anchor a; a.top = 0.1f; a.left = 10; a.right = 10; a.bottom = 50; // <1=%, >1=px
elem->setAnchor(a);  // presety: Anchor::fill(), center(), bottomBar(), leftSidebar()...
gui.handleResize(newW, newH);  // przy resize
```
→ `example_resize`

## Kompozyty — gotowe dialogi

```cpp
DialogBox::createConfirm(gui, "Na pewno?", "Tak", "Nie", [](bool ok){});
DialogBox::createAlert(gui, "Gotowe.", "OK", [](int){});
MessageBox::showInfo(gui, "Zapisano.");     // też Error, Warning
MessageBox::showQuestion(gui, "Kontynuować?", onYes, onNo);
FileDialog::createOpen(gui, "Otwórz", [](const std::string& path){});
FileDialog::createSave(gui, "Zapisz", "plik.txt", [](const std::string& path){});
```
→ `example_dialog`, `example_file_dialog`

## Parsery — GUI z plików

```cpp
JsonParser(gui).loadLayout("layout.json");  // example_json_parser
SGMLParser(gui).loadLayout("layout.xml");   // example_xml_parser
```

## WindowManager — wiele okien

```cpp
WindowManager wm; Window* w = wm.createWindow("Tytuł", 800, 600);
w->getGUIManager().addElement(...);
while (!wm.shouldQuit()) { wm.processEvents(); wm.updateAll(); wm.renderAll(); wm.cleanupAll(); }
```
→ `example_window_manager`, `example_window`

## ScreenManager — ekrany (gry)

```cpp
ScreenManager sm(gui); sm.addScreen("menu", std::make_unique<MenuScreen>());
sm.changeScreen("menu");  // pushScreen/popScreen dla overlay
```
→ `example_screen_manager`

## Wskazówki

- **Cache**: widgety renderują do tekstury; `.markDirty()` wymusza przerysowanie.
- **Dzieci**: `.addChild()` — przycinane do rodzica, usuwane razem z nim.
- **Callbacki**: `std::function`, uwaga na dangling — elementy to `unique_ptr`.
- Tooltip: `.setTooltip()`, kursory: `CursorManager`, edytor: `example_wysiwyg_editor`.
- Linkowanie: `-lSDL3 -lSDL3_image -lSDL3_ttf`.
