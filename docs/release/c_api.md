# C API (`sdl_gui.h`)

Pełnoprawna warstwa C dla biblioteki SDL GUI. Nagłówek jest czystym C11
(`#include <SDL3/SDL.h>`, bez zależności od C++), ale działa też z projektów
C++ (`extern "C"` wewnątrz). Wszystkie funkcje mają prefix `sdlgui_`, a
uchwyty są nieprzezroczyste:

```c
typedef void* sdlgui_t;          /* kontekst: GUIManager + SDL */
typedef void* sdlgui_element_t;  /* dowolny widget (GUIElement*) */
```

## Kontekst i cykl życia

| Funkcja | Opis |
|---------|------|
| `sdlgui_t sdlgui_create(const char* title, int width, int height, int resizable)` | Tworzy kontekst: inicjalizuje SDL, tworzy okno i renderer, nakłada domyślny motyw (Win9x). `resizable`: `0` = stały rozmiar, `1` = okno z możliwością zmiany rozmiaru. Zwraca `NULL` przy błędzie |
| `sdlgui_t sdlgui_create_gpu(const char* title, int width, int height, int resizable)` | Tworzy kontekst z **rendererem GPU (Vulkan/SPIR-V)** — wymagany, aby `ShaderPanel` faktycznie uruchamiał shadery. Zwraca `NULL` przy błędzie (np. brak sterownika Vulkan) |
| `SDL_GPUDevice* sdlgui_get_gpu_device(sdlgui_t gui)` | Urządzenie GPU kontekstu, lub `NULL` dla kontekstu CPU. Przydatne do budowy/kompilacji shaderów SPIR-V przed `sdlgui_shader_panel_set_shader()` |
| `void sdlgui_destroy(sdlgui_t gui)` | Niszczy kontekst i zwalnia zasoby SDL. **Wszystkie uchwyty elementów stają się nieważne** |

Po `sdlgui_create()` motywem domyślnym jest Win9x — można go zmienić w każdej
chwili (patrz niżej).

## Pętla zdarzeń

| Funkcja | Opis |
|---------|------|
| `SDL_Renderer* sdlgui_get_renderer(sdlgui_t gui)` | Renderer kontekstu (do czyszczenia ekranu, `SDL_RenderPresent` itd.) |
| `SDL_Window* sdlgui_get_window(sdlgui_t gui)` | Okno kontekstu (np. do `SDL_WarpMouseInWindow`, przechwytywania myszy) |
| `bool sdlgui_process_event(sdlgui_t gui, const SDL_Event* e)` | Przekazuje zdarzenie SDL do GUI. **Zwraca `bool handled`** — `true` gdy zdarzenie zostało skonsumowane przez widgety (klik, wpisywanie tekstu, nawigacja Tab). Takiego zdarzenia aplikacja nie powinna już przetwarzać sama |
| `void sdlgui_update(sdlgui_t gui)` | Aktualizuje timery, animacje i tooltipy. Wymagane w każdej klatce |
| `void sdlgui_cleanup(sdlgui_t gui)` | Usuwa elementy oznaczone do usunięcia. Wymagane w każdej klatce |
| `void sdlgui_render(sdlgui_t gui)` | Rysuje cały interfejs |
| `void sdlgui_handle_resize(sdlgui_t gui, int w, int h)` | Informuje GUI o zmianie rozmiaru okna (przelicza anchory) — wywołaj przy `SDL_EVENT_WINDOW_RESIZED` |
| `void sdlgui_get_window_size(sdlgui_t gui, int* w, int* h)` | Aktualny rozmiar okna |

Obowiązkowa kolejność w pętli: `process_event` → `update` → `cleanup` →
`render`. Pominięcie `update()` psuje tooltipy; pominięcie `cleanup()`
powoduje wyciek elementów z `sdlgui_element_mark_for_deletion()`.

## Motywy

| Funkcja | Opis |
|---------|------|
| `void sdlgui_theme_win9x(sdlgui_t gui)` | Klasyczny Windows 95/98 (domyślny) |
| `void sdlgui_theme_dark(sdlgui_t gui)` | Ciemny (dark mode) |
| `void sdlgui_theme_light(sdlgui_t gui)` | Jasny, nowoczesny |
| `void sdlgui_theme_high_contrast(sdlgui_t gui)` | Wysoki kontrast, duże fonty |

Motywy można przełączać w czasie działania programu.

## Tooltip

| Funkcja | Opis |
|---------|------|
| `void sdlgui_show_tooltip(sdlgui_t gui, sdlgui_element_t target, const char* text)` | Pokazuje tooltip przy elemencie |
| `void sdlgui_hide_tooltip(sdlgui_t gui)` | Ukrywa aktywny tooltip |

## Timery

| Funkcja | Opis |
|---------|------|
| `uint32_t sdlgui_add_timer(sdlgui_t gui, sdlgui_element_t target, uint32_t delay_ms, int single_shot, sdlgui_timer_callback_t cb, void* userdata)` | Odpala callback po `delay_ms`. `single_shot = 1` — raz; `0` — cyklicznie co `delay_ms`. `target` może być `NULL`; jeśli nie jest, wskaźnik elementu trafia do callbacka. Zwraca ID timera |
| `void sdlgui_remove_timer(sdlgui_t gui, uint32_t timer_id)` | Anuluje timer o danym ID |

## Animacje

| Funkcja | Opis |
|---------|------|
| `uint32_t sdlgui_add_loop_animation(sdlgui_t gui, uint32_t interval_ms, sdlgui_anim_callback_t cb, void* userdata)` | Powtarzany callback co `interval_ms`. Zwraca ID do anulowania |
| `void sdlgui_remove_loop_animation(sdlgui_t gui, uint32_t anim_id)` | Anuluje pętlę animacji |
| `void sdlgui_animate_int(sdlgui_t gui, int* target_property, int start, int end, uint32_t duration_ms, sdlgui_easing_t easing, sdlgui_anim_callback_t on_complete, void* userdata)` | Animuje zmienną `int` od `start` do `end`; `target_property` modyfikowany w miejscu co klatkę. `on_complete` może być `NULL`. **Uwaga**: wskaźnik musi być ważny przez cały czas trwania animacji |
| `void sdlgui_animate_float(sdlgui_t gui, float* target_property, float start, float end, uint32_t duration_ms, sdlgui_easing_t easing, sdlgui_anim_callback_t on_complete, void* userdata)` | Jak wyżej dla `float` |

```c
typedef enum {
    SDLGUI_EASING_LINEAR      = 0,
    SDLGUI_EASING_IN_QUAD     = 1,
    SDLGUI_EASING_OUT_QUAD    = 2,
    SDLGUI_EASING_IN_OUT_QUAD = 3
} sdlgui_easing_t;
```

## Element base API (dotyczy WSZYSTKICH widgetów)

| Funkcja | Opis |
|---------|------|
| `void sdlgui_element_set_position(sdlgui_element_t e, int x, int y)` | Pozycja (względem rodzica; top-level — względem okna) |
| `void sdlgui_element_set_size(sdlgui_element_t e, int w, int h)` | Rozmiar |
| `void sdlgui_element_set_enabled(sdlgui_element_t e, int enabled)` | Włącz/wyłącz (disabled nie reaguje na zdarzenia) |
| `void sdlgui_element_set_visible(sdlgui_element_t e, int visible)` | Widoczność |
| `void sdlgui_element_set_background_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color)` | Kolor tła dla stanu |
| `void sdlgui_element_set_text_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color)` | Kolor tekstu dla stanu |
| `void sdlgui_element_set_border(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color, int width)` | Obramowanie dla stanu |
| `void sdlgui_element_set_border_radius(sdlgui_element_t e, sdlgui_element_state_t state, int radius)` | Zaokrąglenie rogów |
| `void sdlgui_element_set_tooltip(sdlgui_element_t e, const char* text)` | Tooltip elementu |
| `void sdlgui_element_set_id(sdlgui_element_t e, const char* id)` | Identyfikator elementu |
| `const char* sdlgui_element_get_id(sdlgui_element_t e)` | Pobiera ID (ważne do następnej modyfikacji) |
| `const char* sdlgui_element_get_type(sdlgui_element_t e)` | Nazwa typu: `"Button"`, `"Label"`, `"Panel"` itd. |
| `void sdlgui_element_set_anchor(sdlgui_element_t e, sdlgui_anchor_t anchor)` | Anchor (responsywny layout) |
| `void sdlgui_element_set_rotation(sdlgui_element_t e, double angle_degrees)` | Obrót w stopniach |
| `void sdlgui_element_mark_for_deletion(sdlgui_element_t e)` | Oznacza do usunięcia (wykonane przez `sdlgui_cleanup()`) |
| `void sdlgui_element_set_can_get_keyboard_focus(sdlgui_element_t e, int can_focus)` | Czy element może dostać focus z Tab |
| `int sdlgui_element_add_child(sdlgui_element_t parent, sdlgui_element_t child)` | Dynamiczne przeniesienie top-level elementu jako dziecka `parent`. Zwraca `0` przy sukcesie, `-1` gdy `child` nie jest elementem top-level. Po wywołaniu dziecko jest własnością rodzica |

```c
typedef enum {
    SDLGUI_STATE_NORMAL   = 0,
    SDLGUI_STATE_HOVER    = 1,
    SDLGUI_STATE_PRESSED  = 2,
    SDLGUI_STATE_DISABLED = 3
} sdlgui_element_state_t;

typedef struct {
    float left;   /* <0 = nieustawione, 0.0–1.0 = procent, >1.0 = piksele */
    float top;
    float right;
    float bottom;
} sdlgui_anchor_t;
```

## Fabryki anchorów

| Funkcja | Opis |
|---------|------|
| `sdlgui_anchor_t sdlgui_anchor_none(void)` | Brak anchoru (stała pozycja) |
| `sdlgui_anchor_t sdlgui_anchor_top_left(float margin)` | Lewy górny róg |
| `sdlgui_anchor_t sdlgui_anchor_top_right(float margin)` | Prawy górny róg |
| `sdlgui_anchor_t sdlgui_anchor_bottom_left(float margin)` | Lewy dolny róg |
| `sdlgui_anchor_t sdlgui_anchor_bottom_right(float margin)` | Prawy dolny róg |
| `sdlgui_anchor_t sdlgui_anchor_center(void)` | Wyśrodkowanie |
| `sdlgui_anchor_t sdlgui_anchor_fill(int padding)` | Wypełnia rodzica (padding w px) |
| `sdlgui_anchor_t sdlgui_anchor_horizontal_stretch(int pad_left, int pad_right)` | Rozciągnięcie poziome |
| `sdlgui_anchor_t sdlgui_anchor_vertical_stretch(int pad_top, int pad_bottom)` | Rozciągnięcie pionowe |
| `sdlgui_anchor_t sdlgui_anchor_top_bar(int height, int pad_vert, int pad_horiz)` | Pełna szerokość u góry |
| `sdlgui_anchor_t sdlgui_anchor_bottom_bar(int height, int pad_vert, int pad_horiz)` | Pełna szerokość u dołu |
| `sdlgui_anchor_t sdlgui_anchor_left_sidebar(int width, int pad_top, int pad_bottom)` | Pasek boczny po lewej |
| `sdlgui_anchor_t sdlgui_anchor_right_sidebar(int width, int pad_top, int pad_bottom)` | Pasek boczny po prawej |
| `sdlgui_anchor_t sdlgui_anchor_raw(float left, float top, float right, float bottom)` | Surowy anchor (konwencja: `<0` = nieustawione, `0–1` = procent, `>1` = px) |

Anchory przeliczają się przy zmianie rozmiaru — w pętli obsłuż
`SDL_EVENT_WINDOW_RESIZED` i przekaż nowe wymiary do
`sdlgui_handle_resize(gui, w, h)`.

## Widgety — funkcje tworzące

Wszystkie funkcje `sdlgui_<widget>_create` mają ten sam wzorzec:

```c
sdlgui_element_t sdlgui_<widget>_create(sdlgui_t gui, sdlgui_element_t parent, <args...>);
```

- `parent = NULL` → element top-level, automatycznie dodany do GUIManagera.
- `parent != NULL` → dziecko rodzica, automatycznie dodane przez
  `parent->addChild()`.
- Zwracają uchwyt elementu lub `NULL` przy błędzie.

### Button

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_button_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, const char* label)` | Tworzy przycisk |
| `void sdlgui_button_set_on_click(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback kliknięcia |
| `void sdlgui_button_set_on_mouse_over(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback najechania myszą |
| `void sdlgui_button_set_label(sdlgui_element_t e, const char* label)` | Tekst przycisku |

### Label

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_label_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, const char* text, int font_size)` | Tworzy etykietę. `font_size = -1` → domyślny (14pt z motywu). Brak parametrów `w/h` — Label sam dobiera rozmiar do tekstu |
| `void sdlgui_label_set_text(sdlgui_element_t e, const char* text)` | Nowy tekst |
| `const char* sdlgui_label_get_text(sdlgui_element_t e)` | Tekst (ważny do następnego `set_text` na tym elemencie) |

### Panel

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_panel_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy panel |
| `void sdlgui_panel_set_draggable(sdlgui_element_t e, int draggable)` | Czy panel można przeciągać myszą |

Panel pokrywa też Slider (Slider dziedziczy po Panel), więc `set_draggable` i
całe element base API działają również na uchwytach suwaków.

### Slider

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_slider_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, int min_val, int max_val, int initial_val, sdlgui_orientation_t orientation)` | Tworzy suwak |
| `int sdlgui_slider_get_value(sdlgui_element_t e)` | Bieżąca wartość |
| `void sdlgui_slider_set_value(sdlgui_element_t e, int value)` | Ustawia wartość |
| `void sdlgui_slider_set_range(sdlgui_element_t e, int min_val, int max_val)` | Ustawia zakres |
| `int sdlgui_slider_get_min(sdlgui_element_t e)` | Minimalna wartość |
| `int sdlgui_slider_get_max(sdlgui_element_t e)` | Maksymalna wartość |
| `void sdlgui_slider_set_min(sdlgui_element_t e, int min)` | Minimalna wartość |
| `void sdlgui_slider_set_max(sdlgui_element_t e, int max)` | Maksymalna wartość |
| `void sdlgui_slider_set_wheel_step(sdlgui_element_t e, int step)` | Krok zmiany kółkiem myszy |
| `void sdlgui_slider_set_on_change(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback zmiany wartości |
| `int sdlgui_slider_get_orientation(sdlgui_element_t e)` | `0` = poziomy, `1` = pionowy |

```c
typedef enum {
    SDLGUI_ORIENTATION_HORIZONTAL = 0,
    SDLGUI_ORIENTATION_VERTICAL   = 1
} sdlgui_orientation_t;
```

### Checkbox

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_checkbox_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy checkbox. **Nie ma tekstu** — dodaj osobny Label |
| `int sdlgui_checkbox_is_checked(sdlgui_element_t e)` | Stan zaznaczenia |
| `void sdlgui_checkbox_set_checked(sdlgui_element_t e, int checked)` | Ustawia stan |
| `void sdlgui_checkbox_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata)` | Callback zmiany (element = checkbox, `value` = nowy stan) |

### TextInput

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_text_input_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy jednoliniowe pole tekstowe |
| `void sdlgui_text_input_set_text(sdlgui_element_t e, const char* text)` | Ustawia tekst |
| `const char* sdlgui_text_input_get_text(sdlgui_element_t e)` | Tekst (ważny do następnego `set_text`) |
| `void sdlgui_text_input_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback przy każdej zmianie tekstu |
| `void sdlgui_text_input_set_on_enter_pressed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback po Enter |
| `void sdlgui_text_input_set_locked(sdlgui_element_t e, int locked)` | Blokada edycji |
| `int sdlgui_text_input_is_locked(sdlgui_element_t e)` | Czy zablokowany |

### ListView

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_list_view_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy listę |
| `void sdlgui_list_view_add_item(sdlgui_element_t e, const char* item)` | Dodaje pozycję |
| `void sdlgui_list_view_remove_item(sdlgui_element_t e, size_t index)` | Usuwa pozycję |
| `void sdlgui_list_view_clear(sdlgui_element_t e)` | Czyści listę |
| `size_t sdlgui_list_view_get_item_count(sdlgui_element_t e)` | Liczba pozycji |
| `const char* sdlgui_list_view_get_item_text(sdlgui_element_t e, size_t index)` | Tekst pozycji (ważny do modyfikacji/usunięcia pozycji) |
| `void sdlgui_list_view_set_on_row_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata)` | Callback kliknięcia wiersza |
| `void sdlgui_list_view_set_on_row_double_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata)` | Callback podwójnego kliknięcia wiersza |

### ProgressBar

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_progress_bar_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy pasek postępu |
| `float sdlgui_progress_bar_get_value(sdlgui_element_t e)` | Wartość |
| `void sdlgui_progress_bar_set_value(sdlgui_element_t e, float value)` | Ustawia wartość |
| `float sdlgui_progress_bar_get_min(sdlgui_element_t e)` | Minimum |
| `void sdlgui_progress_bar_set_min(sdlgui_element_t e, float min)` | Ustawia minimum |
| `float sdlgui_progress_bar_get_max(sdlgui_element_t e)` | Maksimum |
| `void sdlgui_progress_bar_set_max(sdlgui_element_t e, float max)` | Ustawia maksimum |
| `void sdlgui_progress_bar_set_range(sdlgui_element_t e, float min, float max)` | Ustawia zakres |
| `void sdlgui_progress_bar_set_show_text(sdlgui_element_t e, int show)` | Pokazuje/ukrywa tekst procentowy |
| `void sdlgui_progress_bar_set_text_format(sdlgui_element_t e, const char* format)` | Format tekstu (np. `"%.0f%%"`) |
| `void sdlgui_progress_bar_set_orientation(sdlgui_element_t e, sdlgui_orientation_t orientation)` | Orientacja |

### RadioButton

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_radio_button_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy pojedynczy radio button. **Nie ma tekstu** — użyj `RadioGroup::add_option` lub osobnego Labela |
| `int sdlgui_radio_button_is_selected(sdlgui_element_t e)` | Czy zaznaczony |
| `void sdlgui_radio_button_set_selected(sdlgui_element_t e, int selected)` | Zaznacza/odznacza |
| `void sdlgui_radio_button_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata)` | Callback zmiany stanu |

### RadioGroup

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_radio_group_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy grupę (Panel zarządzający wzajemnie wykluczającymi się RadioButtonami) |
| `sdlgui_element_t sdlgui_radio_group_add_option(sdlgui_element_t e, const char* text, int selected)` | Dodaje opcję z tekstem; zwraca uchwyt RadioButtona (własność grupy) |
| `void sdlgui_radio_group_set_on_selection_change(sdlgui_element_t e, sdlgui_index_text_callback_t cb, void* userdata)` | Callback zmiany zaznaczenia: `(element, index, text, userdata)` — tekst ważny tylko w trakcie callbacka |

### TextArea

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_text_area_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, const char* font_path, int font_size)` | Wieloliniowy obszar tekstu. `font_size <= 0` → domyślny (14pt); `font_path` `NULL`/`""` → domyślny font |
| `void sdlgui_text_area_set_text(sdlgui_element_t e, const char* text)` | Ustawia tekst |
| `const char* sdlgui_text_area_get_text(sdlgui_element_t e)` | Tekst (ważny do następnego `set_text`) |
| `void sdlgui_text_area_set_word_wrap(sdlgui_element_t e, int wrap)` | Zawijanie wierszy |
| `int sdlgui_text_area_get_word_wrap(sdlgui_element_t e)` | Czy zawija wiersze |
| `void sdlgui_text_area_set_locked(sdlgui_element_t e, int locked)` | Blokada edycji |
| `int sdlgui_text_area_is_locked(sdlgui_element_t e)` | Czy zablokowany |
| `void sdlgui_text_area_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback zmiany tekstu |

### ComboBox

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_combo_box_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy listę rozwijaną |
| `void sdlgui_combo_box_add_item(sdlgui_element_t e, const char* item)` | Dodaje pozycję |
| `void sdlgui_combo_box_clear(sdlgui_element_t e)` | Czyści pozycje |
| `size_t sdlgui_combo_box_get_item_count(sdlgui_element_t e)` | Liczba pozycji |
| `const char* sdlgui_combo_box_get_item_text(sdlgui_element_t e, size_t index)` | Tekst pozycji |
| `int sdlgui_combo_box_get_selected_index(sdlgui_element_t e)` | Zaznaczony indeks |
| `void sdlgui_combo_box_set_selected_index(sdlgui_element_t e, int index)` | Ustawia zaznaczenie |
| `void sdlgui_combo_box_set_on_select(sdlgui_element_t e, sdlgui_index_text_callback_t cb, void* userdata)` | Callback wyboru: `(element, index, text, userdata)` — tekst ważny tylko w trakcie callbacka |

### StringGrid

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_string_grid_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, size_t rows, size_t cols)` | Tworzy siatkę tekstową |
| `void sdlgui_string_grid_set_dimensions(sdlgui_element_t e, size_t rows, size_t cols)` | Zmienia wymiary |
| `void sdlgui_string_grid_set_cell(sdlgui_element_t e, size_t row, size_t col, const char* text)` | Ustawia komórkę |
| `const char* sdlgui_string_grid_get_cell(sdlgui_element_t e, size_t row, size_t col)` | Treść komórki (ważna do modyfikacji/wyczyszczenia komórki) |
| `size_t sdlgui_string_grid_get_row_count(sdlgui_element_t e)` | Liczba wierszy |
| `size_t sdlgui_string_grid_get_col_count(sdlgui_element_t e)` | Liczba kolumn |
| `void sdlgui_string_grid_clear(sdlgui_element_t e)` | Czyści siatkę |
| `void sdlgui_string_grid_set_selected_cell(sdlgui_element_t e, size_t row, size_t col)` | Zaznacza komórkę |
| `void sdlgui_string_grid_set_editable(sdlgui_element_t e, int editable)` | Edycja komórek |
| `int sdlgui_string_grid_is_editable(sdlgui_element_t e)` | Czy edytowalna |
| `void sdlgui_string_grid_set_on_cell_click(sdlgui_element_t e, sdlgui_cell_callback_t cb, void* userdata)` | Callback kliknięcia komórki |
| `void sdlgui_string_grid_set_on_cell_double_click(sdlgui_element_t e, sdlgui_cell_callback_t cb, void* userdata)` | Callback podwójnego kliknięcia komórki |

### ScrollArea

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_scroll_area_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy obszar przewijania |
| `void sdlgui_scroll_area_set_content_size(sdlgui_element_t e, int w, int h)` | Rozmiar zawartości |
| `void sdlgui_scroll_area_set_scroll_enabled(sdlgui_element_t e, int vertical, int horizontal)` | Włącza/wyłącza przewijanie w osiach |
| `void sdlgui_scroll_area_get_scroll_offset(sdlgui_element_t e, int* x, int* y)` | Bieżący offset |
| `void sdlgui_scroll_area_set_content(sdlgui_element_t e, sdlgui_element_t content)` | Przekazuje własność zawartości do ScrollArea. `content` musi być top-level (`parent = NULL`); po wywołaniu nie używaj uchwytu poza modyfikacją stylu |

### AnimatedImage

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_animated_image_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy animowany obrazek |
| `void sdlgui_animated_image_set_sprite_sheet(sdlgui_element_t e, const char* path, int total_frames, int rows, int frame_w, int frame_h)` | Ustawia sprite sheet (ścieżka, liczba klatek, wiersze, rozmiar klatki) |
| `void sdlgui_animated_image_set_fps(sdlgui_element_t e, float fps)` | Prędkość animacji |
| `void sdlgui_animated_image_set_loop(sdlgui_element_t e, int loop)` | Zapętlenie |
| `void sdlgui_animated_image_play(sdlgui_element_t e)` | Odtwarzanie |
| `void sdlgui_animated_image_pause(sdlgui_element_t e)` | Pauza |
| `void sdlgui_animated_image_stop(sdlgui_element_t e)` | Stop (powrót do pierwszej klatki) |
| `int sdlgui_animated_image_get_current_frame(sdlgui_element_t e)` | Bieżąca klatka |
| `int sdlgui_animated_image_get_total_frames(sdlgui_element_t e)` | Liczba klatek |
| `int sdlgui_animated_image_is_playing(sdlgui_element_t e)` | Czy odtwarzane |
| `void sdlgui_animated_image_set_frame(sdlgui_element_t e, int frame_index)` | Ręczne ustawienie klatki |

### Canvas

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_canvas_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy płótno do rysowania |
| `void sdlgui_canvas_clear(sdlgui_element_t e)` | Czyści płótno |
| `void sdlgui_canvas_set_pen_color(sdlgui_element_t e, uint8_t r, uint8_t g, uint8_t b, uint8_t a)` | Kolor rysowania |

### ArcContainer

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_arc_container_create(sdlgui_t gui, sdlgui_element_t parent, int centerX, int centerY, int radius, float start_angle_deg, float end_angle_deg)` | Kontener układający dzieci na łuku. `centerX/centerY` — środek łuku (względem pozycji kontenera), `radius` — odległość od środka, kąty w stopniach (domyślnie 0–360) |
| `void sdlgui_arc_container_add_child_at_angle(sdlgui_element_t e, sdlgui_element_t child, float angle_deg, int rotate_child, int offset)` | Ustawia dziecko na łuku pod kątem. `child` musi być top-level; po wywołaniu jest własnością kontenera. `rotate_child` — czy obrócić dziecko do kąta; `offset` — przesunięcie w px od promienia |

### TabControl

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_tab_control_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, int tab_button_height)` | Kontener z zakładkami. `tab_button_height` — wysokość paska zakładek (domyślnie 30) |
| `sdlgui_element_t sdlgui_tab_control_add_tab(sdlgui_element_t e, const char* title)` | Dodaje zakładkę; zwraca uchwyt panelu treści zakładki — widgety twórz z tym panelem jako `parent` |
| `void sdlgui_tab_control_set_active_tab(sdlgui_element_t e, int index)` | Aktywuje zakładkę po indeksie |

### ContextMenu

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_context_menu_create(sdlgui_t gui)` | Tworzy menu kontekstowe (tworzy się raz, pokazuje/ukrywa wg potrzeby) |
| `void sdlgui_context_menu_add_item(sdlgui_element_t e, const char* text, sdlgui_context_menu_callback_t cb, void* userdata)` | Dodaje pozycję menu |
| `void sdlgui_context_menu_add_separator(sdlgui_element_t e)` | Separator |
| `void sdlgui_context_menu_clear_items(sdlgui_element_t e)` | Czyści pozycje |
| `void sdlgui_context_menu_show_at(sdlgui_element_t e, int x, int y)` | Pokazuje w pozycji |
| `void sdlgui_context_menu_hide(sdlgui_element_t e)` | Ukrywa |
| `int sdlgui_context_menu_is_visible(sdlgui_element_t e)` | Czy widoczne |

### RangeSlider

Podwójny suwak zakresu — wybiera przedział `[lower, upper]` między `min` i
`max`. Wartości są automatycznie clampowane (`lower ≤ upper`; podanie
odwróconych wartości przy tworzeniu zamienia je miejscami).

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_range_slider_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h, int min_val, int max_val, int lower_val, int upper_val, sdlgui_orientation_t orientation)` | Tworzy suwak zakresu |
| `int sdlgui_range_slider_get_lower_value(sdlgui_element_t e)` | Dolna wartość przedziału |
| `void sdlgui_range_slider_set_lower_value(sdlgui_element_t e, int value)` | Ustawia dolną wartość (clamp do `≤ upper`) |
| `int sdlgui_range_slider_get_upper_value(sdlgui_element_t e)` | Górna wartość przedziału |
| `void sdlgui_range_slider_set_upper_value(sdlgui_element_t e, int value)` | Ustawia górną wartość (clamp do `≥ lower`) |
| `void sdlgui_range_slider_set_range(sdlgui_element_t e, int min_val, int max_val)` | Zmienia zakres min/max |
| `int sdlgui_range_slider_get_min(sdlgui_element_t e)` / `sdlgui_range_slider_set_min(sdlgui_element_t e, int min)` | Odczyt/zmiana minimum |
| `int sdlgui_range_slider_get_max(sdlgui_element_t e)` / `sdlgui_range_slider_set_max(sdlgui_element_t e, int max)` | Odczyt/zmiana maksimum |
| `void sdlgui_range_slider_set_wheel_step(sdlgui_element_t e, int step)` / `int sdlgui_range_slider_get_wheel_step(sdlgui_element_t e)` | Krok kółka myszy (min. 1) |
| `void sdlgui_range_slider_set_on_change(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata)` | Callback przy zmianie którejkolwiek wartości |

### Cursor

Niestandardowy kursor rysowany jako overlay (ukrywa systemowy kursor).
Każdy stan ma własną teksturę (statyczną lub animowaną sprite-sheet).

```c
typedef enum {
    SDLGUI_CURSOR_NORMAL, SDLGUI_CURSOR_HOVER, SDLGUI_CURSOR_PRESSED,
    SDLGUI_CURSOR_DISABLED, SDLGUI_CURSOR_BUSY, SDLGUI_CURSOR_TEXT,
    SDLGUI_CURSOR_CUSTOM1, SDLGUI_CURSOR_CUSTOM2, SDLGUI_CURSOR_CUSTOM3
} sdlgui_cursor_state_t;
```

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_cursor_create(sdlgui_t gui)` | Tworzy kursor i instaluje go w GUI. Utworzenie kolejnego kursora **zastępuje i niszczy poprzedni** — stare uchwyty stają się nieważne |
| `void sdlgui_cursor_set_texture(sdlgui_element_t e, sdlgui_cursor_state_t state, const char* path, int hotspot_x, int hotspot_y)` | Ustawia statyczną teksturę stanu (obsługuje embedded assets; hotspot = punkt kliknięcia, domyślnie 0,0) |
| `void sdlgui_cursor_set_animated_texture(sdlgui_element_t e, sdlgui_cursor_state_t state, const char* path, int total_frames, int rows, float fps, int hotspot_x, int hotspot_y)` | Animowany sprite-sheet: `total_frames` klatek w `rows` wierszach (kolumny liczone automatycznie). `fps ≤ 0` = 12 fps. `total_frames ≤ 0` = tekstura statyczna |
| `void sdlgui_cursor_set_state(sdlgui_element_t e, sdlgui_cursor_state_t state)` / `int sdlgui_cursor_get_state(sdlgui_element_t e)` | Zmiana/odczyt bieżącego stanu |
| `void sdlgui_cursor_set_offset(sdlgui_element_t e, int offset_x, int offset_y)` / `void sdlgui_cursor_get_offset(sdlgui_element_t e, int* offset_x, int* offset_y)` | Przesunięcie rysowania od hotspotu |
| `void sdlgui_cursor_set_scale(sdlgui_element_t e, float scale)` / `float sdlgui_cursor_get_scale(sdlgui_element_t e)` | Skala (min. 0.1) |
| `void sdlgui_cursor_set_visible(sdlgui_element_t e, int visible)` / `int sdlgui_cursor_is_visible(sdlgui_element_t e)` | Widoczność (przy ukryciu przywracany jest systemowy kursor) |
| `void sdlgui_cursor_set_on_state_changed(sdlgui_element_t e, sdlgui_cursor_state_callback_t cb, void* userdata)` | Callback przy zmianie stanu |

### ShaderPanel (GPU)

Panel renderujący swoją zawartość przez shader fragmentowy (SPIR-V,
entry point `main`). Działa tylko na kontekście GPU (`sdlgui_create_gpu`);
na kontekście CPU renderuje się jak zwykły panel, a shadery są ignorowane.

| Funkcja | Opis |
|---------|------|
| `sdlgui_element_t sdlgui_shader_panel_create(sdlgui_t gui, sdlgui_element_t parent, int x, int y, int w, int h)` | Tworzy panel shaderowy |
| `void sdlgui_shader_panel_set_shader(sdlgui_element_t e, const uint8_t* spirv_data, size_t spirv_size)` | Ustawia bytecode SPIR-V shadera fragmentowego (zastępuje poprzedni). `NULL`/puste dane są ignorowane |
| `void sdlgui_shader_panel_set_shader_enabled(sdlgui_element_t e, int enabled)` / `int sdlgui_shader_panel_is_shader_enabled(sdlgui_element_t e)` | Włącza/wyłącza shader (bez usuwania) |
| `void sdlgui_shader_panel_set_uniform_time(sdlgui_element_t e, float time)` | Uniform czasu (kanał x koloru wierzchołków) |
| `void sdlgui_shader_panel_set_uniform_mouse(sdlgui_element_t e, float x, float y)` | Uniform pozycji myszy (kanały y/z) |

## Callbacki

| Typ | Sygnatura | Używany przez |
|-----|-----------|---------------|
| `sdlgui_callback_t` | `void (*)(sdlgui_element_t element, void* userdata)` | Kliknięcia Button, zmiany Slider, zdarzenia TextInput |
| `sdlgui_bool_callback_t` | `void (*)(sdlgui_element_t element, int value, void* userdata)` | Checkbox (zmiana stanu), RadioButton |
| `sdlgui_size_callback_t` | `void (*)(sdlgui_element_t element, size_t row, void* userdata)` | ListView (klik/podwójne kliknięcie wiersza) |
| `sdlgui_index_text_callback_t` | `void (*)(sdlgui_element_t element, int index, const char* text, void* userdata)` | ComboBox, RadioGroup — **tekst ważny tylko w trakcie callbacka** |
| `sdlgui_cell_callback_t` | `void (*)(sdlgui_element_t element, size_t row, size_t col, void* userdata)` | StringGrid (komórka) |
| `sdlgui_timer_callback_t` | `void (*)(sdlgui_element_t element, void* userdata)` | Timery (`sdlgui_add_timer`) |
| `sdlgui_anim_callback_t` | `void (*)(void* userdata)` | Pętle i zakończenia animacji |
| `sdlgui_context_menu_callback_t` | `void (*)(void* userdata)` | Pozycje ContextMenu (bez elementu i indeksu) |
| `sdlgui_cursor_state_callback_t` | `void (*)(sdlgui_element_t element, sdlgui_cursor_state_t state, void* userdata)` | Cursor — zmiana stanu |

Wzorzec ustawiania: `sdlgui_<widget>_set_on_<event>(element, cb, userdata)` —
`userdata` jest dowolnym wskaźnikiem przekazywanym z powrotem do callbacka
(może być `NULL`).

## Ownership i czas życia uchwytów

- **`parent` w funkcjach create**: `NULL` = top-level (własność GUIManagera);
  nie-`NULL` = dziecko (własność rodzica, współrzędne względem rodzica).
- **Transfer własności**: `sdlgui_element_add_child`,
  `sdlgui_scroll_area_set_content` i
  `sdlgui_arc_container_add_child_at_angle` przenoszą element do nowego
  rodzica — po wywołaniu elementem zarządza rodzic.
- **Cursor**: `sdlgui_cursor_create` instaluje kursor w GUI (własność
  GUIManagera, nie jest top-level w `m_elements`). Kolejne
  `sdlgui_cursor_create` zastępuje i niszczy poprzedni.
- Uchwyt elementu jest surowym wskaźnikiem. Staje się **dangling** po:
  `sdlgui_element_mark_for_deletion()` + `sdlgui_cleanup()`, po
  `sdlgui_destroy()` oraz po zniszczeniu rodzica.
- Po `sdlgui_destroy()` wszystkie uchwyty elementów są nieważne.

## Zwracane stringi

Funkcje zwracające `const char*` dają wskaźnik do wewnętrznego `std::string`:

| Funkcja | Ważność wskaźnika |
|---------|-------------------|
| `sdlgui_label_get_text` | do następnego `sdlgui_label_set_text` na tym elemencie |
| `sdlgui_text_input_get_text` | do następnego `sdlgui_text_input_set_text` |
| `sdlgui_text_area_get_text` | do następnego `sdlgui_text_area_set_text` |
| `sdlgui_list_view_get_item_text` | do modyfikacji/usunięcia pozycji |
| `sdlgui_string_grid_get_cell` | do modyfikacji/wyczyszczenia komórki |
| `sdlgui_element_get_id` | do następnej modyfikacji ID |
| `sdlgui_element_get_type` | stały (nazwa typu) |
| `text` w `sdlgui_index_text_callback_t` | tylko w trakcie callbacka |

Nie zapisuj tych wskaźników na później — skopiuj string, jeśli ma przetrwać
kolejną operację.

## Kompilacja i linkowanie

Kompilacja pliku C (nagłówek jest czystym C11):

```
gcc -std=c11 -pedantic-errors -c main.c -o main.o
```

Biblioteka `libsdl_gui.so` jest skompilowana w C++, więc **finalne
linkowanie musi przejść przez linker C++** (lub `gcc` z `-lstdc++`):

```
g++ main.o -L. -lsdl_gui -lSDL3 -lSDL3_image -lSDL3_ttf -o app
```

Flag kompilacji dla SDL dostarcza `pkg-config`:

```
gcc -std=c11 -pedantic-errors $(pkg-config --cflags sdl3 sdl3-image sdl3-ttf) -c main.c -o main.o
g++ main.o -L. -lsdl_gui $(pkg-config --libs sdl3 sdl3-image sdl3-ttf) -o app
```

Wersja statyczna: `libsdl_gui.a` (te same biblioteki SDL w kolejności
linkowania).

## Przykład w czystym C

```c
#include "sdl_gui.h"
#include <stdbool.h>
#include <stdio.h>

static void on_click(sdlgui_element_t element, void* userdata) {
    sdlgui_label_set_text((sdlgui_element_t)userdata, "Kliknięto!");
}

static void on_slider(sdlgui_element_t element, void* userdata) {
    char buf[32];
    snprintf(buf, sizeof buf, "Wartość: %d", sdlgui_slider_get_value(element));
    sdlgui_label_set_text((sdlgui_element_t)userdata, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C API Demo", 640, 480, 1);
    if (!gui) return 1;
    sdlgui_theme_dark(gui);

    sdlgui_element_t status = sdlgui_label_create(gui, NULL, 200, 20, "Witaj", -1);

    sdlgui_element_t btn = sdlgui_button_create(gui, NULL, 20, 20, 140, 36, "Kliknij");
    sdlgui_button_set_on_click(btn, on_click, status);

    sdlgui_element_t slider = sdlgui_slider_create(
        gui, NULL, 20, 80, 160, 28, 0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);
    sdlgui_slider_set_on_change(slider, on_slider, status);

    sdlgui_element_t check = sdlgui_checkbox_create(gui, NULL, 20, 130, 24, 24);
    sdlgui_checkbox_set_checked(check, 1);

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            } else if (sdlgui_process_event(gui, &e)) {
                /* zdarzenie obsłużone przez GUI — ignoruj */
            } else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                sdlgui_handle_resize(gui, e.window.data1, e.window.data2);
            }
        }
        sdlgui_update(gui);
        sdlgui_cleanup(gui);
        SDL_SetRenderDrawColor(sdlgui_get_renderer(gui), 40, 42, 54, 255);
        SDL_RenderClear(sdlgui_get_renderer(gui));
        sdlgui_render(gui);
        SDL_RenderPresent(sdlgui_get_renderer(gui));
    }

    sdlgui_destroy(gui);
    return 0;
}
```

Kolejność w pętli jest taka sama jak w C++: `process_event` → `update` →
`cleanup` → `render`. Pominięcie `update()` psuje tooltipy, pominięcie
`cleanup()` powoduje wyciek elementów.
