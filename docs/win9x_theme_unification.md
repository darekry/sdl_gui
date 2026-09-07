# Plan unifikacji motywu Win9x / Windows95

> Status: ZREALIZOWANE (2026-09-07). Wpis w `AGENTS.md` → "Unifikacja Win9x → Windows95".
> Odstępstwo od planu: alias bez atrybutu `[[deprecated]]` (sama adnotacja słowna),
> żeby wołania z `sdl_gui_c_api.cpp` nie generowały warningów w buildzie.
> Powiązane: `docs/refactor_plan.md` → "Drobne (przy okazji)" (punkt o dwóch presetach Win95),
> `AGENTS.md` → "StyleResolver faza 1", `CHANGELOG.md` → "Theme::createWindows95Theme() + audyt bevel".

## 1. Problem w jednym akapicie

W `src/theme_presets.hpp` żyją **dwa presety na ten sam wygląd**:

| | Stary | Nowy (kanon) |
|---|---|---|
| Funkcja | `createWin9xTheme()` (l.19) | `createWindows95Theme()` (l.267) |
| Kto go używa | `Theme::createDefaultTheme()` (`src/theme.cpp:47`), `GUIManager` ctor (`src/gui_manager.cpp:26`), `GUIContext` (`src/gui_context.hpp:13,28`), C-API `sdlgui_theme_win9x` (`src/sdl_gui_c_api.cpp:228`), ~40 przykładów, bench, prawie wszystkie testy | tylko `examples/48_win95_bevel.cpp:102` + `tests/test_theme.cpp:184` |
| Mechanika ramki | płaski `borderWidth` + `borderColor` | faza 3D (bevel: `borderColorOuter/InnerTopLeft/BottomRight` przez `withBevel()`) |
| Paleta | lokalne stałe w funkcji (l.22–30) | `constants::kWin95*` + lokalne `kNavy/kWindowText` |

Efekt: **default aplikacji wygląda inaczej niż to, co docs nazywają "autentycznym Win95"**,
a `readme.md:663` ("Windows 95/98 style" o `createDefaultTheme`) jest po dodaniu bevel-presetu nieprawdziwe.

## 2. Inwentaryzacja niespójności

### 2a. Paleta: duplikat + dryf wartości

Stary preset definiuje własną paletę zamiast `src/constants.hpp:16-20`:

| Stary (lokalny, `theme_presets.hpp:22-30`) | `constants::kWin95*` | Werdykt |
|---|---|---|
| `kWindowBg {192,192,192}` | `kWin95Face {192,192,192}` | identyczne — duplikat |
| `kBtnShadow {128,128,128}` + `k3dShadow {128,128,128}` (dwa różne identyfikatory, ta sama wartość!) | `kWin95Shadow {128,128,128}` | duplikat ×2 |
| Hover Button `{223,223,223}` | `kWin95Light {223,223,223}` | identyczne, na twardo |
| `kBtnDarkShadow {64,64,64}` | brak (`kWin95DarkShadow` = `{0,0,0}`) | **rozjazd — kolor nie istnieje w palecie systemowej** |
| Pressed bg `{160,160,160}`, Disabled border `{160,160,160}` | brak | **rozjazd** |
| Disabled bg `{212,208,200}` (klasyczne Win32 "3D Objects") | brak (nowy używa Face `192`) | **rozjazd** |
| `kHighlight navy {0,0,128}` | nowy `kNavy {0,0,128}` | ten sam kolor pod dwiema nazwami |

### 2b. Per-widget: ta sama nazwa typu, inna mechanika i kolory

| Typ | `createWin9xTheme` (stary) | `createWindows95Theme` (nowy) | Niespójność |
|---|---|---|---|
| Button Normal | brak bg (dziedziczy default 192), płaska ramka `borderWidth=2` | Face 192 + `Raised` | mechanika |
| Button Hover | 223 płasko | Light 223 + `Raised` | mechanika |
| Button Pressed | **160** + `kBtnDarkShadow(64)` border | Face 192 + `Sunken` (bg się nie zmienia, tylko bevel) | **kolor + mechanika** |
| Button Disabled | 192 + border 160, szary tekst | Face + `Raised`, szary tekst | mechanika |
| Panel | `borderWidth=2` + Face | flat, bez border ("ramy ustawia user") | **grubość ramki** |
| TextInput / TextArea Normal | biały + płaski border 128, `borderWidth=2` | biały + `Sunken` | mechanika |
| TextInput / TextArea Hover | ramka **navy `0,0,128`** | == Normal (sunken, brak Hover) | **nieautentyczne podświetlenie w starym** — Win95 nie malował pól na navy |
| TextInput / TextArea Disabled | bg `212,208,200` + szara ramka | Face 192 + bevel wyczyszczony (`reset()` na 4 polach), szary tekst | **kolor + mechanika** |
| ComboBox | Normal sunken-like płasko + **Hover navy** | tylko Normal `Sunken` (Hover fallback → Normal) | Hover rozjechany jak wyżej |
| ListView / Canvas | biały + płaski border 2 | biały + `Sunken` | mechanika |
| StringGrid | biały + border 128, `borderWidth=1` | biały + `Sunken`, `borderColor=Shadow` jako kolor linii siatki | mechanika + przeciążona semantyka `borderColor` |
| ProgressBar | biały + `borderColor navy` (= ramka), bez bevel | biały + `Sunken`, `borderColor navy` (= **fill**) | **to samo pole znaczy co innego** (komentarz w kodzie to przyznaje) |
| Slider / RangeSlider | bg Face + `borderWidth=2`, kolor dziedziczony/nieustawiony | bg Face + `borderColor=Shadow` jako kolor thumba | oba nadużywają `borderColor` zamiast `thumbColor` (StyleResolver dodał `thumbColor`/`fillColor` z fallbackiem — żaden preset Win9x ich nie ustawia) |
| TabControl / ScrollArea | bg Face + `borderWidth=1` | bg Face, brak `borderWidth` (dziedziczy default 0) | **grubość ramki** |
| ContextMenu | biały + border 128 `width=1`; Hover navy/biały | identyczne wartości, ale border przez `constants::kWin95Shadow` | wartości OK, inna ścieżka (lokalna vs constants) |
| Checkbox / RadioButton | tylko `textColor` + `fontSize=14` | tylko `textColor` + `fontSize=14` | zgodne (pudełko rysuje widget na twardo — do audytu, czy czyta theme) |
| Label | transparent + czarny tekst + 14 | transparent + czarny tekst + 14 | zgodne |
| AnimatedImage | bg Face | bg Face | zgodne |
| defaultStyle | bg 192, tekst czarny, `borderColor=128`, `borderWidth=0`, `radius=0`, font 14 | bg 192, tekst czarny, **brak `borderColor`** (czyli fallback inny), `borderWidth=0`, `radius=0`, font 14 | defaultowy `borderColor` rozjechany |
| Nieustawiane w obu | `RadioGroup, Cursor, ArcContainer, ShaderPanel, DialogBox, FileDialog, GUIElement` → fallback do defaultu | jw. | dziura: dialogi w Win95 powinny być Face jak Panel |

### 2c. API i nazewnictwo

- `Theme::createDefaultTheme()` → stary; `Theme::createWindows95Theme()` → nowy (`src/theme.hpp:32-33`, `src/theme.cpp:47-52`). Dwie nazwy na jeden wygląd: `Win9x` vs `Windows95`.
- C-API ma tylko `sdlgui_theme_win9x` (stary); autentycznego nie da się włączyć z C (`src/sdl_gui.h:170`, `src/sdl_gui_c_api.cpp:228-246`).
- Docs opisują oba obok siebie bez rozstrzygnięcia który jest kanoniczny: `docs/release/resources.md:130-131` ("Klasyczny" vs "Autentyczny"), `docs/release/patterns.md:108-111`, `docs/release/core.md:319-322`.
- Skill `skills/sdl-gui/references/gui-app.md:127` wymienia "Win9x/Light/HighContrast" — nie wspomina `Windows95`.
- Przykłady `27_themes.cpp` / `28_theme_playground.cpp` przełączają tylko Light/Dark — Win95 w ogóle nie występuje w playgroundzie.

### 2d. Test przypina stary preset

`tests/test_theme.cpp:165` ("Default theme has per-state Button styles") wymaga:
`Hover.bg > Normal.bg` i `Pressed.bg < Normal.bg`. Przechodzi na starym (192 → 223 → 160),
na nowym Pressed ma **ten sam bg** co Normal (192, różni się tylko bevel) — test zablokuje podmianę defaultu.

## 3. Decyzja

**Kanonem jest `createWindows95Theme()` (bevel).** Uzasadnienie: to jedyny preset
spójny z systemem faz 3D (`applyBevelToStyle`, `drawResolvedBorder`, parserowy shorthand
`bevel: Raised|Sunken`, paleta `constants::kWin95*`, przykład 48, testy bevel).
Stary `createWin9xTheme()` staje się cienkim aliasem dla kompatybilności i znika w kolejnym wydaniu.

Odrzucona alternatywa: trzymanie obu z "jasnym podziałem ról" (np. Win9x = lekki bez bevel).
Odrzucona, bo: (a) nikt nie potrzebuje dwóch szarości różniących się o 1–2 piksele ramki,
(b) podwaja koszt każdego przyszłego audytu widgetów, (c) myli użytkowników C-API i skilli.

## 4. Plan prac — fazy

### Faza 0 — Paleta: jedno źródło prawdy
Pliki: `src/constants.hpp`, `src/theme_presets.hpp`.
1. Do `constants.hpp` dopisać brakujące kolory kanoniczne z jedną nazwą:
   `kWin95Navy {0,0,128}`, ewentualnie `kWin95DisabledFace` **tylko jeśli** świadomie
   trzymamy `212,208,200` (decyzja w fazie 1 — rekomendacja: nie, Face 192 jak w nowym).
2. Kolory-widma `{64,64,64}` i `{160,160,160}` **usunąć** (nie występują w realnym Win95;
   Pressed w nowym to Face+Sunken, nie ciemniejszy bg).
3. Z `createWin9xTheme()` usunąć cały blok lokalnych `constexpr` (l.22–30), używać wyłącznie `constants::kWin95*`.
4. Z `createWindows95Theme()` usunąć lokalne `kNavy/kWindowText/kWhite/kHighlightText/kDisabledText`
   na rzecz `constants::` (albo nowej `palette::win95`, jeśli przy okazji robimy porządek
   z `docs/refactor_plan.md:272` — ale to osobny plaster, nie mieszać).

### Faza 1 — Jeden preset: alias + podmiana defaultu
Pliki: `src/theme_presets.hpp`, `src/theme.cpp`, `src/theme.hpp`.
1. Treść `createWin9xTheme()` zastąpić delegacją:
   ```cpp
   [[deprecated("Use createWindows95Theme() — Win9x is now an alias")]]
   inline Theme createWin9xTheme() { return createWindows95Theme(); }
   ```
   (kolejność definicji w pliku: `createWindows95Theme` przed aliasem).
2. `Theme::createDefaultTheme()` → `ThemePresets::createWindows95Theme()`.
3. `Theme::createWindows95Theme()` zostaje (kanon, bez zmian semantyki).
4. Sprawdzić wszystkie miejsca wołające default (GUIManager ctor, GUIContext, bench, testy):
   zmiana bg Pressed 160→192 i Panel border 2→flat **zmieni wygląd** — to zamierzone,
   ale wymaga przebiegu `./nob examples` wzrokowo (zwłaszcza przykład 48 jako referencja).

### Faza 2 — Domknięcie pokrycia typów w kanonie
Plik: `src/theme_presets.hpp` (`createWindows95Theme`).
1. Dodać jawne wpisy (flat Face, `borderRadius=0`, bez bevel — jak Panel):
   `DialogBox`, `FileDialog` (okna w Win95 to Face).
2. Świadomie **nie** dodawać: `RadioGroup, Cursor, ArcContainer, ShaderPanel, GUIElement, Unknown`
   (fallback do defaultu) — dopisać komentarz w kodzie, że to zamierzone, żeby kolejny audyt
   nie zgłaszał "dziury".
3. Zaudytować `Checkbox/RadioButton`: pudełko rysowane przez widget — sprawdzić, czy czyta
   `kWin95*` na twardo czy theme; jeśli na twardo, dopisać `// intentional` albo podpiąć pod theme
   (osobny mini-plaster, nie zmieniać rysowania w tym planie).

### Faza 3 — Koniec nadużycia `borderColor` (StyleResolver dokończony)
Plik: `src/theme_presets.hpp` (`createWindows95Theme`).
1. `Slider` / `RangeSlider`: ustawić `thumbColor = kWin95Shadow` (zamiast `borderColor`),
   `borderColor` wyczyścić/`reset()` — fallback w `Style` i tak wróci do `borderColor`,
   więc stare motywy nie pękną, a semantyka będzie poprawna.
2. `ProgressBar`: ustawić `fillColor = kWin95Navy` (zamiast `borderColor`), `borderColor` zostawić
   puste — sprawdzić w `progress_bar.cpp`, które pole czyta (komentarz w presecie mówi "borderColor = fill").
3. `StringGrid`: zostawić `borderColor` jako kolor linii (udokumentowane odstępstwo), dopisać komentarz.
4. Klucz render-cache już domiesza `thumb/fill` (StyleResolver faza 1) — bez zmian.

### Faza 4 — C-API: parytet z C++
Pliki: `src/sdl_gui.h`, `src/sdl_gui_c_api.cpp`, `tests/test_sdl_gui_c_api.cpp`.
1. Dodać `void sdlgui_theme_windows95(sdlgui_t gui);` (kanon, woła `createWindows95Theme()`).
2. `sdlgui_theme_win9x` zostawić jako alias wołający to samo (kompatybilność ABI — 10 przykładów C w `examples/c/`).
3. Test: w `test_sdl_gui_c_api.cpp:74` dopisać case wołający nowy preset i sprawdzający np.
   bevel Button Normal (`borderColorOuterTopLeft == Highlight`).

### Faza 5 — Testy
Plik: `tests/test_theme.cpp`.
1. `TEST_CASE "Windows95 theme"` (l.184) — zostaje bez zmian (kanon, już zielony).
2. Sekcję `"Default theme has per-state Button styles"` (l.165) przepisać na nową semantykę:
   - Hover bg jaśniejszy od Normal (223 > 192) — zostaje;
   - Pressed: **bg == Normal (192), różni się bevel** (`borderColorOuterTopLeft == Shadow`, `InnerTopLeft == DarkShadow`) — zamiast `Pressed.bg < Normal.bg`.
3. Dodać `TEST_CASE "Win9x alias"`:
   - `createWin9xTheme()` i `createWindows95Theme()` dają identyczne style dla Button/Panel/TextInput
     (po `getStyle`, czyli po mergu z defaultem) — pin parytetu, żeby alias nie oddryfował.
   - `createDefaultTheme()` == `createWindows95Theme()` dla tych samych typów.
4. Opcjonalnie: case "nieustawiane typy fallbackują do Face 192 bez bevel" (DialogBox po fazie 2 ma jawny Face; Unknown dalej fallback).

### Faza 6 — Docs, przykłady, skill
1. `docs/release/resources.md:120-134` — `createDefaultTheme()` deleguje do `createWindows95Theme()`;
   w tabeli `createWin9xTheme()` oznaczyć `(deprecated alias)`.
2. `docs/release/core.md:319-322`, `docs/release/patterns.md:108-111` — jw., jedno zdanie
   "jeśli widziałeś Win9x w starszych przykładach, to ten sam motyw".
3. `docs/refactor_plan.md:276` — wykreślić punkt (zrobione tutaj).
4. `readme.md:663`, `README.pl.md:401` — doprecyzować "(bevel, autentyczny Win95/98)".
5. `skills/sdl-gui/references/gui-app.md:17,127,161` + `SKILL.md:57` — dopisać `createWindows95Theme`
   jako opcję (obok Dark), przykład zostaje na Dark.
6. `examples/27_themes.cpp`, `examples/28_theme_playground.cpp` — dodać RadioButton "Win95"
   obok Light/Dark (2–3 linijki, wzór z istniejącego `setOnChange`), żeby playground pokazywał kanon.
7. `examples/48_win95_bevel.cpp` — bez zmian kodu (już na kanonie); po fazie 1 pozostałe ~40 przykładów
   na defaulcie automatycznie dostaną bevel-look — przebieg wzrokowy.

### Faza 7 — Sprzątanie (po zielonych testach)
1. `git grep -n "64, 64, 64\|160, 160, 160\|212, 208, 200" src/theme_presets.hpp` — ma być pusto
   (poza ewentualnym udokumentowanym `kWin95DisabledFace`).
2. `git grep -n "createWin9xTheme"` — dozwolone wystąpienia: definicja aliasu, test parytetu,
   docs z adnotacją deprecated, C-API alias. Zero w przykładach/testach jako "prawdziwy" preset.
3. Wpis w `AGENTS.md` → "Bieżący stan i ostatnie zmiany" (format jak poprzednie: co/efekt/pliki)
   + dopisanie na górę `CHANGELOG.md`, jeśli repo tego wymaga dla znaczących zmian.

## 5. Kryteria akceptacji

- `./nob test` — 45/45 zielone (w tym przepisana sekcja Default-Button i nowy case aliasu).
- `./nob examples` — 48/48 się buduje; wzrokowo przykład 48 bez zmian, pozostałe na defaulcie
  w bevel-looku (szare Face, Raised buttony, Sunken pola).
- `./nob release` — zielone (dist + C smoke).
- `git grep createWin9xTheme` — tylko alias/test/docs-adnotacja/C-alias.
- Bench: nie wymaga benchmarku (zmiana dotyczy wyłącznie wartości styli, nie gorącej ścieżki;
  `getComposedStyle` cache'owane per epoka — jak w StyleResolver faza 1).

## 6. Ryzyka i non-goals

- **Zmiana wyglądu defaultu** (Pressed 160→192, Panel border 2→flat, Hover pól traci navy):
  zamierzona, ale widoczna w screenshotach/testach pikselowych jeśli któryś pinuje stary default
  (`test_render_pixel.cpp`, `test_render_cache.cpp` używają `createDefaultTheme()` — przed fazą 1
  sprawdzić, czy któryś zakłada konkretne piksele defaultu; jeśli tak, zaktualizować razem z fazą 5).
- **Nie ruszamy w tym planie**: rysowania widgetów (Checkbox-box na twardo?), `BorderRenderer`,
  klucza cache, parsera `bevel:` ani layoutów — tylko wartości w presecie + aliasy + docs.
- Deprecation a nie kasowanie: `createWin9xTheme` i `sdlgui_theme_win9x` zostają (ABI przykładów C),
  usunięcie najwcześniej w kolejnym wydaniu po migrze przykładów.
