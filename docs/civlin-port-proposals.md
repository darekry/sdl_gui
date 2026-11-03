# CivLin → sdl_gui: propozycje zmian i widgetów ułatwiających port

Data: 2026-11-03  
Status: propozycja robocza, nie implementacja  
Źródło potrzeb: `civlin/STATUS.md`, `civlin/docs/review/batches/*.json`,
`civlin/src/gui/*`, `civlin/docs/api-policy.md`

## 0. Streszczenie

CivLin ma 2392 funkcje w inwentarzu audytu; 1437 (60,1%) ma werdykt,
a 78 jest `BLOCKED` z `resolution: DEFERRED`. Około 25 z tych blokad to
warstwa GUI: dialogi Win16, kontrolki owner-draw, MenuZ, tracked dialog,
geometria itemów, metryki fontu i kursor. Pozostałe blokady (RSC, save,
turn/unit/map/city, DIB/HBITMAP, palety) nie należą do sdl_gui.

Najważniejszy wniosek: **największy zysk nie wymaga nowych widgetów**.
Wiele potrzebnych zdolności już istnieje w C++ (`GUIElement::getSize`,
`getAbsolutePosition`, `getParent`, `GUIManager::requestFocus`,
`FontManager::getTextSize`), ale nie jest wystawione w C API. CivLin jest
napisany w C17 i używa wyłącznie `dist/sdl_gui.h`, więc te zdolności są dla
niego niedostępne.

Proponowana kolejność:

1. **P0 — C API parity**: geometria, drzewo elementów, control ID,
   widoczność, focus, metryki tekstu, kursor z `SDL_Surface`.
2. **P1 — dialogi i menu**: `FormDialog` z rejestrem itemów, `MenuList`
   (MenuZ), hooki rysowania/eventów, modal runner, rozszerzenia `TextInput`.
3. **P2 — opcjonalne**: dialog units, per-row state w `ListView`,
   ewentualny `Image`/surface dla portretów w dialogach.

P0 + P1 daje ścieżkę dla około 25 z 78 blokad. To nie znaczy, że funkcje
automatycznie staną się `GOOD` — po stronie CivLin zostaje stan gry,
dekodowanie zasobów i re-audyt. Oznacza, że znika wymówka „brak fasady GUI”.

## 1. Zasady

- sdl_gui pozostaje biblioteką generyczną: zero logiki Civ, zero Win16.
- Zmiany C API są **addytywne**; nie usuwamy istniejących funkcji.
- Każda nowa funkcja dostaje test C API, przykład C i wpis w
  `docs/release/c_api.md`.
- Hooki rysowania dostają prawdziwy `SDL_Renderer*`, lokalny rect i clip.
  Żadnych atrap, żadnego „fake pixels”.
- Tekst gry FNT, mapa, palety, RSC oraz DIB/HBITMAP zostają w CivLin.
  sdl_gui dostarcza tylko punkty zaczepienia i geometrię.
- Nazwy w C API: `sdlgui_*`; typy pomocnicze: `sdlgui_*_t`; brak wycieku
  typów C++ do nagłówka C.

## 2. Mapa braków

| Potrzeba CivLin | Co jest dziś | Czego brakuje | Skutek w CivLin |
|---|---|---|---|
| `GETWINDOWRECT` / `GETDLGITEM` | C++ `getX/getY/getWidth/getHeight/getAbsolutePosition` | brak getterów rect w C API | `dialog_get_item_rect` |
| `SHOWWINDOW` | `sdlgui_element_set_visible` | brak `get_visible` / efektywnej widoczności | `dlg_show_item`, `dlg_hide_item` |
| `GETDLGITEM(id)` | stringowe `set_id/get_id` | brak wyszukiwania po drzewie i po control ID | `dlg_show_item`, `dlg_hide_item`, dialogi tracked |
| `SETFOCUS` / `GetFocus` | C++ `requestFocus`, `getKeyboardFocus` | brak w C API | kontrolki dialogowe, Tab/strzałki |
| `WM_COMMAND` od itemu | callbacki przycisków | brak control ID i notyfikacji do formularza | `dialog_ctrl_*`, dialogi tracked |
| Owner-draw / `DLGPROC` | C++ subclass `GUIElement` | brak hooków draw/event w C API | `dialog_ctrl_paint_*`, `dlg_city_name_edit_proc` |
| `DLGTEMPLATE` / tracked dialog | `DialogBox` z komunikatem i przyciskami | brak generycznego formularza z itemami i wynikiem | 0x89/0x61/0x8e/0x8f/0x94/0x1000 |
| MenuZ | `RadioGroup`, `ListView`, `MenuPopup` | brak listy modalnej z disabled rows, kolumnami i title width | `dialog_menuz`, `dialog_split_menu_text` |
| modal pump | `DialogBox` event-driven | brak synchronicznego runnera | 3 ręczne pętle w `civlin/src/gui/menu.c` |
| metryki fontu | C++ `FontManager::getTextSize` | brak C API, line height i alignment | `status_measure_row`, HoF |
| kursor | tekstury z pliku | brak `SDL_Surface` / system cursor | `cursor_set`, `title_anim_callback` |
| command ID menu | string `command` | brak int ID i mutowalnego tekstu itemu | `gui/menubar.c` robi `strtol` |
| edycja tekstu | `TextInput` z podstawowym API | brak max length, filtra znaków, key hooka | city-name prompt |
| dialog units | brak | brak helpera DLU | `dialog_get_item_rect`, formularze |
| per-row state | `ListView` bez enabled | brak enabled/checked per row | listy w dialogach (opcjonalnie) |

## 3. P0 — tanie i addytywne

### P0.1 Geometria, drzewo, widoczność, focus

Dodać do `src/sdl_gui.h` / `src/sdl_gui_c_api.cpp`:

```c
typedef struct {
    int x, y, w, h;
} sdlgui_rect_t;

int  sdlgui_element_get_rect(sdlgui_element_t e, sdlgui_rect_t* out);
int  sdlgui_element_get_absolute_rect(sdlgui_element_t e, sdlgui_rect_t* out);
int  sdlgui_element_get_visible(sdlgui_element_t e);
int  sdlgui_element_is_shown(sdlgui_element_t e);   /* self + ancestors */
int  sdlgui_element_get_enabled(sdlgui_element_t e);
sdlgui_element_t sdlgui_element_get_parent(sdlgui_element_t e);

size_t          sdlgui_element_get_child_count(sdlgui_element_t e);
sdlgui_element_t sdlgui_element_get_child(sdlgui_element_t e, size_t index);
sdlgui_element_t sdlgui_element_find_by_id(sdlgui_element_t root, const char* id);

int  sdlgui_element_request_focus(sdlgui_element_t e);
sdlgui_element_t sdlgui_get_focused_element(sdlgui_t gui);
void sdlgui_focus_next(sdlgui_t gui, int forward);

int  sdlgui_get_window_rect(sdlgui_t gui, sdlgui_rect_t* out);
```

Mapowanie na Win16:

- `GETWINDOWRECT` + `SCREENTOCLIENT` → `get_absolute_rect` i odjęcie
  absolutnego origin dialogu; `GETDLGITEM` → `find_by_control_id`.
- `SHOWWINDOW` → `set_visible` + `get_visible`; `is_shown` odpowiada
  efektywnej widoczności z uwzględnieniem przodków.
- `GETCLIENTRECT` → `sdlgui_get_window_rect` (dla CivLin to zwykle
  `0,0,w,h`; kontrakt far-pointer zostaje po stronie CivLin).
- `SETFOCUS` / `GetFocus` → `request_focus` / `get_focused_element`.

Cel: `dialog_get_item_rect`, `dlg_show_item`, `dlg_hide_item`,
`sys_get_client_rect` (częściowo), `config_save_prefs` (częściowo),
obsługa fokusu w `dialog_ctrl_*`.

Uwaga implementacyjna: `get_absolute_rect` musi wymusić aktualny layout
(`updateLayout`/cache) albo udokumentować, że jest ważny po `update()`.
C++ `getAbsolutePosition` ma cache unieważniany przy zmianie pozycji —
warto użyć tego samego mechanizmu. `request_focus` respektuje istniejące
`set_can_get_keyboard_focus(1)` i zwraca 0 dla elementu niefokusowalnego.

### P0.2 Control ID i wyszukiwanie itemów

```c
int  sdlgui_element_set_control_id(sdlgui_element_t e, int id);
int  sdlgui_element_get_control_id(sdlgui_element_t e);
sdlgui_element_t sdlgui_element_find_by_control_id(sdlgui_element_t root, int id);
```

`control_id` jest liczbą 16-bitową w CivLin (zakres Win16), ale API może
przyjąć `int` i walidować `0..0xffff`. Wyszukiwanie ogranicza się do
poddrzewa `root`, więc różne dialogi mogą mieć te same ID.

Cel: `GETDLGITEM`, `dlg_show_item`, `dlg_hide_item`, tracked dialog
0x89/0x61/0x8e/0x8f/0x94/0x1000, city-name edit.

### P0.3 Metryki tekstu i alignment

```c
typedef enum {
    SDLGUI_ALIGN_LEFT   = 0,
    SDLGUI_ALIGN_CENTER = 1,
    SDLGUI_ALIGN_RIGHT  = 2
} sdlgui_h_align_t;

typedef enum {
    SDLGUI_ALIGN_TOP    = 0,
    SDLGUI_ALIGN_MIDDLE = 1,
    SDLGUI_ALIGN_BOTTOM = 2
} sdlgui_v_align_t;

int  sdlgui_measure_text(sdlgui_t gui, const char* text, int font_size,
                         int* width, int* height, int* line_height);
int  sdlgui_get_font_metrics(sdlgui_t gui, int font_size,
                             int* line_height, int* ascent, int* descent);

void sdlgui_label_set_alignment(sdlgui_element_t e,
                                sdlgui_h_align_t h_align,
                                sdlgui_v_align_t v_align);
void sdlgui_label_set_font_size(sdlgui_element_t e, int font_size);
size_t sdlgui_button_get_label_buf(sdlgui_element_t e, char* buf, size_t len);
```

Semantyka:

- `width`/`height` = `TTF_GetStringSize` (bbox tekstu).
- `line_height` = `TTF_GetFontHeight`; to jest odpowiednik
  `Text_GetFontHeight`/`tmHeight`, którego potrzebuje `status_measure_row`.
- `h_align`: left/center/right; `v_align`: top/middle/bottom.

Cel: `status_measure_row`, `HoF_Show`, `hof_format_slot`,
`dialog_split_menu_text` (szerokość tytułu), etykiety w dialogach.

### P0.4 MenuBar: integer command ID i mutowalne itemy

```c
typedef void (*sdlgui_menu_command_id_callback_t)(sdlgui_element_t bar,
                                                  int command_id,
                                                  void* userdata);

sdlgui_menu_item_t sdlgui_menu_item_add_item_id(sdlgui_menu_item_t parent,
                                                const char* text, int command_id);
void sdlgui_menu_bar_set_on_command_id(sdlgui_element_t bar,
                                       sdlgui_menu_command_id_callback_t cb,
                                       void* userdata);
void sdlgui_menu_item_set_text(sdlgui_menu_item_t item, const char* text);
size_t sdlgui_menu_item_get_text_buf(sdlgui_menu_item_t item,
                                     char* buf, size_t len);
sdlgui_menu_item_t sdlgui_menu_item_find_by_id(sdlgui_element_t bar, int command_id);
```

Stringowa wersja `sdlgui_menu_item_add_item` zostaje dla kompatybilności.
CivLin mapuje `RT_MENU` z `civ.exe`; command ID to 16-bit `WM_COMMAND`
`wParam`. Dziś `src/gui/menubar.c` serializuje je do stringów i robi
`strtol`, co jest niepotrzebnym glue i utrudnia dynamiczne etykiety.

Cel: wierne `Menu_InitMainBar` / `Menu_DispatchCommand`, mniej kodu
pośredniego; nie odblokowuje bezpośrednio funkcji z listy `BLOCKED`, ale
upraszcza port menu.

### P0.5 Kursor z `SDL_Surface` / systemowy

```c
void sdlgui_cursor_set_surface(sdlgui_element_t e, sdlgui_cursor_state_t state,
                               SDL_Surface* surface, int hotspot_x, int hotspot_y);
void sdlgui_cursor_set_system(sdlgui_element_t e, SDL_SystemCursor cursor);
```

Kontrakt: `set_surface` kopiuje piksele do wewnętrznej tekstury i nie
przejmuje ownershipu `surface`; hotspot jest w pikselach. `set_system`
jest fallbackiem, gdy CivLin nie chce parsować `RT_CURSOR`.

Cel: `cursor_set` i `title_anim_callback`. CivLin nadal parsuje NE
`RT_CURSOR` i buduje `SDL_Surface` w `win16`/`video`; sdl_gui tylko
wyświetla kształt. Nie zgadujemy kształtów po stronie biblioteki.

## 4. P1 — dialogi, menu i hooki

### P1.1 `FormDialog` — generyczny host dialogu

Obecny `DialogBox` jest kompozytem „komunikat + przyciski”. CivLin
potrzebuje pustego dialogu, do którego dodaje itemy o znanych ID,
a następnie je pokazuje, ukrywa, przenosi i odczytuje ich rect.

```c
typedef void (*sdlgui_form_command_callback_t)(sdlgui_element_t form,
                                               int command_id,
                                               void* userdata);

sdlgui_element_t sdlgui_form_create(sdlgui_t gui, const char* title,
                                    int x, int y, int w, int h, int flags);

sdlgui_element_t sdlgui_form_add_widget(sdlgui_element_t form, int id,
                                        sdlgui_element_t child); /* top-level child */
sdlgui_element_t sdlgui_form_add_static(sdlgui_element_t form, int id,
                                        int x, int y, int w, int h,
                                        const char* text, int style);
sdlgui_element_t sdlgui_form_add_button(sdlgui_element_t form, int id,
                                        int x, int y, int w, int h,
                                        const char* text);
sdlgui_element_t sdlgui_form_add_edit(sdlgui_element_t form, int id,
                                      int x, int y, int w, int h,
                                      const char* text);
/* Typy callbacków: patrz P1.3 (draw/event hook). */
sdlgui_element_t sdlgui_form_add_custom(sdlgui_element_t form, int id,
                                        int x, int y, int w, int h,
                                        sdlgui_draw_callback_t paint,
                                        sdlgui_event_callback_t event,
                                        void* userdata);

sdlgui_element_t sdlgui_form_find_item(sdlgui_element_t form, int id);
int sdlgui_form_get_item_rect(sdlgui_element_t form, int id, sdlgui_rect_t* out);
int sdlgui_form_set_item_visible(sdlgui_element_t form, int id, int visible);
int sdlgui_form_set_item_enabled(sdlgui_element_t form, int id, int enabled);
int sdlgui_form_set_item_text(sdlgui_element_t form, int id, const char* text);
int sdlgui_form_set_item_rect(sdlgui_element_t form, int id,
                              int x, int y, int w, int h);

void sdlgui_form_end(sdlgui_element_t form, int result);
int  sdlgui_form_get_result(sdlgui_element_t form);
int  sdlgui_form_is_closed(sdlgui_element_t form);
void sdlgui_form_set_on_command(sdlgui_element_t form,
                                sdlgui_form_command_callback_t cb,
                                void* userdata);

/* Wspólne dla DialogBox/FormDialog: zamknij z wynikiem (dla Esc itd.). */
void sdlgui_dialog_close_with_result(sdlgui_element_t dialog, int result);
```

Opcjonalny wariant „szablonowy”, który najbardziej zbliża CivLin do
`DLGTEMPLATE` bez emulowania Win16:

```c
typedef enum {
    SDLGUI_DLGITEM_STATIC = 0,
    SDLGUI_DLGITEM_BUTTON = 1,
    SDLGUI_DLGITEM_EDIT   = 2,
    SDLGUI_DLGITEM_CUSTOM = 3
} sdlgui_dlgitem_type_t;

typedef struct {
    int id;
    int type;              /* sdlgui_dlgitem_type_t */
    sdlgui_rect_t rect;    /* już w pikselach, po DLU */
    const char* text;
    uint32_t style;
    int enabled;
    int visible;
} sdlgui_dlgitem_desc_t;

sdlgui_element_t sdlgui_form_create_from_items(
    sdlgui_t gui, const char* title, int x, int y, int w, int h,
    const sdlgui_dlgitem_desc_t* items, size_t count);
```

Cel: tracked dialogi 0x89/0x61/0x8e/0x8f/0x94/0x1000 oraz:
`city_prompt_city_name`, `dlg_prompt_city_name_modal`,
`dlg_city_name_edit_proc`, `city_pump_production_dialog`,
`diplo_open_contact_dialog`, `diplo_prompt_contact`,
`diplo_prompt_treaty_menu`, `worldgen_load_earth`,
`newgame_wizard_run`, `dialog_open_help` (host; dekod HLP zostaje
w CivLin).

Uwagi:

- `FormDialog` powinien dziedziczyć po `ModalDialog` (overlay, Esc,
  focus) i trzymać rejestr `{id, GUIElement*}` bez ownershipu — dzieci
  są własnością panelu.
- `add_button` może automatycznie podłączyć notyfikację
  `(id, BN_CLICKED)` do `on_command`, ale nie może wymuszać konkretnego
  zachowania.
- `form_create_from_items` nie jest parserem `DLGTEMPLATE`: CivLin
  tłumaczy własny template na tablicę descriptorów. Dzięki temu
  biblioteka nie zna Win16.

### P1.2 Modal runner

Obecny model jest event-driven: `DialogBox` tworzy widget, a aplikacja
sama pompuje eventy. CivLin ma trzy ręczne pętle modalne w
`src/gui/menu.c` (MenuZ, dialog tekstowy, city-name) i będzie dokładać
kolejne. To miejsce na wspólny, testowalny runner:

```c
typedef struct {
    int keep_background;       /* 0 = wyczyść tło, 1 = zostaw klatkę */
    int clear_r, clear_g, clear_b;
    int cancel_key;            /* np. SDLK_ESCAPE; 0 = brak */
    int cancel_result;         /* wynik dla Esc */
    void (*frame_cb)(void* userdata);                 /* idle/timery gry */
    int  (*event_filter)(const SDL_Event* e, void* userdata); /* 1 = consumed */
} sdlgui_modal_options_t;

int sdlgui_modal_run(sdlgui_t gui, sdlgui_element_t dialog,
                     const sdlgui_modal_options_t* opts,
                     int* result_out);
```

Runner pompuje `SDL_PollEvent` → `sdlgui_process_event` → `update` →
`cleanup` → `render` → `SDL_RenderPresent`, dopóki dialog nie zostanie
zamknięty (`sdlgui_form_end` / `sdlgui_dialog_close_with_result`).
`frame_cb` pozwala CivLin uruchomić własne timery/animacje w tle;
`event_filter` obsługuje quit i globalne skróty, zanim event trafi do
widgetów. `keep_background` usuwa potrzebę ręcznego `gui_modal_keep_background`.

Cel: wszystkie modalne dialogi i MenuZ; usunięcie duplikacji pętli;
spójne zachowanie resize/recenter/Esc/Enter/quit.

Jeżeli pełny runner okaże się zbyt gry-specific, alternatywa to
`sdlgui_modal_pump(gui, dialog, options)` wywoływany z pętli CivLin —
ale kontrakt wyniku i tak powinien być w bibliotece.

### P1.3 Hooki draw/event i `CustomControl`

To najważniejsza zmiana dla kontrolek owner-draw i `DLGPROC`:

```c
typedef enum {
    SDLGUI_DRAW_BEFORE  = 0,
    SDLGUI_DRAW_REPLACE = 1,
    SDLGUI_DRAW_AFTER   = 2
} sdlgui_draw_phase_t;

typedef void (*sdlgui_draw_callback_t)(sdlgui_element_t e,
                                       SDL_Renderer* renderer,
                                       const sdlgui_rect_t* local_rect,
                                       const sdlgui_rect_t* clip_rect,
                                       void* userdata);

typedef int (*sdlgui_event_callback_t)(sdlgui_element_t e,
                                       const SDL_Event* event,
                                       int local_x, int local_y,
                                       void* userdata);

void sdlgui_element_set_draw_hook(sdlgui_element_t e,
                                  sdlgui_draw_phase_t phase,
                                  sdlgui_draw_callback_t cb,
                                  void* userdata);
void sdlgui_element_set_event_hook(sdlgui_element_t e,
                                   sdlgui_event_callback_t cb,
                                   void* userdata);
void sdlgui_element_notify_command(sdlgui_element_t e, int command_id);

sdlgui_element_t sdlgui_custom_control_create(
    sdlgui_t gui, sdlgui_element_t parent,
    int x, int y, int w, int h,
    sdlgui_draw_callback_t paint,
    sdlgui_event_callback_t event,
    void* userdata);
```

Semantyka:

- `DRAW_REPLACE` wyłącza domyślne rysowanie i cache; callback dostaje
  prawdziwy renderer, rect lokalny i clip. To pozwala CivLin rysować
  FNT/gryfami bez przenoszenia atlasu do sdl_gui.
- `DRAW_BEFORE`/`AFTER` uzupełniają domyślny widget (np. tło editu).
- `event_hook` jest wołany przed domyślnym `handleSelf()`; zwrot `1`
  konsumuje event. Dzięki temu `dlg_city_name_edit_proc` może
  przechwycić WM_CHAR/WM_COMMAND, a `TextInput` zachowuje edycję.
- `notify_command` idzie do najbliższego `FormDialog` i wywołuje jego
  `on_command` — odpowiednik `WM_COMMAND` do rodzica.
- `custom_control_create` to wygoda: panel z `DRAW_REPLACE`, event
  hookiem i focusem; nie wymaga nowej klasy w C++.

Opcjonalnie, dla wygody autorów kontrolek, można dodać minimalny
`paint` context:

```c
void sdlgui_paint_fill_rect(sdlgui_element_t e, int x, int y, int w, int h,
                            SDL_Color color);
void sdlgui_paint_frame(sdlgui_element_t e, int x, int y, int w, int h,
                        SDL_Color color, int thickness);
void sdlgui_paint_line(sdlgui_element_t e, int x1, int y1, int x2, int y2,
                       SDL_Color color);
void sdlgui_paint_polyline(sdlgui_element_t e, const SDL_Point* pts, size_t n,
                           SDL_Color color);
void sdlgui_paint_polygon(sdlgui_element_t e, const SDL_Point* pts, size_t n,
                          SDL_Color color);
void sdlgui_paint_pixel(sdlgui_element_t e, int x, int y, SDL_Color color);
```

Cel: `dialog_ctrl_paint_button`, `dialog_ctrl_paint_static`,
`dialog_ctrl_radio_proc`, `dialog_ctrl_static_proc`,
`dialog_ctrl_user_proc`, `dialog_ctrl_region_proc`,
`dlg_city_name_edit_proc`, `diplo_setup_contact_portrait` (część
rysująca; porty/art zostają w CivLin), `worldgen_probe_custom_font`
(jeśli font ma być probe'owany przez API).

### P1.4 `MenuList` — lista modalna w stylu MenuZ

MenuZ nie jest drzewem menu (MenuBar) ani gridem (ListView): to modalna
lista z tytułem, jedną lub dwiema kolumnami, disabled rows, preselekcją,
timerem i kodem anulowania.

```c
typedef struct {
    const char* text;   /* kopiowany przy create; może być NULL dla "" */
    int id;
    int enabled;
} sdlgui_menu_list_item_t;

typedef void (*sdlgui_menu_list_select_cb_t)(sdlgui_element_t e,
                                             int item_id,
                                             const char* text,
                                             void* userdata);

sdlgui_element_t sdlgui_menu_list_create(
    sdlgui_t gui, sdlgui_element_t parent,
    int x, int y, int w, int h,
    const char* title,
    const sdlgui_menu_list_item_t* items, size_t count);

void sdlgui_menu_list_set_columns(sdlgui_element_t e, int columns);
void sdlgui_menu_list_set_title_width(sdlgui_element_t e, int px);
void sdlgui_menu_list_set_disabled_mask(sdlgui_element_t e, uint32_t mask);
void sdlgui_menu_list_set_preselect(sdlgui_element_t e, int item_id);
void sdlgui_menu_list_set_cancel_id(sdlgui_element_t e, int item_id);
void sdlgui_menu_list_set_timer_ms(sdlgui_element_t e, uint32_t ms);

int  sdlgui_menu_list_get_selected_id(sdlgui_element_t e);
void sdlgui_menu_list_set_on_select(sdlgui_element_t e,
                                    sdlgui_menu_list_select_cb_t cb,
                                    void* userdata);

/* Opcjonalnie: własny render wiersza (np. FNT z CivLin). */
typedef void (*sdlgui_menu_list_row_cb_t)(sdlgui_element_t e,
                                          SDL_Renderer* renderer,
                                          const sdlgui_rect_t* row_rect,
                                          const char* text, int item_id,
                                          int selected, int enabled,
                                          void* userdata);
void sdlgui_menu_list_set_row_callback(sdlgui_element_t e,
                                       sdlgui_menu_list_row_cb_t cb,
                                       void* userdata);
```

Wymagania:

- klawiatura: strzałki, Tab, Enter, Esc, pierwsza litera; pomijanie
  disabled; cykliczne zawijanie.
- mysz: hover/press/click; klik w disabled nie wybiera.
- `set_title_width` obsługuje split tytułu (`dialog_split_menu_text`).
- `set_columns` obsługuje `g_nMenuZCols`.
- `set_disabled_mask` obsługuje `g_wMenuZDisabledMask`.
- `set_timer_ms` obsługuje 100 ms timer MenuZ.
- `set_row_callback` pozwala rysować tekst gry FNT zamiast TTF.

Cel: `dialog_menuz`, `dialog_split_menu_text`,
`diplo_run_contact_menu`, `diplo_prompt_treaty_menu`,
`diplo_open_contact_dialog` (menu tail), `unit_setup_orders_menu`
(PENDING, ale ten sam problem).

### P1.5 `TextInput`: limit, filtr, hook klawisza

```c
typedef int (*sdlgui_char_filter_t)(unsigned codepoint, void* userdata);
typedef int (*sdlgui_key_callback_t)(sdlgui_element_t e,
                                     const SDL_KeyboardEvent* key,
                                     void* userdata);

void sdlgui_text_input_set_max_length(sdlgui_element_t e, size_t max_len);
void sdlgui_text_input_set_char_filter(sdlgui_element_t e,
                                       sdlgui_char_filter_t cb,
                                       void* userdata);
void sdlgui_text_input_set_on_key(sdlgui_element_t e,
                                  sdlgui_key_callback_t cb,
                                  void* userdata);
void sdlgui_text_input_set_cursor_pos(sdlgui_element_t e, size_t pos);
void sdlgui_text_input_set_selection(sdlgui_element_t e, size_t start, size_t end);
```

`max_length` jest potrzebny dla 256-bajtowego bufora nazwy miasta;
`on_key`/event hook dla `dlg_city_name_edit_proc` (WM_CHAR, IDOK/IDCANCEL).
`locked` już istnieje i wystarcza dla read-only.

Cel: `city_prompt_city_name`, `dlg_prompt_city_name_modal`,
`dlg_city_name_edit_proc`.

## 5. P2 — opcjonalne

### P2.1 Dialog units (DLU)

```c
int sdlgui_get_dialog_base_units(sdlgui_t gui, int* base_x, int* base_y);
int sdlgui_dialog_units_to_pixels(sdlgui_t gui, int du_x, int du_y,
                                  int* px, int* py);
```

`base_x` = średnia szerokość znaku fontu systemowego, `base_y` = wysokość
linii. CivLin nadal robi własne 16-bit `MulDiv`, jeśli potrzebuje
dokładnej arytmetyki Orig; biblioteka daje tylko jednostki bazowe.
Cel: `dialog_get_item_rect`, formularze z `DLGTEMPLATE`.

### P2.2 `ListView`: per-row enabled/checked

```c
void sdlgui_list_view_set_row_enabled(sdlgui_element_t e, size_t row, int enabled);
void sdlgui_list_view_set_row_checked(sdlgui_element_t e, size_t row, int checked);
void sdlgui_list_view_set_row_height(sdlgui_element_t e, int px);
```

Przydatne dla list produkcji/dyplomacji, jeśli nie chcemy używać
`MenuList`. Nie jest krytyczne dla obecnych blokad.

### P2.3 `Image` / surface w dialogach (opcjonalnie)

Jeżeli w przyszłości portret dyplomaty lub ikony mają być dziećmi
`FormDialog`, warto rozważyć:

```c
sdlgui_element_t sdlgui_image_create(sdlgui_t gui, sdlgui_element_t parent,
                                     int x, int y, int w, int h);
void sdlgui_image_set_rgba(sdlgui_element_t e, const uint8_t* pixels,
                           int w, int h, int pitch);
void sdlgui_image_set_indexed(sdlgui_element_t e, const uint8_t* pixels,
                              int w, int h, int pitch,
                              const SDL_Color* palette, int palette_count);
void sdlgui_image_set_source_rect(sdlgui_element_t e, sdlgui_rect_t src);
void sdlgui_image_set_scale(sdlgui_element_t e, float sx, float sy);
```

To **nie jest** propozycja przeniesienia mapy, portów, DIB/HBITMAP ani
palet z CivLin. Obecny port ma własne `gfx_port_t`; dla dialogów
wystarczy hook rysowania z P1.3. `Image` ma sens tylko wtedy, gdy chcemy,
żeby sdl_gui samo składało portrety/ikony bez udziału CivLin.

## 6. Mapa blokad CivLin → propozycje

| Batch CivLin | Funkcja | Propozycja | Co zostaje po stronie CivLin |
|---|---|---|---|
| `0001-boot-run` | `cursor_set` | P0.5 | parsowanie NE `RT_CURSOR`, mapowanie id→zasób |
| `1861-title-anim-callback` | `title_anim_callback` | P0.5 | to samo |
| `1872-dialog-ctrl-free` | `dialog_ctrl_paint_button`, `dialog_ctrl_paint_static` | P1.3 | logika stanu kontrolek |
| `1879-dialog-dlgunits-to-pixels` | `dialog_get_item_rect` | P0.1, P0.2, P2.1 | własne `MulDiv`/DLU, jeśli potrzebne |
| `1886-city-prompt-city-name` | `city_prompt_city_name`, `dlg_prompt_city_name_modal`, `dlg_city_name_edit_proc`, `dialog_split_menu_text` | P0.3, P1.1, P1.3, P1.4, P1.5 | tabela nazw, `g_szLineBuf`, logika IDOK/IDCANCEL |
| `1898-dlg-create-dlg-template` | `dlg_show_item`, `dlg_hide_item` | P0.1, P0.2 | nic poza mapowaniem ID |
| `1912-city-tile-base-yield` | `city_pump_production_dialog` | P1.1, P1.2 | produkcja miasta, stan gry |
| `2353-dialog-ctrl-radio-proc` | `dialog_ctrl_*` (6 funkcji) | P1.3 | logika RANDOM*PROC i stan kontrolek |
| `2354-diplo-contact-afford-snap` | `diplo_open_contact_dialog`, `diplo_prompt_contact`, `diplo_run_contact_menu` | P1.1, P1.2, P1.4, P1.3 | stan dyplomacji, tech/treasury, replay |
| `2355-diplo-senate-forces-peace` | `dialog_open_help` | P1.1, P2.3 (host) | dekoder HLP 3.x LZSS, help viewer |
| `2509-diplo-prompt-treaty-menu` | `diplo_prompt_treaty_menu` | P1.1, P1.2, P1.4 | stan 1378/12d8, portrety, frees |
| `9008-dialog-menuz-state-gate` | `dialog_menuz` | P1.4 | `g_wMenuZDisabledMask`, `g_nMenuZCols`, timer |
| `2583-rng-next15` | `status_measure_row`, `sys_get_client_rect` | P0.1, P0.3 | kontrakt far-pointer, status-row draw |
| `0447-hof-show` | `HoF_Show` | P0.3, P1.1 | pole population, tabela tytułów |
| `0448-hof-format-slot` | `hof_format_slot` | P0.3, P1.1 | 3-liniowy layout, population |
| `1175-worldgen-load-earth` | `worldgen_load_earth` | P1.1, P1.2 | settings hand-off, generator |
| `0449-newgame-wizard-run` | `newgame_wizard_run` | P1.1, P1.2 | kontrakt back/cancel, kroki 3/4 |
| `9006-smmap-refresh-shown-gate` | `smmap_refresh_dirty_view` | P0.1 (`is_shown`) | per-window shown state, jeśli SmMap nie jest widgetem |
| `1176-config-save-prefs` | `config_save_prefs` | P0.1 (`get_window_rect`) | interop `civ.prf`, layout okna |
| `2615-win16-asset-cb` | `worldgen_probe_custom_font` | P0.3 (font probe) | decyzja WONTFIX vs atlas |

Funkcje spoza tej tabeli (RSC, save, turn, unit/map/city, DIB/HBITMAP,
palety) nie powinny być „naprawiane” w sdl_gui. To warstwa CivLin
`rsc_loader`/`save`/`game`/`video` albo czyste RE.

## 7. Kolejność wdrożenia i kryteria akceptacji

### Sprint 1 — P0.1–P0.5

- C API: recty, drzewo, control ID, visible/focus, metryki, int menu,
  cursor surface.
- Testy w `tests/test_sdl_gui_c_api.cpp`: geometria po `layoutChildren`,
  `find_by_control_id`, `is_shown` dla ukrytego przodka, focus,
  `measure_text` dla pustego i wieloliniowego tekstu, cursor z prostą
  powierzchnią 2×2.
- Przykład C: `examples/c/13_geometry_ids.c`.
- Dokumentacja: `docs/release/c_api.md`.
- Oczekiwane odblokowanie: `dialog_get_item_rect`, `dlg_show/hide_item`,
  `cursor_set`, `title_anim_callback`, `status_measure_row`,
  `sys_get_client_rect` (częściowo), `HoF_*` (częściowo).

### Sprint 2 — P1.1–P1.3

- `FormDialog` + item registry + `create_from_items`.
- `modal_run` z `keep_background`, `frame_cb`, `event_filter`.
- `draw_hook`/`event_hook` + `custom_control`.
- Testy: form item show/hide/rect, result code, modal run z syntetycznym
  Esc/Enter, owner-draw callback (pixel test), event hook consumption.
- Przykłady C: `examples/c/14_form_dialog.c`,
  `examples/c/15_custom_control.c`.
- Oczekiwane odblokowanie: city-name trio, production dialog,
  diplo contact/treaty (częściowo), worldgen earth, newgame wizard,
  `dialog_ctrl_*`, `dlg_city_name_edit_proc`.

### Sprint 3 — P1.4–P2

- `MenuList` z disabled/cols/title/timer/row callback.
- `TextInput` max/filter/key; `ListView` row state; DLU helper.
- Testy: MenuList pomija disabled, zawija nawigację, zwraca ID,
  honoruje cancel ID; TextInput limit; DLU.
- Przykład C: `examples/c/16_menu_list.c`.
- Oczekiwane odblokowanie: `dialog_menuz`, `dialog_split_menu_text`,
  `diplo_run_contact_menu`, `diplo_prompt_treaty_menu`,
  `diplo_open_contact_dialog` (menu tail).

### Gate dla każdego sprintu

```sh
./nob test
./nob examples
./nob release
```

Dodatkowo:

- `dist/sdl_gui.h` i `dist/libsdl_gui.a` generowane przez `./nob release`
  (nie edytować `dist/` ręcznie).
- C API tylko append-only; istniejące sygnatury bez zmian.
- Nowe widgety: wpis w `docs/release/widgets/README.md`, test Catch2,
  przykład, dokumentacja release.

### Po stronie CivLin

- Bump pinu `vendor/sdl_gui` i wpis w planie (AGENTS.md wymaga
  logowania modyfikacji sdl_gui).
- Dla każdej odblokowanej funkcji: nowy batch audytu, stary batch
  dostaje `superseded_by`; `STATUS.md` regenerowany przez
  `tools/review_status.py --write`.
- Nie patchować `vendor/sdl_gui` z CivLin poza jawnymi modami; zmiany
  idą do repo biblioteki i są pinowane.
- Nie przenosić FNT/mapy/RSC do sdl_gui; CivLin nadal rysuje tekst gry
  przez `text_draw_at` w hooku `DRAW_REPLACE`.

## 8. Non-goals

- Emulacja Win16 `WM_*`, `DLGTEMPLATE`, `DLGPROC` w sdl_gui. Biblioteka
  dostaje generyczne hooki i deskryptory; mapowanie robi CivLin.
- Wspólny model tekstu FNT + TTF. Tekst gry zostaje w CivLin; sdl_gui
  rysuje tylko chrome/system text.
- Renderer mapy, tilemapy, portów, palet, DIB/HBITMAP, RSC.
- `WindowManager` dla wielu okien Civ. CivLin ma jedno okno i porty.
- Automatyczne „naprawianie” audytu: zmiany GUI nie zmieniają werdyktów
  bez nowego batcha i dowodów RE.

## 9. Otwarte decyzje

1. **FormDialog vs rozszerzenie DialogBox**: rekomendacja — nowy
   `FormDialog` dziedziczący `ModalDialog`, `DialogBox` bez zmian
   (kompatybilność i prostsze testy).
2. **MenuList vs rozszerzenie MenuPopup**: rekomendacja — nowy widget;
   `MenuPopup` jest drzewem podręcznym z overlay semantics, a MenuZ jest
   modalną listą z kolumnami i disabled mask.
3. **Hooki: `SDL_Event*` vs znormalizowany event**: rekomendacja —
   `SDL_Event*` + lokalne x/y w MVP; znormalizowany enum później, jeśli
   pojawi się potrzeba C++/C bez SDL w API.
4. **Modal runner w bibliotece vs w CivLin**: rekomendacja — runner w
   bibliotece z `frame_cb`/`event_filter`; jeśli okaże się zbyt
   gry-specific, zostawić `modal_pump` i ręczną pętlę CivLin.
5. **Paint primitives**: rekomendacja — najpierw renderer + rect + clip;
   `sdlgui_paint_*` dopiero, gdy owner-draw będzie używany poza CivLin.
6. **Image/surface**: rekomendacja — poza zakresem pierwszego wdrożenia;
   wystarczy `DRAW_REPLACE` i własny `gfx_port_t` CivLin.

## 10. Oczekiwany efekt

Po P0+P1:

- ~25 z 78 funkcji `BLOCKED` ma komplet narzędzi GUI; część nadal
  wymaga stanu gry/RE, ale blokada „brak widgetu” znika.
- CivLin nie musi utrzymywać trzech ręcznych pętli modalnych ani
  własnego rejestru itemów dialogu.
- `FormDialog`/`MenuList`/`CustomControl` są generyczne i testowalne
  w sdl_gui, więc nie zamrażają biblioteki na Civ1.
- Kolejne porty (Civ2, inne gry Win16) dostają te same prymitywy:
  control ID, recty, hooki rysowania, modal runner i listę wyboru.
