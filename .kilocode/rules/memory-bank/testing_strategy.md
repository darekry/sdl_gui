# Strategia testów

## Framework & Tools
- **Catch2** (amalgamated: `lib/catch_amalgamated.hpp`)
- **Runner**: `./nob test`
- **Helper**: `tests/test_helper.hpp/cpp` (headless SDL init)

## Pokrycie testami (29 plików, ~28 widget/system tests)

### Widgety (17)
Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor

### Menedżery (4)
FontManager, TextureManager, TimerManager, AnimationManager

### Systemy (5)
GUIElement, GUIManager, Theme, Easing, UTF8

### Screen/Window (2)
ScreenManager, WindowManager

### Brakujące testy
- ArcContainer, ProgressBar, ScrollArea, ShaderPanel (nowe widgety)
- test_text_editable.cpp
- Style, SGMLParser, JsonParser, LayoutParser (header-only/interface)

## Testy integracyjne

38 examples w `examples/` - manualna weryfikacja wizualna.

Kluczowe: example_button, example_string_grid, example_json_parser, example_wysiwyg_editor, example_window_manager, example_gpu_shader, example_theme_playground