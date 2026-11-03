#include "gui_context.hpp"
#include "theme_presets.hpp"
#include "easing.hpp"
#include "constants.hpp"
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
#include "menu_bar.hpp"
#include "menu_item.hpp"
#include "range_slider.hpp"
#include "cursor.hpp"
#include "shader_panel.hpp"
#include "widget_factory.hpp"
#include "layout.hpp"
#include "custom_control.hpp"
#include "menu_list.hpp"
#include "composite/modal_dialog.hpp"
#include "composite/form_dialog.hpp"
#include "composite/dialog_box.hpp"
#include "composite/file_dialog.hpp"
#include "composite/color_dialog.hpp"
#include "composite/message_box.hpp"

#include "sdl_gui.h"

#include <cassert>
#include <cstring>
#include <memory>

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

// MenuItem handles are raw pointers owned by the MenuBar tree. The C API
// documents their validity window (until clear/destroy); only null is checked.
MenuItem* unwrap_menu_item(sdlgui_menu_item_t item, const char* fn) {
    if (!item) {
        set_c_error(std::string(fn) + ": null menu item handle");
        return nullptr;
    }
    return static_cast<MenuItem*>(item);
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

sdlgui_t sdlgui_attach(SDL_Renderer* renderer, int width, int height) {
    try {
        if (!renderer || width <= 0 || height <= 0) return nullptr;
        return new CContext(renderer, width, height);
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
    ctx->getGUIManager().setTheme(ThemePresets::createWindows95Theme());
}

void sdlgui_theme_windows95(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    ctx->getGUIManager().setTheme(ThemePresets::createWindows95Theme());
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

static Dock convert_dock(int d) {
    switch (d) {
        case 1: return Dock::Top;
        case 2: return Dock::Bottom;
        case 3: return Dock::Left;
        case 4: return Dock::Right;
        case 5: return Dock::Fill;
        default: return Dock::None;
    }
}

void sdlgui_element_set_dock(sdlgui_element_t e, int dock) {
    unwrap_elem(e)->setDock(convert_dock(dock));
}

int sdlgui_element_get_dock(sdlgui_element_t e) {
    switch (unwrap_elem(e)->getDock()) {
        case Dock::Top:    return 1;
        case Dock::Bottom: return 2;
        case Dock::Left:   return 3;
        case Dock::Right:  return 4;
        case Dock::Fill:   return 5;
        default:           return 0;
    }
}

void sdlgui_element_set_dock_layout(sdlgui_element_t e, int spacing,
                                    int pad_left, int pad_top,
                                    int pad_right, int pad_bottom) {
    unwrap_elem(e)->setLayoutManager(
        std::make_unique<DockLayout>(spacing, pad_left, pad_top, pad_right, pad_bottom));
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
   Geometry, tree, visibility, focus, control ids
   ═══════════════════════════════════════════════════════════ */

void sdlgui_element_get_rect(sdlgui_element_t e, sdlgui_rect_t* out) {
    if (!out) return;
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_rect: null element handle");
        return;
    }
    out->x = elem->getX();
    out->y = elem->getY();
    out->w = elem->getWidth();
    out->h = elem->getHeight();
}

void sdlgui_element_get_absolute_rect(sdlgui_element_t e, sdlgui_rect_t* out) {
    if (!out) return;
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_absolute_rect: null element handle");
        return;
    }
    const SDL_Point abs = elem->getAbsolutePosition();
    out->x = abs.x;
    out->y = abs.y;
    out->w = elem->getWidth();
    out->h = elem->getHeight();
}

int sdlgui_element_get_visible(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_visible: null element handle");
        return 0;
    }
    return elem->isVisible() ? 1 : 0;
}

int sdlgui_element_is_shown(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_is_shown: null element handle");
        return 0;
    }
    return elem->isEffectivelyVisible() ? 1 : 0;
}

int sdlgui_element_get_enabled(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_enabled: null element handle");
        return 0;
    }
    return elem->isEnabled() ? 1 : 0;
}

sdlgui_element_t sdlgui_element_get_parent(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_parent: null element handle");
        return nullptr;
    }
    return wrap_elem(elem->getParent());
}

size_t sdlgui_element_get_child_count(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_child_count: null element handle");
        return 0;
    }
    return elem->getChildCount();
}

sdlgui_element_t sdlgui_element_get_child(sdlgui_element_t e, size_t index) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_child: null element handle");
        return nullptr;
    }
    GUIElement* child = elem->getChild(index);
    if (!child) {
        set_c_error("sdlgui_element_get_child: index out of range");
        return nullptr;
    }
    return wrap_elem(child);
}

sdlgui_element_t sdlgui_element_find_by_id(sdlgui_element_t e, const char* id) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_find_by_id: null element handle");
        return nullptr;
    }
    GUIElement* found = elem->findById(id ? id : "");
    if (!found) {
        set_c_error("sdlgui_element_find_by_id: id not found");
        return nullptr;
    }
    return wrap_elem(found);
}

int sdlgui_element_request_focus(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_request_focus: null element handle");
        return 0;
    }
    if (!elem->getManager().requestFocus(elem)) {
        set_c_error("sdlgui_element_request_focus: element cannot take focus "
                    "(disabled, hidden or not focusable)");
        return 0;
    }
    return 1;
}

sdlgui_element_t sdlgui_get_focused_element(sdlgui_t gui) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_get_focused_element: null gui handle");
        return nullptr;
    }
    return wrap_elem(ctx->getGUIManager().getKeyboardFocus());
}

void sdlgui_focus_next(sdlgui_t gui, int forward) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_focus_next: null gui handle");
        return;
    }
    ctx->getGUIManager().focusNextElement(forward != 0);
}

void sdlgui_get_window_rect(sdlgui_t gui, sdlgui_rect_t* out) {
    if (!out) return;
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_get_window_rect: null gui handle");
        return;
    }
    int w = 0;
    int h = 0;
    ctx->getGUIManager().getWindowSize(w, h);
    out->x = 0;
    out->y = 0;
    out->w = w;
    out->h = h;
}

void sdlgui_element_set_control_id(sdlgui_element_t e, int control_id) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_set_control_id: null element handle");
        return;
    }
    if (control_id < 0 || control_id > 0xffff) {
        set_c_error("sdlgui_element_set_control_id: control id out of range (0..0xffff)");
        return;
    }
    elem->setControlId(control_id);
}

int sdlgui_element_get_control_id(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_get_control_id: null element handle");
        return -1;
    }
    return elem->getControlId();
}

sdlgui_element_t sdlgui_element_find_by_control_id(sdlgui_element_t e, int control_id) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_find_by_control_id: null element handle");
        return nullptr;
    }
    GUIElement* found = elem->findByControlId(control_id);
    if (!found) {
        set_c_error("sdlgui_element_find_by_control_id: control id not found");
        return nullptr;
    }
    return wrap_elem(found);
}

/* ═══════════════════════════════════════════════════════════
   Text metrics + Label alignment
   ═══════════════════════════════════════════════════════════ */

namespace {

struct DefaultFontSelection {
    std::string path;
    int size = 0;
    explicit DefaultFontSelection(GUIManager& manager, int font_size) {
        const Style& style = manager.getTheme().getDefaultStyle();
        path = style.fontName.value_or(constants::kDefaultFontPath);
        size = font_size > 0 ? font_size : style.fontSize.value_or(16);
    }
};

}  // namespace

int sdlgui_measure_text(sdlgui_t gui, const char* text, int font_size,
                        int* width, int* height, int* line_height) {
    if (width) *width = 0;
    if (height) *height = 0;
    if (line_height) *line_height = 0;

    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_measure_text: null gui handle");
        return 0;
    }
    auto& manager = ctx->getGUIManager();
    const DefaultFontSelection selection(manager, font_size);
    auto font = manager.getFontManager().loadFont(selection.path, selection.size);
    if (!font) {
        set_c_error("sdlgui_measure_text: could not load default font");
        return 0;
    }

    const int lineHeight = TTF_GetFontHeight(font.get());
    if (line_height) *line_height = lineHeight;

    const std::string_view source = text ? std::string_view(text) : std::string_view();
    if (source.empty()) {
        if (height) *height = lineHeight;
        return 1;
    }

    int maxWidth = 0;
    int lines = 0;
    size_t start = 0;
    while (true) {
        const size_t end = source.find('\n', start);
        std::string_view line = (end == std::string_view::npos)
            ? source.substr(start)
            : source.substr(start, end - start);
        while (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }
        if (!line.empty()) {
            int lineWidth = 0;
            if (TTF_GetStringSize(font.get(), line.data(), line.size(), &lineWidth, nullptr)) {
                maxWidth = std::max(maxWidth, lineWidth);
            }
        }
        ++lines;
        if (end == std::string_view::npos) break;
        start = end + 1;
    }

    if (width) *width = maxWidth;
    if (height) *height = lines * lineHeight;
    return 1;
}

int sdlgui_get_font_metrics(sdlgui_t gui, int font_size,
                            int* line_height, int* ascent, int* descent) {
    if (line_height) *line_height = 0;
    if (ascent) *ascent = 0;
    if (descent) *descent = 0;

    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_get_font_metrics: null gui handle");
        return 0;
    }
    auto& manager = ctx->getGUIManager();
    const DefaultFontSelection selection(manager, font_size);
    if (!manager.getFontManager().getFontMetrics(selection.path, selection.size,
                                                 line_height, ascent, descent)) {
        set_c_error("sdlgui_get_font_metrics: could not load default font");
        return 0;
    }
    return 1;
}

void sdlgui_label_set_alignment(sdlgui_element_t e, int h, int v) {
    auto* label = checked_elem<Label>(e, ComponentType::Label, __func__);
    if (!label) return;
    if (h < 0 || h > 2 || v < 0 || v > 2) {
        set_c_error("sdlgui_label_set_alignment: alignment out of range (0..2)");
        return;
    }
    label->setAlignment(static_cast<TextHAlign>(h), static_cast<TextVAlign>(v));
}

void sdlgui_label_set_font_size(sdlgui_element_t e, int font_size) {
    auto* label = checked_elem<Label>(e, ComponentType::Label, __func__);
    if (!label) return;
    label->setFontSize(font_size);
}

size_t sdlgui_button_get_label_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* button = checked_elem<Button>(e, ComponentType::Button, __func__);
    if (!button) return 0;
    return copy_c_string(button->getText(), buf, buf_len);
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
   Phase 2: MenuBar
   ═══════════════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_menu_bar_create(sdlgui_t gui, sdlgui_element_t parent,
                                        int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_menu_bar_create: null gui");
        return nullptr;
    }
    auto bar = std::make_unique<MenuBar>(ctx->getGUIManager(), x, y, w, h);
    return add_element(ctx, parent, std::move(bar));
}

void sdlgui_menu_bar_set_on_command(sdlgui_element_t bar,
                                    sdlgui_menu_command_callback_t cb,
                                    void* userdata) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return;
    if (!cb) {
        menu->setCommandHandler(nullptr);
        return;
    }
    menu->setCommandHandler([bar, cb, userdata](std::string_view command) {
        const std::string text(command);
        cb(bar, text.c_str(), userdata);
    });
}

sdlgui_menu_item_t sdlgui_menu_bar_add_menu(sdlgui_element_t bar, const char* text) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return nullptr;
    return static_cast<sdlgui_menu_item_t>(&menu->addMenu(text ? text : ""));
}

void sdlgui_menu_bar_open_menu(sdlgui_element_t bar, int index) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu || index < 0) return;
    menu->openMenu(static_cast<size_t>(index));
}

void sdlgui_menu_bar_close(sdlgui_element_t bar) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return;
    menu->closeMenus();
}

int sdlgui_menu_bar_is_open(sdlgui_element_t bar) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    return (menu && menu->isMenuOpen()) ? 1 : 0;
}

int sdlgui_menu_bar_get_menu_count(sdlgui_element_t bar) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    return menu ? static_cast<int>(menu->getMenuCount()) : 0;
}

int sdlgui_menu_bar_get_height(sdlgui_element_t bar) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    return menu ? menu->getHeight() : 0;
}

void sdlgui_menu_bar_clear(sdlgui_element_t bar) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return;
    menu->clearMenus();
}

sdlgui_menu_item_t sdlgui_menu_item_add_item(sdlgui_menu_item_t parent,
                                             const char* text,
                                             const char* command) {
    auto* node = unwrap_menu_item(parent, __func__);
    if (!node) return nullptr;
    MenuItem& item = node->addItem(text ? text : "");
    if (command && *command) item.setCommand(command);
    return static_cast<sdlgui_menu_item_t>(&item);
}

sdlgui_menu_item_t sdlgui_menu_item_add_submenu(sdlgui_menu_item_t parent,
                                                const char* text) {
    auto* node = unwrap_menu_item(parent, __func__);
    if (!node) return nullptr;
    return static_cast<sdlgui_menu_item_t>(&node->addSubmenu(text ? text : ""));
}

void sdlgui_menu_item_add_separator(sdlgui_menu_item_t parent) {
    auto* node = unwrap_menu_item(parent, __func__);
    if (!node) return;
    node->addSeparator();
}

void sdlgui_menu_item_set_shortcut(sdlgui_menu_item_t item, const char* shortcut) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    node->setShortcut(shortcut ? shortcut : "");
}

void sdlgui_menu_item_set_shortcut_display(sdlgui_menu_item_t item, const char* shortcut) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    node->setShortcutDisplay(shortcut ? shortcut : "");
}

void sdlgui_menu_item_set_enabled(sdlgui_menu_item_t item, int enabled) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    node->setEnabled(enabled != 0);
}

void sdlgui_menu_item_set_check_type(sdlgui_menu_item_t item,
                                     sdlgui_menu_check_t type) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    switch (type) {
        case SDLGUI_MENU_CHECK_NONE: node->check = MenuCheck::None; break;
        case SDLGUI_MENU_CHECK:      node->check = MenuCheck::Check; break;
        case SDLGUI_MENU_RADIO:      node->check = MenuCheck::Radio; break;
        default:
            set_c_error("sdlgui_menu_item_set_check_type: invalid check type");
            return;
    }
    node->notifyChanged();
}

void sdlgui_menu_item_set_checked(sdlgui_menu_item_t item, int checked) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    if (node->check == MenuCheck::None) node->check = MenuCheck::Check;
    node->checked = checked != 0;
    node->notifyChanged();
}

int sdlgui_menu_item_is_checked(sdlgui_menu_item_t item) {
    auto* node = unwrap_menu_item(item, __func__);
    return (node && node->checked) ? 1 : 0;
}

sdlgui_menu_item_t sdlgui_menu_item_add_item_id(sdlgui_menu_item_t parent,
                                                const char* text, int command_id) {
    auto* node = unwrap_menu_item(parent, __func__);
    if (!node) return nullptr;
    MenuItem& item = node->addItem(text ? text : "");
    item.setCommandId(command_id);
    return static_cast<sdlgui_menu_item_t>(&item);
}

void sdlgui_menu_bar_set_on_command_id(sdlgui_element_t bar,
                                       sdlgui_menu_command_id_callback_t cb,
                                       void* userdata) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return;
    if (!cb) {
        menu->setCommandIdHandler(nullptr);
        return;
    }
    menu->setCommandIdHandler([bar, cb, userdata](int command_id) {
        cb(bar, command_id, userdata);
    });
}

void sdlgui_menu_item_set_text(sdlgui_menu_item_t item, const char* text) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return;
    node->setText(text ? text : "");
}

size_t sdlgui_menu_item_get_text_buf(sdlgui_menu_item_t item,
                                     char* buf, size_t buf_len) {
    auto* node = unwrap_menu_item(item, __func__);
    if (!node) return 0;
    return copy_c_string(node->text, buf, buf_len);
}

sdlgui_menu_item_t sdlgui_menu_item_find_by_id(sdlgui_element_t bar, int command_id) {
    auto* menu = checked_elem<MenuBar>(bar, ComponentType::MenuBar, __func__);
    if (!menu) return nullptr;

    std::function<MenuItem*(MenuItem*)> walk = [&](MenuItem* node) -> MenuItem* {
        if (!node) return nullptr;
        if (node->hasCommandId && node->commandId == command_id) return node;
        for (auto& child : node->submenu) {
            if (MenuItem* found = walk(child.get())) return found;
        }
        return nullptr;
    };

    for (size_t i = 0; i < menu->getMenuCount(); ++i) {
        if (MenuItem* found = walk(menu->getMenu(i))) {
            return static_cast<sdlgui_menu_item_t>(found);
        }
    }
    set_c_error("sdlgui_menu_item_find_by_id: command id not found");
    return nullptr;
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

void sdlgui_cursor_set_surface(sdlgui_element_t e, sdlgui_cursor_state_t state,
                               SDL_Surface* surface, int hotspot_x, int hotspot_y) {
    auto* cursor = checked_elem<Cursor>(e, ComponentType::Cursor, __func__);
    if (!cursor) return;
    if (!surface) {
        set_c_error("sdlgui_cursor_set_surface: null surface");
        return;
    }
    cursor->setCursorSurface(convert_cursor_state(state), surface, hotspot_x, hotspot_y);
}

void sdlgui_cursor_set_system(sdlgui_element_t e, SDL_SystemCursor cursor) {
    auto* c = checked_elem<Cursor>(e, ComponentType::Cursor, __func__);
    if (!c) return;
    c->setSystemCursor(cursor);
}

int sdlgui_cursor_is_system(sdlgui_element_t e) {
    auto* c = checked_elem<Cursor>(e, ComponentType::Cursor, __func__);
    return (c && c->isSystemCursor()) ? 1 : 0;
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

/* ═══════════════════════════════════════════════════════════════════
   Dialogs: DialogBox / FileDialog / ColorDialog / MessageBox
   ═══════════════════════════════════════════════════════════════════ */

void sdlgui_dialog_close(sdlgui_element_t e) {
    GUIElement* base = unwrap_elem(e);
    if (!base) {
        set_c_error("sdlgui_dialog_close: null element handle");
        return;
    }
    auto* dlg = dynamic_cast<ModalDialog*>(base);
    if (!dlg) {
        set_c_error(std::string("sdlgui_dialog_close: type mismatch (handle is ") +
                    std::string(componentTypeToString(base->getComponentTypeId())) + ")");
        return;
    }
    g_lastError.clear();
    dlg->close();
}

int sdlgui_dialog_is_open(sdlgui_element_t e) {
    GUIElement* base = unwrap_elem(e);
    if (!base) {
        set_c_error("sdlgui_dialog_is_open: null element handle");
        return 0;
    }
    auto* dlg = dynamic_cast<ModalDialog*>(base);
    if (!dlg) {
        set_c_error(std::string("sdlgui_dialog_is_open: type mismatch (handle is ") +
                    std::string(componentTypeToString(base->getComponentTypeId())) + ")");
        return 0;
    }
    g_lastError.clear();
    return dlg->isOpen() ? 1 : 0;
}

void sdlgui_dialog_center(sdlgui_element_t e) {
    GUIElement* base = unwrap_elem(e);
    if (!base) {
        set_c_error("sdlgui_dialog_center: null element handle");
        return;
    }
    auto* dlg = dynamic_cast<ModalDialog*>(base);
    if (!dlg) {
        set_c_error(std::string("sdlgui_dialog_center: type mismatch (handle is ") +
                    std::string(componentTypeToString(base->getComponentTypeId())) + ")");
        return;
    }
    g_lastError.clear();
    dlg->centerInViewport();
}

/* ── DialogBox ── */

sdlgui_element_t sdlgui_dialog_box_create_confirm(sdlgui_t gui, const char* message,
                                                  const char* yes_label, const char* no_label,
                                                  sdlgui_dialog_confirm_callback_t cb, void* userdata,
                                                  int width, int height) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_dialog_box_create_confirm: null context");
        return nullptr;
    }
    if (width <= 0) width = 400;
    if (height <= 0) height = 150;
    // Slot trick: the factory callback needs the dialog handle, which only
    // exists after the factory returns. The shared slot outlives this call
    // via the lambda's copy (freed with the dialog) — no per-dialog leak.
    auto slot = std::make_shared<DialogBox*>(nullptr);
    auto dlg = DialogBox::createConfirm(
        ctx->getGUIManager(),
        message ? message : "",
        yes_label ? yes_label : "Tak",
        no_label ? no_label : "Nie",
        [cb, userdata, slot](bool confirmed) {
            DialogBox* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), confirmed ? 1 : 0, userdata);
        },
        width, height);
    if (!dlg) {
        set_c_error("sdlgui_dialog_box_create_confirm: factory failed");
        return nullptr;
    }
    *slot = dlg.get();
    g_lastError.clear();
    return add_element(ctx, nullptr, std::move(dlg));
}

sdlgui_element_t sdlgui_dialog_box_create_alert(sdlgui_t gui, const char* message,
                                                const char* ok_label,
                                                sdlgui_dialog_button_callback_t cb, void* userdata,
                                                int width, int height) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_dialog_box_create_alert: null context");
        return nullptr;
    }
    if (width <= 0) width = 350;
    if (height <= 0) height = 120;
    auto slot = std::make_shared<DialogBox*>(nullptr);
    auto dlg = DialogBox::createAlert(
        ctx->getGUIManager(),
        message ? message : "",
        ok_label ? ok_label : "OK",
        [cb, userdata, slot](int index) {
            DialogBox* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), index, userdata);
        },
        width, height);
    if (!dlg) {
        set_c_error("sdlgui_dialog_box_create_alert: factory failed");
        return nullptr;
    }
    *slot = dlg.get();
    g_lastError.clear();
    return add_element(ctx, nullptr, std::move(dlg));
}

namespace {
// Shared helper: converts a C label array to std::vector<std::string>.
// Returns false + error on empty input (caller returns nullptr).
bool copy_c_labels(const char** labels, size_t count, const char* fn,
                   std::vector<std::string>& out) {
    if (count == 0 || !labels) {
        set_c_error(std::string(fn) + ": no button labels");
        return false;
    }
    out.clear();
    out.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        out.emplace_back(labels[i] ? labels[i] : "");
    }
    return true;
}
}  // namespace

sdlgui_element_t sdlgui_dialog_box_create_custom(sdlgui_t gui, const char* message,
                                                 const char** button_labels, size_t button_count,
                                                 sdlgui_dialog_button_callback_t cb, void* userdata,
                                                 int width, int height) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_dialog_box_create_custom: null context");
        return nullptr;
    }
    std::vector<std::string> labels;
    if (!copy_c_labels(button_labels, button_count,
                       "sdlgui_dialog_box_create_custom", labels)) {
        return nullptr;
    }
    if (width <= 0) width = 400;
    if (height <= 0) height = 150;
    auto slot = std::make_shared<DialogBox*>(nullptr);
    auto dlg = DialogBox::createCustom(
        ctx->getGUIManager(),
        message ? message : "",
        labels,
        [cb, userdata, slot](int index) {
            DialogBox* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), index, userdata);
        },
        width, height);
    if (!dlg) {
        set_c_error("sdlgui_dialog_box_create_custom: factory failed");
        return nullptr;
    }
    *slot = dlg.get();
    g_lastError.clear();
    return add_element(ctx, nullptr, std::move(dlg));
}

sdlgui_element_t sdlgui_dialog_box_create_with_title(sdlgui_t gui, const char* title,
                                                     const char* message,
                                                     const char** button_labels, size_t button_count,
                                                     sdlgui_dialog_button_callback_t cb, void* userdata,
                                                     int width, int height) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_dialog_box_create_with_title: null context");
        return nullptr;
    }
    std::vector<std::string> labels;
    if (!copy_c_labels(button_labels, button_count,
                       "sdlgui_dialog_box_create_with_title", labels)) {
        return nullptr;
    }
    if (width <= 0) width = 400;
    if (height <= 0) height = 180;
    auto slot = std::make_shared<DialogBox*>(nullptr);
    auto dlg = DialogBox::createWithTitle(
        ctx->getGUIManager(),
        title ? title : "",
        message ? message : "",
        labels,
        [cb, userdata, slot](int index) {
            DialogBox* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), index, userdata);
        },
        width, height);
    if (!dlg) {
        set_c_error("sdlgui_dialog_box_create_with_title: factory failed");
        return nullptr;
    }
    *slot = dlg.get();
    g_lastError.clear();
    return add_element(ctx, nullptr, std::move(dlg));
}

void sdlgui_dialog_box_set_message(sdlgui_element_t e, const char* message) {
    auto* dlg = checked_elem<DialogBox>(e, ComponentType::DialogBox, __func__);
    if (!dlg) return;
    dlg->setMessage(message ? message : "");
}

void sdlgui_dialog_box_set_title(sdlgui_element_t e, const char* title) {
    auto* dlg = checked_elem<DialogBox>(e, ComponentType::DialogBox, __func__);
    if (!dlg) return;
    dlg->setTitle(title ? title : "");
}

int sdlgui_dialog_box_get_last_clicked(sdlgui_element_t e) {
    auto* dlg = checked_elem<DialogBox>(e, ComponentType::DialogBox, __func__);
    return dlg ? dlg->getLastClickedButton() : -1;
}

int sdlgui_dialog_box_get_type(sdlgui_element_t e) {
    auto* dlg = checked_elem<DialogBox>(e, ComponentType::DialogBox, __func__);
    if (!dlg) return -1;
    switch (dlg->getDialogType()) {
        case DialogBox::DialogType::Confirm: return SDLGUI_DIALOG_TYPE_CONFIRM;
        case DialogBox::DialogType::Alert:   return SDLGUI_DIALOG_TYPE_ALERT;
        case DialogBox::DialogType::Custom:  return SDLGUI_DIALOG_TYPE_CUSTOM;
    }
    return SDLGUI_DIALOG_TYPE_CUSTOM;
}

/* ── FileDialog ── */

sdlgui_element_t sdlgui_file_dialog_create_open(sdlgui_t gui, const char* title,
                                                const char* start_path, const char* filter,
                                                sdlgui_file_dialog_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_file_dialog_create_open: null context");
        return nullptr;
    }
    auto slot = std::make_shared<FileDialog*>(nullptr);
    FileDialog* raw = FileDialog::createOpen(
        ctx->getGUIManager(),
        title ? title : "",
        [cb, userdata, slot](const std::string& path) {
            FileDialog* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), path.c_str(), userdata);
        },
        start_path ? start_path : "",
        filter ? filter : "*.*");
    if (!raw) {
        set_c_error("sdlgui_file_dialog_create_open: factory failed");
        return nullptr;
    }
    *slot = raw;
    g_lastError.clear();
    return wrap_elem(raw);
}

sdlgui_element_t sdlgui_file_dialog_create_save(sdlgui_t gui, const char* title,
                                                const char* start_path, const char* filter,
                                                sdlgui_file_dialog_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_file_dialog_create_save: null context");
        return nullptr;
    }
    auto slot = std::make_shared<FileDialog*>(nullptr);
    FileDialog* raw = FileDialog::createSave(
        ctx->getGUIManager(),
        title ? title : "",
        [cb, userdata, slot](const std::string& path) {
            FileDialog* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), path.c_str(), userdata);
        },
        start_path ? start_path : "",
        filter ? filter : "*.*");
    if (!raw) {
        set_c_error("sdlgui_file_dialog_create_save: factory failed");
        return nullptr;
    }
    *slot = raw;
    g_lastError.clear();
    return wrap_elem(raw);
}

void sdlgui_file_dialog_set_current_path(sdlgui_element_t e, const char* path) {
    auto* dlg = checked_elem<FileDialog>(e, ComponentType::FileDialog, __func__);
    if (!dlg) return;
    dlg->setCurrentPath(path ? path : "");
}

const char* sdlgui_file_dialog_get_current_path(sdlgui_element_t e) {
    // Legacy: pointer to internal storage, stable until the next
    // setCurrentPath(). Prefer the _buf variant.
    auto* dlg = checked_elem<FileDialog>(e, ComponentType::FileDialog, __func__);
    if (!dlg) return "";
    return dlg->getCurrentPath().c_str();
}

size_t sdlgui_file_dialog_get_current_path_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* dlg = checked_elem<FileDialog>(e, ComponentType::FileDialog, __func__);
    if (!dlg) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(dlg->getCurrentPath(), buf, buf_len);
}

const char* sdlgui_file_dialog_get_selected_file(sdlgui_element_t e) {
    // Legacy: pointer to internal storage. Prefer the _buf variant.
    auto* dlg = checked_elem<FileDialog>(e, ComponentType::FileDialog, __func__);
    if (!dlg) return "";
    return dlg->getSelectedFile().c_str();
}

size_t sdlgui_file_dialog_get_selected_file_buf(sdlgui_element_t e, char* buf, size_t buf_len) {
    auto* dlg = checked_elem<FileDialog>(e, ComponentType::FileDialog, __func__);
    if (!dlg) {
        if (buf && buf_len > 0) buf[0] = '\0';
        return 0;
    }
    return copy_c_string(dlg->getSelectedFile(), buf, buf_len);
}

/* ── ColorDialog ── */

sdlgui_element_t sdlgui_color_dialog_create(sdlgui_t gui, const char* title,
                                            SDL_Color initial,
                                            sdlgui_color_dialog_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_color_dialog_create: null context");
        return nullptr;
    }
    auto slot = std::make_shared<ColorDialog*>(nullptr);
    ColorDialog* raw = ColorDialog::create(
        ctx->getGUIManager(),
        title ? title : "",
        initial,
        [cb, userdata, slot](SDL_Color color) {
            ColorDialog* d = slot ? *slot : nullptr;
            if (cb) cb(wrap_elem(d), color, userdata);
        });
    if (!raw) {
        set_c_error("sdlgui_color_dialog_create: factory failed");
        return nullptr;
    }
    *slot = raw;
    g_lastError.clear();
    return wrap_elem(raw);
}

void sdlgui_color_dialog_set_color(sdlgui_element_t e, SDL_Color color) {
    auto* dlg = checked_elem<ColorDialog>(e, ComponentType::ColorDialog, __func__);
    if (!dlg) return;
    dlg->setColor(color);
}

SDL_Color sdlgui_color_dialog_get_color(sdlgui_element_t e) {
    auto* dlg = checked_elem<ColorDialog>(e, ComponentType::ColorDialog, __func__);
    if (!dlg) return SDL_Color{0, 0, 0, 255};
    return dlg->getColor();
}

SDL_Color sdlgui_color_dialog_get_initial(sdlgui_element_t e) {
    auto* dlg = checked_elem<ColorDialog>(e, ComponentType::ColorDialog, __func__);
    if (!dlg) return SDL_Color{0, 0, 0, 255};
    return dlg->getInitial();
}

/* ── MessageBox ── */

sdlgui_element_t sdlgui_message_box_show_info(sdlgui_t gui, const char* message,
                                              sdlgui_simple_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_message_box_show_info: null context");
        return nullptr;
    }
    GUIElement* raw = MessageBox::showInfo(
        ctx->getGUIManager(), message ? message : "",
        cb ? [cb, userdata]() { cb(userdata); } : std::function<void()>(nullptr));
    if (!raw) {
        set_c_error("sdlgui_message_box_show_info: factory failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

sdlgui_element_t sdlgui_message_box_show_error(sdlgui_t gui, const char* message,
                                               sdlgui_simple_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_message_box_show_error: null context");
        return nullptr;
    }
    GUIElement* raw = MessageBox::showError(
        ctx->getGUIManager(), message ? message : "",
        cb ? [cb, userdata]() { cb(userdata); } : std::function<void()>(nullptr));
    if (!raw) {
        set_c_error("sdlgui_message_box_show_error: factory failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

sdlgui_element_t sdlgui_message_box_show_warning(sdlgui_t gui, const char* message,
                                                 sdlgui_simple_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_message_box_show_warning: null context");
        return nullptr;
    }
    GUIElement* raw = MessageBox::showWarning(
        ctx->getGUIManager(), message ? message : "",
        cb ? [cb, userdata]() { cb(userdata); } : std::function<void()>(nullptr));
    if (!raw) {
        set_c_error("sdlgui_message_box_show_warning: factory failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

sdlgui_element_t sdlgui_message_box_show_question(sdlgui_t gui, const char* message,
                                                  sdlgui_simple_callback_t on_yes, void* yes_userdata,
                                                  sdlgui_simple_callback_t on_no, void* no_userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_message_box_show_question: null context");
        return nullptr;
    }
    GUIElement* raw = MessageBox::showQuestion(
        ctx->getGUIManager(), message ? message : "",
        on_yes ? [on_yes, yes_userdata]() { on_yes(yes_userdata); } : std::function<void()>(nullptr),
        on_no ? [on_no, no_userdata]() { on_no(no_userdata); } : std::function<void()>(nullptr));
    if (!raw) {
        set_c_error("sdlgui_message_box_show_question: factory failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

sdlgui_element_t sdlgui_message_box_show_custom(sdlgui_t gui, const char* title,
                                                const char* message, const char* button_text,
                                                sdlgui_simple_callback_t cb, void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_message_box_show_custom: null context");
        return nullptr;
    }
    GUIElement* raw = MessageBox::showCustom(
        ctx->getGUIManager(),
        title ? title : "",
        message ? message : "",
        button_text ? button_text : "OK",
        MessageBox::IconType::None,
        cb ? [cb, userdata]() { cb(userdata); } : std::function<void()>(nullptr));
    if (!raw) {
        set_c_error("sdlgui_message_box_show_custom: factory failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

/* ═══════════════════════════════════════════════════════════
   Parity P1: draw/event hooks + CustomControl
   ═══════════════════════════════════════════════════════════ */

void sdlgui_element_set_draw_hook(sdlgui_element_t e, sdlgui_draw_phase_t phase,
                                  sdlgui_draw_callback_t cb, void* userdata) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_set_draw_hook: null element handle");
        return;
    }
    if (phase < SDLGUI_DRAW_BEFORE || phase > SDLGUI_DRAW_AFTER) {
        set_c_error("sdlgui_element_set_draw_hook: invalid phase");
        return;
    }
    const DrawPhase internal = static_cast<DrawPhase>(phase);
    if (!cb) {
        elem->setDrawHook(internal, nullptr, nullptr);
        return;
    }
    elem->setDrawHook(internal,
        [e, cb, userdata](SDL_Renderer* renderer, const SDL_Rect& local_rect,
                          const SDL_Rect& clip_rect, void*) {
            const sdlgui_rect_t rect{local_rect.x, local_rect.y,
                                     local_rect.w, local_rect.h};
            const sdlgui_rect_t clip{clip_rect.x, clip_rect.y,
                                     clip_rect.w, clip_rect.h};
            cb(e, renderer, &rect, &clip, userdata);
        }, userdata);
}

void sdlgui_element_clear_draw_hooks(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_clear_draw_hooks: null element handle");
        return;
    }
    elem->clearDrawHooks();
}

void sdlgui_element_set_event_hook(sdlgui_element_t e, sdlgui_event_callback_t cb,
                                   void* userdata) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_set_event_hook: null element handle");
        return;
    }
    if (!cb) {
        elem->clearEventHook();
        return;
    }
    elem->setEventHook(
        [e, cb, userdata](const SDL_Event& event, int local_x, int local_y, void*) {
            return cb(e, &event, local_x, local_y, userdata) != 0;
        }, userdata);
}

void sdlgui_element_clear_event_hook(sdlgui_element_t e) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_clear_event_hook: null element handle");
        return;
    }
    elem->clearEventHook();
}

void sdlgui_element_notify_command(sdlgui_element_t e, int command_id) {
    auto* elem = unwrap_elem(e);
    if (!elem) {
        set_c_error("sdlgui_element_notify_command: null element handle");
        return;
    }
    elem->notifyCommand(command_id);
}

sdlgui_element_t sdlgui_custom_control_create(sdlgui_t gui, sdlgui_element_t parent,
                                               int x, int y, int w, int h,
                                               sdlgui_draw_callback_t paint,
                                               sdlgui_event_callback_t event,
                                               void* userdata) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_custom_control_create: null context");
        return nullptr;
    }
    auto control = std::make_unique<CustomControl>(ctx->getGUIManager(), x, y, w, h);
    CustomControl* raw = control.get();
    sdlgui_element_t handle = add_element(ctx, parent, std::move(control));
    if (paint) {
        sdlgui_element_set_draw_hook(handle, SDLGUI_DRAW_REPLACE, paint, userdata);
    }
    if (event) {
        sdlgui_element_set_event_hook(handle, event, userdata);
    }
    g_lastError.clear();
    (void)raw;
    return handle;
}

/* ═══════════════════════════════════════════════════════════
   Parity P1: FormDialog
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_form_create(sdlgui_t gui, const char* title,
                                    int x, int y, int w, int h) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_form_create: null context");
        return nullptr;
    }
    auto form = std::make_unique<FormDialog>(ctx->getGUIManager(), x, y, w, h,
                                             title ? title : "");
    return add_element(ctx, nullptr, std::move(form));
}

sdlgui_element_t sdlgui_form_add_widget(sdlgui_element_t form_handle, int id,
                                        sdlgui_element_t child_handle) {
    auto* form = checked_elem<FormDialog>(form_handle, ComponentType::FormDialog, __func__);
    if (!form) return nullptr;
    auto* child = unwrap_elem(child_handle);
    if (!child) {
        set_c_error("sdlgui_form_add_widget: null child handle");
        return nullptr;
    }
    if (child->getParent() != nullptr) {
        set_c_error("sdlgui_form_add_widget: child is not a top-level element");
        return nullptr;
    }
    auto owned = child->getManager().detachElement(child);
    if (!owned) {
        set_c_error("sdlgui_form_add_widget: could not detach child from manager");
        return nullptr;
    }
    GUIElement* added = form->addWidget(id, std::move(owned));
    if (!added) {
        set_c_error("sdlgui_form_add_widget: addWidget failed");
        return nullptr;
    }
    g_lastError.clear();
    return wrap_elem(added);
}

sdlgui_element_t sdlgui_form_add_static(sdlgui_element_t form, int id,
                                        const char* text, int x, int y, int w, int h) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return f ? wrap_elem(f->addStatic(id, text ? text : "", x, y, w, h)) : nullptr;
}

sdlgui_element_t sdlgui_form_add_button(sdlgui_element_t form, int id,
                                        const char* text, int x, int y, int w, int h) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return f ? wrap_elem(f->addButton(id, text ? text : "", x, y, w, h)) : nullptr;
}

sdlgui_element_t sdlgui_form_add_edit(sdlgui_element_t form, int id,
                                      const char* text, int x, int y, int w, int h) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return f ? wrap_elem(f->addEdit(id, text ? text : "", x, y, w, h)) : nullptr;
}

sdlgui_element_t sdlgui_form_add_custom(sdlgui_element_t form, int id,
                                        int x, int y, int w, int h) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return f ? wrap_elem(f->addCustom(id, x, y, w, h)) : nullptr;
}

sdlgui_element_t sdlgui_form_find_item(sdlgui_element_t form, int id) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    if (!f) return nullptr;
    GUIElement* found = f->findItem(id);
    if (!found) {
        set_c_error("sdlgui_form_find_item: control id not found");
        return nullptr;
    }
    return wrap_elem(found);
}

int sdlgui_form_get_item_rect(sdlgui_element_t form, int id, sdlgui_rect_t* out) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    if (!f || !out) return 0;
    if (!f->findItem(id)) {
        set_c_error("sdlgui_form_get_item_rect: control id not found");
        return 0;
    }
    const SDL_Rect rect = f->getItemRect(id);
    out->x = rect.x;
    out->y = rect.y;
    out->w = rect.w;
    out->h = rect.h;
    g_lastError.clear();
    return 1;
}

int sdlgui_form_set_item_visible(sdlgui_element_t form, int id, int visible) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return (f && f->setItemVisible(id, visible != 0)) ? 1 : 0;
}

int sdlgui_form_set_item_enabled(sdlgui_element_t form, int id, int enabled) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return (f && f->setItemEnabled(id, enabled != 0)) ? 1 : 0;
}

int sdlgui_form_set_item_text(sdlgui_element_t form, int id, const char* text) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return (f && f->setItemText(id, text ? text : "")) ? 1 : 0;
}

int sdlgui_form_set_item_rect(sdlgui_element_t form, int id,
                              int x, int y, int w, int h) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return (f && f->setItemRect(id, x, y, w, h)) ? 1 : 0;
}

void sdlgui_form_end(sdlgui_element_t form, int result) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    if (!f) return;
    f->end(result);
}

int sdlgui_form_get_result(sdlgui_element_t form) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return f ? f->getResult() : -1;
}

int sdlgui_form_is_closed(sdlgui_element_t form) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    return (f && f->isClosed()) ? 1 : 0;
}

void sdlgui_form_set_on_command(sdlgui_element_t form,
                                sdlgui_form_command_callback_t cb,
                                void* userdata) {
    auto* f = checked_elem<FormDialog>(form, ComponentType::FormDialog, __func__);
    if (!f) return;
    if (!cb) {
        f->setOnCommand(nullptr);
        return;
    }
    f->setOnCommand([form, cb, userdata](int id) { cb(form, id, userdata); });
}

sdlgui_element_t sdlgui_form_create_from_items(sdlgui_t gui, const char* title,
                                               int x, int y, int w, int h,
                                               const sdlgui_form_item_desc_t* items,
                                               size_t count) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_form_create_from_items: null context");
        return nullptr;
    }
    auto form = std::make_unique<FormDialog>(ctx->getGUIManager(), x, y, w, h,
                                             title ? title : "");
    FormDialog* raw = form.get();
    GUIElement* handle = ctx->getGUIManager().addElement(std::move(form));
    (void)handle;

    for (size_t i = 0; i < count; ++i) {
        const sdlgui_form_item_desc_t& desc = items[i];
        const char* text = desc.text ? desc.text : "";
        switch (desc.type) {
            case SDLGUI_FORM_ITEM_STATIC:
                raw->addStatic(desc.id, text, desc.x, desc.y, desc.w, desc.h);
                break;
            case SDLGUI_FORM_ITEM_BUTTON:
                raw->addButton(desc.id, text, desc.x, desc.y, desc.w, desc.h);
                break;
            case SDLGUI_FORM_ITEM_EDIT:
                raw->addEdit(desc.id, text, desc.x, desc.y, desc.w, desc.h);
                break;
            case SDLGUI_FORM_ITEM_CUSTOM:
                raw->addCustom(desc.id, desc.x, desc.y, desc.w, desc.h);
                break;
            default:
                set_c_error("sdlgui_form_create_from_items: invalid item type");
                break;
        }
    }
    g_lastError.clear();
    return wrap_elem(raw);
}

void sdlgui_dialog_close_with_result(sdlgui_element_t e, int result) {
    auto* dialog = dynamic_cast<ModalDialog*>(unwrap_elem(e));
    if (!dialog) {
        set_c_error("sdlgui_dialog_close_with_result: element is not a ModalDialog");
        return;
    }
    dialog->closeWithResult(result);
}

/* ═══════════════════════════════════════════════════════════
   Parity P1: modal runner
   ═══════════════════════════════════════════════════════════ */

int sdlgui_modal_run(sdlgui_t gui, sdlgui_element_t dialog_handle,
                     const sdlgui_modal_options_t* opts, int* result_out) {
    auto* ctx = unwrap_ctx(gui);
    auto* dialog = dynamic_cast<ModalDialog*>(unwrap_elem(dialog_handle));
    if (!ctx || !dialog) {
        set_c_error("sdlgui_modal_run: null context or dialog is not a ModalDialog");
        if (result_out) *result_out = -1;
        return -1;
    }

    auto& manager = ctx->getGUIManager();
    SDL_Renderer* renderer = manager.getRenderer();
    const int cancel_key = opts ? opts->cancel_key : 0;
    const int cancel_result = opts ? opts->cancel_result : -1;
    void* userdata = opts ? opts->userdata : nullptr;

    int result = cancel_result;

    while (true) {
        bool closed = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (opts && opts->event_filter &&
                opts->event_filter(&event, userdata)) {
                continue;
            }
            if (event.type == SDL_EVENT_QUIT) {
                dialog->closeWithResult(cancel_result);
                result = cancel_result;
                closed = true;
                break;
            }
            if (cancel_key != 0 && event.type == SDL_EVENT_KEY_DOWN &&
                event.key.key == static_cast<SDL_Keycode>(cancel_key)) {
                dialog->closeWithResult(cancel_result);
                result = cancel_result;
                closed = true;
                break;
            }
            manager.processEvent(event);
            if (!dialog->isOpen()) {
                result = dialog->getResult();
                closed = true;
                break;
            }
        }

        if (opts && opts->frame_cb) {
            opts->frame_cb(userdata);
        }
        manager.update();

        // Capture the result BEFORE cleanup: cleanup may destroy the dialog.
        if (!dialog->isOpen()) {
            result = dialog->getResult();
            closed = true;
        }

        manager.cleanup();

        if (closed) {
            break;
        }

        if (opts && !opts->keep_background) {
            SDL_SetRenderDrawColor(renderer,
                                   static_cast<Uint8>(opts->clear_r),
                                   static_cast<Uint8>(opts->clear_g),
                                   static_cast<Uint8>(opts->clear_b),
                                   255);
            SDL_RenderClear(renderer);
        }
        manager.render();
        SDL_RenderPresent(renderer);
    }

    if (result_out) *result_out = result;
    g_lastError.clear();
    return result;
}

/* ═══════════════════════════════════════════════════════════
   Parity P1: MenuList
   ═══════════════════════════════════════════════════════════ */

sdlgui_element_t sdlgui_menu_list_create(sdlgui_t gui, sdlgui_element_t parent,
                                          int x, int y, int w, int h,
                                          const char* title,
                                          const sdlgui_menu_list_item_t* items,
                                          size_t count) {
    auto* ctx = unwrap_ctx(gui);
    if (!ctx) {
        set_c_error("sdlgui_menu_list_create: null context");
        return nullptr;
    }
    std::vector<MenuList::Item> entries;
    entries.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const sdlgui_menu_list_item_t& src = items[i];
        entries.push_back(MenuList::Item{
            src.text ? std::string(src.text) : std::string(),
            src.id,
            src.enabled != 0});
    }
    auto list = std::make_unique<MenuList>(ctx->getGUIManager(), x, y, w, h,
                                           title ? title : "", std::move(entries));
    return add_element(ctx, parent, std::move(list));
}

void sdlgui_menu_list_set_columns(sdlgui_element_t e, int columns) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setColumns(columns);
}

void sdlgui_menu_list_set_title_width(sdlgui_element_t e, int px) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setTitleWidth(px);
}

void sdlgui_menu_list_set_disabled_mask(sdlgui_element_t e, uint32_t mask) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setDisabledMask(mask);
}

void sdlgui_menu_list_set_preselect(sdlgui_element_t e, int item_id) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setPreselect(item_id);
}

void sdlgui_menu_list_set_cancel_id(sdlgui_element_t e, int item_id) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setCancelId(item_id);
}

void sdlgui_menu_list_set_timer_ms(sdlgui_element_t e, uint32_t ms) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (list) list->setTimerMs(ms);
}

int sdlgui_menu_list_get_selected_id(sdlgui_element_t e) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    return list ? list->getSelectedId() : -1;
}

void sdlgui_menu_list_set_on_select(sdlgui_element_t e,
                                    sdlgui_menu_list_select_cb_t cb,
                                    void* userdata) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (!list) return;
    if (!cb) {
        list->setOnSelect(nullptr);
        return;
    }
    list->setOnSelect([e, cb, userdata](int item_id, std::string_view text) {
        const std::string copy(text);
        cb(e, item_id, copy.c_str(), userdata);
    });
}

void sdlgui_menu_list_set_row_callback(sdlgui_element_t e,
                                       sdlgui_menu_list_row_cb_t cb,
                                       void* userdata) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (!list) return;
    if (!cb) {
        list->setRowCallback(nullptr);
        return;
    }
    list->setRowCallback([e, cb, userdata](SDL_Renderer* renderer,
                                           const SDL_Rect& row_rect,
                                           std::string_view text, int item_id,
                                           bool selected, bool enabled) {
        const sdlgui_rect_t rect{row_rect.x, row_rect.y, row_rect.w, row_rect.h};
        const std::string copy(text);
        cb(e, renderer, &rect, copy.c_str(), item_id,
           selected ? 1 : 0, enabled ? 1 : 0, userdata);
    });
}

void sdlgui_menu_list_set_on_tick(sdlgui_element_t e,
                                  sdlgui_menu_list_tick_cb_t cb,
                                  void* userdata) {
    auto* list = checked_elem<MenuList>(e, ComponentType::MenuList, __func__);
    if (!list) return;
    if (!cb) {
        list->setOnTick(nullptr);
        return;
    }
    list->setOnTick([e, cb, userdata]() { cb(e, userdata); });
}

/* ═══════════════════════════════════════════════════════════
   Parity P1: TextInput extensions
   ═══════════════════════════════════════════════════════════ */

void sdlgui_text_input_set_max_length(sdlgui_element_t e, size_t max_length) {
    auto* input = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (input) input->setMaxLength(max_length);
}

void sdlgui_text_input_set_char_filter(sdlgui_element_t e,
                                       int (*filter)(unsigned codepoint, void* userdata),
                                       void* userdata) {
    auto* input = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!input) return;
    if (!filter) {
        input->setCharFilter(nullptr);
        return;
    }
    input->setCharFilter([filter, userdata](unsigned codepoint) {
        return filter(codepoint, userdata) != 0;
    });
}

void sdlgui_text_input_set_on_key(sdlgui_element_t e,
                                  int (*cb)(sdlgui_element_t e,
                                            const SDL_KeyboardEvent* key,
                                            void* userdata),
                                  void* userdata) {
    auto* input = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!input) return;
    if (!cb) {
        input->setOnKey(nullptr);
        return;
    }
    input->setOnKey([e, cb, userdata](const SDL_KeyboardEvent& key) {
        return cb(e, &key, userdata) != 0;
    });
}

void sdlgui_text_input_set_cursor_pos(sdlgui_element_t e, size_t char_pos) {
    auto* input = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (input) input->setCursorPos(char_pos);
}

void sdlgui_text_input_set_selection(sdlgui_element_t e, size_t start, size_t end) {
    auto* input = checked_elem<TextInput>(e, ComponentType::TextInput, __func__);
    if (!input) return;
    input->setSelection(start, end);
    input->markDirty();
}

} // extern "C"
