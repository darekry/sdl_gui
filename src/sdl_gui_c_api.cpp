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
#include "range_slider.hpp"
#include "cursor.hpp"
#include "shader_panel.hpp"
#include "widget_factory.hpp"

#include "sdl_gui.h"

#include <cassert>
#include <cstring>

/* ═══════════════════════════════════════════════════════════
   Internal context
   ═══════════════════════════════════════════════════════════ */

using CContext = GUIContext;

static CContext* unwrap_ctx(sdlgui_t gui) { return static_cast<CContext*>(gui); }
static GUIElement* unwrap_elem(sdlgui_element_t e) { return static_cast<GUIElement*>(e); }
static sdlgui_element_t wrap_elem(GUIElement* e) { return e; }

/* ═══════════════════════════════════════════════════════════
   Handle validation (point 5): type-tagged checked casts + errors
   ═══════════════════════════════════════════════════════════ */

namespace {

thread_local std::string g_lastError;

void set_c_error(const std::string& msg) { g_lastError = msg; }

// Checked downcast: null handle or wrong widget type -> nullptr + recorded
// error. dynamic_cast (not type-id equality) so documented upcasts keep
// working — e.g. panel API on a Slider (Slider inherits Panel).
// Replaces bare static_cast<T*> which was UB on type mismatch.
template<typename T>
T* checked_elem(sdlgui_element_t e, ComponentType expected, const char* fn) {
    GUIElement* base = unwrap_elem(e);
    if (!base) {
        set_c_error(std::string(fn) + ": null element handle");
        return nullptr;
    }
    T* typed = dynamic_cast<T*>(base);
    if (!typed) {
        set_c_error(std::string(fn) + ": type mismatch (handle is " +
                    std::string(componentTypeToString(expected)) + "-incompatible " +
                    std::string(componentTypeToString(base->getComponentTypeId())) + ")");
        return nullptr;
    }
    g_lastError.clear();
    return typed;
}

// Copies an internal string into the caller buffer (no dangling c_str()).
// Returns bytes written excluding NUL; truncates safely when too small.
size_t copy_c_string(const std::string& src, char* buf, size_t len) {
    if (!buf || len == 0) return 0;
    size_t n = std::min(src.size(), len - 1);
    if (n > 0) std::memcpy(buf, src.data(), n);
    buf[n] = '\0';
    return n;
}

}  // namespace

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
    auto convH = [](int h) {
        switch (h) {
            case SDLGUI_H_LEFT: return HAnchor::Left;
            case SDLGUI_H_CENTER: return HAnchor::Center;
            case SDLGUI_H_RIGHT: return HAnchor::Right;
            case SDLGUI_H_STRETCH: return HAnchor::Stretch;
            default: return HAnchor::None;
        }
    };
    auto convV = [](int v) {
        switch (v) {
            case SDLGUI_V_TOP: return VAnchor::Top;
            case SDLGUI_V_CENTER: return VAnchor::Center;
            case SDLGUI_V_BOTTOM: return VAnchor::Bottom;
            case SDLGUI_V_STRETCH: return VAnchor::Stretch;
            default: return VAnchor::None;
        }
    };
    return Anchor::pinned(convH(a->h), convV(a->v), a->left, a->top, a->right, a->bottom);
}

static sdlgui_anchor_t wrap_anchor(const Anchor& a) {
    return {static_cast<int>(a.h), static_cast<int>(a.v), a.left, a.top, a.right, a.bottom};
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

sdlgui_t sdlgui_create_gpu(const char* title, int width, int height, int resizable) {
    try {
        auto* ctx = new CContext(title, width, height, resizable != 0, GPU_VULKAN);
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

SDL_GPUDevice* sdlgui_get_gpu_device(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    return ctx->getGPUDevice();
}

/* ═══════════════════════════════════════════════════════════
   Core loop API
   ═══════════════════════════════════════════════════════════ */

SDL_Renderer* sdlgui_get_renderer(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    return ctx->getRenderer();
}

SDL_Window* sdlgui_get_window(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    return ctx->getWindow();
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
    // Boundary conversion: ID -> string for C callers.
    return componentTypeToString(unwrap_elem(e)->getComponentTypeId()).data();
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
    return wrap_anchor(Anchor::none());
}

sdlgui_anchor_t sdlgui_anchor_top_left(int margin) {
    return wrap_anchor(Anchor::topLeft(margin));
}

sdlgui_anchor_t sdlgui_anchor_top_right(int margin) {
    return wrap_anchor(Anchor::topRight(margin));
}

sdlgui_anchor_t sdlgui_anchor_bottom_left(int margin) {
    return wrap_anchor(Anchor::bottomLeft(margin));
}

sdlgui_anchor_t sdlgui_anchor_bottom_right(int margin) {
    return wrap_anchor(Anchor::bottomRight(margin));
}

sdlgui_anchor_t sdlgui_anchor_center(void) {
    return wrap_anchor(Anchor::center());
}

sdlgui_anchor_t sdlgui_anchor_fill(int padding) {
    return wrap_anchor(Anchor::fill(padding));
}

sdlgui_anchor_t sdlgui_anchor_horizontal_stretch(int pad_left, int pad_right) {
    return wrap_anchor(Anchor::horizontalStretch(pad_left, pad_right));
}

sdlgui_anchor_t sdlgui_anchor_vertical_stretch(int pad_top, int pad_bottom) {
    return wrap_anchor(Anchor::verticalStretch(pad_top, pad_bottom));
}

sdlgui_anchor_t sdlgui_anchor_top_bar(int height, int pad_horiz) {
    return wrap_anchor(Anchor::topBar(height, pad_horiz, pad_horiz));
}

sdlgui_anchor_t sdlgui_anchor_bottom_bar(int height, int pad_horiz) {
    return wrap_anchor(Anchor::bottomBar(height, pad_horiz, pad_horiz));
}

sdlgui_anchor_t sdlgui_anchor_left_sidebar(int pad_top, int pad_bottom) {
    return wrap_anchor(Anchor::leftSidebar(pad_top, pad_bottom));
}

sdlgui_anchor_t sdlgui_anchor_right_sidebar(int pad_top, int pad_bottom) {
    return wrap_anchor(Anchor::rightSidebar(pad_top, pad_bottom));
}

sdlgui_anchor_t sdlgui_anchor_make(int h, int v, int left, int top, int right, int bottom) {
    return {h, v, left, top, right, bottom};
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
    auto* btn = checked_elem<Button>(e, ComponentType::Button, __func__);
    if (!btn) return;
    btn->setOnClickCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_button_set_on_mouse_over(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* btn = checked_elem<Button>(e, ComponentType::Button, __func__);
    if (!btn) return;
    btn->setOnMouseOverCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_button_set_label(sdlgui_element_t e, const char* label) {
    auto* btn = checked_elem<Button>(e, ComponentType::Button, __func__);
    if (!btn) return;
    Label* lbl = nullptr;
    for (auto& child : btn->getChildren()) {
        if (child->getComponentTypeId() == ComponentType::Label) {
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
    auto* lbl = checked_elem<Label>(e, ComponentType::Label, __func__);
    if (!lbl) return;
    lbl->setText(text ? text : "");
}

const char* sdlgui_label_get_text(sdlgui_element_t e) {
    // Legacy: pointer to internal storage, valid until next set_text() on
    // the same element. Prefer sdlgui_label_get_text_buf() (caller buffer).
    auto* lbl = checked_elem<Label>(e, ComponentType::Label, __func__);
    if (!lbl) return "";
    return lbl->getText().c_str();
}

size_t sdlgui_label_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* lbl = checked_elem<Label>(e, ComponentType::Label, __func__);
    if (!lbl) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(std::string(lbl->getText()), buf, buf_len);
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
    auto* pnl = checked_elem<Panel>(e, ComponentType::Panel, __func__);
    if (!pnl) return;
    pnl->setDraggable(draggable != 0);
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
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    return s ? s->getValue() : 0;
}

void sdlgui_slider_set_value(sdlgui_element_t e, int value) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return;
    s->setValue(value);
}

void sdlgui_slider_set_range(sdlgui_element_t e, int min_val, int max_val) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return;
    s->setRange(min_val, max_val);
}

int sdlgui_slider_get_min(sdlgui_element_t e) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    return s ? s->getMin() : 0;
}

int sdlgui_slider_get_max(sdlgui_element_t e) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    return s ? s->getMax() : 0;
}

void sdlgui_slider_set_min(sdlgui_element_t e, int min) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return;
    s->setMin(min);
}

void sdlgui_slider_set_max(sdlgui_element_t e, int max) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return;
    s->setMax(max);
}

void sdlgui_slider_set_wheel_step(sdlgui_element_t e, int step) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return;
    s->setWheelStep(step);
}

void sdlgui_slider_set_on_change(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* sld = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!sld) return;
    sld->setOnChangeCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

int sdlgui_slider_get_orientation(sdlgui_element_t e) {
    auto* s = checked_elem<Slider>(e, ComponentType::Slider, __func__);
    if (!s) return 0;
    return s->getOrientation() == Orientation::Vertical ? 1 : 0;
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
    auto* c = checked_elem<Checkbox>(e, ComponentType::Checkbox, __func__);
    return (c && c->isChecked()) ? 1 : 0;
}

void sdlgui_checkbox_set_checked(sdlgui_element_t e, int checked) {
    auto* c = checked_elem<Checkbox>(e, ComponentType::Checkbox, __func__);
    if (!c) return;
    c->setChecked(checked != 0);
}

void sdlgui_checkbox_set_on_change(sdlgui_element_t e, sdlgui_bool_callback_t cb, void* userdata) {
    auto* chk = checked_elem<Checkbox>(e, ComponentType::Checkbox, __func__);
    if (!chk) return;
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
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) return;
    ti->setText(text ? text : "");
}

const char* sdlgui_text_input_get_text(sdlgui_element_t e) {
    // Legacy: pointer to internal storage. Prefer _buf variant.
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) return "";
    return ti->getText().c_str();
}

size_t sdlgui_text_input_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(std::string(ti->getText()), buf, buf_len);
}

void sdlgui_text_input_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) return;
    ti->setOnTextChanged([cb, userdata](TextInput* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_text_input_set_on_enter_pressed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) return;
    ti->setOnEnterPressed([cb, userdata](TextInput* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

void sdlgui_text_input_set_locked(sdlgui_element_t e, int locked) {
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!ti) return;
    ti->setLocked(locked != 0);
}

int sdlgui_text_input_is_locked(sdlgui_element_t e) {
    auto* ti = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    return (ti && ti->isLocked()) ? 1 : 0;
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
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return;
    lv->addItem(item ? item : "");
}

void sdlgui_list_view_remove_item(sdlgui_element_t e, size_t index) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return;
    lv->removeItem(index);
}

void sdlgui_list_view_clear(sdlgui_element_t e) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return;
    lv->clearItems();
}

size_t sdlgui_list_view_get_item_count(sdlgui_element_t e) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    return lv ? lv->getItemCount() : 0;
}

const char* sdlgui_list_view_get_item_text(sdlgui_element_t e, size_t index) {
    // Legacy: pointer to internal storage. Prefer _buf variant.
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return "";
    if (index >= lv->getItemCount()) {
        set_c_error(std::string(__func__) + ": item index out of range");
        return "";
    }
    auto sv = lv->getItem(index);
    return sv.data();
}

size_t sdlgui_list_view_get_item_text_buf(sdlgui_element_t e, size_t index, char* buf, size_t buf_len) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    if (index >= lv->getItemCount()) {
        set_c_error(std::string(__func__) + ": item index out of range");
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(std::string(lv->getItem(index)), buf, buf_len);
}

void sdlgui_list_view_set_on_row_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return;
    lv->setOnRowClick([cb, userdata](ListView* elem, size_t row) {
        if (cb) cb(wrap_elem(elem), row, userdata);
    });
}

void sdlgui_list_view_set_on_row_double_click(sdlgui_element_t e, sdlgui_size_callback_t cb, void* userdata) {
    auto* lv = checked_elem<ListView>(e, ComponentType::ListView, __func__);
    if (!lv) return;
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
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) return;
    ta->setText(text ? text : "");
}

const char* sdlgui_text_area_get_text(sdlgui_element_t e) {
    // Legacy: pointer to internal storage. Prefer _buf variant.
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) return "";
    return ta->getText().c_str();
}

size_t sdlgui_text_area_get_text_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(std::string(ta->getText()), buf, buf_len);
}

void sdlgui_text_area_set_word_wrap(sdlgui_element_t e, int wrap) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) return;
    ta->setWordWrap(wrap != 0);
}

int sdlgui_text_area_get_word_wrap(sdlgui_element_t e) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    return (ta && ta->getWordWrap()) ? 1 : 0;
}

void sdlgui_text_area_set_locked(sdlgui_element_t e, int locked) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) return;
    ta->setLocked(locked != 0);
}

int sdlgui_text_area_is_locked(sdlgui_element_t e) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    return (ta && ta->isLocked()) ? 1 : 0;
}

void sdlgui_text_area_set_on_text_changed(sdlgui_element_t e, sdlgui_callback_t cb, void* userdata) {
    auto* ta = checked_elem<TextArea>(e, ComponentType::TextArea, __func__);
    if (!ta) return;
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
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!cb) return;
    cb->addItem(item ? item : "");
}

void sdlgui_combo_box_clear(sdlgui_element_t e) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!cb) return;
    cb->clearItems();
}

size_t sdlgui_combo_box_get_item_count(sdlgui_element_t e) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    return cb ? cb->getItemCount() : 0;
}

const char* sdlgui_combo_box_get_item_text(sdlgui_element_t e, size_t index) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!cb) return "";
    if (index >= cb->getItemCount()) {
        set_c_error(std::string(__func__) + ": item index out of range");
        return "";
    }
    auto item = cb->getItem(index);  /* returns by value — c_str() safe until next set/get on same cb */
    /* Note: returning stale .c_str() from temporary — caller must use immediately */
    static thread_local std::string cached;
    cached = item;
    return cached.c_str();
}

size_t sdlgui_combo_box_get_item_text_buf(sdlgui_element_t e, size_t index, char* buf, size_t buf_len) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!cb) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    if (index >= cb->getItemCount()) {
        set_c_error(std::string(__func__) + ": item index out of range");
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(std::string(cb->getItem(index)), buf, buf_len);
}

int sdlgui_combo_box_get_selected_index(sdlgui_element_t e) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    return cb ? cb->getSelectedIndex() : -1;
}

void sdlgui_combo_box_set_selected_index(sdlgui_element_t e, int index) {
    auto* cb = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!cb) return;
    cb->setSelectedIndex(index);
}

void sdlgui_combo_box_set_on_select(sdlgui_element_t e,
                                    sdlgui_index_text_callback_t cb, void* userdata) {
    auto* combo = checked_elem<ComboBox>(e, ComponentType::ComboBox, __func__);
    if (!combo) return;
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

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: RangeSlider
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_range_slider_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h,
                                             int min_val, int max_val,
                                             int lower_val, int upper_val,
                                             sdlgui_orientation_t orientation) {
    auto* ctx = unwrap_ctx(gui);
    Orientation orient = (orientation == SDLGUI_ORIENTATION_VERTICAL)
                         ? Orientation::Vertical
                         : Orientation::Horizontal;
    auto rs = std::make_unique<RangeSlider>(ctx->getGUIManager(), x, y, w, h,
                                            min_val, max_val, lower_val, upper_val, orient);
    return add_element(ctx, parent, std::move(rs));
}

int sdlgui_range_slider_get_lower_value(sdlgui_element_t e) {
    return static_cast<RangeSlider*>(unwrap_elem(e))->getLowerValue();
}

void sdlgui_range_slider_set_lower_value(sdlgui_element_t e, int value) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setLowerValue(value);
}

int sdlgui_range_slider_get_upper_value(sdlgui_element_t e) {
    return static_cast<RangeSlider*>(unwrap_elem(e))->getUpperValue();
}

void sdlgui_range_slider_set_upper_value(sdlgui_element_t e, int value) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setUpperValue(value);
}

void sdlgui_range_slider_set_range(sdlgui_element_t e, int min_val, int max_val) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setRange(min_val, max_val);
}

int sdlgui_range_slider_get_min(sdlgui_element_t e) {
    return static_cast<RangeSlider*>(unwrap_elem(e))->getMin();
}

int sdlgui_range_slider_get_max(sdlgui_element_t e) {
    return static_cast<RangeSlider*>(unwrap_elem(e))->getMax();
}

void sdlgui_range_slider_set_min(sdlgui_element_t e, int min) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setMin(min);
}

void sdlgui_range_slider_set_max(sdlgui_element_t e, int max) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setMax(max);
}

void sdlgui_range_slider_set_wheel_step(sdlgui_element_t e, int step) {
    static_cast<RangeSlider*>(unwrap_elem(e))->setWheelStep(step);
}

int sdlgui_range_slider_get_wheel_step(sdlgui_element_t e) {
    return static_cast<RangeSlider*>(unwrap_elem(e))->getWheelStep();
}

void sdlgui_range_slider_set_on_change(sdlgui_element_t e,
                                       sdlgui_callback_t cb, void* userdata) {
    auto* rs = static_cast<RangeSlider*>(unwrap_elem(e));
    rs->setOnChangeCallback([cb, userdata](GUIElement* elem) {
        if (cb) cb(wrap_elem(elem), userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: Cursor
   ═══════════════════════════════════════════════════════════════════ */

static CursorState convert_cursor_state(sdlgui_cursor_state_t s) {
    switch (s) {
        case SDLGUI_CURSOR_NORMAL:   return CursorState::Normal;
        case SDLGUI_CURSOR_HOVER:    return CursorState::Hover;
        case SDLGUI_CURSOR_PRESSED:  return CursorState::Pressed;
        case SDLGUI_CURSOR_DISABLED: return CursorState::Disabled;
        case SDLGUI_CURSOR_BUSY:     return CursorState::Busy;
        case SDLGUI_CURSOR_TEXT:     return CursorState::Text;
        case SDLGUI_CURSOR_CUSTOM1:  return CursorState::Custom1;
        case SDLGUI_CURSOR_CUSTOM2:  return CursorState::Custom2;
        case SDLGUI_CURSOR_CUSTOM3:  return CursorState::Custom3;
    }
    return CursorState::Normal;
}

static sdlgui_cursor_state_t convert_cursor_state_back(CursorState s) {
    switch (s) {
        case CursorState::Normal:   return SDLGUI_CURSOR_NORMAL;
        case CursorState::Hover:    return SDLGUI_CURSOR_HOVER;
        case CursorState::Pressed:  return SDLGUI_CURSOR_PRESSED;
        case CursorState::Disabled: return SDLGUI_CURSOR_DISABLED;
        case CursorState::Busy:     return SDLGUI_CURSOR_BUSY;
        case CursorState::Text:     return SDLGUI_CURSOR_TEXT;
        case CursorState::Custom1:  return SDLGUI_CURSOR_CUSTOM1;
        case CursorState::Custom2:  return SDLGUI_CURSOR_CUSTOM2;
        case CursorState::Custom3:  return SDLGUI_CURSOR_CUSTOM3;
    }
    return SDLGUI_CURSOR_NORMAL;
}

sdlgui_element_t sdlgui_cursor_create(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    auto cursor = std::make_unique<Cursor>(ctx->getGUIManager());
    Cursor* raw = cursor.get();
    ctx->getGUIManager().setCursor(std::move(cursor));
    return wrap_elem(raw);
}

void sdlgui_cursor_set_texture(sdlgui_element_t e, sdlgui_cursor_state_t state,
                               const char* path, int hotspot_x, int hotspot_y) {
    static_cast<Cursor*>(unwrap_elem(e))->setCursorTexture(
        convert_cursor_state(state), path ? path : "", hotspot_x, hotspot_y);
}

void sdlgui_cursor_set_animated_texture(sdlgui_element_t e, sdlgui_cursor_state_t state,
                                        const char* path, int total_frames, int rows,
                                        float fps, int hotspot_x, int hotspot_y) {
    static_cast<Cursor*>(unwrap_elem(e))->setAnimatedCursor(
        convert_cursor_state(state), path ? path : "", total_frames, rows, fps, hotspot_x, hotspot_y);
}

void sdlgui_cursor_set_state(sdlgui_element_t e, sdlgui_cursor_state_t state) {
    static_cast<Cursor*>(unwrap_elem(e))->setState(convert_cursor_state(state));
}

int sdlgui_cursor_get_state(sdlgui_element_t e) {
    return static_cast<int>(convert_cursor_state_back(static_cast<Cursor*>(unwrap_elem(e))->getState()));
}

void sdlgui_cursor_set_offset(sdlgui_element_t e, int offset_x, int offset_y) {
    static_cast<Cursor*>(unwrap_elem(e))->setOffset(offset_x, offset_y);
}

void sdlgui_cursor_get_offset(sdlgui_element_t e, int* offset_x, int* offset_y) {
    int x = 0, y = 0;
    static_cast<Cursor*>(unwrap_elem(e))->getOffset(x, y);
    if (offset_x) *offset_x = x;
    if (offset_y) *offset_y = y;
}

void sdlgui_cursor_set_scale(sdlgui_element_t e, float scale) {
    static_cast<Cursor*>(unwrap_elem(e))->setScale(scale);
}

float sdlgui_cursor_get_scale(sdlgui_element_t e) {
    return static_cast<Cursor*>(unwrap_elem(e))->getScale();
}

void sdlgui_cursor_set_visible(sdlgui_element_t e, int visible) {
    static_cast<Cursor*>(unwrap_elem(e))->setVisible(visible != 0);
}

int sdlgui_cursor_is_visible(sdlgui_element_t e) {
    return static_cast<Cursor*>(unwrap_elem(e))->isVisible() ? 1 : 0;
}

void sdlgui_cursor_set_on_state_changed(sdlgui_element_t e,
                                        sdlgui_cursor_state_callback_t cb, void* userdata) {
    auto* cursor = static_cast<Cursor*>(unwrap_elem(e));
    cursor->setOnStateChanged([cb, userdata, cursor](CursorState state) {
        if (cb) cb(wrap_elem(cursor), convert_cursor_state_back(state), userdata);
    });
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 3: ShaderPanel
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_shader_panel_create(sdlgui_t gui, sdlgui_element_t parent,
                                             int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    auto sp = std::make_unique<ShaderPanel>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(sp));
}

void sdlgui_shader_panel_set_shader(sdlgui_element_t e,
                                    const uint8_t* spirv_data, size_t spirv_size) {
    if (!spirv_data || spirv_size == 0) {
        return;
    }
    static_cast<ShaderPanel*>(unwrap_elem(e))->setShader(spirv_data, spirv_size);
}

void sdlgui_shader_panel_set_shader_enabled(sdlgui_element_t e, int enabled) {
    static_cast<ShaderPanel*>(unwrap_elem(e))->setShaderEnabled(enabled != 0);
}

int sdlgui_shader_panel_is_shader_enabled(sdlgui_element_t e) {
    return static_cast<ShaderPanel*>(unwrap_elem(e))->isShaderEnabled() ? 1 : 0;
}

void sdlgui_shader_panel_set_uniform_time(sdlgui_element_t e, float time) {
    static_cast<ShaderPanel*>(unwrap_elem(e))->setUniformTime(time);
}

void sdlgui_shader_panel_set_uniform_mouse(sdlgui_element_t e, float x, float y) {
    static_cast<ShaderPanel*>(unwrap_elem(e))->setUniformMouse(x, y);
}

/* ═══════════════════════════════════════════════════════════════════
   Point 5: Lifetime + WidgetFactory boundary API
   ═══════════════════════════════════════════════════════════════════ */

const char* sdlgui_last_error(void) {
    return g_lastError.c_str();
}

int sdlgui_element_is_alive(sdlgui_t gui, sdlgui_element_t e) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx || !e) return 0;
    return ctx->getGUIManager().isElementAlive(unwrap_elem(e)) ? 1 : 0;
}

sdlgui_element_t sdlgui_create_widget(sdlgui_t gui, sdlgui_element_t parent,
                                      const char* type, int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_create_widget: null context");
        return nullptr;
    }
    if (!type || !WidgetFactory::isKnownType(type)) {
        set_c_error(std::string("sdlgui_create_widget: unknown widget type '") +
                    (type ? type : "(null)") + "'");
        return nullptr;
    }
    WidgetProps props;
    props.x = x;
    props.y = y;
    props.w = w;
    props.h = h;
    auto widget = WidgetFactory::create(ctx->getGUIManager(), type, props);
    if (!widget) {
        set_c_error(std::string("sdlgui_create_widget: factory failed for '") + type + "'");
        return nullptr;
    }
    return add_element(ctx, parent, std::move(widget));
}

} // extern "C"
