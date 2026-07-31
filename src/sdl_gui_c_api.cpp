#include "gui_context.hpp"
#include "theme_presets.hpp"
#include "easing.hpp"
#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "slider.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "list_view.hpp"
#include "progress_bar.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "text_area.hpp"
#include "combobox.hpp"
#include "string_grid.hpp"
#include "scroll_area.hpp"
#include "animated_image.hpp"
#include "canvas.hpp"
#include "arc_container.hpp"
#include "tab_control.hpp"
#include "context_menu.hpp"

#include "sdl_gui.h"

#include <cstring>
#include <cassert>

/* ═══════════════════════════════════════════════════════════
   Internal context
   ═══════════════════════════════════════════════════════════ */

using CContext = GUIContext;

static CContext* unwrap_ctx(sdlgui_t gui) { return static_cast<CContext*>(gui); }
static GUIElement* unwrap_elem(sdlgui_element_t e) { return static_cast<GUIElement*>(e); }
static sdlgui_element_t wrap_elem(GUIElement* e) { return e; }

/* ═══════════════════════════════════════════════════════════
   State conversion
   ═══════════════════════════════════════════════════════════ */

static ElementState convert_state(sdlgui_element_state_t s) {
    switch (s) {
        case SDLGUI_STATE_NORMAL:   return ElementState::Normal;
        case SDLGUI_STATE_HOVER:    return ElementState::Hover;
        case SDLGUI_STATE_PRESSED:  return ElementState::Pressed;
        case SDLGUI_STATE_DISABLED: return ElementState::Disabled;
    }
    return ElementState::Normal;
}

/* ═══════════════════════════════════════════════════════════
   Anchor conversion
   ═══════════════════════════════════════════════════════════ */

static Anchor convert_anchor(const sdlgui_anchor_t* a) {
    Anchor anchor;
    anchor.left   = a->left;
    anchor.top    = a->top;
    anchor.right  = a->right;
    anchor.bottom = a->bottom;
    return anchor;
}

/* ═══════════════════════════════════════════════════════════
   Helper: add element to parent or top-level
   ═══════════════════════════════════════════════════════════ */

template<typename T>
static sdlgui_element_t add_element(CContext* ctx, sdlgui_element_t parent, std::unique_ptr<T> elem) {
    auto* raw = elem.get();
    if (parent) {
        unwrap_elem(parent)->addChild(std::move(elem));
    } else {
        ctx->getGUIManager().addElement(std::move(elem));
    }
    return wrap_elem(raw);
}

/* ═══════════════════════════════════════════════════════════
   Context lifecycle
   ═══════════════════════════════════════════════════════════ */

extern "C" {

sdlgui_t sdlgui_create(const char* title, int width, int height, int resizable) {
    try {
        auto* ctx = new CContext(title, width, height, resizable != 0);
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

void sdlgui_destroy(sdlgui_t gui) {
    if (!gui) return;
    delete unwrap_ctx(gui);
}

/* ═══════════════════════════════════════════════════════════
   Core loop API
   ═══════════════════════════════════════════════════════════ */

SDL_Renderer* sdlgui_get_renderer(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    return ctx->getRenderer();
}

bool sdlgui_process_event(sdlgui_t gui, const SDL_Event* e) {
    auto* ctx = unwrap_ctx(gui);
   return ctx->getGUIManager().processEvent(*e);
}

void sdlgui_update(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().update();
}

void sdlgui_cleanup(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().cleanup();
}

void sdlgui_render(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().render();
}

void sdlgui_handle_resize(sdlgui_t gui, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().handleResize(w, h);
}

void sdlgui_get_window_size(sdlgui_t gui, int* w, int* h) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().getWindowSize(*w, *h);
}

/* ═══════════════════════════════════════════════════════════
   Theme presets
   ═══════════════════════════════════════════════════════════ */

void sdlgui_theme_win9x(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().setTheme(ThemePresets::createWin9xTheme());
}

void sdlgui_theme_dark(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().setTheme(ThemePresets::createDarkTheme());
}

void sdlgui_theme_light(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().setTheme(ThemePresets::createLightTheme());
}

void sdlgui_theme_high_contrast(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().setTheme(ThemePresets::createHighContrastTheme());
}

/* ═══════════════════════════════════════════════════════════
   Tooltip
   ═══════════════════════════════════════════════════════════ */

void sdlgui_show_tooltip(sdlgui_t gui, sdlgui_element_t target, const char* text) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().showTooltip(unwrap_elem(target), text ? text : "");
}

void sdlgui_hide_tooltip(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().hideTooltip();
}

/* ═══════════════════════════════════════════════════════════════════
   TimerManager
   ═══════════════════════════════════════════════════════════════════ */

uint32_t sdlgui_add_timer(sdlgui_t gui, sdlgui_element_t target,
                          uint32_t delay_ms, int single_shot,
                          sdlgui_timer_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    auto* elem = target ? unwrap_elem(target) : nullptr;
    return ctx->getGUIManager().getTimerManager()->addTimer(elem, delay_ms, single_shot != 0,
        [cb, userdata, elem](GUIElement* /*target*/) {
            if (cb) cb(wrap_elem(elem), userdata);
        });
}

void sdlgui_remove_timer(sdlgui_t gui, uint32_t timer_id) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().getTimerManager()->removeTimer(timer_id);
}

/* ═══════════════════════════════════════════════════════════════════
   AnimationManager — Looping
   ═══════════════════════════════════════════════════════════════════ */

uint32_t sdlgui_add_loop_animation(sdlgui_t gui, uint32_t interval_ms,
                                   sdlgui_anim_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    return ctx->getGUIManager().getAnimationManager()->addAnimation(interval_ms,
        [cb, userdata]() {
            if (cb) cb(userdata);
        });
}

void sdlgui_remove_loop_animation(sdlgui_t gui, uint32_t anim_id) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().getAnimationManager()->removeAnimation(anim_id);
}

/* ═══════════════════════════════════════════════════════════════════
   AnimationManager — Property tweens
   ═══════════════════════════════════════════════════════════════════ */

static std::function<float(float)> convert_easing(sdlgui_easing_t e) {
    switch (e) {
        case SDLGUI_EASING_LINEAR:      return Easing::linear;
        case SDLGUI_EASING_IN_QUAD:     return Easing::easeInQuad;
        case SDLGUI_EASING_OUT_QUAD:    return Easing::easeOutQuad;
        case SDLGUI_EASING_IN_OUT_QUAD: return Easing::easeInOutQuad;
    }
    return Easing::linear;
}

void sdlgui_animate_int(sdlgui_t gui, int* target_property,
                        int start, int end,
                        uint32_t duration_ms, sdlgui_easing_t easing,
                        sdlgui_anim_callback_t on_complete, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().getAnimationManager()->createAnimation(
        target_property,
        static_cast<float>(start), static_cast<float>(end),
        duration_ms, convert_easing(easing),
        on_complete ? [on_complete, userdata]() { on_complete(userdata); } : Animation::CompleteCallback(nullptr)
    );
}

void sdlgui_animate_float(sdlgui_t gui, float* target_property,
                          float start, float end,
                          uint32_t duration_ms, sdlgui_easing_t easing,
                          sdlgui_anim_callback_t on_complete, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().getAnimationManager()->createAnimation(
        target_property,
        start, end,
        duration_ms, convert_easing(easing),
        on_complete ? [on_complete, userdata]() { on_complete(userdata); } : Animation::CompleteCallback(nullptr)
    );
}

/* ═══════════════════════════════════════════════════════════
   Element base API
   ═══════════════════════════════════════════════════════════ */

void sdlgui_element_set_position(sdlgui_element_t e, int x, int y) {
    unwrap_elem(e)->setPosition(x, y);
}

void sdlgui_element_set_size(sdlgui_element_t e, int w, int h) {
    unwrap_elem(e)->setSize(w, h);
}

void sdlgui_element_set_enabled(sdlgui_element_t e, int enabled) {
    unwrap_elem(e)->setEnabled(enabled != 0);
}

void sdlgui_element_set_visible(sdlgui_element_t e, int visible) {
    unwrap_elem(e)->setVisible(visible != 0);
}

void sdlgui_element_set_background_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color) {
    unwrap_elem(e)->setBackgroundColor(convert_state(state), color);
}

void sdlgui_element_set_text_color(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color) {
    unwrap_elem(e)->setTextColor(convert_state(state), color);
}

void sdlgui_element_set_border(sdlgui_element_t e, sdlgui_element_state_t state, SDL_Color color, int width) {
    unwrap_elem(e)->setBorder(convert_state(state), color, width);
}

void sdlgui_element_set_border_radius(sdlgui_element_t e, sdlgui_element_state_t state, int radius) {
    unwrap_elem(e)->setBorderRadius(convert_state(state), radius);
}

void sdlgui_element_set_tooltip(sdlgui_element_t e, const char* text) {
    unwrap_elem(e)->setTooltip(text ? text : "");
}

void sdlgui_element_set_id(sdlgui_element_t e, const char* id) {
    unwrap_elem(e)->setID(id ? id : "");
}

const char* sdlgui_element_get_id(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    auto id = elem->getID();
    if (id.empty()) return "";
    return id.data();
}

const char* sdlgui_element_get_type(sdlgui_element_t e) {
    return unwrap_elem(e)->getComponentType();
}

void sdlgui_element_set_anchor(sdlgui_element_t e, sdlgui_anchor_t anchor) {
    unwrap_elem(e)->setAnchor(convert_anchor(&anchor));
}

void sdlgui_element_set_rotation(sdlgui_element_t e, double angle_degrees) {
    unwrap_elem(e)->setRotation(angle_degrees);
}

void sdlgui_element_mark_for_deletion(sdlgui_element_t e) {
    unwrap_elem(e)->markForDeletion();
}

void sdlgui_element_set_can_get_keyboard_focus(sdlgui_element_t e, int can_focus) {
    unwrap_elem(e)->setCanGetKeyboardFocus(can_focus != 0);
}

int sdlgui_element_add_child(sdlgui_element_t parent, sdlgui_element_t child) {
    auto* p = unwrap_elem(parent);
    auto* c = unwrap_elem(child);
    auto& mgr = p->getManager();
    auto detached = mgr.detachElement(c);
    if (detached) {
        p->addChild(std::move(detached));
        return 0;
    }
    return -1;
}

/* ═══════════════════════════════════════════════════════════
   Anchor factories
   ═══════════════════════════════════════════════════════════ */

sdlgui_anchor_t sdlgui_anchor_none(void) {
    auto a = Anchor::none();
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_top_left(float margin) {
    auto a = Anchor::topLeft(margin);
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_top_right(float margin) {
    auto a = Anchor::topRight(margin);
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_bottom_left(float margin) {
    auto a = Anchor::bottomLeft(margin);
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_bottom_right(float margin) {
    auto a = Anchor::bottomRight(margin);
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_center(void) {
    auto a = Anchor::center();
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_fill(int padding) {
    auto a = Anchor::fill(static_cast<float>(padding));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_horizontal_stretch(int pad_left, int pad_right) {
    auto a = Anchor::horizontalStretch(static_cast<float>(pad_left), static_cast<float>(pad_right));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_vertical_stretch(int pad_top, int pad_bottom) {
    auto a = Anchor::verticalStretch(static_cast<float>(pad_top), static_cast<float>(pad_bottom));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_top_bar(int height, int pad_vert, int pad_horiz) {
    (void)pad_vert;
    auto a = Anchor::topBar(static_cast<float>(height), static_cast<float>(pad_horiz), static_cast<float>(pad_horiz));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_bottom_bar(int height, int pad_vert, int pad_horiz) {
    (void)pad_vert;
    auto a = Anchor::bottomBar(static_cast<float>(height), static_cast<float>(pad_horiz), static_cast<float>(pad_horiz));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_left_sidebar(int width, int pad_top, int pad_bottom) {
    auto a = Anchor::leftSidebar(static_cast<float>(width), static_cast<float>(pad_top), static_cast<float>(pad_bottom));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_right_sidebar(int width, int pad_top, int pad_bottom) {
    auto a = Anchor::rightSidebar(static_cast<float>(width), static_cast<float>(pad_top), static_cast<float>(pad_bottom));
    return {a.left, a.top, a.right, a.bottom};
}

sdlgui_anchor_t sdlgui_anchor_raw(float left, float top, float right, float bottom) {
    return {left, top, right, bottom};
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: Button
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_button_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h, const char* label) {
    auto* ctx = unwrap_ctx(gui);
    auto btn = std::make_unique<Button>(ctx->getGUIManager(), x, y, w, h, label ? label : "");
    return add_element(ctx, parent, std::move(btn));
}

void sdlgui_button_set_on_click(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* btn = static_cast<Button*>(unwrap_elem(e));
    btn->setOnClickCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_button_set_on_mouse_over(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* btn = static_cast<Button*>(unwrap_elem(e));
    btn->setOnMouseOverCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_button_set_label(sdlgui_element_t e, const char* label) {
    auto* btn = static_cast<Button*>(unwrap_elem(e));
    Label* lbl = nullptr;
    for (auto& child : btn->getChildren()) {
        if (std::strcmp(child->getComponentType(), "Label") == 0) {
            lbl = static_cast<Label*>(child.get());
            break;
        }
    }
    if (lbl) {
        lbl->setText(label ? label : "");
    }
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: Label
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_label_create(sdlgui_t gui, sdlgui_element_t parent,
                                      int x, int y, const char* text, int font_size) {
    auto* ctx = unwrap_ctx(gui);
    auto lbl = std::make_unique<Label>(ctx->getGUIManager(), x, y, text ? text : "", font_size);
    return add_element(ctx, parent, std::move(lbl));
}

void sdlgui_label_set_text(sdlgui_element_t e, const char* text) {
    static_cast<Label*>(unwrap_elem(e))->setText(text ? text : "");
}

const char* sdlgui_label_get_text(sdlgui_element_t e) {
    return static_cast<Label*>(unwrap_elem(e))->getText().c_str();
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: Panel
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_panel_create(sdlgui_t gui, sdlgui_element_t parent,
                                      int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto pnl = std::make_unique<Panel>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(pnl));
}

void sdlgui_panel_set_draggable(sdlgui_element_t e, int draggable) {
    static_cast<Panel*>(unwrap_elem(e))->setDraggable(draggable != 0);
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: Slider
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_slider_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h,
                                       int min_val, int max_val, int initial_val,
                                       sdlgui_orientation_t orientation) {
    auto* ctx = unwrap_ctx(gui);
    Orientation orient = (orientation == SDLGUI_ORIENTATION_VERTICAL)
                         ? Orientation::Vertical
                         : Orientation::Horizontal;
    auto sld = std::make_unique<Slider>(ctx->getGUIManager(), x, y, w, h,
                                        min_val, max_val, initial_val, orient);
    return add_element(ctx, parent, std::move(sld));
}

int sdlgui_slider_get_value(sdlgui_element_t e) {
    return static_cast<Slider*>(unwrap_elem(e))->getValue();
}

void sdlgui_slider_set_value(sdlgui_element_t e, int value) {
    static_cast<Slider*>(unwrap_elem(e))->setValue(value);
}

void sdlgui_slider_set_range(sdlgui_element_t e, int min_val, int max_val) {
    static_cast<Slider*>(unwrap_elem(e))->setRange(min_val, max_val);
}

int sdlgui_slider_get_min(sdlgui_element_t e) {
    return static_cast<Slider*>(unwrap_elem(e))->getMin();
}

int sdlgui_slider_get_max(sdlgui_element_t e) {
    return static_cast<Slider*>(unwrap_elem(e))->getMax();
}

void sdlgui_slider_set_min(sdlgui_element_t e, int min) {
    static_cast<Slider*>(unwrap_elem(e))->setMin(min);
}

void sdlgui_slider_set_max(sdlgui_element_t e, int max) {
    static_cast<Slider*>(unwrap_elem(e))->setMax(max);
}

void sdlgui_slider_set_wheel_step(sdlgui_element_t e, int step) {
    static_cast<Slider*>(unwrap_elem(e))->setWheelStep(step);
}

void sdlgui_slider_set_on_change(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* sld = static_cast<Slider*>(unwrap_elem(e));
    sld->setOnChangeCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

int sdlgui_slider_get_orientation(sdlgui_element_t e) {
    return static_cast<Slider*>(unwrap_elem(e))->getOrientation() == Orientation::Vertical ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: Checkbox
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_checkbox_create(sdlgui_t gui, sdlgui_element_t parent,
                                         int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto cb = std::make_unique<Checkbox>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(cb));
}

int sdlgui_checkbox_is_checked(sdlgui_element_t e) {
    return static_cast<Checkbox*>(unwrap_elem(e))->isChecked() ? 1 : 0;
}

void sdlgui_checkbox_set_checked(sdlgui_element_t e, int checked) {
    static_cast<Checkbox*>(unwrap_elem(e))->setChecked(checked != 0);
}

void sdlgui_checkbox_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata) {
    auto* chk = static_cast<Checkbox*>(unwrap_elem(e));
    chk->setOnChange([cb, userdata](Checkbox* element, bool value) {
        if (cb) cb(wrap_elem(element), value ? 1 : 0, userdata);
    });
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: TextInput
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_text_input_create(sdlgui_t gui, sdlgui_element_t parent,
                                           int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto ti = std::make_unique<TextInput>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(ti));
}

void sdlgui_text_input_set_text(sdlgui_element_t e, const char* text) {
    static_cast<TextInput*>(unwrap_elem(e))->setText(text ? text : "");
}

const char* sdlgui_text_input_get_text(sdlgui_element_t e) {
    return static_cast<TextInput*>(unwrap_elem(e))->getText().c_str();
}

void sdlgui_text_input_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ti = static_cast<TextInput*>(unwrap_elem(e));
    ti->setOnTextChanged([cb, userdata](TextInput* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_text_input_set_on_enter_pressed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ti = static_cast<TextInput*>(unwrap_elem(e));
    ti->setOnEnterPressed([cb, userdata](TextInput* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_text_input_set_locked(sdlgui_element_t e, int locked) {
    static_cast<TextInput*>(unwrap_elem(e))->setLocked(locked != 0);
}

int sdlgui_text_input_is_locked(sdlgui_element_t e) {
    return static_cast<TextInput*>(unwrap_elem(e))->isLocked() ? 1 : 0;
}

/* ═══════════════════════════════════════════════════════════
   Phase 1: ListView
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_list_view_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto lv = std::make_unique<ListView>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(lv));
}

void sdlgui_list_view_add_item(sdlgui_element_t e, const char* item) {
    static_cast<ListView*>(unwrap_elem(e))->addItem(item ? item : "");
}

void sdlgui_list_view_remove_item(sdlgui_element_t e, size_t index) {
    static_cast<ListView*>(unwrap_elem(e))->removeItem(index);
}

void sdlgui_list_view_clear(sdlgui_element_t e) {
    static_cast<ListView*>(unwrap_elem(e))->clearItems();
}

size_t sdlgui_list_view_get_item_count(sdlgui_element_t e) {
    return static_cast<ListView*>(unwrap_elem(e))->getItemCount();
}

const char* sdlgui_list_view_get_item_text(sdlgui_element_t e, size_t index) {
    auto sv = static_cast<ListView*>(unwrap_elem(e))->getItem(index);
    return sv.data();
}

void sdlgui_list_view_set_on_row_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata) {
    auto* lv = static_cast<ListView*>(unwrap_elem(e));
    lv->setOnRowClick([cb, userdata](ListView* elem, size_t row) {
        if (cb) cb(wrap_elem(elem), row, userdata);
    });
}

void sdlgui_list_view_set_on_row_double_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata) {
    auto* lv = static_cast<ListView*>(unwrap_elem(e));
    lv->setOnRowDoubleClick([cb, userdata](ListView* elem, size_t row) {
        if (cb) cb(wrap_elem(elem), row, userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ProgressBar
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_progress_bar_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto pb = std::make_unique<ProgressBar>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(pb));
}

float sdlgui_progress_bar_get_value(sdlgui_element_t e) {
    return static_cast<ProgressBar*>(unwrap_elem(e))->getValue();
}

void sdlgui_progress_bar_set_value(sdlgui_element_t e, float value) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setValue(value);
}

float sdlgui_progress_bar_get_min(sdlgui_element_t e) {
    return static_cast<ProgressBar*>(unwrap_elem(e))->getMin();
}

void sdlgui_progress_bar_set_min(sdlgui_element_t e, float min) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setMin(min);
}

float sdlgui_progress_bar_get_max(sdlgui_element_t e) {
    return static_cast<ProgressBar*>(unwrap_elem(e))->getMax();
}

void sdlgui_progress_bar_set_max(sdlgui_element_t e, float max) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setMax(max);
}

void sdlgui_progress_bar_set_range(sdlgui_element_t e, float min, float max) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setRange(min, max);
}

void sdlgui_progress_bar_set_show_text(sdlgui_element_t e, int show) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setShowText(show != 0);
}

void sdlgui_progress_bar_set_text_format(sdlgui_element_t e, const char* format) {
    static_cast<ProgressBar*>(unwrap_elem(e))->setTextFormat(format ? format : "%.0f%%");
}

void sdlgui_progress_bar_set_orientation(sdlgui_element_t e, sdlgui_orientation_t orientation) {
    Orientation orient = (orientation == SDLGUI_ORIENTATION_VERTICAL)
                         ? Orientation::Vertical
                         : Orientation::Horizontal;
    static_cast<ProgressBar*>(unwrap_elem(e))->setOrientation(orient);
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: RadioButton
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_radio_button_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto rb = std::make_unique<RadioButton>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(rb));
}

int sdlgui_radio_button_is_selected(sdlgui_element_t e) {
    return static_cast<RadioButton*>(unwrap_elem(e))->isSelected() ? 1 : 0;
}

void sdlgui_radio_button_set_selected(sdlgui_element_t e, int selected) {
    static_cast<RadioButton*>(unwrap_elem(e))->setSelected(selected != 0);
}

void sdlgui_radio_button_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata) {
    auto* rb = static_cast<RadioButton*>(unwrap_elem(e));
    rb->setOnChange([cb, userdata](RadioButton* elem, bool value) {
        if (cb) cb(wrap_elem(elem), value ? 1 : 0, userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: RadioGroup
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_radio_group_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto rg = std::make_unique<RadioGroup>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(rg));
}

sdlgui_element_t sdlgui_radio_group_add_option(sdlgui_element_t e, const char* text, int selected) {
    auto* rg = static_cast<RadioGroup*>(unwrap_elem(e));
    auto* rb = rg->addOption(text ? text : "", selected != 0);
    return wrap_elem(rb);
}

void sdlgui_radio_group_set_on_selection_change(sdlgui_element_t e,
                                                sdlgui_index_text_callback_t cb, void* userdata) {
    auto* rg = static_cast<RadioGroup*>(unwrap_elem(e));
    rg->setOnSelectionChange([cb, userdata](int index, const std::string& text) {
        if (cb) cb(wrap_elem(nullptr), index, text.c_str(), userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: TextArea
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_text_area_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h,
                                          const char* font_path, int font_size) {
    auto* ctx = unwrap_ctx(gui);
    auto ta = std::make_unique<TextArea>(ctx->getGUIManager(), x, y, w, h,
                                          font_path ? font_path : "",
                                          font_size > 0 ? font_size : 14);
    return add_element(ctx, parent, std::move(ta));
}

void sdlgui_text_area_set_text(sdlgui_element_t e, const char* text) {
    static_cast<TextArea*>(unwrap_elem(e))->setText(text ? text : "");
}

const char* sdlgui_text_area_get_text(sdlgui_element_t e) {
    return static_cast<TextArea*>(unwrap_elem(e))->getText().c_str();
}

void sdlgui_text_area_set_word_wrap(sdlgui_element_t e, int wrap) {
    static_cast<TextArea*>(unwrap_elem(e))->setWordWrap(wrap != 0);
}

int sdlgui_text_area_get_word_wrap(sdlgui_element_t e) {
    return static_cast<TextArea*>(unwrap_elem(e))->getWordWrap() ? 1 : 0;
}

void sdlgui_text_area_set_locked(sdlgui_element_t e, int locked) {
    static_cast<TextArea*>(unwrap_elem(e))->setLocked(locked != 0);
}

int sdlgui_text_area_is_locked(sdlgui_element_t e) {
    return static_cast<TextArea*>(unwrap_elem(e))->isLocked() ? 1 : 0;
}

void sdlgui_text_area_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ta = static_cast<TextArea*>(unwrap_elem(e));
    ta->setOnTextChanged([cb, userdata](TextArea* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ComboBox
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_combo_box_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto cb = std::make_unique<ComboBox>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(cb));
}

void sdlgui_combo_box_add_item(sdlgui_element_t e, const char* item) {
    static_cast<ComboBox*>(unwrap_elem(e))->addItem(item ? item : "");
}

void sdlgui_combo_box_clear(sdlgui_element_t e) {
    static_cast<ComboBox*>(unwrap_elem(e))->clearItems();
}

size_t sdlgui_combo_box_get_item_count(sdlgui_element_t e) {
    return static_cast<ComboBox*>(unwrap_elem(e))->getItemCount();
}

const char* sdlgui_combo_box_get_item_text(sdlgui_element_t e, size_t index) {
    auto* cb = static_cast<ComboBox*>(unwrap_elem(e));
    auto item = cb->getItem(index);  /* returns by value — c_str() safe until next set/get on same cb */
    /* Note: returning stale .c_str() from temporary — caller must use immediately */
    static thread_local std::string cached;
    cached = item;
    return cached.c_str();
}

int sdlgui_combo_box_get_selected_index(sdlgui_element_t e) {
    return static_cast<ComboBox*>(unwrap_elem(e))->getSelectedIndex();
}

void sdlgui_combo_box_set_selected_index(sdlgui_element_t e, int index) {
    static_cast<ComboBox*>(unwrap_elem(e))->setSelectedIndex(index);
}

void sdlgui_combo_box_set_on_select(sdlgui_element_t e,
                                    sdlgui_index_text_callback_t cb, void* userdata) {
    auto* combo = static_cast<ComboBox*>(unwrap_elem(e));
    combo->on_selection_changed = [cb, userdata](int index, const std::string& text) {
        if (cb) cb(wrap_elem(nullptr), index, text.c_str(), userdata);
    };
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: StringGrid
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_string_grid_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h,
                                            size_t rows, size_t cols) {
    auto* ctx = unwrap_ctx(gui);
    auto sg = std::make_unique<StringGrid>(ctx->getGUIManager(), x, y, w, h, rows, cols);
    return add_element(ctx, parent, std::move(sg));
}

void sdlgui_string_grid_set_dimensions(sdlgui_element_t e, size_t rows, size_t cols) {
    auto* sg = static_cast<StringGrid*>(unwrap_elem(e));
    sg->setRowCount(rows);
    sg->setColumnCount(cols);
}

void sdlgui_string_grid_set_cell(sdlgui_element_t e, size_t row, size_t col, const char* text) {
    static_cast<StringGrid*>(unwrap_elem(e))->setCellText(row, col, text ? text : "");
}

const char* sdlgui_string_grid_get_cell(sdlgui_element_t e, size_t row, size_t col) {
    return static_cast<StringGrid*>(unwrap_elem(e))->getCellText(row, col).data();
}

size_t sdlgui_string_grid_get_row_count(sdlgui_element_t e) {
    return static_cast<StringGrid*>(unwrap_elem(e))->getRowCount();
}

size_t sdlgui_string_grid_get_col_count(sdlgui_element_t e) {
    return static_cast<StringGrid*>(unwrap_elem(e))->getColumnCount();
}

void sdlgui_string_grid_clear(sdlgui_element_t e) {
    static_cast<StringGrid*>(unwrap_elem(e))->clear();
}

void sdlgui_string_grid_set_selected_cell(sdlgui_element_t e, size_t row, size_t col) {
    static_cast<StringGrid*>(unwrap_elem(e))->setSelectedCell(row, col);
}

void sdlgui_string_grid_set_editable(sdlgui_element_t e, int editable) {
    static_cast<StringGrid*>(unwrap_elem(e))->setEditable(editable != 0);
}

int sdlgui_string_grid_is_editable(sdlgui_element_t e) {
    return static_cast<StringGrid*>(unwrap_elem(e))->isEditable() ? 1 : 0;
}

void sdlgui_string_grid_set_on_cell_click(sdlgui_element_t e,
                                          sdlgui_cell_callback_t cb, void* userdata) {
    auto* sg = static_cast<StringGrid*>(unwrap_elem(e));
    sg->setOnCellClick([cb, userdata](StringGrid* elem, CellCoord cell) {
        if (cb) cb(wrap_elem(elem), cell.row, cell.col, userdata);
    });
}

void sdlgui_string_grid_set_on_cell_double_click(sdlgui_element_t e,
                                                  sdlgui_cell_callback_t cb, void* userdata) {
    auto* sg = static_cast<StringGrid*>(unwrap_elem(e));
    sg->setOnCellDoubleClick([cb, userdata](StringGrid* elem, CellCoord cell) {
        if (cb) cb(wrap_elem(elem), cell.row, cell.col, userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ScrollArea
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_scroll_area_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto sa = std::make_unique<ScrollArea>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(sa));
}

void sdlgui_scroll_area_set_content(sdlgui_element_t e, sdlgui_element_t content) {
    auto* sa = static_cast<ScrollArea*>(unwrap_elem(e));
    auto* elem = unwrap_elem(content);
    auto& mgr = sa->getManager();
    auto detached = mgr.detachElement(elem);
    if (detached) {
        sa->setContent(std::move(detached));
    }
}

void sdlgui_scroll_area_set_content_size(sdlgui_element_t e, int w, int h) {
    static_cast<ScrollArea*>(unwrap_elem(e))->setContentSize(w, h);
}

void sdlgui_scroll_area_get_scroll_offset(sdlgui_element_t e, int* x, int* y) {
    auto* sa = static_cast<ScrollArea*>(unwrap_elem(e));
    if (x) *x = sa->getScrollOffsetX();
    if (y) *y = sa->getScrollOffsetY();
}

void sdlgui_scroll_area_set_scroll_enabled(sdlgui_element_t e, int vertical, int horizontal) {
    static_cast<ScrollArea*>(unwrap_elem(e))->setScrollEnabled(vertical != 0, horizontal != 0);
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: AnimatedImage
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_animated_image_create(sdlgui_t gui, sdlgui_element_t parent,
                                               int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto ai = std::make_unique<AnimatedImage>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(ai));
}

void sdlgui_animated_image_set_sprite_sheet(sdlgui_element_t e,
                                            const char* path, int total_frames,
                                            int rows, int frame_w, int frame_h) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->setSpriteSheet(
        path ? path : "", total_frames, rows, frame_w, frame_h);
}

void sdlgui_animated_image_set_fps(sdlgui_element_t e, float fps) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->setFPS(fps);
}

void sdlgui_animated_image_set_loop(sdlgui_element_t e, int loop) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->setLoop(loop != 0);
}

void sdlgui_animated_image_play(sdlgui_element_t e) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->play();
}

void sdlgui_animated_image_pause(sdlgui_element_t e) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->pause();
}

void sdlgui_animated_image_stop(sdlgui_element_t e) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->stop();
}

int sdlgui_animated_image_get_current_frame(sdlgui_element_t e) {
    return static_cast<AnimatedImage*>(unwrap_elem(e))->getCurrentFrame();
}

int sdlgui_animated_image_get_total_frames(sdlgui_element_t e) {
    return static_cast<AnimatedImage*>(unwrap_elem(e))->getTotalFrames();
}

int sdlgui_animated_image_is_playing(sdlgui_element_t e) {
    return static_cast<AnimatedImage*>(unwrap_elem(e))->isPlaying() ? 1 : 0;
}

void sdlgui_animated_image_set_frame(sdlgui_element_t e, int frame_index) {
    static_cast<AnimatedImage*>(unwrap_elem(e))->setFrame(frame_index);
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: Canvas
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_canvas_create(sdlgui_t gui, sdlgui_element_t parent,
                                       int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto cv = std::make_unique<Canvas>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(cv));
}

void sdlgui_canvas_clear(sdlgui_element_t e) {
    static_cast<Canvas*>(unwrap_elem(e))->clear();
}

void sdlgui_canvas_set_pen_color(sdlgui_element_t e, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    static_cast<Canvas*>(unwrap_elem(e))->setPenColor({r, g, b, a});
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ArcContainer
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_arc_container_create(sdlgui_t gui, sdlgui_element_t parent,
                                              int centerX, int centerY, int radius,
                                              float start_angle_deg, float end_angle_deg) {
    auto* ctx = unwrap_ctx(gui);
    auto ac = std::make_unique<ArcContainer>(ctx->getGUIManager(), centerX, centerY, radius,
                                              start_angle_deg, end_angle_deg);
    return add_element(ctx, parent, std::move(ac));
}

void sdlgui_arc_container_add_child_at_angle(sdlgui_element_t e,
                                             sdlgui_element_t child,
                                             float angle_deg,
                                             int rotate_child, int offset) {
    auto* ac = static_cast<ArcContainer*>(unwrap_elem(e));
    auto* c = unwrap_elem(child);
    auto& mgr = ac->getManager();
    auto detached = mgr.detachElement(c);
    if (detached) {
        ac->addChildAtAngle(std::move(detached), angle_deg, rotate_child != 0, offset);
    }
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: TabControl
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_tab_control_create(sdlgui_t gui, sdlgui_element_t parent,
                                            int x, int y, int w, int h,
                                            int tab_button_height) {
    auto* ctx = unwrap_ctx(gui);
    auto tc = std::make_unique<TabControl>(ctx->getGUIManager(), x, y, w, h, tab_button_height);
    return add_element(ctx, parent, std::move(tc));
}

sdlgui_element_t sdlgui_tab_control_add_tab(sdlgui_element_t e, const char* title) {
    auto* tc = static_cast<TabControl*>(unwrap_elem(e));
    auto* panel = tc->addTab(title ? title : "");
    return wrap_elem(panel);
}

void sdlgui_tab_control_set_active_tab(sdlgui_element_t e, int index) {
    static_cast<TabControl*>(unwrap_elem(e))->setActiveTab(index);
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 2: ContextMenu
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_context_menu_create(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    auto cm = std::make_unique<ContextMenu>(ctx->getGUIManager());
    return add_element(ctx, nullptr, std::move(cm));
}

void sdlgui_context_menu_add_item(sdlgui_element_t e,
                                  const char* text,
                                  sdlgui_context_menu_callback_t cb,
                                  void* userdata) {
    auto* cm = static_cast<ContextMenu*>(unwrap_elem(e));
    cm->addItem(text ? text : "", [cb, userdata]() {
        if (cb) cb(userdata);
    });
}

void sdlgui_context_menu_add_separator(sdlgui_element_t e) {
    static_cast<ContextMenu*>(unwrap_elem(e))->addSeparator();
}

void sdlgui_context_menu_clear_items(sdlgui_element_t e) {
    static_cast<ContextMenu*>(unwrap_elem(e))->clearItems();
}

void sdlgui_context_menu_show_at(sdlgui_element_t e, int x, int y) {
    static_cast<ContextMenu*>(unwrap_elem(e))->showAt(x, y);
}

void sdlgui_context_menu_hide(sdlgui_element_t e) {
    static_cast<ContextMenu*>(unwrap_elem(e))->hide();
}

int sdlgui_context_menu_is_visible(sdlgui_element_t e) {
    return static_cast<ContextMenu*>(unwrap_elem(e))->isVisible() ? 1 : 0;
}

} // extern "C"
