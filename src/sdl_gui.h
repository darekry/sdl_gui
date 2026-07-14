/*
 * sdl_gui.h — Public C API for SDL GUI library
 *
 * Pure C11 header. Include this in C or C++ projects.
 * Link with -lsdl_gui -lSDL3 -lSDL3_image -lSDL3_ttf
 */

#ifndef SDL_GUI_H
#define SDL_GUI_H

#include <stdint.h>
#include <stddef.h>      /* size_t */
#include <SDL3/SDL.h>    /* SDL_Color, SDL_Event, SDL_Renderer */

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════
   Opaque handles
   ═══════════════════════════════════════════════════════════════ */

typedef void* sdlgui_t;            /* GUIManager + SDL context */
typedef void* sdlgui_element_t;    /* any widget (GUIElement*)   */

/* ═══════════════════════════════════════════════════════════════════
   Enums
   ═══════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_STATE_NORMAL   = 0,
    SDLGUI_STATE_HOVER    = 1,
    SDLGUI_STATE_PRESSED  = 2,
    SDLGUI_STATE_DISABLED = 3
} sdlgui_element_state_t;

typedef enum {
    SDLGUI_ORIENTATION_HORIZONTAL = 0,
    SDLGUI_ORIENTATION_VERTICAL   = 1
} sdlgui_orientation_t;

/* ═══════════════════════════════════════════════════════════════════
   Anchor struct (same memory layout as C++ Anchor)
   ═══════════════════════════════════════════════════════════════ */

typedef struct {
    float left;    /* <0 = unset, 0.0-1.0 = percentage, >1.0 = pixels */
    float top;
    float right;
    float bottom;
} sdlgui_anchor_t;

/* ═══════════════════════════════════════════════════════════════════
   Callback types
   ═══════════════════════════════════════════════════════════════ */

/*
 * Generic callback: element is the widget that triggered the event.
 * Used for Button clicks, Slider changes, TextInput events.
 */
typedef void (*sdlgui_callback_t)(sdlgui_element_t element, void* userdata);

/*
 * Bool callback: value is the new boolean state.
 * Used for Checkbox on_change.
 */
typedef void (*sdlgui_bool_callback_t)(sdlgui_element_t element, int value, void* userdata);

/*
 * Size callback: row is the row index that was clicked/activated.
 * Used for ListView row events.
 */
typedef void (*sdlgui_size_callback_t)(sdlgui_element_t element, size_t row, void* userdata);

/*
 * Index+text callback: index + text of the selected item.
 * Used for ComboBox and RadioGroup selection changes.
 * The text pointer is valid only during callback execution.
 */
typedef void (*sdlgui_index_text_callback_t)(sdlgui_element_t element, int index, const char* text, void* userdata);

/*
 * Cell callback: row and column of a cell interaction.
 * Used for StringGrid cell click/double-click events.
 */
typedef void (*sdlgui_cell_callback_t)(sdlgui_element_t element, size_t row, size_t col, void* userdata);

/*
 * Timer callback: element is the target element passed to add_timer.
 */
typedef void (*sdlgui_timer_callback_t)(sdlgui_element_t element, void* userdata);

/*
 * Animation callback: userdata only (no element — suitable for both
 * loop animations and tween completion callbacks).
 */
typedef void (*sdlgui_anim_callback_t)(void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Easing types for property animations
   ═══════════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_EASING_LINEAR      = 0,
    SDLGUI_EASING_IN_QUAD     = 1,
    SDLGUI_EASING_OUT_QUAD    = 2,
    SDLGUI_EASING_IN_OUT_QUAD = 3
} sdlgui_easing_t;

/* ═══════════════════════════════════════════════════════════════════
   Context lifecycle (convenience wrapper)
   ═══════════════════════════════════════════════════════════════ */

/*
 * Create a GUI context. Internally initializes SDL, creates a window
 * and renderer, and applies the default theme (Win9x).
 *
 * title     — window title (UTF-8)
 * width, height — initial window dimensions
 * resizable — 0 = fixed, 1 = user-resizable
 *
 * Returns NULL on failure.
 */
sdlgui_t sdlgui_create(const char* title, int width, int height, int resizable);

/*
 * Destroy the GUI context and release all SDL resources.
 * All element handles become invalid.
 */
void     sdlgui_destroy(sdlgui_t gui);

/* ═══════════════════════════════════════════════════════════════════
   Core loop API
   ═══════════════════════════════════════════════════════════════ */

SDL_Renderer*  sdlgui_get_renderer(sdlgui_t gui);
void           sdlgui_process_event(sdlgui_t gui, const SDL_Event* e);
void           sdlgui_update(sdlgui_t gui);
void           sdlgui_cleanup(sdlgui_t gui);
void           sdlgui_render(sdlgui_t gui);
void           sdlgui_handle_resize(sdlgui_t gui, int w, int h);
void           sdlgui_get_window_size(sdlgui_t gui, int* w, int* h);

/* ═══════════════════════════════════════════════════════════════════
   Theme presets
   ═══════════════════════════════════════════════════════════════ */

/* Default after sdlgui_create() is Win9x. Swappable at runtime. */

void sdlgui_theme_win9x(sdlgui_t gui);
void sdlgui_theme_dark(sdlgui_t gui);
void sdlgui_theme_light(sdlgui_t gui);
void sdlgui_theme_high_contrast(sdlgui_t gui);

/* ═══════════════════════════════════════════════════════════════════
   Tooltip
   ═══════════════════════════════════════════════════════════════ */

void sdlgui_show_tooltip(sdlgui_t gui, sdlgui_element_t target, const char* text);
void sdlgui_hide_tooltip(sdlgui_t gui);

/* ═══════════════════════════════════════════════════════════════════
   TimerManager
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Schedule a timer. delay_ms is the delay before the callback fires.
 * single_shot=1: fires once. single_shot=0: repeats every delay_ms.
 * target can be NULL; if non-NULL, the element pointer is passed to callback.
 * Returns a timer ID that can be cancelled with remove_timer.
 */
uint32_t sdlgui_add_timer(sdlgui_t gui, sdlgui_element_t target,
                          uint32_t delay_ms, int single_shot,
                          sdlgui_timer_callback_t cb, void* userdata);
void     sdlgui_remove_timer(sdlgui_t gui, uint32_t timer_id);

/* ═══════════════════════════════════════════════════════════════════
   AnimationManager — Looping animations
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Schedule a repeating animation callback. Fires every interval_ms.
 * Returns an animation ID that can be cancelled with remove_loop_animation.
 */
uint32_t sdlgui_add_loop_animation(sdlgui_t gui, uint32_t interval_ms,
                                   sdlgui_anim_callback_t cb, void* userdata);
void     sdlgui_remove_loop_animation(sdlgui_t gui, uint32_t anim_id);

/* ═══════════════════════════════════════════════════════════════════
   AnimationManager — Property tweens
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Animate an integer property from start to end over duration_ms.
 * The target_property is updated in-place each frame.
 * on_complete is called when the animation finishes (can be NULL).
 * WARNING: target_property must remain valid for the animation duration.
 */
void sdlgui_animate_int(sdlgui_t gui, int* target_property,
                        int start, int end,
                        uint32_t duration_ms, sdlgui_easing_t easing,
                        sdlgui_anim_callback_t on_complete, void* userdata);

/*
 * Animate a float property from start to end over duration_ms.
 */
void sdlgui_animate_float(sdlgui_t gui, float* target_property,
                          float start, float end,
                          uint32_t duration_ms, sdlgui_easing_t easing,
                          sdlgui_anim_callback_t on_complete, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Element base API (applies to ALL widgets)
   ═══════════════════════════════════════════════════════════════ */

void        sdlgui_element_set_position(sdlgui_element_t e, int x, int y);
void        sdlgui_element_set_size(sdlgui_element_t e, int w, int h);
void        sdlgui_element_set_enabled(sdlgui_element_t e, int enabled);
void        sdlgui_element_set_visible(sdlgui_element_t e, int visible);
void        sdlgui_element_set_background_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color);
void        sdlgui_element_set_text_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color);
void        sdlgui_element_set_border(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color, int width);
void        sdlgui_element_set_border_radius(sdlgui_element_t e, sdlgui_element_state_t state, int radius);
void        sdlgui_element_set_tooltip(sdlgui_element_t e, const char* text);
void        sdlgui_element_set_id(sdlgui_element_t e, const char* id);
const char* sdlgui_element_get_id(sdlgui_element_t e);
const char* sdlgui_element_get_type(sdlgui_element_t e);   /* "Button", "Label", "Panel", etc. */
void        sdlgui_element_set_anchor(sdlgui_element_t e, sdlgui_anchor_t anchor);
void        sdlgui_element_set_rotation(sdlgui_element_t e, double angle_degrees);
void        sdlgui_element_mark_for_deletion(sdlgui_element_t e);
void        sdlgui_element_set_can_get_keyboard_focus(sdlgui_element_t e, int can_focus);

/*
 * Dynamic reparenting: moves a top-level element to become a child of parent.
 * The child element must be a top-level element (created with parent=NULL).
 * After calling this, the child is owned by the parent.
 * Returns 0 on success, -1 if child is not a top-level element.
 */
int         sdlgui_element_add_child(sdlgui_element_t parent, sdlgui_element_t child);

/* ═══════════════════════════════════════════════════════════════════
   Anchor factories
   ═══════════════════════════════════════════════════════════════ */

sdlgui_anchor_t sdlgui_anchor_none(void);                          /* no anchor (fixed position) */
sdlgui_anchor_t sdlgui_anchor_top_left(float margin);
sdlgui_anchor_t sdlgui_anchor_top_right(float margin);
sdlgui_anchor_t sdlgui_anchor_bottom_left(float margin);
sdlgui_anchor_t sdlgui_anchor_bottom_right(float margin);
sdlgui_anchor_t sdlgui_anchor_center(void);                        /* element centered, no corner offset */
sdlgui_anchor_t sdlgui_anchor_fill(int padding);                   /* fill parent */
sdlgui_anchor_t sdlgui_anchor_horizontal_stretch(int pad_left, int pad_right);
sdlgui_anchor_t sdlgui_anchor_vertical_stretch(int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_top_bar(int height, int pad_vert, int pad_horiz);
sdlgui_anchor_t sdlgui_anchor_bottom_bar(int height, int pad_vert, int pad_horiz);
sdlgui_anchor_t sdlgui_anchor_left_sidebar(int width, int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_right_sidebar(int width, int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_raw(float left, float top, float right, float bottom);

/* ═══════════════════════════════════════════════════════════════════
   Phase 1: Widget create functions
   ═══════════════════════════════════════════════════════════════ */

/*
 * All create_* functions follow the pattern:
 *   sdlgui_element_t sdlgui_<widget>_create(sdlgui_t gui, sdlgui_element_t parent, <args...>);
 *
 *   parent = NULL  → top-level element, auto-added to GUIManager
 *   parent != NULL → child of parent, auto-added via parent->addChild()
 *
 * Returns the element handle, or NULL on failure.
 * The element handle is a raw pointer. It becomes dangling after:
 *   - sdlgui_cleanup() + sdlgui_element_mark_for_deletion()
 *   - sdlgui_destroy()
 *   - parent destruction
 */

/* 1.1 Button */
sdlgui_element_t sdlgui_button_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h, const char* label);
void sdlgui_button_set_on_click(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);
void sdlgui_button_set_on_mouse_over(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);
void sdlgui_button_set_label(sdlgui_element_t e, const char* label);

/* 1.2 Label */
/*
 * font_size = -1 means default (14pt from theme).
 * Label auto-sizes width/height from text (no w,h params — matches C++ constructor).
 * get_text() returns pointer to internal std::string, valid until next set_text() on same element.
 */
sdlgui_element_t sdlgui_label_create(sdlgui_t gui, sdlgui_element_t parent,
                                      int x, int y, const char* text, int font_size);
void             sdlgui_label_set_text(sdlgui_element_t e, const char* text);
const char*      sdlgui_label_get_text(sdlgui_element_t e);

/* 1.3 Panel */
/*
 * Panel also covers Slider (Slider inherits Panel), so set_draggable
 * and all element base API work on slider handles too.
 */
sdlgui_element_t sdlgui_panel_create(sdlgui_t gui, sdlgui_element_t parent,
                                      int x, int y, int w, int h);
void             sdlgui_panel_set_draggable(sdlgui_element_t e, int draggable);

/* 1.4 Slider */
sdlgui_element_t sdlgui_slider_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h,
                                       int min_val, int max_val, int initial_val,
                                       sdlgui_orientation_t orientation);
int  sdlgui_slider_get_value(sdlgui_element_t e);
void sdlgui_slider_set_value(sdlgui_element_t e, int value);
void sdlgui_slider_set_range(sdlgui_element_t e, int min_val, int max_val);
int  sdlgui_slider_get_min(sdlgui_element_t e);
int  sdlgui_slider_get_max(sdlgui_element_t e);
void sdlgui_slider_set_min(sdlgui_element_t e, int min);
void sdlgui_slider_set_max(sdlgui_element_t e, int max);
void sdlgui_slider_set_wheel_step(sdlgui_element_t e, int step);
void sdlgui_slider_set_on_change(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);
int  sdlgui_slider_get_orientation(sdlgui_element_t e);  /* 0=horizontal, 1=vertical */

/* 1.5 Checkbox */
/*
 * Checkbox has no text — add a separate Label for the label text.
 * sdlgui_bool_callback_t: element = checkbox, value = new checked state.
 */
sdlgui_element_t sdlgui_checkbox_create(sdlgui_t gui, sdlgui_element_t parent,
                                         int x, int y, int w, int h);
int  sdlgui_checkbox_is_checked(sdlgui_element_t e);
void sdlgui_checkbox_set_checked(sdlgui_element_t e, int checked);
void sdlgui_checkbox_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata);

/* 1.6 TextInput */
/*
 * get_text() returns pointer to internal std::string via TextEditable::getText(),
 * stable until next set_text().
 */
sdlgui_element_t sdlgui_text_input_create(sdlgui_t gui, sdlgui_element_t parent,
                                           int x, int y, int w, int h);
void             sdlgui_text_input_set_text(sdlgui_element_t e, const char* text);
const char*      sdlgui_text_input_get_text(sdlgui_element_t e);
void             sdlgui_text_input_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);
void             sdlgui_text_input_set_on_enter_pressed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);
void             sdlgui_text_input_set_locked(sdlgui_element_t e, int locked);
int              sdlgui_text_input_is_locked(sdlgui_element_t e);

/* 1.7 ListView */
/*
 * get_item_text() uses ListView::getItem() which returns string_view.
 * The returned pointer points to internal std::string storage and is
 * null-terminated in practice. Valid until item is modified or removed.
 */
sdlgui_element_t sdlgui_list_view_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h);
void             sdlgui_list_view_add_item(sdlgui_element_t e, const char* item);
void             sdlgui_list_view_remove_item(sdlgui_element_t e, size_t index);
void             sdlgui_list_view_clear(sdlgui_element_t e);
size_t           sdlgui_list_view_get_item_count(sdlgui_element_t e);
const char*      sdlgui_list_view_get_item_text(sdlgui_element_t e, size_t index);
void             sdlgui_list_view_set_on_row_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata);
void             sdlgui_list_view_set_on_row_double_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ProgressBar
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_progress_bar_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h);
float            sdlgui_progress_bar_get_value(sdlgui_element_t e);
void             sdlgui_progress_bar_set_value(sdlgui_element_t e, float value);
float            sdlgui_progress_bar_get_min(sdlgui_element_t e);
void             sdlgui_progress_bar_set_min(sdlgui_element_t e, float min);
float            sdlgui_progress_bar_get_max(sdlgui_element_t e);
void             sdlgui_progress_bar_set_max(sdlgui_element_t e, float max);
void             sdlgui_progress_bar_set_range(sdlgui_element_t e, float min, float max);
void             sdlgui_progress_bar_set_show_text(sdlgui_element_t e, int show);
void             sdlgui_progress_bar_set_text_format(sdlgui_element_t e, const char* format);
void             sdlgui_progress_bar_set_orientation(sdlgui_element_t e, sdlgui_orientation_t orientation);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: RadioButton
   ═══════════════════════════════════════════════════════════════════ */

/*
 * RadioButton has no text — use RadioGroup::addOption for text labels,
 * or add a separate Label as sibling.
 */
sdlgui_element_t sdlgui_radio_button_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h);
int              sdlgui_radio_button_is_selected(sdlgui_element_t e);
void             sdlgui_radio_button_set_selected(sdlgui_element_t e, int selected);
void             sdlgui_radio_button_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: RadioGroup
   ═══════════════════════════════════════════════════════════════════ */

/*
 * RadioGroup is a Panel that manages a set of mutually exclusive RadioButtons.
 *
 * add_option creates a RadioButton with text label inside the group.
 * Returns the RadioButton handle. Ownership is managed by the RadioGroup.
 */
sdlgui_element_t sdlgui_radio_group_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h);
sdlgui_element_t sdlgui_radio_group_add_option(sdlgui_element_t e, const char* text, int selected);

/*
 * Callback: (element, index, text, userdata)
 * The text pointer is valid only during callback execution.
 */
void             sdlgui_radio_group_set_on_selection_change(sdlgui_element_t e,
                                                            sdlgui_index_text_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: TextArea
   ═══════════════════════════════════════════════════════════════════ */

/*
 * font_size <= 0 means default (14pt).
 * font_path NULL or "" means empty path (system/theme default).
 * get_text() returns pointer to internal std::string, stable until next set_text().
 */
sdlgui_element_t sdlgui_text_area_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h,
                                          const char* font_path, int font_size);
void             sdlgui_text_area_set_text(sdlgui_element_t e, const char* text);
const char*      sdlgui_text_area_get_text(sdlgui_element_t e);
void             sdlgui_text_area_set_word_wrap(sdlgui_element_t e, int wrap);
int              sdlgui_text_area_get_word_wrap(sdlgui_element_t e);
void             sdlgui_text_area_set_locked(sdlgui_element_t e, int locked);
int              sdlgui_text_area_is_locked(sdlgui_element_t e);
void             sdlgui_text_area_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ComboBox
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_combo_box_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h);
void             sdlgui_combo_box_add_item(sdlgui_element_t e, const char* item);
void             sdlgui_combo_box_clear(sdlgui_element_t e);
size_t           sdlgui_combo_box_get_item_count(sdlgui_element_t e);
const char*      sdlgui_combo_box_get_item_text(sdlgui_element_t e, size_t index);
int              sdlgui_combo_box_get_selected_index(sdlgui_element_t e);
void             sdlgui_combo_box_set_selected_index(sdlgui_element_t e, int index);

/*
 * Callback: (element, index, text, userdata)
 * The text pointer is valid only during callback execution.
 */
void             sdlgui_combo_box_set_on_select(sdlgui_element_t e,
                                                sdlgui_index_text_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: StringGrid
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_string_grid_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h,
                                            size_t rows, size_t cols);
void             sdlgui_string_grid_set_dimensions(sdlgui_element_t e, size_t rows, size_t cols);
void             sdlgui_string_grid_set_cell(sdlgui_element_t e, size_t row, size_t col, const char* text);

/*
 * get_cell returns pointer to internal std::string storage.
 * Valid until the cell is modified or cleared.
 */
const char*      sdlgui_string_grid_get_cell(sdlgui_element_t e, size_t row, size_t col);
size_t           sdlgui_string_grid_get_row_count(sdlgui_element_t e);
size_t           sdlgui_string_grid_get_col_count(sdlgui_element_t e);
void             sdlgui_string_grid_clear(sdlgui_element_t e);
void             sdlgui_string_grid_set_selected_cell(sdlgui_element_t e, size_t row, size_t col);
void             sdlgui_string_grid_set_editable(sdlgui_element_t e, int editable);
int              sdlgui_string_grid_is_editable(sdlgui_element_t e);
void             sdlgui_string_grid_set_on_cell_click(sdlgui_element_t e,
                                                      sdlgui_cell_callback_t cb, void* userdata);
void             sdlgui_string_grid_set_on_cell_double_click(sdlgui_element_t e,
                                                              sdlgui_cell_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ScrollArea
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_scroll_area_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h);
void             sdlgui_scroll_area_set_content_size(sdlgui_element_t e, int w, int h);
void             sdlgui_scroll_area_set_scroll_enabled(sdlgui_element_t e, int vertical, int horizontal);
void             sdlgui_scroll_area_get_scroll_offset(sdlgui_element_t e, int* x, int* y);

/*
 * set_content transfers ownership of the content element to the ScrollArea.
 * The content element must have been created with parent=NULL (top-level).
 * After calling this, the content handle should not be used directly
 * except for style modifications — it is now owned by the ScrollArea.
 */
void             sdlgui_scroll_area_set_content(sdlgui_element_t e, sdlgui_element_t content);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: AnimatedImage
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_animated_image_create(sdlgui_t gui, sdlgui_element_t parent,
                                               int x, int y, int w, int h);
void             sdlgui_animated_image_set_sprite_sheet(sdlgui_element_t e,
                                                        const char* path, int total_frames,
                                                        int rows, int frame_w, int frame_h);
void             sdlgui_animated_image_set_fps(sdlgui_element_t e, float fps);
void             sdlgui_animated_image_set_loop(sdlgui_element_t e, int loop);
void             sdlgui_animated_image_play(sdlgui_element_t e);
void             sdlgui_animated_image_pause(sdlgui_element_t e);
void             sdlgui_animated_image_stop(sdlgui_element_t e);
int              sdlgui_animated_image_get_current_frame(sdlgui_element_t e);
int              sdlgui_animated_image_get_total_frames(sdlgui_element_t e);
int              sdlgui_animated_image_is_playing(sdlgui_element_t e);
void             sdlgui_animated_image_set_frame(sdlgui_element_t e, int frame_index);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: Canvas
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_canvas_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h);
void             sdlgui_canvas_clear(sdlgui_element_t e);
void             sdlgui_canvas_set_pen_color(sdlgui_element_t e, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ArcContainer
   ═══════════════════════════════════════════════════════════════════ */

/*
 * centerX, centerY — center of the arc layout (relative to ArcContainer position)
 * radius — distance from center for child placement
 * startAngleDeg/endAngleDeg — arc range in degrees (0–360 default)
 */
sdlgui_element_t sdlgui_arc_container_create(sdlgui_t gui, sdlgui_element_t parent,
                                              int centerX, int centerY, int radius,
                                              float start_angle_deg, float end_angle_deg);

/*
 * add_child_at_angle: positions a child element at the given angle on the arc.
 * The child element must have been created with parent=NULL (top-level).
 * After calling this, the child is owned by the ArcContainer.
 * rotateChild — whether to rotate the child to match the angle
 * offset — pixel offset from the arc radius
 */
void             sdlgui_arc_container_add_child_at_angle(sdlgui_element_t e,
                                                         sdlgui_element_t child,
                                                         float angle_deg,
                                                         int rotate_child, int offset);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: TabControl
   ═══════════════════════════════════════════════════════════════════ */

/*
 * TabControl is a Panel that manages tabs. Each tab has a button header
 * and a content panel.
 *
 * tab_button_height — height of the tab button bar (default 30)
 * add_tab creates a new tab and returns the tab's content Panel handle.
 * Users should create widgets with the returned Panel as parent.
 */
sdlgui_element_t sdlgui_tab_control_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h,
                                            int tab_button_height);
sdlgui_element_t sdlgui_tab_control_add_tab(sdlgui_element_t e, const char* title);
void             sdlgui_tab_control_set_active_tab(sdlgui_element_t e, int index);

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ContextMenu
   ═══════════════════════════════════════════════════════════════════ */

/*
 * ContextMenu is a popup menu. Create once, add items, show/hide as needed.
 * Item callbacks: void (*)(void* userdata) — no element parameter since
 * ContextMenu doesn't pass the item index to the callback.
 */
typedef void (*sdlgui_context_menu_callback_t)(void* userdata);

sdlgui_element_t sdlgui_context_menu_create(sdlgui_t gui);
void             sdlgui_context_menu_add_item(sdlgui_element_t e,
                                              const char* text,
                                              sdlgui_context_menu_callback_t cb,
                                              void* userdata);
void             sdlgui_context_menu_add_separator(sdlgui_element_t e);
void             sdlgui_context_menu_clear_items(sdlgui_element_t e);
void             sdlgui_context_menu_show_at(sdlgui_element_t e, int x, int y);
void             sdlgui_context_menu_hide(sdlgui_element_t e);
int              sdlgui_context_menu_is_visible(sdlgui_element_t e);

#ifdef __cplusplus
}
#endif

#endif /* SDL_GUI_H */
