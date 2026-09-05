# Plan dużego refaktoru SDL GUI

> Status: propozycja (2026-09-05). Punkt 5 (unifikacja TextArea/TextEditable)
> zrealizowany — patrz `AGENTS.md` → „Unifikacja TextArea z TextEditable".
> Reszta to otwarta lista — bierz po jednym punkcie, każdy kończ zielonymi
> testami (`./nob test`) i zbudowanymi przykładami (`./nob examples`).

## TL;DR — 6 zmian strukturalnych

| # | Zmiana | Co kasuje |
|---|--------|-----------|
| 1 | `EventDispatcher` + `FocusManager` + `PointerCapture` | DFS w każdym widgecie, `m_isHovered`-gating, focus-steal, duchy focus-overlay, cursor-forward w gałęzi capture |
| 2 | `Layout pass (Measure/Arrange)` + `Anchor` jako enum | magiczne floaty (`<0/0-1/>1/==0.5`), `onSizeChanged`, ręczne centrowanie labeli, dummy `(0,0)` w parserze, fallback `800x600` |
| 3 | `StyleResolver (ComputedStyle)` + `ComponentType: enum` | `unordered_map<string,...>`, hash stringów w renderze, `mergeWith` na gorąco, bevel-`if` per widget, nadużycie `borderColor` jako thumb/fill |
| 4 | Jeden `TextModel (char-index UTF-8)` | ✅ ZROBIONE — duplikacja TextInput/TextArea, bugi bajtowe |
| 5 | `Lifetime Handle (SlotMap/generation, weak_ptr)` | `m_liveElements + raw*`, `isElementAlive`-guardy w lambdach, wiszące `c_str()` w C-API, niestabilne indeksy w edytorze |
| 6 | `WidgetFactory` + `OverlayStack` + `RenderPolicy` | 3 kopie `if type=="Button"`, singleton-menu/tooltip ping-pong, `isOverlay()`, `wantsDirectRender` + 3 poziomy cache |

Kolejność odklejania: `5 Lifetime/Focus → 3 Style → 2 Layout → 1 Event → 6 Factory/Overlay/Render`.
Każdy punkt usuwa hacki z punktów niższych (np. Layout usuwa połowę powodów,
dla których Event musi robić DFS).

---

## 1. Event / hover / focus

### Obecne hacki
- `src/gui.cpp:295-324 handleEvent` — moloch: DFS dzieci + `if(!enabled)` +
  hover + `processButtonEvent` + `processRightClick(return true)`. Każdy widget
  nadpisujący musi pamiętać o bazie.
- `src/gui.hpp:136 processRightClick` protected helper — `Button/TextInput/
  TextArea` muszą go ręcznie wołać, inaczej RMB nie działa.
- `src/gui.cpp:334-359 processButtonEvent` czyta `m_isHovered` (stawiany tylko
  na `MOTION`) zamiast `contains(x,y)` z eventu. `SDL3` ma `x/y` w evencie.
- Duplikacja DFS: `button.cpp:43, panel.cpp:23, checkbox.cpp:32,
  string_grid.cpp:611, text_area.cpp:218` — każdy parsuje `SDL_EVENT_*`.
  `RadioButton:44` nie obsługuje `BUTTON_UP`, wnioskuje klik ze zmiany
  `Pressed->Hover`. `Slider/RangeSlider`: `if(base::handleEvent) return true`
  — dzieci zjadają track-drag.
- Focus-steal: `button.cpp:52, checkbox.cpp:35` robią `setKeyboardFocus+capture`
  w `BUTTON_DOWN`. `context_menu.cpp:51 hide()` spaceruje po rodzicach i oddaje
  focus — inaczej `gui_manager.cpp:151 render` maluje focus-overlay ukrytego
  buttona („duch klikniętego itemu").
- `gui_manager.cpp:102 click-outside` sprawdza tylko `contains`, nie
  `canGetKeyboardFocus`. `gui_manager.cpp:69 capture short-circuit` + specjalny
  forward do `cursor->handleEvent`. `text_input.cpp:255` vs `text_area.cpp:268`
  — inne guardy czyszczenia fokusu.
- `TextArea` (przed unifikacją): pisanie na `m_isHovered || hasFocus` (hover
  wystarczał), drag tylko w `m_isHovered` (gubił się poza bounds).

### Docelowo
`Dispatcher{hitTest, dragGesture, focusPolicy}`. Widget implementuje
`onPress/onDrag/onKey` zamiast `SDL_EVENT_*`. Jeden `contains()` + hover/tooltip
w bazie (Template Method: `handleEvent(){propagate; handleSelf();}` — RMB tylko
w bazie). Fokus tylko przez `FocusManager::requestFocus`, popup pushuje
`FocusScope` z auto-restore. Cursor to `MousePositionService`, nie `GUIElement`.
`PointerCapture{owner, autoReleaseOnUp, moveOutside}` w dispatcherze — widget
dostaje `onDragMove` także poza bounds.

### Edge-case'y, które znikają
pisanie bez fokusu • drag gubiony poza widgetem • focus-overlay ukrytego
buttona • kursor zamierający podczas dragu (brak forwardu) • dzieci zjadające
drag rodzica • klik jako zmiana `Pressed->Hover` • rozjazdy guardów
`hasFocus` vs `m_isHovered` między widgetami.

---

## 2. Layout / Anchor

### Obecne hacki
- `src/gui.cpp:739 hash(type), 846 toPixels, 871/895 if(left==0.5f)`:
  `<0=unset, 0-1=%, >1=px, 0.5=center`. `1.0` to `100%`, więc `1px`
  nieosiągalne; porównanie float na `==`; `0.5` znaczy naraz `50%` i `centruj`.
- `src/gui.cpp:821 m_originalW/H` — zapamiętane raz (`if both==0`), nigdy nie
  aktualizowane. `layout_parser.cpp:299 setAnchor+storeOriginal` po `setSize`
  psuje matematykę center.
- `src/gui.hpp:156 onSizeChanged` — istnieje tylko bo `Button(0,0)` + parser
  `setSize()` zostawiał label na ujemnych coords. Kopie hacka:
  `button.cpp:6, slider.cpp:31, tab_control.cpp:13 (current_x+=width+5)`.
- `src/gui_manager.cpp:47 addElement guard window>0` vs `gui.cpp:264 addChild`
  bez guarda; `handleResize:384` updatuje tylko `hasAnchor()` — top-level bez
  anchora ale z dziećmi z anchorami nie dostaje propagacji.
- `scroll_area.hpp:29 updateLayout()` bez args shadowuje
  `GUIElement::updateLayout(w,h)`.
- `context_menu.cpp:137 fallback 800x600` gdy `getWindowSize()==0x0`
  (`m_windowWidth=0` na starcie).
- `anchor.hpp:29-119` — trzy rozproszone silniki: `applyAnchor/updateLayout/
  onParentResize` + `ScrollArea::updateLayout` + `DialogBox::layoutButtons`
  (ręczne piksele). Brak fazy `Measure/Arrange`, brak `minSize/contentSize`.

### Docelowo
`Viewport{NonZero size}` wstrzykiwany do `GUIManager` w ctorze (brak `0x0`).
`Anchor{Left|Center|Right|Stretch}` jako enum, nie float. Jeden `LayoutPass`:
`ILayoutManager{measure(constraints)->Size; arrange(rect);}` — implementacje
`AnchorLayout, StackLayout, FlexLayout`. `Anchor` to jeden z managerów, nie pole
bazy. `layoutChildren()` zamiast `onSizeChanged`. Parser tworzy od razu docelowy
`rect` przez `WidgetFactory::create(type,rect,props)`, bez dummy `(0,0)`.
`DialogBox/FileDialog/Editor` na `StackLayout` — znika ręczna matematyka.

### Edge-case'y, które znikają
`1px` nieosiągalne • `0.5` = `%` i `center` naraz • historia `m_original`
rozjeżdżająca center • brak propagacji resize do dzieci bezanchrowego rodzica •
niespójny guard `>0` • `ScrollArea` vs baza — dwa `updateLayout` • tooltip/menu
na `(0,0)` przed pierwszym resize • labelki na ujemnych coords po `setSize`.

---

## 3. Style / Theme / Bevel

### Obecne hacki
- `style.hpp:31 12x optional`, `mergeWith 12x if`, `operator== 40 linii`.
  `getComposedStyle()` wołana w `draw()` i 2x w `buildRenderCacheKey()`,
  za każdym razem hash stringa typu + kolory + `fontName`. `texture`
  porównywana przez raw pointer w kluczu cache.
- `theme.hpp:26 unordered_map<string,array<Style,4>>`, klucz
  `getComponentType()` (`"Button"`…). Literówka = ciche spadnięcie do defaultu.
  `setTheme()` robi `markDirtyRecursively O(n)`.
- Bevel: `gui.cpp:653-722 drawBackgroundAndBorder (bevel>border)`,
  `radio_button.cpp:73` kopia brancha `hasBevel?drawStyleBevel:RenderRect`
  (bo `style.texture` = indykator, nie tło). `slider/range/progress:
  thumbColor = borderColor`. `theme_presets.hpp:290 withBevel()+reset
  w Disabled`. `layout_parser.cpp:407` shorthand `bevel:` + 4 jawne kolory,
  kolejność nadpisywania niejawna.

### Docelowo
`StyleResolver`: `ComputedStyle` płaski (bez optionali) + wersjonowanie
`themeEpoch/localEpoch`. `resolve(handle)` O(1), invalidacja tylko przy
`setTheme/setStyle`. `ComponentType:uint8_t`, ID internowane w `WidgetFactory`
— parser/C-API/edytor mapują string→ID raz na granicy, nie w pętli renderu.
`BorderStyle{plain|bevel{outerTL,outerBR,innerTL,innerBR}}` jako wariant +
jeden `BorderRenderer`. Widget deklaruje `supportsBevel`. `thumb/fillColor`
jako osobne pola stylu.

### Edge-case'y, które znikają
ciche spadanie do defaultu przy literówce • `O(n)` dirty przy zmianie themu •
bevel-`if` w każdym custom-draw • `borderColor` znaczące co innego per widget •
niejawna kolejność `bevel:` vs jawne kolory w parserze.

---

## 4. Tekst — ✅ ZROBIONE (2026-09-05)

`TextArea : public TextEditable` (wcześniej `: GUIElement` z równoległym API
byte-index). Usunięte duplikaty: tekst/selekcja/schowek/focus/blink/menu/locked.
`setLocked/isLocked` i `setContextMenuEnabled` w bazie (`TextInput::setLocked`
deleguje). Wszystkie pozycje w znakach UTF-8. Zlikwidowane: pisanie na hoverze,
drag gubiony poza bounds, wrap tylko ostatniego paragrafu, gubienie spacji,
brak łamania długich słów, mieszane jednostki bajt/znak, `strlen` na clipboard.
Szczegóły: wpis w `AGENTS.md` + `docs/release/widgets/TextArea.md`.

---

## 5. Lifetime (`m_liveElements`, focus/capture, C-API stringi, edytor)

### Obecne hacki
- `gui_manager.hpp:138 m_liveElements + register/unregister`, surowe
  `m_focus/m_capture/m_tooltipLabel`, guardy `if(isAlive(self))` w lambdach
  menu — łatwe do zapomnienia. `cleanup:173 hasAncestorMarkedForDeletion +
  komentarz o wiszących wskaźnikach`.
- `m_tooltipPanel/m_tooltipLabel` ping-pong `move()` zamiast `setVisible()`.
  `m_contextMenu raw* + static_cast` zamiast `unique_ptr` poza `m_elements`.
  `getActiveOverlay()` linear scan `O(n)` + założenie jednego modala.
- `sdl_gui_c_api.cpp` (1345 linii): `static_cast<Button*>` bez checka typu,
  `strcmp(type,"Label")==0` by znaleźć labelkę, `get_text()->c_str()` wisi po
  `setText`. `layout_parser.cpp:61`, `preview_window.cpp:63`, C-API: trzy kopie
  `if type=="Button"`.
- `preview_window.hpp:35 m_widgetMap<size_t,…>` — klucz-index niestabilny,
  `refresh=remove+create` traci focus/scroll.

### Docelowo
`SlotMap+Generation → WeakHandle`, focus/capture jako handle (samoczynny null
po zniszczeniu, bez spacerów po rodzicach). Jeden `WidgetFactory registry
string_view→creator` dla parsera/C-API/edytora. Tabela uchwytów
`id→{ptr,typeTag}` w C-API (check tagu, kod błędu; stringi kopiowane do bufora
callera). `ElementID uuid` + `Document→View diff` w edytorze (update props, nie
recreate).

### Edge-case'y, które znikają
zapomniany `isAlive`-guard • focus/capture wiszące po `cleanup` • tooltip/menu
jako przenoszony ownership + downcasty • `O(n)` skan modala • ten sam
`if type==` w 3 miejscach (rozjazd przy dodaniu widgetu) • utrata
focus/scroll/selekcji przy każdym `refresh` w edytorze • wiszący `c_str()`.

---

## 6. Render / cache / overlay

### Obecne hacki
- `gui.hpp:162 canShareRenderCache()=true` domyślnie — widget stanowy musi się
  sam wypisać, inaczej dzieli obcy cache. `label.hpp:17 suffix=m_text+font_size`
  (nie kolor/font — polega na bazie).
- `gui.cpp:398 render()`: ręczne `SDL_GetRectIntersection` + krojenie
  `src/dst`, `wantsDirectRender` czyści `dirty=false` bez cache,
  `if(rotation==0)` — dzieci w ogóle nie renderowane + focus-obwódka raz poza
  cache (`424`), raz wpieczona (`486`).
- `gui.cpp:493 wpiekanie dzieci` w teksturę rotowanego rodzica
  (`SetRenderTarget` tam-i-z-powrotem) — psuje granularność dirty.
  `markDirty(cascadeToParents=true)` brudzi ścieżkę do root.
  `kPruneThreshold=256` magiczna w `update()`.
- Obok globalnego cache: `string_grid:1202 własny m_localTextureCache bez
  limitu`, `text_input:80/text_area:830 m_textTexture`,
  `combobox:100/context_menu:87 createButtons() w draw()` — mutacja w ścieżce
  renderu. `Cursor` trzyma własny `m_mouseX/Y` + fallback `SDL_GetMouseState`
  (dwa źródła prawdy).
- `DialogBox::isOverlay()==true` leży w tym samym `m_elements` —
  `render/getActiveOverlay/collectFocusableElements` muszą go filtrować.
  `WindowManager::processEvents` = switch na `windowID`; `ScreenManager` rysuje
  wszystkie screeny ze stacka + `guiManager.render()` tylko dla top — dwie
  prawdy o widoczności.

### Docelowo
`RenderPolicy{StaticCacheable|DynamicDirect|TextRun}`, centralny `TextShaper
(text/font/color->texture)`, zakaz mutacji w `draw()`. Fokus jako
`renderFocusOverlay()` niezależny od rotacji; rotacja tylko na blit.
`OverlayStack` (tooltip/contextMenu/cursor/dialog) — overlay to warstwa
w `RenderGraph`, nie flaga w widgetcie. `AppShell`: `Window=viewport`,
`Screen=root node`, jeden event-bubbling `capture→target→bubble`.
LRU z limitem bajtów w `TextureManager` zamiast licznika w `update()`.

### Edge-case'y, które znikają
stale shared-texture po edycji • milczące współdzielenie cache przez widget
stanowy • focus malowany mimo ukrytego menu • wpiekanie dzieci unieważniające
dirty • mutacja hierarchii w `draw()` • dwa źródła pozycji kursora • modalność
filtrowana w 3 miejscach • resize nie płynący `Window→GUIManager` automatycznie.

---

## Drobne (przy okazji)

- `constants.hpp` — globalne kolory/tooltip/bevel Win95 w jednym worku; rozdzielić
  na `palette::win95`, `tooltip::…`.
- `gui_context.hpp:9-33` — 3 ctory duplikujące `setTheme+setWindowSize`;
  `GUIContext::run` ukrywa kolejność `processEvent→update→cleanup→render`.
- `ThemePresets::createWin9xTheme` vs `createWindows95Theme` — dwa presety Win95.
- `gui.cpp:192 setSize` zawsze `markDirty()` (brak early-out przy tym samym
  rozmiarze); `setParent()` invaliduje cache ale nie dirty — jedna metoda
  `moveTo(parent,x,y)`.
- `m_rotationCenter={-1,-1}` sentinel zamiast `optional<Point>`.

## Jak brać się za punkty

1. Jeden punkt = jeden wpis w `AGENTS.md` („Bieżący stan") + update
   `docs/release/…` jeśli zmienia semantykę widgetu.
2. Testy: `./nob test <filtr>` w trakcie, pełne `./nob test` + `./nob examples`
   na koniec (przykłady bez zmian kodu jeśli API kompatybilne — jak w punkcie 4).
3. Nowe bugi UTF-8/focus/layout dopisuj jako case'y w `tests/test_*.cpp`
   (wzór: `test_text_area.cpp` → „UTF-8 char-index model").
