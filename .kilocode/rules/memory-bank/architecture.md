# Architektura SDL GUI

## Core

| Component | Role | Files |
|-----------|------|-------|
| **GUIManager** | Central controller, resource managers, event/render loop, resize handling | gui_manager.hpp/cpp |
| **GUIElement** | Abstract base, hierarchy, texture cache, anchor system | gui.hpp/cpp |
| **TextEditable** | Abstract base for TextInput/TextArea - selection, clipboard | text_editable.hpp/cpp |
| **Anchor** | Responsive layout (%, px, stretch, center) | anchor.hpp |

## Widgety (21)

| Category | Widgets |
|----------|---------|
| Containers | Panel, TabControl, ScrollArea, ArcContainer |
| Input | Button, Checkbox, RadioButton, RadioGroup, Slider, TextInput, TextArea, ComboBox |
| Display | Label, StringGrid, ListView, ProgressBar |
| Graphics | AnimatedImage, Canvas, ShaderPanel |
| Menu | ContextMenu, Cursor |

## Composite (src/composite/)

DialogBox (draggable, factory methods), MessageBox (showInfo/Error/Warning/Question), FileDialog (file picker)

## Editor (src/editor/)

EditorWindow (main editor UI), EditorState (state management), PreviewWindow (live preview), LayoutImporter (load layouts), LayoutExporter (save layouts)

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

- Style: optional fields (backgroundColor, textColor, borderColor, borderRadius, fontSize, font)
- Theme: per-type, per-state storage (`map<string, map<ElementState, Style>>`)
- Composition: local[state] → theme[type][state] → theme[type][Normal] → default
- ElementState enum: Normal, Hovered, Pressed, Disabled

## Anchor (Responsive Layout)

- Coordinates: &lt;0=unset, 0-1=%, &gt;1=px, 0.5=center
- Stretch: both edges set → element stretches to fill
- Presets: center(), fill(), topLeft(), bottomBar(), leftSidebar(), topBar(), rightSidebar(), horizontalStretch(), verticalStretch()
- Resize: onParentResize() updates position/size via GUIManager::handleResize()

## Render Flow

1. GUIManager::render() → GUIElement::render()
2. wantsDirectRender()? drawDirect() : renderToCache() → m_cachedTexture
3. Children rendered with parent_clip_rect
4. GPU elements use SDL_gpu via m_gpuState

## Memory

- Elements: unique_ptr hierarchy
- SDL resources: SharedTexture/SharedFont (shared_ptr with custom deleters in sdl_deleters.hpp)
- Render cache: m_cachedTexture invalidated by m_isDirty
- Element lifecycle tracking: m_liveElements (unordered_set) for ElementRef safety