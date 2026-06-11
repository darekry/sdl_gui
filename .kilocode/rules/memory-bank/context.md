# Aktualny stan projektu (2026-06-11)

## Status: STABILNY - SAFE ELEMENT REFERENCES ADDED

**Repozytorium:** 34 examples, 31 test files, 2,500+ assertions, all tests passing (1 pre-existing leak in test_radio_group)

## Ostatnie zmiany

### Safe Element References (2026-06-11)
- **Problem**: Raw pointers to GUIElement captured in callbacks become dangling when target element is deleted
- **Solution**: `ElementRef<T>` wrapper + automatic `registerElement`/`unregisterElement` via `m_liveElements` set
- **Core files**: `src/gui_manager.hpp` (ElementRef template + makeRef), `src/gui.cpp` (register/unregister in addChild/~GUIElement), `src/gui_manager.cpp` (register in addElement)
- **Updated**: 14 example files — raw pointer captures replaced with `ElementRef` + null-check
- **Pattern**: `auto* ptr = element.get()` → `auto ref = manager.makeRef(element.get());` + `if (ref) ref->method()`

### Regression Fix (2026-06-05)
- `getComposedStyle()` merge direction inverted — local overrides silently ignored. Reversed order: local style first, fill gaps from theme.

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 34 |
| Test files | 31 |
| Test assertions | ~2,500 |
| Widget types | 16 |