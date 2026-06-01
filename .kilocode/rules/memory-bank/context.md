# Aktualny stan projektu (2026-06-01)

## Status: STABILNY

**Repozytorium:** 32 examples, 29 test files, all tests passing

## Ostatnie zmiany (Maj 2026)

### WYSIWYG Editor (May 27)
- Dual-window architecture: EditorWindow + PreviewWindow
- WindowManager zarządza dwoma oknami SDL
- Features: palette (14 widgets), drag-to-move, grid snap (20px), properties panel, XML/JSON export/import
- Files: `src/editor/` (editor_state, editor_window, preview_window, layout_exporter, layout_importer)

### TextEditable Base Class (May 25)
- Abstrakcyjna klasa dla TextInput/TextArea
- Selection API: hasSelection(), getSelection(), clearSelection(), setSelection()
- Clipboard: copy/paste/cut z SDL_SetClipboardText
- Files: `src/text_editable.hpp/cpp`

### Theme/Style Per-State (May 28)
- Storage: `map<string, map<ElementState, Style>>` - per-type, per-state
- API: setStyle(type, state, style), getStyle(type, state)
- Composition: local[state] → theme[type][state] → theme[type][Normal] → default

### Responsive Anchor System (May 22)
- Anchor struct: left/top/right/bottom coordinates (-1=unset, 0-1=%, >1=px, 0.5=center)
- Presets: center(), fill(), topLeft(), bottomBar(), leftSidebar()
- Automatic resize handling via handleParentResize()

### ScreenManager/WindowManager (May 23)
- ScreenManager: screen stack, changeScreen(), pushScreen()/popScreen() for overlays
- WindowManager: multiple SDL windows, auto event routing via SDL_WINDOWID
- Tests: test_screen_manager.cpp, test_window_manager.cpp

### nob.c Build System (May 22)
- Replaced Makefile with nob.c (nob.h v3.8.0)
- Unity build, C++23 modules, compile_commands.json auto-generated
- Commands: ./nob (build), ./nob test, ./nob release, ./nob clean

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 32 |
| Test files | 29 |
| Test assertions | ~1850 |
| Widget types | 16 |
| Composite components | DialogBox, MessageBox |
| Build time (unity) | ~15s |

## Next steps considerations

- FileDialog (planned, not implemented)
- test_text_editable.cpp (missing)
- StringGrid: setCellBackgroundColor, setGridLineColor API (TODO.md)