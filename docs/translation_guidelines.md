# SDL GUI Translation Guidelines and Terminology (Profile A)

This document defines the translation policy, style standards, terminology, and workflows for internationalizing SDL GUI documentation and code comments. It implements Profile A: en-US as the authoritative language, with Polish kept as an archival mirror.

This page references project files such as [`src/gui.hpp`](src/gui.hpp), [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14), [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1), [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7), [`src/theme.hpp`](src/theme.hpp:10), [`src/style.hpp`](src/style.hpp:17), [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24). Rendering flow details for reference: [`GUIManager::render()`](src/gui_manager.cpp:51), [`GUIElement::render(renderer)`](src/gui.cpp:127), [`GUIElement::renderToCache()`](src/gui.cpp:174).

If you are reading the Polish version, it should be available at: docs/pl/translation_guidelines.pl.md. English remains the source of truth.

## Table of Contents

- 1. Purpose and Language Policy (Profile A)
- 2. en-US Style Standards
  - 2.1 Tone and clarity
  - 2.2 Technical terminology
  - 2.3 Capitalization
  - 2.4 Punctuation
  - 2.5 Dates, times, numbers, and units
- 3. Glossary (PL → EN) with usage notes and collocations
- 4. Documentation Translation Rules
  - 4.1 What to translate vs. keep as is
  - 4.2 Links and navigation conventions
  - 4.3 Cross-file consistency and migration to docs/en and docs/pl
- 5. Code Comment Translation Rules (English only)
  - 5.1 Preferred comment/doc style (Doxygen)
  - 5.2 What never to translate
  - 5.3 When to add explanatory notes
  - 5.4 Before → After examples
- 6. Translation Workflow and QA
  - 6.1 Order of work
  - 6.2 Terminology QA and link checks
  - 6.3 Markdown rendering checks
  - 6.4 Commit message pattern and versioning notes
- 7. Difficult Cases (Edge Cases)
- 8. Appendices
  - 8.1 Style quick reference
  - 8.2 Glossary quick lookup

---

## 1. Purpose and Language Policy (Profile A)

- Authoritative language: en-US across the repository for documentation and all code comments.
- Polish is maintained as an archival mirror, published under docs/pl as a post-translation snapshot of en-US content.
- Target documentation structure:
  - English: [`docs/en/`](docs/en/)
  - Polish: [`docs/pl/`](docs/pl/)
  - Top-level READMEs:
    - EN: [`readme.md`](readme.md) (authoritative)
    - PL: [`README.pl.md`](README.pl.md) (archival mirror)
- Code comments across [`src/`](src/), [`examples/`](examples/), and [`tests/`](tests/) must be English only. Do not keep Polish comments in code after translation.

Rationale: developers and external contributors expect English API and comment conventions; Polish remains accessible for local users while avoiding divergence.

---

## 2. en-US Style Standards

### 2.1 Tone and clarity

- Professional, concise, and developer-focused.
- Prefer plain English over jargon; if a term is standard in SDL/GUI domains, use it consistently (see glossary).
- Avoid idioms and culture-specific references.
- Prefer active voice and short sentences. One idea per sentence where possible.

Examples:
- Prefer “Use the cache to avoid repeated rendering.” over “It is important that the cache is used to avoid rendering repeatedly.”
- Prefer “Click” over “Press with the mouse” unless differentiating from keyboard input.

### 2.2 Technical terminology

- Align with SDL2/SDL_ttf/SDL_image naming and semantics where applicable; quote original terms when needed and link to SDL docs if necessary.
- Treat API identifiers as code, not natural language. Keep exact spelling and casing.

### 2.3 Capitalization

- Headings: Title Case for H1–H3; Sentence case may be used for H4+ when headings are long or procedural.
- UI labels and doc section titles: Title Case.
- Common nouns in running text: sentence case unless part of an official name (e.g., “Texture Manager” in prose vs. [`TextureManager`](src/texture_manager.hpp:15) in code).

### 2.4 Punctuation

- Use the Oxford (serial) comma: “a, b, and c”.
- Use en-dash (–) for ranges; hyphen (-) for compound adjectives (e.g., “cache-aware rendering”).
- Periods inside parentheses only if the entire sentence is parenthetical.

### 2.5 Dates, times, numbers, and units

- Date format: ISO-like “YYYY-MM-DD” in metadata and changelogs. Use clear month names in prose if ambiguity is possible (“August 30, 2025”).
- Time: 24-hour time with time zone if relevant (“14:35 CET”).
- Numbers: use thin spaces or commas appropriately in prose only when needed for readability (1,024). In code, follow the language’s conventions.
- Units: Space between number and unit (16 px, 2 ms, 3 MB). Use lowercase for units except MB/KB/GB.

---

## 3. Glossary (PL → EN) with usage notes and collocations

Notes on two commonly confused pairs:
- “cached texture” vs. “render cache”
  - cached texture: a specific texture object kept for reuse (per-element cache), e.g., “update the cached texture.”
  - render cache: the concept or mechanism of caching the render output to avoid re-drawing; broader than a single texture.
- “clip rect” vs. “clipping”
  - clip rect: the rectangle used to clip.
  - clipping: the operation/effect of restricting drawing to a region.

Terminology list (≥ 80 entries). Each line: PL → EN — usage/collocations.

1) bufor → buffer — vertex buffer, frame buffer
2) pamięć podręczna (renderu) → render cache — populate the render cache; cache invalidation
3) tekstura cache’owana → cached texture — refresh cached texture; update cached texture
4) prostokąt przycinania → clip rect — set clip rect; parent clip rect
5) przycinanie (operacja) → clipping — enable/disable clipping; clipping region
6) renderowanie bezpośrednie → direct rendering — enable direct rendering path
7) renderowanie z cache → cached rendering — switch to cached rendering
8) znacznik do usunięcia → mark for deletion — mark element for deletion; scheduled cleanup
9) pętla zdarzeń → event loop — run the event loop; pump events
10) menedżer GUI → GUI manager — initialize the GUI manager; global GUI manager
11) element GUI → GUI element — top-level GUI element; child GUI element
12) rodzic/dziecko (hierarchia) → parent/child — parent element; child elements
13) hierarchia elementów → element hierarchy — flatten the hierarchy; traverse hierarchy
14) stan elementu → element state — Normal, Hover, Pressed
15) menedżer tekstur → texture manager — global texture manager; load via texture manager
16) menedżer czcionek → font manager — retrieve font from font manager
17) wspólny wskaźnik → shared pointer — hold texture as a shared pointer
18) unikalny wskaźnik → unique pointer — own children via unique pointers
19) niestandardowy deleter → custom deleter — shared pointer with a custom deleter
20) unieważnienie cache → cache invalidation — invalidate cache on state change
21) przebudowa cache → cache rebuild — rebuild cache on resize
22) cel renderowania → render target — set render target; restore previous render target
23) mapa tekstur → texture map — texture cache map; resource map
24) rozmiar tekstu → text size — measure text size; compute text bounds
25) prostokąt docelowy → destination rect — render to destination rect
26) prostokąt źródłowy → source rect — copy from source rect
27) zdarzenie myszy → mouse event — mouse button down; mouse move
28) zdarzenie klawiatury → keyboard event — key down; key up
29) przechwycenie zdarzenia → event handling — process event; handle event
30) renderowanie partii → batch rendering — batch draw calls; reduce draw calls
31) wskaźnik współdzielony na teksturę → SharedTexture — return a SharedTexture
32) wskaźnik współdzielony na czcionkę → SharedFont — retrieve SharedFont
33) styl → style — default style; style overrides
34) motyw → theme — global theme; theme-provided defaults
35) tooltip → tooltip — show tooltip; tooltip delay
36) rozwijane menu kontekstowe → context menu — open context menu; context menu item
37) menu kontekstowe → context menu — same as above (preferred term)
38) combobox → combo box — open combo box; combo box selection
39) przycisk → button — button press; button click callback
40) suwak → slider — slider handle; slider value
41) pole wyboru → checkbox — checkbox state; toggle checkbox
42) przyciski radiowe → radio buttons — radio group; selected radio
43) panel → panel — panel background; panel layout
44) zakładki → tabs (tab control) — tab control; active tab
45) pole tekstowe (pojedyncza linia) → text input — text input caret; input focus
46) obszar tekstowy (wielolinia) → text area — text area scroll; wrap text
47) etykieta → label — label text; static label
48) obraz animowany → animated image — animated frames; frame duration
49) menedżer animacji → animation manager — schedule animation; update animations
50) menedżer timerów → timer manager — create timer; cancel timer
51) easing → easing — easing function; easing curve
52) FPS → FPS — FPS counter; frame timing
53) licznik FPS → FPS counter — display FPS counter
54) czas renderowania klatki → frame render time — measure frame time
55) prostokąt przycinania rodzica → parent clip rect — intersect with parent clip rect
56) przecinanie prostokątów → rect intersection — compute intersection
57) docelowy renderer → renderer — pass renderer; main renderer
58) przywrócić cel renderowania → restore render target — restore previous target
59) API zdarzeń SDL → SDL event API — SDL_PollEvent; event types
60) operacje rysowania → draw operations — minimize draw operations
61) kopiowanie tekstury → texture copy — SDL_RenderCopy; copy to renderer
62) tworzenie tekstury z tekstu → create texture from text — render text to texture
63) ładowanie obrazu → image loading — load image file
64) cache zasobów → resource cache — cache textures and fonts
65) błąd ładowania → loading error — log loading error
66) log błędów → error log — write to error log
67) przyjazne API → ergonomic API — ergonomic API design
68) flagi stanu → state flags — set state flag; clear state flag
69) opóźnienie tooltipa → tooltip delay — set tooltip delay
70) wsparcie formatów obrazów → image format support — PNG/JPEG support
71) GPU-accelerated kopia → GPU-accelerated copy — accelerated blit/copy
72) przejrzystość/alpha → alpha (transparency) — set alpha; alpha blending
73) mieszanie → blending — enable blending; blending mode
74) doc string (komentarz doc) → docstring / documentation comment — Doxygen block
75) przykład użycia → usage example — add usage example
76) nagłówek pliku → file header — file header comment
77) ścieżka względna → relative path — use relative links
78) kotwica nagłówka → heading anchor — matching anchors EN↔PL
79) migracja dokumentacji → documentation migration — migrate to docs/en and docs/pl
80) zgodność odnośników → link consistency — maintain cross-links
81) znacznik sekcji → section tag — Doxygen @section / @note
82) wątki/zdarzenia asynchroniczne → async events — asynchronous callbacks
83) re-renderowanie → re-render — trigger re-render
84) przywrócenie stanu → state restore — restore state after draw
85) ścieżki zasobów → asset paths — verify asset paths
86) wcięcia i styl kodu → code style & indentation — follow .clang-format
87) podpis licencji → license header — include license header
88) testy jednostkowe → unit tests — run unit tests
89) testy integracyjne → integration tests — manual examples
90) scenariusz wydajnościowy → performance scenario — stress test
91) walidacja linków → link validation — check all links
92) wersjonowanie tłumaczeń → translation versioning — record source commit

---

## 4. Documentation Translation Rules

### 4.1 What to translate vs. keep as is

Translate:
- All headings and body text in docs.
- Alt text for images (keep concise and meaningful).
- Explanatory text for code snippets.

Do not translate (keep exact strings):
- API names, class/struct/enum/namespace identifiers, method/function names, and constants.
- File and directory names, including all paths.
- Strings mandated by SDL or other external APIs.

Examples:
- Keep [`GUIElement`](src/gui.hpp) and “GUIElement” in code and code-like contexts. In prose, refer to it as “the GUIElement class” if needed, still capitalized.

### 4.2 Links and navigation conventions

- Use relative links within the repository, e.g., [`docs/en/creating_new_widget.md`](docs/en/creating_new_widget.md).
- Maintain parallel navigation: each English page should link to its Polish counterpart and vice versa near the top of the page, for example:
  - “This page is also available in Polish: [`docs/pl/creating_new_widget.md`](docs/pl/creating_new_widget.md)”
  - “Ta strona jest dostępna po angielsku: [`docs/en/creating_new_widget.md`](docs/en/creating_new_widget.md)”
- Headings should use stable text so that automatic anchor generation remains consistent between languages; if anchors differ, add explicit link targets in both versions.
- For cross-file references to code, prefer referencing the file and, when helpful, also mention the known point of interest:
  - File-level: [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
  - Function-level: [`GUIManager::render()`](src/gui_manager.cpp:51), [`GUIElement::render(renderer)`](src/gui.cpp:127), [`GUIElement::renderToCache()`](src/gui.cpp:174)

### 4.3 Cross-file consistency and migration to docs/en and docs/pl

- When migrating existing PL documents from [`docs/`](docs/) to [`docs/pl/`](docs/pl/), update all relative links to point to their Polish neighbors in [`docs/pl/`](docs/pl/).
- When creating the EN canonical equivalents under [`docs/en/`](docs/en/), mirror the structure and update cross-links to point to EN resources by default. Keep a bilingual toggle near the top.
- If a page moves or is renamed, update:
  - All in-repo references to that page.
  - The bilingual toggles on both language versions.
- Use consistent filenames:
  - English: keep the original base filename (e.g., creating_new_widget.md).
  - Polish: same base filename in [`docs/pl/`](docs/pl/) or add “.pl.md” suffix only at the repository root (e.g., [`README.pl.md`](README.pl.md)).

---

## 5. Code Comment Translation Rules (English only)

All code comments in [`src/`](src/), [`examples/`](examples/), and [`tests/`](tests/) must be in English after translation.

### 5.1 Preferred comment/doc style (Doxygen)

- Use Doxygen-style documentation comments for public classes, functions, and important internal helpers.
- Recommended tags:
  - @brief, @param, @return, @note, @warning, @example, @see
- Prefer short @brief, additional details in paragraphs or @note.
- Keep code-like items monospaced when appropriate.

### 5.2 What never to translate

- Identifiers: class, struct, enum names, method names, and function names.
- Type names and template parameters.
- Function signatures and API-string literals, including messages defined by SDL or other libraries.

### 5.3 When to add explanatory notes

- If an identifier uses a domain term that may be ambiguous, keep the identifier unchanged and add a clarifying sentence in the comment.
- Do not rename symbols to “translate” them.
- For behavior tied to SDL semantics, quote the SDL concept and link to the SDL documentation if the meaning might be unclear.

### 5.4 Before → After examples

Short inline comment:

Before:
```cpp
// Przycinamy rysowanie do prostokąta rodzica
```

After:
```cpp
// Clip drawing to the parent clip rect
```

API documentation block for a method:

Before:
```cpp
/**
 * Rysuje element do cache, jeśli jest nieaktualny.
 * Wykorzystuje teksturę lokalną elementu.
 */
```

After:
```cpp
/**
 * @brief Renders the element into its cached texture when dirty.
 * @note Uses the element's local cached texture and restores the previous render target.
 */
```

File header block:

Before:
```cpp
// Menedżer tekstur: ładowanie i cache'owanie tekstur z plików i tekstu.
```

After:
```cpp
// Texture Manager: loads and caches textures from files and rendered text.
// See also: resource cache, cached texture, and SDL render targets.
```

---

## 6. Translation Workflow and QA

### 6.1 Order of work

1) Top-level READMEs
- Make [`readme.md`](readme.md) authoritative in English.
- Create/update [`README.pl.md`](README.pl.md) as Polish archival mirror.

2) Migrate existing Polish docs
- Move current Polish documents under [`docs/pl/`](docs/pl/).
- Fix all relative links to point within [`docs/pl/`](docs/pl/).

3) Create English canonical structure
- Create the mirrored tree under [`docs/en/`](docs/en/) and populate pages.
- For example, translate and publish: [`docs/en/creating_new_widget.md`](docs/en/creating_new_widget.md), [`docs/en/animated_image.md`](docs/en/animated_image.md), [`docs/en/context_menu.md`](docs/en/context_menu.md), [`docs/en/testing_strategy.md`](docs/en/testing_strategy.md).
- Add bilingual toggles on each page.

4) Translate docs page-by-page
- Prioritize conceptual and how-to guides used by contributors.
- Keep code excerpts intact; translate only explanatory text around them.

5) Translate code comments (English only)
- Start with core modules: [`src/gui.hpp`](src/gui.hpp), [`src/gui.cpp`](src/gui.cpp:8), [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14), [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1), [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7), [`src/theme.hpp`](src/theme.hpp:10), [`src/style.hpp`](src/style.hpp:17), [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24).
- Then widget implementations in [`src/`](src/).
- Then [`examples/`](examples/), then [`tests/`](tests/).

### 6.2 Terminology QA and link checks

Terminology consistency:
- Keep this guideline open when translating; verify each domain term against Section 3.
- Use a search across the repo to spot variant terms and unify them.

Link checks:
- Validate all relative links after moving files.
- Ensure EN pages default-link to EN neighbors; PL pages to PL neighbors.
- Maintain bilingual toggles on each page.

### 6.3 Markdown rendering checks

- Preview every page in a Markdown renderer to verify code blocks, lists, and tables render correctly.
- Ensure images have meaningful alt text; verify relative paths to assets.
- Confirm that inline code and identifiers are styled correctly and do not wrap awkwardly.

### 6.4 Commit message pattern and versioning notes

- Commit message prefix: “[docs][i18n] Translate: …”
  - Example: “[docs][i18n] Translate: creating_new_widget.md (PL→EN, add EN canonical)”
- At the top or bottom of a translated Polish page, add a versioning note:
  - “Translated from English: commit <sha> on branch <name>”
- When significant EN changes occur, bump the note on the PL mirror with the new source commit.

---

## 7. Difficult Cases (Edge Cases)

- cache vs. cached texture: see Section 3 notes; use “render cache” for the broader mechanism, “cached texture” for the concrete texture resource.
- clipping vs. scissoring: prefer “clipping” and “clip rect.” Avoid “scissoring” unless quoting a specific API that uses it.
- theme vs. style: “theme” supplies default styles globally; a “style” is a set of properties applied to an element or scope.
- icon vs. glyph: “icon” for UI symbol images; “glyph” for font-rendered character shapes.
- font family vs. typeface: prefer “font family” in user-facing text; use “typeface” only when discussing typography specifics.
- pointer semantics: “shared pointer” for shared ownership; “unique pointer” for exclusive ownership. Do not translate or rename identifiers; explain ownership in comments if unclear.
- SDL-specific behavior: when in doubt, quote the SDL concept and link to official docs; keep SDL identifiers exact. Example mentions elsewhere: [`GUIElement::render(renderer)`](src/gui.cpp:127) sets clip rects and copies cached textures via SDL calls; refer to the SDL rendering API and keep names intact.

UI microcopy:
- Keep labels short and unambiguous (“Open,” “Close,” “Apply,” “Cancel”).
- Prefer verbs for actions, nouns for states.
- Avoid obscure abbreviations; if space-constrained, prefer well-known ones (FPS, ms).

---

## 8. Appendices

### 8.1 Style quick reference

- Language: en-US. English-only in code comments.
- Tone: concise, professional, developer-focused.
- Capitalization: Title Case for H1–H3; sentence case allowed for deeper headings if long.
- Punctuation: use the serial comma; hyphenate compound adjectives.
- Dates/times: ISO-like dates, 24-hour times.
- Units: space between number and unit (2 ms, 16 px).
- Keep identifiers and file paths unchanged.
- Use relative links; maintain bilingual toggles.
- Prefer Doxygen for API docs with @brief, @param, @return.
- For rendering concepts, differentiate “render cache” (mechanism) vs. “cached texture” (object).

Common references:
- [`GUIManager::render()`](src/gui_manager.cpp:51)
- [`GUIElement::render(renderer)`](src/gui.cpp:127)
- [`GUIElement::renderToCache()`](src/gui.cpp:174)
- [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10)

### 8.2 Glossary quick lookup (high-frequency)

- przycinanie → clipping; prostokąt przycinania → clip rect
- cache renderu → render cache; tekstura cache’owana → cached texture
- renderowanie bezpośrednie → direct rendering; renderowanie z cache → cached rendering
- znacznik do usunięcia → mark for deletion
- pętla zdarzeń → event loop
- wspólny/unikalny wskaźnik → shared/unique pointer
- tooltip → tooltip; combobox → combo box; menu kontekstowe → context menu
- suwak → slider; przyciski radiowe → radio buttons; pole wyboru → checkbox
- pole tekstowe → text input; obszar tekstowy → text area
- menedżer tekstur/czcionek → texture/font manager
- motyw → theme; styl → style
- cel renderowania → render target; kopiowanie tekstury → texture copy
- czas renderowania klatki → frame render time; licznik FPS → FPS counter

---

End of document.