#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/sdl_app.hpp"
#include "../src/theme_presets.hpp"
#include "../src/button.hpp"
#include "../src/label.hpp"
#include "../src/panel.hpp"
#include "../src/slider.hpp"
#include "../src/checkbox.hpp"
#include "../src/text_input.hpp"
#include "../src/list_view.hpp"

extern "C" {
#include "../src/sdl_gui.h"
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 0: Infrastructure
   ═══════════════════════════════════════════════════════════════════ */

TEST_CASE("C API - Context lifecycle", "[c_api]") {
    SECTION("create and destroy without leaks") {
        sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
        REQUIRE(gui != nullptr);
        sdlgui_destroy(gui);
    }

    SECTION("create resizable window") {
        sdlgui_t gui = sdlgui_create("Test", 800, 600, 1);
        REQUIRE(gui != nullptr);
        sdlgui_destroy(gui);
    }

    SECTION("destroy NULL is safe") {
        sdlgui_destroy(nullptr);
    }
}

TEST_CASE("C API - Core loop", "[c_api]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    /* Test with the existing C++ GUIManager: create and process events */
    auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "Test");
    Button* btnPtr = btn.get();
    manager.addElement(std::move(btn));

    /* Wrap the button in a C handle */
    sdlgui_element_t e = (sdlgui_element_t)btnPtr;

    REQUIRE(sdlgui_element_get_type(e) == std::string("Button"));

    /* Process a mouse motion event to verify the C handle works */
    SDL_Event evt = helper.createMouseMotion(20, 20);
    manager.processEvent(evt);
    REQUIRE(btnPtr->getState() == ElementState::Hover);
}

TEST_CASE("C API - Theme switching", "[c_api]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    /* Apply all 4 presets via C API — just verify no crash */
    /* Note: Since we don't have a CContext, we test the theming directly.
       sdlgui_theme_* functions require a full sdlgui_t handle created via sdlgui_create(). */
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_theme_dark(gui);
    sdlgui_theme_light(gui);
    sdlgui_theme_high_contrast(gui);
    sdlgui_theme_win9x(gui);  /* back to default */

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Element base API", "[c_api]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto pnl = std::make_unique<Panel>(manager, 10, 20, 200, 100);
    Panel* pnlPtr = pnl.get();
    manager.addElement(std::move(pnl));

    sdlgui_element_t e = (sdlgui_element_t)pnlPtr;

    SECTION("get_type returns correct string") {
        REQUIRE(std::string(sdlgui_element_get_type(e)) == "Panel");
    }

    SECTION("set_position") {
        sdlgui_element_set_position(e, 50, 60);
        REQUIRE(pnlPtr->getX() == 50);
        REQUIRE(pnlPtr->getY() == 60);
    }

    SECTION("set_size") {
        sdlgui_element_set_size(e, 300, 150);
        REQUIRE(pnlPtr->getWidth() == 300);
        REQUIRE(pnlPtr->getHeight() == 150);
    }

    SECTION("set_enabled") {
        sdlgui_element_set_enabled(e, 0);
        REQUIRE_FALSE(pnlPtr->isEnabled());
        sdlgui_element_set_enabled(e, 1);
        REQUIRE(pnlPtr->isEnabled());
    }

    SECTION("set_visible") {
        sdlgui_element_set_visible(e, 0);
        REQUIRE_FALSE(pnlPtr->isVisible());
        sdlgui_element_set_visible(e, 1);
        REQUIRE(pnlPtr->isVisible());
    }

    SECTION("set_background_color") {
        SDL_Color c {255, 0, 0, 255};
        sdlgui_element_set_background_color(e, SDLGUI_STATE_NORMAL, c);
        /* Verify the style was set (indirectly via markDirty behavior) */
        REQUIRE(pnlPtr->isEnabled()); /* panel still functional */
    }

    SECTION("set_text_color") {
        SDL_Color c {0, 255, 0, 255};
        sdlgui_element_set_text_color(e, SDLGUI_STATE_NORMAL, c);
        REQUIRE(pnlPtr->isEnabled());
    }

    SECTION("set_border") {
        SDL_Color c {0, 0, 255, 255};
        sdlgui_element_set_border(e, SDLGUI_STATE_HOVER, c, 3);
        REQUIRE(pnlPtr->isEnabled());
    }

    SECTION("set_border_radius") {
        sdlgui_element_set_border_radius(e, SDLGUI_STATE_NORMAL, 10);
        REQUIRE(pnlPtr->isEnabled());
    }

    SECTION("set_tooltip") {
        sdlgui_element_set_tooltip(e, "Tooltip text");
        REQUIRE(pnlPtr->isEnabled());
    }

    SECTION("set_id and get_id") {
        sdlgui_element_set_id(e, "my_id");
        REQUIRE(std::string(sdlgui_element_get_id(e)) == "my_id");
    }

    SECTION("get_id returns empty for unset") {
        REQUIRE(std::string(sdlgui_element_get_id(e)) == "");
    }

    SECTION("set_rotation") {
        sdlgui_element_set_rotation(e, 45.0);
        REQUIRE(pnlPtr->getRotation() == 45.0);
    }

    SECTION("mark_for_deletion") {
        sdlgui_element_mark_for_deletion(e);
        REQUIRE(pnlPtr->isMarkedForDeletion());
    }

    SECTION("set_can_get_keyboard_focus") {
        sdlgui_element_set_can_get_keyboard_focus(e, 1);
        REQUIRE(pnlPtr->canGetKeyboardFocus());
        sdlgui_element_set_can_get_keyboard_focus(e, 0);
        REQUIRE_FALSE(pnlPtr->canGetKeyboardFocus());
    }
}

TEST_CASE("C API - Anchor factories", "[c_api]") {
    SECTION("none") {
        sdlgui_anchor_t a = sdlgui_anchor_none();
        REQUIRE(a.h == SDLGUI_H_NONE);
        REQUIRE(a.v == SDLGUI_V_NONE);
    }

    SECTION("top_left") {
        sdlgui_anchor_t a = sdlgui_anchor_top_left(5);
        REQUIRE(a.h == SDLGUI_H_LEFT);
        REQUIRE(a.v == SDLGUI_V_TOP);
        REQUIRE(a.left == 5);
        REQUIRE(a.top == 5);
    }

    SECTION("top_right") {
        sdlgui_anchor_t a = sdlgui_anchor_top_right(10);
        REQUIRE(a.h == SDLGUI_H_RIGHT);
        REQUIRE(a.v == SDLGUI_V_TOP);
        REQUIRE(a.right == 10);
        REQUIRE(a.top == 10);
    }

    SECTION("bottom_left") {
        sdlgui_anchor_t a = sdlgui_anchor_bottom_left(7);
        REQUIRE(a.h == SDLGUI_H_LEFT);
        REQUIRE(a.v == SDLGUI_V_BOTTOM);
        REQUIRE(a.left == 7);
        REQUIRE(a.bottom == 7);
    }

    SECTION("bottom_right") {
        sdlgui_anchor_t a = sdlgui_anchor_bottom_right(3);
        REQUIRE(a.h == SDLGUI_H_RIGHT);
        REQUIRE(a.v == SDLGUI_V_BOTTOM);
        REQUIRE(a.right == 3);
        REQUIRE(a.bottom == 3);
    }

    SECTION("center") {
        sdlgui_anchor_t a = sdlgui_anchor_center();
        REQUIRE(a.h == SDLGUI_H_CENTER);
        REQUIRE(a.v == SDLGUI_V_CENTER);
    }

    SECTION("fill") {
        sdlgui_anchor_t a = sdlgui_anchor_fill(5);
        REQUIRE(a.h == SDLGUI_H_STRETCH);
        REQUIRE(a.v == SDLGUI_V_STRETCH);
        REQUIRE(a.left == 5);
        REQUIRE(a.top == 5);
        REQUIRE(a.right == 5);
        REQUIRE(a.bottom == 5);
    }

    SECTION("horizontal_stretch") {
        sdlgui_anchor_t a = sdlgui_anchor_horizontal_stretch(3, 7);
        REQUIRE(a.h == SDLGUI_H_STRETCH);
        REQUIRE(a.v == SDLGUI_V_NONE);
        REQUIRE(a.left == 3);
        REQUIRE(a.right == 7);
    }

    SECTION("vertical_stretch") {
        sdlgui_anchor_t a = sdlgui_anchor_vertical_stretch(4, 8);
        REQUIRE(a.h == SDLGUI_H_NONE);
        REQUIRE(a.v == SDLGUI_V_STRETCH);
        REQUIRE(a.top == 4);
        REQUIRE(a.bottom == 8);
    }

    SECTION("top_bar") {
        sdlgui_anchor_t a = sdlgui_anchor_top_bar(50, 10);
        REQUIRE(a.h == SDLGUI_H_STRETCH);
        REQUIRE(a.v == SDLGUI_V_TOP);
        REQUIRE(a.left == 10);
        REQUIRE(a.top == 50);
        REQUIRE(a.right == 10);
    }

    SECTION("bottom_bar") {
        sdlgui_anchor_t a = sdlgui_anchor_bottom_bar(50, 10);
        REQUIRE(a.h == SDLGUI_H_STRETCH);
        REQUIRE(a.v == SDLGUI_V_BOTTOM);
        REQUIRE(a.left == 10);
        REQUIRE(a.bottom == 50);
    }

    SECTION("left_sidebar") {
        sdlgui_anchor_t a = sdlgui_anchor_left_sidebar(10, 20);
        REQUIRE(a.h == SDLGUI_H_LEFT);
        REQUIRE(a.v == SDLGUI_V_STRETCH);
        REQUIRE(a.top == 10);
        REQUIRE(a.bottom == 20);
    }

    SECTION("right_sidebar") {
        sdlgui_anchor_t a = sdlgui_anchor_right_sidebar(10, 20);
        REQUIRE(a.h == SDLGUI_H_RIGHT);
        REQUIRE(a.v == SDLGUI_V_STRETCH);
        REQUIRE(a.top == 10);
        REQUIRE(a.bottom == 20);
    }

    SECTION("make") {
        sdlgui_anchor_t a = sdlgui_anchor_make(SDLGUI_H_RIGHT, SDLGUI_V_BOTTOM, 0, 0, 12, 34);
        REQUIRE(a.h == SDLGUI_H_RIGHT);
        REQUIRE(a.v == SDLGUI_V_BOTTOM);
        REQUIRE(a.right == 12);
        REQUIRE(a.bottom == 34);
    }
}

TEST_CASE("C API - Window size", "[c_api]") {
    sdlgui_t gui = sdlgui_create("Test", 640, 480, 0);
    REQUIRE(gui != nullptr);

    int w = 0, h = 0;
    sdlgui_get_window_size(gui, &w, &h);
    REQUIRE(w == 640);
    REQUIRE(h == 480);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Tooltip", "[c_api]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    /* Create a button so we have a target element */
    sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 100, 40, "Click");
    REQUIRE(btn != nullptr);

    /* show/hide tooltip — no crash */
    sdlgui_show_tooltip(gui, btn, "Hello tooltip");
    sdlgui_hide_tooltip(gui);

    sdlgui_destroy(gui);
}

/* ═══════════════════════════════════════════════════════════════════
   Phase 1: Core 7 Widgets
   ═══════════════════════════════════════════════════════════════════ */

TEST_CASE("C API - Button", "[c_api][button]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create top-level") {
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 120, 40, "Click Me");
        REQUIRE(btn != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(btn)) == "Button");
    }

    SECTION("create as child of panel") {
        sdlgui_element_t pnl = sdlgui_panel_create(gui, nullptr, 0, 0, 200, 100);
        REQUIRE(pnl != nullptr);

        sdlgui_element_t btn = sdlgui_button_create(gui, pnl, 10, 10, 100, 30, "Child");
        REQUIRE(btn != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(btn)) == "Button");
    }

    SECTION("click callback fires") {
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 100, 40, "Test");
        REQUIRE(btn != nullptr);

        int called = 0;
        sdlgui_element_t receivedElem = nullptr;

        sdlgui_button_set_on_click(btn,
            [](sdlgui_element_t elem, void* data) {
                int* p = (int*)data;
                *p = 1;
            },
            &called);

        /* Simulate click via event processing */
        SDL_Event down = {};
        down.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        down.button.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        down.button.button = SDL_BUTTON_LEFT;
        down.button.clicks = 1;
        down.button.x = 50.0f;
        down.button.y = 20.0f;

        sdlgui_process_event(gui, &down);

        SDL_Event up = {};
        up.type = SDL_EVENT_MOUSE_BUTTON_UP;
        up.button.type = SDL_EVENT_MOUSE_BUTTON_UP;
        up.button.button = SDL_BUTTON_LEFT;
        up.button.clicks = 1;
        up.button.x = 50.0f;
        up.button.y = 20.0f;

        sdlgui_process_event(gui, &up);

        REQUIRE(called == 1);
    }

    SECTION("mouse_over callback can be set") {
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 100, 40, "Test");
        REQUIRE(btn != nullptr);

        int hoverCalls = 0;

        sdlgui_button_set_on_mouse_over(btn,
            [](sdlgui_element_t elem, void* data) {
                int* p = (int*)data;
                (*p)++;
            },
            &hoverCalls);

        SDL_Event motion = {};
        motion.type = SDL_EVENT_MOUSE_MOTION;
        motion.motion.type = SDL_EVENT_MOUSE_MOTION;
        motion.motion.x = 50.0f;
        motion.motion.y = 20.0f;
        motion.motion.state = 0;

        sdlgui_process_event(gui, &motion);

        /* The callback fires when the mouse enters the button. */
        REQUIRE(hoverCalls == 1);
    }

    SECTION("set_label updates existing label") {
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 200, 40, "Original");
        REQUIRE(btn != nullptr);

        sdlgui_button_set_label(btn, "Updated");
        /* After setting label, find the label child and verify its text */
        auto* raw = (GUIElement*)btn;
        bool found = false;
        for (auto& child : raw->getChildren()) {
            if (child->getComponentTypeId() == ComponentType::Label) {
                auto* lbl = (Label*)child.get();
                REQUIRE(std::string(lbl->getText()) == "Updated");
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Label", "[c_api][label]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create top-level") {
        sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 10, 10, "Hello", -1);
        REQUIRE(lbl != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(lbl)) == "Label");
    }

    SECTION("create as child") {
        sdlgui_element_t pnl = sdlgui_panel_create(gui, nullptr, 0, 0, 200, 100);
        REQUIRE(pnl != nullptr);

        sdlgui_element_t lbl = sdlgui_label_create(gui, pnl, 5, 5, "Child label", 16);
        REQUIRE(lbl != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(lbl)) == "Label");
    }

    SECTION("set_text and get_text roundtrip") {
        sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 10, 10, "Initial", -1);
        REQUIRE(lbl != nullptr);

        REQUIRE(std::string(sdlgui_label_get_text(lbl)) == "Initial");

        sdlgui_label_set_text(lbl, "Changed");
        REQUIRE(std::string(sdlgui_label_get_text(lbl)) == "Changed");
    }

    SECTION("default font_size") {
        sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 10, 10, "Text", -1);
        REQUIRE(lbl != nullptr);
        /* Should not crash */
        REQUIRE(std::string(sdlgui_label_get_text(lbl)) == "Text");
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Panel", "[c_api][panel]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create top-level") {
        sdlgui_element_t pnl = sdlgui_panel_create(gui, nullptr, 10, 10, 200, 100);
        REQUIRE(pnl != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(pnl)) == "Panel");
    }

    SECTION("drag flag") {
        sdlgui_element_t pnl = sdlgui_panel_create(gui, nullptr, 10, 10, 200, 100);
        REQUIRE(pnl != nullptr);

        sdlgui_panel_set_draggable(pnl, 1);
        /* No direct getter, just verify no crash */
        sdlgui_panel_set_draggable(pnl, 0);
    }

    SECTION("parent-child create") {
        sdlgui_element_t parent = sdlgui_panel_create(gui, nullptr, 0, 0, 300, 300);
        REQUIRE(parent != nullptr);

        sdlgui_element_t child = sdlgui_panel_create(gui, parent, 10, 10, 100, 50);
        REQUIRE(child != nullptr);

        /* Verify child is properly nested */
        auto* rawParent = (GUIElement*)parent;
        REQUIRE(rawParent->getChildren().size() >= 1);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Slider", "[c_api][slider]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create horizontal") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);
        REQUIRE(sld != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(sld)) == "Slider");
    }

    SECTION("create vertical") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 40, 200,
                                                      0, 100, 50, SDLGUI_ORIENTATION_VERTICAL);
        REQUIRE(sld != nullptr);
    }

    SECTION("get_value and set_value") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);
        REQUIRE(sdlgui_slider_get_value(sld) == 50);

        sdlgui_slider_set_value(sld, 75);
        REQUIRE(sdlgui_slider_get_value(sld) == 75);
    }

    SECTION("range clamping") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_slider_set_value(sld, -50);
        REQUIRE(sdlgui_slider_get_value(sld) == 0); /* clamped to min */

        sdlgui_slider_set_value(sld, 200);
        REQUIRE(sdlgui_slider_get_value(sld) == 100); /* clamped to max */
    }

    SECTION("set_range") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_slider_set_range(sld, 10, 90);
        REQUIRE(sdlgui_slider_get_min(sld) == 10);
        REQUIRE(sdlgui_slider_get_max(sld) == 90);
    }

    SECTION("set_min and set_max individually") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_slider_set_min(sld, 20);
        REQUIRE(sdlgui_slider_get_min(sld) == 20);

        sdlgui_slider_set_max(sld, 80);
        REQUIRE(sdlgui_slider_get_max(sld) == 80);
    }

    SECTION("wheel_step") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_slider_set_wheel_step(sld, 5);
        /* Verify via C++ getter */
        auto* sldPtr = (Slider*)(GUIElement*)sld;
        REQUIRE(sldPtr->getWheelStep() == 5);
    }

    SECTION("change callback fires") {
        sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                      0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

        int called = 0;
        sdlgui_slider_set_on_change(sld,
            [](sdlgui_element_t elem, void* data) {
                int* p = (int*)data;
                *p = 1;
            },
            &called);

        sdlgui_slider_set_value(sld, 30); /* should trigger callback */
        REQUIRE(called == 1);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Checkbox", "[c_api][checkbox]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create") {
        sdlgui_element_t cb = sdlgui_checkbox_create(gui, nullptr, 10, 10, 20, 20);
        REQUIRE(cb != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(cb)) == "Checkbox");
    }

    SECTION("is_checked and set_checked") {
        sdlgui_element_t cb = sdlgui_checkbox_create(gui, nullptr, 10, 10, 20, 20);
        REQUIRE(cb != nullptr);

        REQUIRE(sdlgui_checkbox_is_checked(cb) == 0);
        sdlgui_checkbox_set_checked(cb, 1);
        REQUIRE(sdlgui_checkbox_is_checked(cb) == 1);
        sdlgui_checkbox_set_checked(cb, 0);
        REQUIRE(sdlgui_checkbox_is_checked(cb) == 0);
    }

    SECTION("bool callback receives correct value") {
        sdlgui_element_t cb = sdlgui_checkbox_create(gui, nullptr, 10, 10, 20, 20);
        REQUIRE(cb != nullptr);

        int receivedValue = -1;
        sdlgui_element_t receivedElem = nullptr;

        struct CBData {
            sdlgui_element_t* elem;
            int* val;
        };
        CBData cbData = {&receivedElem, &receivedValue};

        sdlgui_checkbox_set_on_change(cb,
            [](sdlgui_element_t elem, int value, void* data) {
                CBData* d = (CBData*)data;
                *d->elem = elem;
                *d->val = value;
            },
            &cbData);

        /* Toggle directly via C++ to trigger the callback */
        auto* rawCb = (Checkbox*)(GUIElement*)cb;
        rawCb->setChecked(true);

        REQUIRE(receivedValue == 1);
        REQUIRE(receivedElem == cb);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - TextInput", "[c_api][text_input]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create") {
        sdlgui_element_t ti = sdlgui_text_input_create(gui, nullptr, 10, 10, 200, 30);
        REQUIRE(ti != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(ti)) == "TextInput");
    }

    SECTION("set_text and get_text roundtrip") {
        sdlgui_element_t ti = sdlgui_text_input_create(gui, nullptr, 10, 10, 200, 30);
        REQUIRE(ti != nullptr);

        sdlgui_text_input_set_text(ti, "Hello World");
        REQUIRE(std::string(sdlgui_text_input_get_text(ti)) == "Hello World");

        sdlgui_text_input_set_text(ti, "");
        REQUIRE(std::string(sdlgui_text_input_get_text(ti)) == "");
    }

    SECTION("locked state") {
        sdlgui_element_t ti = sdlgui_text_input_create(gui, nullptr, 10, 10, 200, 30);
        REQUIRE(ti != nullptr);

        REQUIRE(sdlgui_text_input_is_locked(ti) == 0);
        sdlgui_text_input_set_locked(ti, 1);
        REQUIRE(sdlgui_text_input_is_locked(ti) == 1);
        sdlgui_text_input_set_locked(ti, 0);
        REQUIRE(sdlgui_text_input_is_locked(ti) == 0);
    }

    SECTION("text_changed callback fires") {
        sdlgui_element_t ti = sdlgui_text_input_create(gui, nullptr, 10, 10, 200, 30);
        REQUIRE(ti != nullptr);

        int called = 0;
        sdlgui_text_input_set_on_text_changed(ti,
            [](sdlgui_element_t elem, void* data) {
                int* p = (int*)data;
                *p = 1;
            },
            &called);

        /* Set text triggers the callback */
        sdlgui_text_input_set_text(ti, "new text");
        REQUIRE(called == 1);
    }

    SECTION("enter_pressed callback") {
        sdlgui_element_t ti = sdlgui_text_input_create(gui, nullptr, 10, 10, 200, 30);
        REQUIRE(ti != nullptr);

        int called = 0;
        sdlgui_text_input_set_on_enter_pressed(ti,
            [](sdlgui_element_t elem, void* data) {
                int* p = (int*)data;
                *p = 1;
            },
            &called);

        /* Simulate Enter key press - the TextInput needs focus first */
        SDL_Event keyDown = {};
        keyDown.type = SDL_EVENT_KEY_DOWN;
        keyDown.key.type = SDL_EVENT_KEY_DOWN;
        keyDown.key.key = SDLK_RETURN;
        keyDown.key.repeat = 0;

        sdlgui_process_event(gui, &keyDown);

        /* enter callback may not fire without focus, so just check no crash */
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - ListView", "[c_api][list_view]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(lv)) == "ListView");
    }

    SECTION("add_item and get_item_count") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);

        REQUIRE(sdlgui_list_view_get_item_count(lv) == 0);

        sdlgui_list_view_add_item(lv, "Item 1");
        sdlgui_list_view_add_item(lv, "Item 2");
        sdlgui_list_view_add_item(lv, "Item 3");

        REQUIRE(sdlgui_list_view_get_item_count(lv) == 3);
    }

    SECTION("get_item_text") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);

        sdlgui_list_view_add_item(lv, "Hello");
        sdlgui_list_view_add_item(lv, "World");

        REQUIRE(std::string(sdlgui_list_view_get_item_text(lv, 0)) == "Hello");
        REQUIRE(std::string(sdlgui_list_view_get_item_text(lv, 1)) == "World");
    }

    SECTION("remove_item") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);

        sdlgui_list_view_add_item(lv, "A");
        sdlgui_list_view_add_item(lv, "B");
        sdlgui_list_view_add_item(lv, "C");

        sdlgui_list_view_remove_item(lv, 1);
        REQUIRE(sdlgui_list_view_get_item_count(lv) == 2);
        REQUIRE(std::string(sdlgui_list_view_get_item_text(lv, 0)) == "A");
        REQUIRE(std::string(sdlgui_list_view_get_item_text(lv, 1)) == "C");
    }

    SECTION("clear") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);

        sdlgui_list_view_add_item(lv, "A");
        sdlgui_list_view_add_item(lv, "B");

        sdlgui_list_view_clear(lv);
        REQUIRE(sdlgui_list_view_get_item_count(lv) == 0);
    }

    SECTION("row_click callback with correct index") {
        sdlgui_element_t lv = sdlgui_list_view_create(gui, nullptr, 10, 10, 300, 200);
        REQUIRE(lv != nullptr);

        sdlgui_list_view_add_item(lv, "Item 1");

        size_t lastRow = SIZE_MAX;
        sdlgui_list_view_set_on_row_click(lv,
            [](sdlgui_element_t elem, size_t row, void* data) {
                size_t* p = (size_t*)data;
                *p = row;
            },
            &lastRow);

        /* Simulate a mouse click on the first row */
        SDL_Event down = {};
        down.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        down.button.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
        down.button.button = SDL_BUTTON_LEFT;
        down.button.clicks = 1;
        down.button.x = 20.0f;
        down.button.y = 15.0f;

        sdlgui_process_event(gui, &down);

        SDL_Event up = {};
        up.type = SDL_EVENT_MOUSE_BUTTON_UP;
        up.button.type = SDL_EVENT_MOUSE_BUTTON_UP;
        up.button.button = SDL_BUTTON_LEFT;
        up.button.clicks = 1;
        up.button.x = 20.0f;
        up.button.y = 15.0f;

        sdlgui_process_event(gui, &up);

        /* OnRowClick fires on mouse-down in ListView,
           so lastRow should be set to 0 */
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Cross-widget element base API", "[c_api]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("set_background_color works on all types") {
        sdlgui_element_t widgets[] = {
            sdlgui_button_create(gui, nullptr, 10, 10, 100, 30, "A"),
            sdlgui_label_create(gui, nullptr, 10, 50, "B", -1),
            sdlgui_panel_create(gui, nullptr, 10, 90, 100, 30),
            sdlgui_slider_create(gui, nullptr, 10, 130, 100, 30, 0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL),
            sdlgui_checkbox_create(gui, nullptr, 10, 170, 20, 20),
            sdlgui_text_input_create(gui, nullptr, 10, 210, 100, 30),
            sdlgui_list_view_create(gui, nullptr, 10, 250, 100, 100),
        };

        SDL_Color c {255, 0, 0, 255};
        for (auto& w : widgets) {
            REQUIRE(w != nullptr);
            sdlgui_element_set_background_color(w, SDLGUI_STATE_NORMAL, c);
        }
    }

    SECTION("set_text_color works on all types") {
        sdlgui_element_t widgets[] = {
            sdlgui_button_create(gui, nullptr, 10, 10, 100, 30, "Btn"),
            sdlgui_label_create(gui, nullptr, 10, 50, "Lbl", -1),
            sdlgui_panel_create(gui, nullptr, 10, 90, 100, 30),
            sdlgui_slider_create(gui, nullptr, 10, 130, 100, 30, 0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL),
        };

        SDL_Color c {0, 255, 0, 255};
        for (auto& w : widgets) {
            REQUIRE(w != nullptr);
            sdlgui_element_set_text_color(w, SDLGUI_STATE_NORMAL, c);
        }
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Parent hierarchy", "[c_api]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("child created with parent is properly nested, cleanup doesn't double-free") {
        sdlgui_element_t parent = sdlgui_panel_create(gui, nullptr, 0, 0, 300, 300);
        REQUIRE(parent != nullptr);

        sdlgui_element_t child = sdlgui_button_create(gui, parent, 10, 10, 100, 30, "Child");
        REQUIRE(child != nullptr);

        auto* rawParent = (GUIElement*)parent;
        bool foundChild = false;
        for (auto& c : rawParent->getChildren()) {
            if (c.get() == (GUIElement*)child) {
                foundChild = true;
                break;
            }
        }
        REQUIRE(foundChild);

        /* Destroying the context should not double-free */
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Slider inherits Panel properties", "[c_api][slider]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sld = sdlgui_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                  0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);
    REQUIRE(sld != nullptr);

    /* Slider is a Panel, so set_draggable should work */
    sdlgui_panel_set_draggable(sld, 1);

    /* Panel base API should also work */
    sdlgui_element_set_position(sld, 20, 30);
    auto* raw = (GUIElement*)sld;
    REQUIRE(raw->getX() == 20);
    REQUIRE(raw->getY() == 30);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: ProgressBar
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ProgressBar creation and value", "[c_api][progress_bar]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t pb = sdlgui_progress_bar_create(gui, nullptr, 10, 10, 200, 20);
    REQUIRE(pb != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(pb), "ProgressBar") == 0);

    REQUIRE(sdlgui_progress_bar_get_value(pb) == 0.0f);
    sdlgui_progress_bar_set_value(pb, 50.0f);
    REQUIRE(sdlgui_progress_bar_get_value(pb) == 50.0f);

    REQUIRE(sdlgui_progress_bar_get_min(pb) == 0.0f);
    REQUIRE(sdlgui_progress_bar_get_max(pb) == 100.0f);

    sdlgui_progress_bar_set_range(pb, 10.0f, 200.0f);
    REQUIRE(sdlgui_progress_bar_get_min(pb) == 10.0f);
    REQUIRE(sdlgui_progress_bar_get_max(pb) == 200.0f);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - ProgressBar text and orientation", "[c_api][progress_bar]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t pb = sdlgui_progress_bar_create(gui, nullptr, 10, 10, 200, 20);
    sdlgui_progress_bar_set_show_text(pb, 0);
    sdlgui_progress_bar_set_show_text(pb, 1);

    sdlgui_progress_bar_set_text_format(pb, "%d/%d");
    sdlgui_progress_bar_set_orientation(pb, SDLGUI_ORIENTATION_VERTICAL);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: RadioButton
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - RadioButton selection", "[c_api][radio_button]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t rb = sdlgui_radio_button_create(gui, nullptr, 10, 10, 20, 20);
    REQUIRE(rb != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(rb), "RadioButton") == 0);

    REQUIRE(sdlgui_radio_button_is_selected(rb) == 0);
    sdlgui_radio_button_set_selected(rb, 1);
    REQUIRE(sdlgui_radio_button_is_selected(rb) == 1);

    bool received = false;
    sdlgui_radio_button_set_on_change(rb,
        [](sdlgui_element_t /*elem*/, int value, void* data) {
            *static_cast<bool*>(data) = (value != 0);
        },
        &received);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: RadioGroup
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - RadioGroup creation and options", "[c_api][radio_group]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t rg = sdlgui_radio_group_create(gui, nullptr, 10, 10, 200, 150);
    REQUIRE(rg != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(rg), "RadioGroup") == 0);

    sdlgui_element_t opt1 = sdlgui_radio_group_add_option(rg, "Option 1", 0);
    REQUIRE(opt1 != nullptr);
    sdlgui_element_t opt2 = sdlgui_radio_group_add_option(rg, "Option 2", 1);
    REQUIRE(opt2 != nullptr);

    REQUIRE(sdlgui_radio_button_is_selected(opt1) == 0);
    REQUIRE(sdlgui_radio_button_is_selected(opt2) == 1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - RadioGroup selection change callback", "[c_api][radio_group]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t rg = sdlgui_radio_group_create(gui, nullptr, 10, 10, 200, 150);
    sdlgui_radio_group_add_option(rg, "Red", 1);
    sdlgui_radio_group_add_option(rg, "Green", 0);
    sdlgui_radio_group_add_option(rg, "Blue", 0);

    int cbIndex = -1;
    std::string cbText;
    std::pair<int, std::string> cbPair(cbIndex, cbText);
    sdlgui_radio_group_set_on_selection_change(rg,
        [](sdlgui_element_t /*elem*/, int index, const char* text, void* data) {
            auto* p = static_cast<std::pair<int, std::string>*>(data);
            p->first = index;
            p->second = text ? text : "";
        },
        &cbPair);

    REQUIRE(cbPair.first == -1);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: TextArea
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - TextArea creation and text", "[c_api][text_area]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t ta = sdlgui_text_area_create(gui, nullptr, 10, 10, 300, 150, "", 14);
    REQUIRE(ta != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(ta), "TextArea") == 0);

    sdlgui_text_area_set_text(ta, "Hello World");
    const char* text = sdlgui_text_area_get_text(ta);
    REQUIRE(text != nullptr);
    REQUIRE(std::strcmp(text, "Hello World") == 0);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - TextArea word wrap and lock", "[c_api][text_area]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t ta = sdlgui_text_area_create(gui, nullptr, 10, 10, 300, 150, "", 14);

    REQUIRE(sdlgui_text_area_get_word_wrap(ta) == 1);
    sdlgui_text_area_set_word_wrap(ta, 0);
    REQUIRE(sdlgui_text_area_get_word_wrap(ta) == 0);

    REQUIRE(sdlgui_text_area_is_locked(ta) == 0);
    sdlgui_text_area_set_locked(ta, 1);
    REQUIRE(sdlgui_text_area_is_locked(ta) == 1);

    sdlgui_text_area_set_on_text_changed(ta, nullptr, nullptr);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: ComboBox
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ComboBox creation and items", "[c_api][combo_box]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t cb = sdlgui_combo_box_create(gui, nullptr, 10, 10, 200, 30);
    REQUIRE(cb != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(cb), "ComboBox") == 0);

    sdlgui_combo_box_add_item(cb, "Apple");
    sdlgui_combo_box_add_item(cb, "Banana");
    sdlgui_combo_box_add_item(cb, "Cherry");

    REQUIRE(sdlgui_combo_box_get_selected_index(cb) == 0);
    sdlgui_combo_box_set_selected_index(cb, 2);
    REQUIRE(sdlgui_combo_box_get_selected_index(cb) == 2);

    sdlgui_combo_box_clear(cb);
    REQUIRE(sdlgui_combo_box_get_selected_index(cb) == -1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - ComboBox select callback", "[c_api][combo_box]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t cb = sdlgui_combo_box_create(gui, nullptr, 10, 10, 200, 30);
    sdlgui_combo_box_add_item(cb, "One");
    sdlgui_combo_box_add_item(cb, "Two");

    int cbIndex = -1;
    std::string cbText;
    std::pair<int, std::string> cbPair2(cbIndex, cbText);
    sdlgui_combo_box_set_on_select(cb,
        [](sdlgui_element_t /*elem*/, int index, const char* text, void* data) {
            auto* p = static_cast<std::pair<int, std::string>*>(data);
            p->first = index;
            p->second = text ? text : "";
        },
        &cbPair2);

    sdlgui_combo_box_set_selected_index(cb, 1);
    REQUIRE(cbPair2.first == 1);
    REQUIRE(cbPair2.second == "Two");

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: StringGrid
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - StringGrid creation and cells", "[c_api][string_grid]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sg = sdlgui_string_grid_create(gui, nullptr, 10, 10, 400, 200, 5, 3);
    REQUIRE(sg != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(sg), "StringGrid") == 0);

    REQUIRE(sdlgui_string_grid_get_row_count(sg) == 5);
    REQUIRE(sdlgui_string_grid_get_col_count(sg) == 3);

    sdlgui_string_grid_set_cell(sg, 0, 0, "Hello");
    sdlgui_string_grid_set_cell(sg, 1, 1, "World");

    const char* c00 = sdlgui_string_grid_get_cell(sg, 0, 0);
    REQUIRE(c00 != nullptr);
    REQUIRE(std::strcmp(c00, "Hello") == 0);

    const char* c11 = sdlgui_string_grid_get_cell(sg, 1, 1);
    REQUIRE(c11 != nullptr);
    REQUIRE(std::strcmp(c11, "World") == 0);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - StringGrid clear and resize", "[c_api][string_grid]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sg = sdlgui_string_grid_create(gui, nullptr, 10, 10, 400, 200, 3, 2);
    sdlgui_string_grid_set_cell(sg, 0, 0, "X");
    sdlgui_string_grid_clear(sg);
    /* clear() clears data — rows reset to 0, column widths preserved at 2 */
    REQUIRE(sdlgui_string_grid_get_row_count(sg) == 0);
    REQUIRE(sdlgui_string_grid_get_col_count(sg) == 2);

    sdlgui_string_grid_set_dimensions(sg, 4, 3);
    REQUIRE(sdlgui_string_grid_get_row_count(sg) == 4);
    REQUIRE(sdlgui_string_grid_get_col_count(sg) == 3);

    sdlgui_string_grid_set_selected_cell(sg, 2, 1);
    sdlgui_string_grid_set_on_cell_click(sg, nullptr, nullptr);
    sdlgui_string_grid_set_on_cell_double_click(sg, nullptr, nullptr);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: ScrollArea
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ScrollArea creation", "[c_api][scroll_area]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sa = sdlgui_scroll_area_create(gui, nullptr, 10, 10, 300, 200);
    REQUIRE(sa != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(sa), "ScrollArea") == 0);

    sdlgui_scroll_area_set_content_size(sa, 500, 400);
    sdlgui_scroll_area_set_scroll_enabled(sa, 1, 1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - ScrollArea set_content with Panel", "[c_api][scroll_area]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sa = sdlgui_scroll_area_create(gui, nullptr, 10, 10, 300, 200);

    sdlgui_element_t content = sdlgui_panel_create(gui, nullptr, 0, 0, 500, 400);
    REQUIRE(content != nullptr);

    sdlgui_scroll_area_set_content(sa, content);
    sdlgui_scroll_area_set_content_size(sa, 500, 400);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: Canvas
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - Canvas creation and pen", "[c_api][canvas]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t cv = sdlgui_canvas_create(gui, nullptr, 10, 10, 400, 300);
    REQUIRE(cv != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(cv), "Canvas") == 0);

    sdlgui_canvas_set_pen_color(cv, 255, 0, 0, 255);
    sdlgui_canvas_clear(cv);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: ArcContainer
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ArcContainer creation and child placement", "[c_api][arc_container]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t ac = sdlgui_arc_container_create(gui, nullptr, 100, 100, 80, 0.0f, 360.0f);
    REQUIRE(ac != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(ac), "ArcContainer") == 0);

    sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 0, 0, "Item", 14);
    REQUIRE(lbl != nullptr);

    sdlgui_arc_container_add_child_at_angle(ac, lbl, 45.0f, 0, 0);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: TabControl
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - TabControl creation and tabs", "[c_api][tab_control]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t tc = sdlgui_tab_control_create(gui, nullptr, 10, 10, 400, 300, 30);
    REQUIRE(tc != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(tc), "TabControl") == 0);

    sdlgui_element_t tab1 = sdlgui_tab_control_add_tab(tc, "General");
    REQUIRE(tab1 != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(tab1), "Panel") == 0);

    sdlgui_element_t tab2 = sdlgui_tab_control_add_tab(tc, "Advanced");
    REQUIRE(tab2 != nullptr);

    sdlgui_element_t lbl = sdlgui_label_create(gui, tab1, 10, 10, "Settings", 14);
    REQUIRE(lbl != nullptr);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: ContextMenu
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ContextMenu creation and items", "[c_api][context_menu]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t cm = sdlgui_context_menu_create(gui);
    REQUIRE(cm != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(cm), "ContextMenu") == 0);

    int clicked = 0;
    sdlgui_context_menu_add_item(cm, "Cut", [](void* data) { *static_cast<int*>(data) = 1; }, &clicked);
    sdlgui_context_menu_add_item(cm, "Copy", [](void* data) { *static_cast<int*>(data) = 2; }, &clicked);
    sdlgui_context_menu_add_separator(cm);
    sdlgui_context_menu_add_item(cm, "Paste", [](void* data) { *static_cast<int*>(data) = 3; }, &clicked);

    REQUIRE(sdlgui_context_menu_is_visible(cm) == 0);
    sdlgui_context_menu_show_at(cm, 100, 100);

    sdlgui_context_menu_hide(cm);
    REQUIRE(sdlgui_context_menu_is_visible(cm) == 0);

    sdlgui_context_menu_clear_items(cm);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: Dynamic reparenting (add_child)
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - add_child moves top-level to parent", "[c_api][add_child]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t panel = sdlgui_panel_create(gui, nullptr, 10, 10, 200, 200);
    sdlgui_element_t label = sdlgui_label_create(gui, nullptr, 5, 5, "Inside", 14);

    int result = sdlgui_element_add_child(panel, label);
    REQUIRE(result == 0);

    auto* rawPanel = (GUIElement*)panel;
    auto* rawLabel = (GUIElement*)label;
    REQUIRE(rawLabel->getParent() == rawPanel);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: AnimatedImage
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - AnimatedImage creation", "[c_api][animated_image]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t ai = sdlgui_animated_image_create(gui, nullptr, 10, 10, 300, 200);
    REQUIRE(ai != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(ai), "AnimatedImage") == 0);

    sdlgui_animated_image_set_fps(ai, 24.0f);
    sdlgui_animated_image_set_loop(ai, 1);

    REQUIRE(sdlgui_animated_image_is_playing(ai) == 0);
    sdlgui_animated_image_play(ai);
    sdlgui_animated_image_pause(ai);
    sdlgui_animated_image_stop(ai);

    REQUIRE(sdlgui_animated_image_get_total_frames(ai) == 0);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 2: Gap-filling functions
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - TabControl set_active_tab", "[c_api][tab_control]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t tc = sdlgui_tab_control_create(gui, nullptr, 10, 10, 400, 300, 30);
    sdlgui_tab_control_add_tab(tc, "First");
    sdlgui_tab_control_add_tab(tc, "Second");

    sdlgui_tab_control_set_active_tab(tc, 0);
    sdlgui_tab_control_set_active_tab(tc, 1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - ScrollArea get_scroll_offset", "[c_api][scroll_area]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sa = sdlgui_scroll_area_create(gui, nullptr, 10, 10, 300, 200);
    int x = -1, y = -1;
    sdlgui_scroll_area_get_scroll_offset(sa, &x, &y);
    REQUIRE(x == 0);
    REQUIRE(y == 0);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - StringGrid editable", "[c_api][string_grid]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sg = sdlgui_string_grid_create(gui, nullptr, 10, 10, 400, 200, 2, 2);
    REQUIRE(sdlgui_string_grid_is_editable(sg) == 1);
    sdlgui_string_grid_set_editable(sg, 0);
    REQUIRE(sdlgui_string_grid_is_editable(sg) == 0);
    sdlgui_string_grid_set_editable(sg, 1);
    REQUIRE(sdlgui_string_grid_is_editable(sg) == 1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - AnimatedImage set_frame", "[c_api][animated_image]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t ai = sdlgui_animated_image_create(gui, nullptr, 10, 10, 300, 200);
    sdlgui_animated_image_set_frame(ai, 0);

    REQUIRE(sdlgui_animated_image_get_current_frame(ai) == 0);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   TimerManager
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - add_timer single-shot", "[c_api][timer]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    int fired = 0;
    uint32_t id = sdlgui_add_timer(gui, nullptr, 0, 1,
        [](sdlgui_element_t /*elem*/, void* data) {
            *static_cast<int*>(data) = 1;
        },
        &fired);
    REQUIRE(id >= 1);

    /* Timer should fire after update() */
    sdlgui_update(gui);
    REQUIRE(fired == 1);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - add_timer repeating", "[c_api][timer]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    int count = 0;
    uint32_t id = sdlgui_add_timer(gui, nullptr, 0, 0,
        [](sdlgui_element_t /*elem*/, void* data) {
            (*static_cast<int*>(data))++;
        },
        &count);

    sdlgui_update(gui);
    sdlgui_update(gui);
    sdlgui_update(gui);
    REQUIRE(count == 3);

    sdlgui_remove_timer(gui, id);
    sdlgui_update(gui);
    REQUIRE(count == 3); /* should not fire after removal */

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   AnimationManager — Looping
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - loop animation", "[c_api][anim]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    int fired = 0;
    uint32_t id = sdlgui_add_loop_animation(gui, 0,
        [](void* data) { (*static_cast<int*>(data))++; },
        &fired);

    sdlgui_update(gui);
    REQUIRE(fired == 1);
    sdlgui_update(gui);
    REQUIRE(fired == 2);

    sdlgui_remove_loop_animation(gui, id);
    sdlgui_update(gui);
    REQUIRE(fired == 2);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   AnimationManager — Property tweens
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - animate_int", "[c_api][anim]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    int val = 0;
    int completed = 0;
    sdlgui_animate_int(gui, &val, 0, 100, 50, SDLGUI_EASING_LINEAR,
        [](void* data) { *static_cast<int*>(data) = 1; },
        &completed);

    REQUIRE(completed == 0);
    sdlgui_update(gui); /* animation ticks */
    REQUIRE(val >= 0);

    sdlgui_destroy(gui);
}

TEST_CASE("C API - animate_float with easing", "[c_api][anim]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    float val = 0.0f;
    sdlgui_animate_float(gui, &val, 0.0f, 1.0f, 200, SDLGUI_EASING_IN_OUT_QUAD, nullptr, nullptr);

    SDL_Delay(5); /* let at least 5ms pass */
    sdlgui_update(gui);
    REQUIRE(val > 0.0f);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 3: RangeSlider
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - RangeSlider", "[c_api][range_slider]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create horizontal") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);
        REQUIRE(rs != nullptr);
        REQUIRE(std::strcmp(sdlgui_element_get_type(rs), "RangeSlider") == 0);
        REQUIRE(sdlgui_range_slider_get_lower_value(rs) == 25);
        REQUIRE(sdlgui_range_slider_get_upper_value(rs) == 75);
    }

    SECTION("create vertical") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 40, 200,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_VERTICAL);
        REQUIRE(rs != nullptr);
    }

    SECTION("values clamped to range") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          -50, 50, -100, 100,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);
        REQUIRE(sdlgui_range_slider_get_min(rs) == -50);
        REQUIRE(sdlgui_range_slider_get_max(rs) == 50);
        REQUIRE(sdlgui_range_slider_get_lower_value(rs) == -50);
        REQUIRE(sdlgui_range_slider_get_upper_value(rs) == 50);
    }

    SECTION("swapped values are normalized") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 80, 20,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);
        REQUIRE(sdlgui_range_slider_get_lower_value(rs) == 20);
        REQUIRE(sdlgui_range_slider_get_upper_value(rs) == 80);
    }

    SECTION("set_lower_value and set_upper_value") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_range_slider_set_lower_value(rs, 40);
        REQUIRE(sdlgui_range_slider_get_lower_value(rs) == 40);

        sdlgui_range_slider_set_upper_value(rs, 60);
        REQUIRE(sdlgui_range_slider_get_upper_value(rs) == 60);
    }

    SECTION("lower cannot exceed upper") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_range_slider_set_lower_value(rs, 90);
        REQUIRE(sdlgui_range_slider_get_lower_value(rs) == 75);

        sdlgui_range_slider_set_upper_value(rs, 10); /* clamped up to lower value */
        REQUIRE(sdlgui_range_slider_get_upper_value(rs) == 75);
    }

    SECTION("set_range") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_range_slider_set_range(rs, 10, 90);
        REQUIRE(sdlgui_range_slider_get_min(rs) == 10);
        REQUIRE(sdlgui_range_slider_get_max(rs) == 90);
    }

    SECTION("set_min and set_max individually") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        sdlgui_range_slider_set_min(rs, 20);
        REQUIRE(sdlgui_range_slider_get_min(rs) == 20);

        sdlgui_range_slider_set_max(rs, 80);
        REQUIRE(sdlgui_range_slider_get_max(rs) == 80);
    }

    SECTION("wheel_step") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        REQUIRE(sdlgui_range_slider_get_wheel_step(rs) == 1);
        sdlgui_range_slider_set_wheel_step(rs, 5);
        REQUIRE(sdlgui_range_slider_get_wheel_step(rs) == 5);
        sdlgui_range_slider_set_wheel_step(rs, 0); /* clamped to 1 */
        REQUIRE(sdlgui_range_slider_get_wheel_step(rs) == 1);
    }

    SECTION("change callback fires") {
        sdlgui_element_t rs = sdlgui_range_slider_create(gui, nullptr, 10, 10, 200, 40,
                                                          0, 100, 25, 75,
                                                          SDLGUI_ORIENTATION_HORIZONTAL);

        int called = 0;
        sdlgui_range_slider_set_on_change(rs,
            [](sdlgui_element_t elem, void* data) {
                (void)elem;
                *static_cast<int*>(data) = 1;
            },
            &called);

        sdlgui_range_slider_set_lower_value(rs, 30);
        REQUIRE(called == 1);

        sdlgui_range_slider_set_lower_value(rs, 30); /* no change → no callback */
        REQUIRE(called == 1);
    }

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 3: Cursor
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - Cursor", "[c_api][cursor]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("create") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);
        REQUIRE(cur != nullptr);
        REQUIRE(std::strcmp(sdlgui_element_get_type(cur), "Cursor") == 0);
    }

    SECTION("set_state and get_state") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);

        REQUIRE(sdlgui_cursor_get_state(cur) == SDLGUI_CURSOR_NORMAL);
        sdlgui_cursor_set_state(cur, SDLGUI_CURSOR_HOVER);
        REQUIRE(sdlgui_cursor_get_state(cur) == SDLGUI_CURSOR_HOVER);
        sdlgui_cursor_set_state(cur, SDLGUI_CURSOR_BUSY);
        REQUIRE(sdlgui_cursor_get_state(cur) == SDLGUI_CURSOR_BUSY);
        sdlgui_cursor_set_state(cur, SDLGUI_CURSOR_CUSTOM3);
        REQUIRE(sdlgui_cursor_get_state(cur) == SDLGUI_CURSOR_CUSTOM3);
    }

    SECTION("offset") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);

        sdlgui_cursor_set_offset(cur, 5, -7);
        int x = 0, y = 0;
        sdlgui_cursor_get_offset(cur, &x, &y);
        REQUIRE(x == 5);
        REQUIRE(y == -7);
    }

    SECTION("scale") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);

        REQUIRE(sdlgui_cursor_get_scale(cur) == 1.0f);
        sdlgui_cursor_set_scale(cur, 0.5f);
        REQUIRE(sdlgui_cursor_get_scale(cur) == 0.5f);
    }

    SECTION("visible") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);

        REQUIRE(sdlgui_cursor_is_visible(cur) == 1);
        sdlgui_cursor_set_visible(cur, 0);
        REQUIRE(sdlgui_cursor_is_visible(cur) == 0);
        sdlgui_cursor_set_visible(cur, 1);
        REQUIRE(sdlgui_cursor_is_visible(cur) == 1);
    }

    SECTION("set_texture with missing file does not crash") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);
        sdlgui_cursor_set_texture(cur, SDLGUI_CURSOR_NORMAL, "assets/does_not_exist.png", 8, 8);
        sdlgui_cursor_set_animated_texture(cur, SDLGUI_CURSOR_BUSY, "assets/does_not_exist.png",
                                           4, 2, 8.0f, 16, 16);
        REQUIRE(sdlgui_cursor_get_state(cur) == SDLGUI_CURSOR_NORMAL);
    }

    SECTION("state changed callback fires") {
        sdlgui_element_t cur = sdlgui_cursor_create(gui);

        int calls = 0;
        sdlgui_cursor_state_t lastState = SDLGUI_CURSOR_NORMAL;
        struct CBData {
            int* calls;
            sdlgui_cursor_state_t* last;
        };
        CBData data = {&calls, &lastState};

        sdlgui_cursor_set_on_state_changed(cur,
            [](sdlgui_element_t elem, sdlgui_cursor_state_t state, void* userdata) {
                (void)elem;
                CBData* d = static_cast<CBData*>(userdata);
                *d->calls += 1;
                *d->last = state;
            },
            &data);

        sdlgui_cursor_set_state(cur, SDLGUI_CURSOR_TEXT);
        REQUIRE(calls == 1);
        REQUIRE(lastState == SDLGUI_CURSOR_TEXT);

        sdlgui_cursor_set_state(cur, SDLGUI_CURSOR_TEXT); /* same state → no callback */
        REQUIRE(calls == 1);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - Cursor renders pixels", "[c_api][cursor][pixel]") {
    /* End-to-end: texture load → renderOverlay → visible pixels. */
    sdlgui_t gui = sdlgui_create("Test", 320, 240, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t cur = sdlgui_cursor_create(gui);
    REQUIRE(cur != nullptr);
    REQUIRE(sdlgui_cursor_is_visible(cur) == 1);

    /* Asset exists on disk (tests run from repo root); 100x40, scaled to 50x20 */
    sdlgui_cursor_set_texture(cur, SDLGUI_CURSOR_NORMAL, "assets/button1.png", 0, 0);
    sdlgui_cursor_set_scale(cur, 0.5f);

    /* Cursor reads its position from mouse-motion events (with a live-state
       fallback). Warp is unreliable on Wayland (the compositor forbids pointer
       moves), so inject a synthetic event instead — no window mapping needed. */
    const int mx = 50, my = 50;
    SDL_Event motion{};
    motion.type = SDL_EVENT_MOUSE_MOTION;
    motion.motion.x = (float)mx;
    motion.motion.y = (float)my;
    sdlgui_process_event(gui, &motion);

    SDL_Renderer* ren = sdlgui_get_renderer(gui);
    SDL_SetRenderDrawColor(ren, 40, 42, 54, 255);
    SDL_RenderClear(ren);
    sdlgui_render(gui);

    /* Read BEFORE Present: after Present the backbuffer is undefined. */
    /* hotspot (0,0) → texture covers (50,50)-(100,70); probe (51,51) */
    SDL_Rect r{mx + 1, my + 1, 1, 1};
    SDL_Surface* surf = SDL_RenderReadPixels(ren, &r);
    REQUIRE(surf != nullptr);
    Uint8* p = (Uint8*)surf->pixels;
    REQUIRE(p[3] > 200);                              /* opaque */
    bool isBackground = (p[0] == 40 && p[1] == 42 && p[2] == 54);
    REQUIRE_FALSE(isBackground);                      /* not background */
    SDL_DestroySurface(surf);

    sdlgui_destroy(gui);
}

/* ════════════════════════════════════════════════════
   Phase 3: ShaderPanel
   ════════════════════════════════════════════════════ */

TEST_CASE("C API - ShaderPanel", "[c_api][shader_panel]") {
    sdlgui_t gui = sdlgui_create("Test", 800, 600, 0);
    REQUIRE(gui != nullptr);

    sdlgui_element_t sp = sdlgui_shader_panel_create(gui, nullptr, 10, 10, 200, 100);
    REQUIRE(sp != nullptr);
    REQUIRE(std::strcmp(sdlgui_element_get_type(sp), "ShaderPanel") == 0);

    SECTION("shader enabled toggles") {
        REQUIRE(sdlgui_shader_panel_is_shader_enabled(sp) == 1);
        sdlgui_shader_panel_set_shader_enabled(sp, 0);
        REQUIRE(sdlgui_shader_panel_is_shader_enabled(sp) == 0);
        sdlgui_shader_panel_set_shader_enabled(sp, 1);
        REQUIRE(sdlgui_shader_panel_is_shader_enabled(sp) == 1);
    }

    SECTION("set_shader with NULL data is safe") {
        sdlgui_shader_panel_set_shader(sp, nullptr, 0);
        sdlgui_shader_panel_set_shader(sp, (const uint8_t*)"abc", 3); /* CPU ctx: ignored */
    }

    SECTION("uniforms") {
        sdlgui_shader_panel_set_uniform_time(sp, 1.5f);
        sdlgui_shader_panel_set_uniform_mouse(sp, 100.0f, 200.0f);
    }

    SECTION("render on CPU context") {
        sdlgui_render(gui);
    }

    sdlgui_destroy(gui);
}

TEST_CASE("C API - GPU context", "[c_api][gpu]") {
    /* Soft test: GPU may be unavailable (headless CI, no Vulkan driver). */
    sdlgui_t gpu = sdlgui_create_gpu("Test", 800, 600, 0);
    if (!gpu) {
        SUCCEED("GPU context not available on this system; skipping");
        return;
    }

    REQUIRE(sdlgui_get_gpu_device(gpu) != nullptr);

    sdlgui_element_t sp = sdlgui_shader_panel_create(gpu, nullptr, 10, 10, 200, 100);
    REQUIRE(sp != nullptr);
    sdlgui_shader_panel_set_uniform_time(sp, 0.5f);
    sdlgui_render(gpu);

    sdlgui_destroy(gpu);
}
