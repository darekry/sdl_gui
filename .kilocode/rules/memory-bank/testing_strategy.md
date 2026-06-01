# Strategia testów

## Framework & Tools
- **Catch2** (amalgamated: `lib/catch_amalgamated.hpp`)
- **Runner**: `./nob test`
- **Helper**: `tests/test_helper.hpp/cpp` (headless SDL init)

## Pokrycie testami (29 plików, ~1850 asercji)

### Widgety (17)
Button, Checkbox, ComboBox, Canvas, ContextMenu, Label, ListView, Panel, RadioButton, RadioGroup, Slider, StringGrid, TabControl, TextArea, TextInput, AnimatedImage, Cursor

### Menedżery (4)
FontManager, TextureManager, TimerManager, AnimationManager

### Systemy (4)
GUIElement, GUIManager, Theme, Easing

### Screen/Window (2)
ScreenManager, WindowManager

### Brakujące
- test_text_editable.cpp
- Style, SGMLParser, JsonParser, LayoutParser (header-only/interface)

## Testy integracyjne

32 examples w `examples/` - manualna weryfikacja wizualna.

Kluczowe: example_button, example_string_grid, example_json_parser, example_wysiwyg_editor, example_window_manager

## Testy wydajności

`examples/example_performance.cpp` - dynamic add/remove, FPS counter