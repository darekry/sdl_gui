# Aktualny stan projektu (2026-06-12)

## Status: STABILNY - SCROLL AREA + TEXTURE REFACTOR

**Repozytorium:** 36 examples, 31 test files, 2,500+ assertions, all tests passing (1 pre-existing leak in test_radio_group)

## Ostatnie zmiany

### ProgressBar Widget (2026-06-12)
- **Nowy widget**: `ProgressBar` — pasek postępu (poziomy/pionowy) dziedziczący po `Panel`
- **Core files**: `src/progress_bar.hpp`, `src/progress_bar.cpp`
- **Example**: `examples/example_progress_bar.cpp`
- **API**: `setValue(float)`, `setRange(min,max)`, `setOrientation()`, `setShowText()`, `setTextFormat()`
- **Styling**: dodany domyślny styl w `theme.cpp` dla "ProgressBar"

### ScrollArea Widget (2026-06-12)
- **Nowy widget**: `ScrollArea` — generyczny, przewijalny kontener dla dowolnych drzew widgetów
- **Architektura**: kompozycja z istniejących widgetów (Panel viewport + Panel content + Slider)
- **Core files**: `src/scroll_area.hpp`, `src/scroll_area.cpp`
- **Example**: `examples/example_scroll_area.cpp` — demonstruje pionowe/poziome przewijanie, auto-hide suwaków, mouse wheel
- **API**: `setContent()`, `setContentSize()`, `setScrollEnabled()`, `setScrollOffset()`

### Texture Rendering Refactor (2026-06-12)
- **Problem**: `drawBackgroundAndBorder()` nie renderował `style.texture` — tylko Button i Panel robiły to ręcznie
- **Solution**: Tekstura przeniesiona do `drawBackgroundAndBorder()` w `gui.cpp`, pomiędzy tłem a obramowaniem
- **Usunięcie duplikacji**: Button::draw i Panel::draw — usunięty ręczny kod renderowania tekstury
- **Checkbox fix**: Usunięta ścieżka `style.texture` jako symbolu checkmarka (kolidowała z nowym znaczeniem)
- **Efekt**: Wszystkie widgety wołające `drawBackgroundAndBorder()` obsługują teraz tekstury per-stan automatycznie

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 36 |
| Test files | 31 |
| Test assertions | ~2,500 |
| Widget types | 18 |
