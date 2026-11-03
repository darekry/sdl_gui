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
#include <SDL3/SDL_gpu.h> /* SDL_GPUDevice */

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
   Anchor (enum per axis + pixel margins — no magic floats)
   ═══════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_H_NONE = 0,
    SDLGUI_H_LEFT = 1,
    SDLGUI_H_CENTER = 2,
    SDLGUI_H_RIGHT = 3,
    SDLGUI_H_STRETCH = 4
} sdlgui_h_anchor_t;

typedef enum {
    SDLGUI_V_NONE = 0,
    SDLGUI_V_TOP = 1,
    SDLGUI_V_CENTER = 2,
    SDLGUI_V_BOTTOM = 3,
    SDLGUI_V_STRETCH = 4
} sdlgui_v_anchor_t;

typedef struct {
    int h;       /* sdlgui_h_anchor_t */
    int v;       /* sdlgui_v_anchor_t */
    int left;    /* px from left (Left/Stretch) */
    int top;     /* px from top (Top/Stretch) */
    int right;   /* px from right (Right/Stretch) */
    int bottom;  /* px from bottom (Bottom/Stretch) */
} sdlgui_anchor_t;

/* ═══════════════════════════════════════════════════════════════════
   Dock (edge for DockLayout — dock wins over anchor)
   ═══════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_DOCK_NONE   = 0,
    SDLGUI_DOCK_TOP    = 1,
    SDLGUI_DOCK_BOTTOM = 2,
    SDLGUI_DOCK_LEFT   = 3,
    SDLGUI_DOCK_RIGHT  = 4,
    SDLGUI_DOCK_FILL   = 5
} sdlgui_dock_t;

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
 * and renderer, and applies the default theme (Windows95).
 *
 * title     — window title (UTF-8)
 * width, height — initial window dimensions
 * resizable — 0 = fixed, 1 = user-resizable
 *
 * Returns NULL on failure.
 */
sdlgui_t sdlgui_create(const char* title, int width, int height, int resizable);

/*
 * Attach to an externally owned renderer (game engine's own window).
 * Does NOT call SDL_CreateWindow/SDL_CreateRenderer/TTF_Init — the caller
 * must have initialized SDL/TTF and created the renderer. Returns NULL on
 * failure (null renderer or non-positive dimensions).
 * sdlgui_destroy() on an attached context frees only the GUI, the caller's
 * window/renderer stay alive. All other loop calls work unchanged:
 * sdlgui_get_renderer/process_event/update/cleanup/render/handle_resize.
 */
sdlgui_t sdlgui_attach(SDL_Renderer* renderer, int width, int height);

/*
 * Destroy the GUI context and release all SDL resources.
 * Attached contexts (sdlgui_attach) free only the GUI — the externally
 * owned window/renderer are left untouched.
 * All element handles become invalid.
 */
void     sdlgui_destroy(sdlgui_t gui);

/* ═══════════════════════════════════════════════════════════════════
   Core loop API
   ═══════════════════════════════════════════════════════════════ */

SDL_Renderer*  sdlgui_get_renderer(sdlgui_t gui);
SDL_Window*    sdlgui_get_window(sdlgui_t gui);
bool           sdlgui_process_event(sdlgui_t gui, const SDL_Event* e);
void           sdlgui_update(sdlgui_t gui);
void           sdlgui_cleanup(sdlgui_t gui);
void           sdlgui_render(sdlgui_t gui);
void           sdlgui_handle_resize(sdlgui_t gui, int w, int h);
void           sdlgui_get_window_size(sdlgui_t gui, int* w, int* h);

/* ═══════════════════════════════════════════════════════════════════
   Theme presets
   ═══════════════════════════════════════════════════════════════ */

/* Default after sdlgui_create() is Windows95 (bevel). Swappable at runtime. */

void sdlgui_theme_win9x(sdlgui_t gui);      /* alias for sdlgui_theme_windows95 */
void sdlgui_theme_windows95(sdlgui_t gui);
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
void        sdlgui_element_set_dock(sdlgui_element_t e, int dock);   /* sdlgui_dock_t */
int         sdlgui_element_get_dock(sdlgui_element_t e);             /* sdlgui_dock_t */
/*
 * Install a DockLayout manager on a container (replaces the default
 * AnchorLayout). spacing = px between docked strips; pads = container
 * padding. Triggers an immediate re-layout. Pass NULL manager equivalent:
 * there is no "clear" — set another manager or leave as-is.
 */
void        sdlgui_element_set_dock_layout(sdlgui_element_t e, int spacing,
                                           int pad_left, int pad_top,
                                           int pad_right, int pad_bottom);
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
   Geometry, tree, visibility, focus, control ids (parity P0)
   ═══════════════════════════════════════════════════════════════ */

typedef struct {
    int x;
    int y;
    int w;
    int h;
} sdlgui_rect_t;

/*
 * Local rect (x/y = m_x/m_y, unreachable through the old API) and absolute
 * rect (getAbsolutePosition() + size). Both read the current layout state;
 * call them after updateLayout/resize (elements added with add_child already
 * went through it).
 */
void sdlgui_element_get_rect(sdlgui_element_t e, sdlgui_rect_t* out);
void sdlgui_element_get_absolute_rect(sdlgui_element_t e, sdlgui_rect_t* out);

int  sdlgui_element_get_visible(sdlgui_element_t e);
/* 1 only when the element AND all ancestors are visible. */
int  sdlgui_element_is_shown(sdlgui_element_t e);
int  sdlgui_element_get_enabled(sdlgui_element_t e);

sdlgui_element_t sdlgui_element_get_parent(sdlgui_element_t e);
size_t           sdlgui_element_get_child_count(sdlgui_element_t e);
sdlgui_element_t sdlgui_element_get_child(sdlgui_element_t e, size_t index);

/* DFS search inside the element's subtree (self included). NULL + last_error. */
sdlgui_element_t sdlgui_element_find_by_id(sdlgui_element_t e, const char* id);

/*
 * Keyboard focus. request_focus returns 1 on success, 0 when the element
 * cannot take focus, is disabled or is hidden (including via an ancestor).
 */
int              sdlgui_element_request_focus(sdlgui_element_t e);
sdlgui_element_t sdlgui_get_focused_element(sdlgui_t gui);
void             sdlgui_focus_next(sdlgui_t gui, int forward);

/* Window rect in pixels: {0, 0, window_w, window_h}. */
void sdlgui_get_window_rect(sdlgui_t gui, sdlgui_rect_t* out);

/*
 * Dialog control ids (Win16 DLGITEM ids). Valid range 0..0xffff; out of
 * range is rejected with last_error and leaves the element unchanged.
 * find_by_control_id searches the element's subtree, first DFS match.
 */
void             sdlgui_element_set_control_id(sdlgui_element_t e, int control_id);
int              sdlgui_element_get_control_id(sdlgui_element_t e);
sdlgui_element_t sdlgui_element_find_by_control_id(sdlgui_element_t e, int control_id);

/* ═══════════════════════════════════════════════════════════════════
   Text metrics + Label alignment
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Measures text with the default theme font (or font_size when > 0).
 * Multi-line text is split on '\n'; width = widest line, height = lines *
 * line_height (line_height = TTF_GetFontHeight). Empty text returns
 * 0/line_height without touching TTF layout. Any out pointer may be NULL.
 * Returns 0 when the font cannot be loaded (outputs zeroed).
 */
int sdlgui_measure_text(sdlgui_t gui, const char* text, int font_size,
                        int* width, int* height, int* line_height);
int sdlgui_get_font_metrics(sdlgui_t gui, int font_size,
                            int* line_height, int* ascent, int* descent);

/* h/v accept TextHAlign/TextVAlign: 0=left/top, 1=center/middle, 2=right/bottom. */
void sdlgui_label_set_alignment(sdlgui_element_t e, int h, int v);
void sdlgui_label_set_font_size(sdlgui_element_t e, int font_size);
/* Button label via Button::getText(); caller buffer semantics. */
size_t sdlgui_button_get_label_buf(sdlgui_element_t e, char* buf, size_t buf_len);

/* ═══════════════════════════════════════════════════════════════════
   Anchor factories
   ═══════════════════════════════════════════════════════════════ */

sdlgui_anchor_t sdlgui_anchor_none(void);                          /* no anchor (fixed position) */
sdlgui_anchor_t sdlgui_anchor_top_left(int margin);
sdlgui_anchor_t sdlgui_anchor_top_right(int margin);
sdlgui_anchor_t sdlgui_anchor_bottom_left(int margin);
sdlgui_anchor_t sdlgui_anchor_bottom_right(int margin);
sdlgui_anchor_t sdlgui_anchor_center(void);                        /* element centered, no corner offset */
sdlgui_anchor_t sdlgui_anchor_fill(int padding);                   /* fill parent */
sdlgui_anchor_t sdlgui_anchor_horizontal_stretch(int pad_left, int pad_right);
sdlgui_anchor_t sdlgui_anchor_vertical_stretch(int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_top_bar(int height, int pad_horiz);
sdlgui_anchor_t sdlgui_anchor_bottom_bar(int height, int pad_horiz);
sdlgui_anchor_t sdlgui_anchor_left_sidebar(int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_right_sidebar(int pad_top, int pad_bottom);
sdlgui_anchor_t sdlgui_anchor_make(int h, int v, int left, int top, int right, int bottom);

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

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: MenuBar (Win32-style menu strip)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * MenuBar owns the whole menu tree. Item handles are stable until
 * sdlgui_menu_bar_clear() or bar destruction — after that they are invalid.
 *
 * Items carry command id strings instead of callbacks. Set one handler per
 * bar; it receives the bar handle and the command id (valid only during the
 * call). Check/radio state is updated before the command fires, so the
 * handler can query it with sdlgui_menu_item_is_checked().
 *
 * height <= 0 -> auto height from the theme font.
 */
typedef void* sdlgui_menu_item_t;   /* MenuItem*, owned by the MenuBar */

typedef void (*sdlgui_menu_command_callback_t)(sdlgui_element_t bar,
                                               const char* command,
                                               void* userdata);

/* Integer command id callback (Win16 command parity). */
typedef void (*sdlgui_menu_command_id_callback_t)(sdlgui_element_t bar,
                                                  int command_id,
                                                  void* userdata);

typedef enum {
    SDLGUI_MENU_CHECK_NONE  = 0,
    SDLGUI_MENU_CHECK       = 1,
    SDLGUI_MENU_RADIO       = 2
} sdlgui_menu_check_t;

sdlgui_element_t sdlgui_menu_bar_create(sdlgui_t gui, sdlgui_element_t parent,
                                        int x, int y, int w, int h);
void             sdlgui_menu_bar_set_on_command(sdlgui_element_t bar,
                                                sdlgui_menu_command_callback_t cb,
                                                void* userdata);
sdlgui_menu_item_t sdlgui_menu_bar_add_menu(sdlgui_element_t bar, const char* text);
void             sdlgui_menu_bar_open_menu(sdlgui_element_t bar, int index);
void             sdlgui_menu_bar_close(sdlgui_element_t bar);
int              sdlgui_menu_bar_is_open(sdlgui_element_t bar);
int              sdlgui_menu_bar_get_menu_count(sdlgui_element_t bar);
int              sdlgui_menu_bar_get_height(sdlgui_element_t bar);
void             sdlgui_menu_bar_clear(sdlgui_element_t bar);

sdlgui_menu_item_t sdlgui_menu_item_add_item(sdlgui_menu_item_t parent,
                                             const char* text,
                                             const char* command);
sdlgui_menu_item_t sdlgui_menu_item_add_submenu(sdlgui_menu_item_t parent,
                                                const char* text);
void               sdlgui_menu_item_add_separator(sdlgui_menu_item_t parent);
void               sdlgui_menu_item_set_shortcut(sdlgui_menu_item_t item,
                                                 const char* shortcut);
/* Display-only variant: draws the shortcut but registers no accelerator. */
void               sdlgui_menu_item_set_shortcut_display(sdlgui_menu_item_t item,
                                                         const char* shortcut);
void               sdlgui_menu_item_set_enabled(sdlgui_menu_item_t item, int enabled);
void               sdlgui_menu_item_set_check_type(sdlgui_menu_item_t item,
                                                   sdlgui_menu_check_t type);
void               sdlgui_menu_item_set_checked(sdlgui_menu_item_t item, int checked);
int                sdlgui_menu_item_is_checked(sdlgui_menu_item_t item);

/*
 * Integer command ids: activation order is action -> commandId -> dispatch
 * string, so items with an id do not fire the string command handler.
 * Changing text/enabled/check re-measures titles and refreshes popups.
 */
sdlgui_menu_item_t sdlgui_menu_item_add_item_id(sdlgui_menu_item_t parent,
                                                const char* text, int command_id);
void               sdlgui_menu_bar_set_on_command_id(sdlgui_element_t bar,
                                                     sdlgui_menu_command_id_callback_t cb,
                                                     void* userdata);
void               sdlgui_menu_item_set_text(sdlgui_menu_item_t item, const char* text);
size_t             sdlgui_menu_item_get_text_buf(sdlgui_menu_item_t item,
                                                 char* buf, size_t buf_len);
/* DFS (including submenus) for hasCommandId == command_id; NULL + last_error. */
sdlgui_menu_item_t sdlgui_menu_item_find_by_id(sdlgui_element_t bar, int command_id);

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: GPU context (Vulkan + SDL_CreateGPURenderer)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Create a GUI context with a GPU renderer (Vulkan/SPIR-V).
 * Required for ShaderPanel to actually run shaders.
 * Returns NULL on failure (e.g. no Vulkan driver).
 */
sdlgui_t       sdlgui_create_gpu(const char* title, int width, int height, int resizable);

/*
 * Returns the underlying SDL_GPUDevice, or NULL for a CPU context.
 * Usable to build/validate SPIR-V shaders before sdlgui_shader_panel_set_shader().
 */
SDL_GPUDevice* sdlgui_get_gpu_device(sdlgui_t gui);

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: RangeSlider
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Dual-thumb slider: selects a [lower, upper] value range.
 * lower_val and upper_val are clamped to [min_val, max_val]
 * (lower_val > upper_val swaps them).
 */
sdlgui_element_t sdlgui_range_slider_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h,
                                             int min_val, int max_val,
                                             int lower_val, int upper_val,
                                             sdlgui_orientation_t orientation);
int              sdlgui_range_slider_get_lower_value(sdlgui_element_t e);
void             sdlgui_range_slider_set_lower_value(sdlgui_element_t e, int value);
int              sdlgui_range_slider_get_upper_value(sdlgui_element_t e);
void             sdlgui_range_slider_set_upper_value(sdlgui_element_t e, int value);
void             sdlgui_range_slider_set_range(sdlgui_element_t e, int min_val, int max_val);
int              sdlgui_range_slider_get_min(sdlgui_element_t e);
int              sdlgui_range_slider_get_max(sdlgui_element_t e);
void             sdlgui_range_slider_set_min(sdlgui_element_t e, int min);
void             sdlgui_range_slider_set_max(sdlgui_element_t e, int max);
void             sdlgui_range_slider_set_wheel_step(sdlgui_element_t e, int step);
int              sdlgui_range_slider_get_wheel_step(sdlgui_element_t e);
void             sdlgui_range_slider_set_on_change(sdlgui_element_t e,
                                                   sdlgui_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: Cursor
   ═══════════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_CURSOR_NORMAL   = 0,
    SDLGUI_CURSOR_HOVER    = 1,
    SDLGUI_CURSOR_PRESSED  = 2,
    SDLGUI_CURSOR_DISABLED = 3,
    SDLGUI_CURSOR_BUSY     = 4,
    SDLGUI_CURSOR_TEXT     = 5,
    SDLGUI_CURSOR_CUSTOM1  = 6,
    SDLGUI_CURSOR_CUSTOM2  = 7,
    SDLGUI_CURSOR_CUSTOM3  = 8
} sdlgui_cursor_state_t;

/*
 * Cursor state callback: state is the new cursor state.
 */
typedef void (*sdlgui_cursor_state_callback_t)(sdlgui_element_t element,
                                               sdlgui_cursor_state_t state,
                                               void* userdata);

/*
 * Create a custom cursor overlay and install it on the GUI.
 * The system cursor is hidden while a custom cursor is installed.
 * Creating another cursor replaces (and destroys) the previous one —
 * old handles become invalid.
 * Returns the cursor element handle (top-level, owned by the GUI).
 */
sdlgui_element_t sdlgui_cursor_create(sdlgui_t gui);

/*
 * Set the texture for a given cursor state. path is loaded via
 * TextureManager (embedded assets supported). hotspot_x/y is the
 * click point inside the texture (default 0,0 = top-left).
 */
void sdlgui_cursor_set_texture(sdlgui_element_t e, sdlgui_cursor_state_t state,
                               const char* path, int hotspot_x, int hotspot_y);

/*
 * Set an animated sprite-sheet cursor for a state.
 * total_frames frames laid out in `rows` rows (columns are computed).
 * fps controls animation speed (default 12 if <= 0).
 * If total_frames <= 0, falls back to a static texture.
 */
void sdlgui_cursor_set_animated_texture(sdlgui_element_t e, sdlgui_cursor_state_t state,
                                        const char* path, int total_frames, int rows,
                                        float fps, int hotspot_x, int hotspot_y);

/*
 * Build the cursor from caller-owned surface pixels (copied into a texture;
 * the surface is not adopted and may be freed immediately). Returns to
 * overlay mode, hiding any system cursor.
 */
void sdlgui_cursor_set_surface(sdlgui_element_t e, sdlgui_cursor_state_t state,
                               SDL_Surface* surface, int hotspot_x, int hotspot_y);

/*
 * Show a native SDL system cursor (SDL_SystemCursor) and stop drawing the
 * overlay. set_texture/set_surface/animated switch back to overlay mode.
 */
void sdlgui_cursor_set_system(sdlgui_element_t e, SDL_SystemCursor cursor);
int  sdlgui_cursor_is_system(sdlgui_element_t e);

void  sdlgui_cursor_set_state(sdlgui_element_t e, sdlgui_cursor_state_t state);
int   sdlgui_cursor_get_state(sdlgui_element_t e);
void  sdlgui_cursor_set_offset(sdlgui_element_t e, int offset_x, int offset_y);
void  sdlgui_cursor_get_offset(sdlgui_element_t e, int* offset_x, int* offset_y);
void  sdlgui_cursor_set_scale(sdlgui_element_t e, float scale);
float sdlgui_cursor_get_scale(sdlgui_element_t e);
void  sdlgui_cursor_set_visible(sdlgui_element_t e, int visible);
int   sdlgui_cursor_is_visible(sdlgui_element_t e);
void  sdlgui_cursor_set_on_state_changed(sdlgui_element_t e,
                                         sdlgui_cursor_state_callback_t cb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: ShaderPanel (GPU)
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Panel that renders its content through a fragment shader.
 * Requires a GPU context (sdlgui_create_gpu). On a CPU context the
 * panel still renders (as a plain panel), but shaders are ignored.
 */
sdlgui_element_t sdlgui_shader_panel_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h);

/*
 * Upload SPIR-V fragment shader bytecode. Entry point must be "main".
 * Replaces any previously set shader. Has no effect on CPU contexts.
 */
void sdlgui_shader_panel_set_shader(sdlgui_element_t e,
                                    const uint8_t* spirv_data, size_t spirv_size);

void sdlgui_shader_panel_set_shader_enabled(sdlgui_element_t e, int enabled);
int  sdlgui_shader_panel_is_shader_enabled(sdlgui_element_t e);

/*
 * Uniforms passed to the shader as vertex colors:
 * time (x channel), mouse (y/z channels).
 */
void sdlgui_shader_panel_set_uniform_time(sdlgui_element_t e, float time);
void sdlgui_shader_panel_set_uniform_mouse(sdlgui_element_t e, float x, float y);

/* ═══════════════════════════════════════════════════════════════════
   Point 5: Lifetime + WidgetFactory boundary API
   ═══════════════════════════════════════════════════════════════════ */

/*
 * Last error string from a checked API call (type mismatch, null handle,
 * unknown widget type, index out of range). Empty when the last call
 * succeeded. Thread-local; valid until the next failing call on this thread.
 */
const char*      sdlgui_last_error(void);

/*
 * Non-zero when the element handle is still registered with the GUI
 * (generational check — safe against address reuse after destroy).
 */
int              sdlgui_element_is_alive(sdlgui_t gui, sdlgui_element_t e);

/*
 * Generic widget creation through the shared WidgetFactory registry.
 * type is one of: Panel, Button, Label, Checkbox, RadioButton, RadioGroup,
 * Slider, RangeSlider, StringGrid, ListView, TextInput, TextArea, ComboBox,
 * TabControl, AnimatedImage, Canvas, ProgressBar, ScrollArea, ArcContainer.
 * Returns NULL + sdlgui_last_error() on unknown type. Widgets are created
 * with default props — configure afterwards with the type-specific API.
 */
sdlgui_element_t sdlgui_create_widget(sdlgui_t gui, sdlgui_element_t parent,
                                      const char* type, int x, int y, int w, int h);

/*
 * Caller-buffer string getters — the fix for dangling c_str() pointers.
 * Copy into buf (NUL-terminated, truncated when too small) and return bytes
 * written excluding NUL. Never return internal pointers.
 */
size_t           sdlgui_label_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len);
size_t           sdlgui_text_input_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len);
size_t           sdlgui_text_area_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len);
size_t           sdlgui_list_view_get_item_text_buf(sdlgui_element_t e, size_t index,
                                                   char* buf, size_t buf_len);
size_t           sdlgui_combo_box_get_item_text_buf(sdlgui_element_t e, size_t index,
                                                   char* buf, size_t buf_len);

/* ═══════════════════════════════════════════════════════════════════
   Parity P1: draw/event hooks + CustomControl
   ═══════════════════════════════════════════════════════════════════ */

typedef enum {
    SDLGUI_DRAW_BEFORE  = 0,
    SDLGUI_DRAW_REPLACE = 1,
    SDLGUI_DRAW_AFTER   = 2
} sdlgui_draw_phase_t;

/*
 * Draw callback runs in ELEMENT-LOCAL coordinates: local_rect is
 * {0,0,w,h} and clip_rect is the ancestors' clip translated to local
 * space. Installing any draw hook switches the element to the direct path
 * (no render cache) and disables rotation for it.
 */
typedef void (*sdlgui_draw_callback_t)(sdlgui_element_t e, SDL_Renderer* renderer,
                                       const sdlgui_rect_t* local_rect,
                                       const sdlgui_rect_t* clip_rect,
                                       void* userdata);

/*
 * Event callback fires before child propagation. Mouse events get element
 * local x/y, other events 0/0. Returning 1 consumes the event.
 */
typedef int (*sdlgui_event_callback_t)(sdlgui_element_t e, const SDL_Event* event,
                                       int local_x, int local_y, void* userdata);

void sdlgui_element_set_draw_hook(sdlgui_element_t e, sdlgui_draw_phase_t phase,
                                  sdlgui_draw_callback_t cb, void* userdata);
void sdlgui_element_clear_draw_hooks(sdlgui_element_t e);
void sdlgui_element_set_event_hook(sdlgui_element_t e, sdlgui_event_callback_t cb,
                                   void* userdata);
void sdlgui_element_clear_event_hook(sdlgui_element_t e);
/* Routes command_id to the nearest FormDialog's on_command (WM_COMMAND). */
void sdlgui_element_notify_command(sdlgui_element_t e, int command_id);

/* Focusable owner-draw panel; paint == REPLACE hook, event == event hook. */
sdlgui_element_t sdlgui_custom_control_create(sdlgui_t gui, sdlgui_element_t parent,
                                               int x, int y, int w, int h,
                                               sdlgui_draw_callback_t paint,
                                               sdlgui_event_callback_t event,
                                               void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Parity P1: FormDialog
   ═══════════════════════════════════════════════════════════════ */

/* Command callback: (form, item_id, userdata). */
typedef void (*sdlgui_form_command_callback_t)(sdlgui_element_t form, int item_id,
                                               void* userdata);

sdlgui_element_t sdlgui_form_create(sdlgui_t gui, const char* title,
                                    int x, int y, int w, int h);
/*
 * Reparents a top-level element (created with parent == NULL) into the form
 * and assigns its control id. Returns the element or NULL + last_error.
 */
sdlgui_element_t sdlgui_form_add_widget(sdlgui_element_t form, int id,
                                        sdlgui_element_t child);
sdlgui_element_t sdlgui_form_add_static(sdlgui_element_t form, int id,
                                        const char* text, int x, int y, int w, int h);
sdlgui_element_t sdlgui_form_add_button(sdlgui_element_t form, int id,
                                        const char* text, int x, int y, int w, int h);
sdlgui_element_t sdlgui_form_add_edit(sdlgui_element_t form, int id,
                                      const char* text, int x, int y, int w, int h);
sdlgui_element_t sdlgui_form_add_custom(sdlgui_element_t form, int id,
                                        int x, int y, int w, int h);
sdlgui_element_t sdlgui_form_find_item(sdlgui_element_t form, int id);
int              sdlgui_form_get_item_rect(sdlgui_element_t form, int id,
                                           sdlgui_rect_t* out);
int              sdlgui_form_set_item_visible(sdlgui_element_t form, int id, int visible);
int              sdlgui_form_set_item_enabled(sdlgui_element_t form, int id, int enabled);
int              sdlgui_form_set_item_text(sdlgui_element_t form, int id, const char* text);
int              sdlgui_form_set_item_rect(sdlgui_element_t form, int id,
                                           int x, int y, int w, int h);
void             sdlgui_form_end(sdlgui_element_t form, int result);
int              sdlgui_form_get_result(sdlgui_element_t form);
int              sdlgui_form_is_closed(sdlgui_element_t form);
void             sdlgui_form_set_on_command(sdlgui_element_t form,
                                            sdlgui_form_command_callback_t cb,
                                            void* userdata);

/* Description-driven construction (CivLin translates its DLGTEMPLATE). */
typedef enum {
    SDLGUI_FORM_ITEM_STATIC = 0,
    SDLGUI_FORM_ITEM_BUTTON = 1,
    SDLGUI_FORM_ITEM_EDIT   = 2,
    SDLGUI_FORM_ITEM_CUSTOM = 3
} sdlgui_form_item_type_t;

typedef struct {
    int id;
    int type;        /* sdlgui_form_item_type_t */
    const char* text;
    int x, y, w, h;
} sdlgui_form_item_desc_t;

sdlgui_element_t sdlgui_form_create_from_items(sdlgui_t gui, const char* title,
                                               int x, int y, int w, int h,
                                               const sdlgui_form_item_desc_t* items,
                                               size_t count);

/* Works on any ModalDialog (FormDialog stores the result; DialogBox closes). */
void sdlgui_dialog_close_with_result(sdlgui_element_t e, int result);

/* ═══════════════════════════════════════════════════════════════════
   Parity P1: modal runner
   ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    int keep_background;          /* 0 = clear, 1 = keep last frame */
    int clear_r, clear_g, clear_b;
    int cancel_key;               /* SDL_Keycode; 0 = none */
    int cancel_result;            /* result for Esc/quit (default -1) */
    void (*frame_cb)(void* userdata);
    int  (*event_filter)(const SDL_Event* e, void* userdata); /* 1 = consumed */
    void* userdata;
} sdlgui_modal_options_t;

/*
 * Pumps SDL_PollEvent → process_event → update → cleanup → render →
 * SDL_RenderPresent until the dialog closes (form_end / dialog_close).
 * Works with sdlgui_attach (uses the manager's renderer, not SDLApp::run).
 * The result is copied before cleanup(), so result_out stays valid even when
 * the dialog is destroyed by the final cleanup. Returns the result code.
 */
int sdlgui_modal_run(sdlgui_t gui, sdlgui_element_t dialog,
                     const sdlgui_modal_options_t* opts, int* result_out);

/* ═══════════════════════════════════════════════════════════════════
   Parity P1: MenuList
   ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* text;   /* copied at create; NULL = "" */
    int id;
    int enabled;
} sdlgui_menu_list_item_t;

typedef void (*sdlgui_menu_list_select_cb_t)(sdlgui_element_t e, int item_id,
                                             const char* text, void* userdata);
typedef void (*sdlgui_menu_list_row_cb_t)(sdlgui_element_t e,
                                          SDL_Renderer* renderer,
                                          const sdlgui_rect_t* row_rect,
                                          const char* text, int item_id,
                                          int selected, int enabled,
                                          void* userdata);
typedef void (*sdlgui_menu_list_tick_cb_t)(sdlgui_element_t e, void* userdata);

sdlgui_element_t sdlgui_menu_list_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h,
                                          const char* title,
                                          const sdlgui_menu_list_item_t* items,
                                          size_t count);
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
void sdlgui_menu_list_set_row_callback(sdlgui_element_t e,
                                       sdlgui_menu_list_row_cb_t cb,
                                       void* userdata);
void sdlgui_menu_list_set_on_tick(sdlgui_element_t e,
                                  sdlgui_menu_list_tick_cb_t cb,
                                  void* userdata);

/* ═══════════════════════════════════════════════════════════════════
   Parity P1: TextInput extensions
   ═══════════════════════════════════════════════════════════════ */

/* max_length in UTF-8 characters; 0 = unlimited. Overlong input is truncated. */
void sdlgui_text_input_set_max_length(sdlgui_element_t e, size_t max_length);
/* char_filter gets a Unicode codepoint; returning 0 rejects the character. */
void sdlgui_text_input_set_char_filter(sdlgui_element_t e,
                                       int (*filter)(unsigned codepoint, void* userdata),
                                       void* userdata);
/*
 * on_key fires at the start of handleEvent (after locked/enabled/visible).
 * Returning 1 consumes the key. Locked input never reaches the hook.
 */
void sdlgui_text_input_set_on_key(sdlgui_element_t e,
                                  int (*cb)(sdlgui_element_t e,
                                            const SDL_KeyboardEvent* key,
                                            void* userdata),
                                  void* userdata);
void sdlgui_text_input_set_cursor_pos(sdlgui_element_t e, size_t char_pos);
void sdlgui_text_input_set_selection(sdlgui_element_t e, size_t start, size_t end);

/* ═══════════════════════════════════════════════════════════════════
   Dialogs (DialogBox / FileDialog / ColorDialog / MessageBox)
   ═══════════════════════════════════════════════════════════════ */

/*
 * Dialog callbacks.
 *
 * button_index — clicked button (0-based). -1 means the dialog was
 *   cancelled via Esc (DialogBox::onCancel path).
 * confirmed — 1 when the "yes" button (index 0) was clicked, 0 otherwise
 *   (includes Esc cancellation, which reports index -1).
 * path — full selected path, valid only during callback execution.
 * color — the confirmed color (alpha always 255).
 */
typedef void (*sdlgui_dialog_button_callback_t)(sdlgui_element_t element, int button_index, void* userdata);
typedef void (*sdlgui_dialog_confirm_callback_t)(sdlgui_element_t element, int confirmed, void* userdata);
typedef void (*sdlgui_file_dialog_callback_t)(sdlgui_element_t element, const char* path, void* userdata);
typedef void (*sdlgui_color_dialog_callback_t)(sdlgui_element_t element, SDL_Color color, void* userdata);

/*
 * Simple void callback (no element): used by MessageBox info/error/
 * warning/custom, which carry no result value.
 */
typedef void (*sdlgui_simple_callback_t)(void* userdata);

/*
 * Dialog type IDs returned by sdlgui_dialog_box_get_type().
 */
typedef enum {
    SDLGUI_DIALOG_TYPE_CONFIRM = 0,
    SDLGUI_DIALOG_TYPE_ALERT   = 1,
    SDLGUI_DIALOG_TYPE_CUSTOM  = 2
} sdlgui_dialog_type_t;

/*
 * Modal base — works on any dialog handle (DialogBox, FileDialog,
 * ColorDialog, MessageBox result). Wrong-type handle is a no-op
 * (close/center) or returns 0 (is_open) + sdlgui_last_error().
 */
void sdlgui_dialog_close(sdlgui_element_t e);
int  sdlgui_dialog_is_open(sdlgui_element_t e);
void sdlgui_dialog_center(sdlgui_element_t e);

/*
 * DialogBox — modal composite (Panel + message Label + Buttons).
 *
 * All four factories center the dialog in the viewport and add it to
 * the manager as a top-level overlay (no parent param — dialogs are
 * always top-level). width/height <= 0 selects the C++ default
 * (confirm/custom 400x150, alert 350x120, with_title 400x180).
 * NULL message/title/labels fall back to "" (confirm labels fall back
 * to "Tak"/"Nie", alert label to "OK").
 * custom/with_title with button_count == 0 returns NULL + last error.
 */
sdlgui_element_t sdlgui_dialog_box_create_confirm(sdlgui_t gui, const char* message,
                                                  const char* yes_label, const char* no_label,
                                                  sdlgui_dialog_confirm_callback_t cb, void* userdata,
                                                  int width, int height);
sdlgui_element_t sdlgui_dialog_box_create_alert(sdlgui_t gui, const char* message,
                                                 const char* ok_label,
                                                 sdlgui_dialog_button_callback_t cb, void* userdata,
                                                 int width, int height);
sdlgui_element_t sdlgui_dialog_box_create_custom(sdlgui_t gui, const char* message,
                                                 const char** button_labels, size_t button_count,
                                                 sdlgui_dialog_button_callback_t cb, void* userdata,
                                                 int width, int height);
sdlgui_element_t sdlgui_dialog_box_create_with_title(sdlgui_t gui, const char* title,
                                                      const char* message,
                                                      const char** button_labels, size_t button_count,
                                                      sdlgui_dialog_button_callback_t cb, void* userdata,
                                                      int width, int height);
void             sdlgui_dialog_box_set_message(sdlgui_element_t e, const char* message);
void             sdlgui_dialog_box_set_title(sdlgui_element_t e, const char* title);
int              sdlgui_dialog_box_get_last_clicked(sdlgui_element_t e);  /* -1 = none/cancelled */
int              sdlgui_dialog_box_get_type(sdlgui_element_t e);          /* sdlgui_dialog_type_t, -1 on error */

/*
 * FileDialog — modal file picker (title bar + dir/file grids +
 * filename input + Open/Cancel). Centered, top-level, already added
 * to the manager on return. NULL title/start_path/filter fall back to
 * "" (empty start path = current working directory, "*" / "*.*" = all
 * files). The callback fires only on confirm with an existing file
 * (Open) or any non-empty name (Save); Esc/Cancel closes without
 * firing. path is valid only during callback execution.
 */
sdlgui_element_t sdlgui_file_dialog_create_open(sdlgui_t gui, const char* title,
                                                const char* start_path, const char* filter,
                                                sdlgui_file_dialog_callback_t cb, void* userdata);
sdlgui_element_t sdlgui_file_dialog_create_save(sdlgui_t gui, const char* title,
                                                const char* start_path, const char* filter,
                                                sdlgui_file_dialog_callback_t cb, void* userdata);
void             sdlgui_file_dialog_set_current_path(sdlgui_element_t e, const char* path);
const char*      sdlgui_file_dialog_get_current_path(sdlgui_element_t e);
size_t           sdlgui_file_dialog_get_current_path_buf(sdlgui_element_t e, char* buf, size_t buf_len);
const char*      sdlgui_file_dialog_get_selected_file(sdlgui_element_t e);
size_t           sdlgui_file_dialog_get_selected_file_buf(sdlgui_element_t e, char* buf, size_t buf_len);

/*
 * ColorDialog — modal RGB/HSV picker (6 sliders + HEX input + old/new
 * preview + 12 presets + OK/Cancel). Centered (460x560), top-level.
 * NULL title falls back to "". The callback fires only on OK (Enter
 * or OK button); Cancel/Esc closes without firing.
 */
sdlgui_element_t sdlgui_color_dialog_create(sdlgui_t gui, const char* title,
                                             SDL_Color initial,
                                             sdlgui_color_dialog_callback_t cb, void* userdata);
void             sdlgui_color_dialog_set_color(sdlgui_element_t e, SDL_Color color);
SDL_Color        sdlgui_color_dialog_get_color(sdlgui_element_t e);
SDL_Color        sdlgui_color_dialog_get_initial(sdlgui_element_t e);

/*
 * MessageBox — one-shot static helpers (DialogBox::createWithTitle /
 * createConfirm + addElement). Each call creates, centers and shows a
 * new top-level dialog and returns its handle (type "DialogBox").
 * Callbacks are optional (may be NULL). show_question fires on_yes on
 * "Yes" and on_no on "No"/Esc; the rest fire once on button click or
 * Esc (treated as dismissal).
 */
sdlgui_element_t sdlgui_message_box_show_info(sdlgui_t gui, const char* message,
                                              sdlgui_simple_callback_t cb, void* userdata);
sdlgui_element_t sdlgui_message_box_show_error(sdlgui_t gui, const char* message,
                                               sdlgui_simple_callback_t cb, void* userdata);
sdlgui_element_t sdlgui_message_box_show_warning(sdlgui_t gui, const char* message,
                                                 sdlgui_simple_callback_t cb, void* userdata);
sdlgui_element_t sdlgui_message_box_show_question(sdlgui_t gui, const char* message,
                                                  sdlgui_simple_callback_t on_yes, void* yes_userdata,
                                                  sdlgui_simple_callback_t on_no, void* no_userdata);
sdlgui_element_t sdlgui_message_box_show_custom(sdlgui_t gui, const char* title,
                                                const char* message, const char* button_text,
                                                sdlgui_simple_callback_t cb, void* userdata);

#ifdef __cplusplus
}
#endif

#endif /* SDL_GUI_H */
