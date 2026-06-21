# Aktualny stan projektu (2026-06-20)

## Status: Rounded corner texture clipping fix ✅

## Ostatnie zmiany

### Rounded corner texture clipping (2026-06-20)
- **Problem**: Tekstura (`style.texture`) renderowana przez `drawBackgroundAndBorder` ignorowała zaokrąglenie rogów - `SDL_RenderTexture` rysował pełny prostokąt wystający poza obramowanie rounded rect
- **Fix**: Nowa funkcja `drawRoundedTexturedRect` w `src/gui.cpp:46-98` rysuje teksturę przyciętą do kształtu zaokrąglonego prostokąta używając `SDL_RenderGeometry` z teksturą i znormalizowanymi koordynatami UV
- **Pliki zmienione**:
  - `src/gui.hpp:173` - dodana deklaracja `drawRoundedTexturedRect`
  - `src/gui.cpp:46-98` - implementacja funkcji
  - `src/gui.cpp:623-626` - `drawBackgroundAndBorder` używa nowej funkcji zamiast `SDL_RenderTexture`
- **Build**: 39/39 examples, 29/29 tests kompiluje się (1 test fail: combobox - pre-existing heap-use-after-free)

### Embedded Assets (2026-06-20)
- **Manifest**: `assets.embed` z listą plików do osadzenia (ścieżka + opcjonalny font size)
- **Build**: `nob.c::build_embedded_assets()` używa `ld -r -b binary` do tworzenia .o z binariami
- **Header**: auto-generowany `output/embedded_assets.hpp` z `extern "C"` symbolami i tabelą `g_embeddedAssets[]`
- **TextureManager**: nowa metoda `loadTextureFromMemory(data, size, key)` używająca `SDL_IOFromConstMem` + `IMG_Load_IO`
- **FontManager**: nowa metoda `loadFontFromMemory(data, size, fontSize, key)` używająca `SDL_IOFromConstMem` + `TTF_OpenFontIO`
- **Example**: `examples/example_embedded_assets.cpp` demonstruje rejestrację i użycie embedded assetów przez standardowe API
- **Auto-rejestracja**: wpięte do `build_examples` i `build_tests` — każdy binarek linkuje embedded .o
- **Rebuild detection**: działa — pomija gdy nic się nie zmieniło (0 built, 38 skipped)

### SDL2 → SDL3 Migration (2026-06-13) ✅ Complete

Wszystkie 10 stages migracji zakończone. Projekt w pełni funkcjonalny na SDL3.

## Kluczowe stats

| Metric | Value |
|--------|-------|
| Examples | 39 (all compile) |
| Test files | 31 total (29 test + 2 infra: test_helper, test_main) |
| Widget types | 21 (+ 3 composite: DialogBox, MessageBox, FileDialog) |
| Editor modules | 5 (EditorWindow, EditorState, PreviewWindow, LayoutImporter, LayoutExporter) |
| Parser-supported widgets | 18 (JSON + XML)
