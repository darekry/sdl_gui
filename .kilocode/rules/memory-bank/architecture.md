# Architektura SDL GUI

## Core

| Component | Role | Files |
|-----------|------|-------|
| **GUIManager** | Central controller, resource managers, event/render loop | gui_manager.hpp/cpp |
| **GUIElement** | Abstract base, hierarchy, texture cache | gui.hpp/cpp |
| **TextEditable** | Abstract base for TextInput/TextArea - selection, clipboard | text_editable.hpp/cpp |

## Widgety (16)

| Category | Widgets |
|----------|---------|
| Containers | Panel, TabControl |
| Input | Button, Checkbox, RadioButton, RadioGroup, Slider, TextInput, TextArea, ComboBox |
| Display | Label, StringGrid, ListView |
| Graphics | AnimatedImage, Canvas |
| Menu | ContextMenu |

## Composite (src/composite/)

DialogBox (draggable, factory methods), MessageBox (showInfo/Error/Warning/Question)

## Screen/Window Systems

| System | Purpose | Files |
|--------|---------|-------|
| ScreenManager | Single window, multiple screens (games) | screen.hpp, screen_manager.hpp/cpp |
| WindowManager | Multiple system windows (desktop) | window.hpp, window_manager.hpp/cpp |

## Resource Managers

TextureManager (SharedTexture cache), FontManager (SharedFont cache), TimerManager, AnimationManager

## Parsers

JsonParser, SGMLParser (define GUI from JSON/XML), LayoutParser (interface)

## Style & Theme

- Style: optional fields (backgroundColor, textColor, borderColor, borderRadius, fontSize)
- Theme: per-type, per-state storage (`map<string, map<ElementState, Style>>`)
- Composition: local[state] → theme[type][state] → theme[type][Normal] → default

## Anchor (Responsive Layout)

- Coordinates: -1=unset, 0-1=%, >1=px, 0.5=center
- Stretch: both edges set → element stretches
- Presets: center(), fill(), topLeft(), bottomBar(), leftSidebar()
- Resize: handleParentResize() updates position/size

## Render Flow

1. GUIManager::render() → GUIElement::render()
2. wantsDirectRender()? drawDirect() : renderToCache() → m_cachedTexture
3. Children rendered with parent_clip_rect

## Memory

- Elements: unique_ptr hierarchy
- SDL resources: SharedTexture/SharedFont (shared_ptr with custom deleters)
- Render cache: m_cachedTexture invalidated by m_isDirty