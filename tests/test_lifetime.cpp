#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include <cstring>

#include "test_helper.hpp"
#include "../src/button.hpp"
#include "../src/checkbox.hpp"
#include "../src/combobox.hpp"
#include "../src/gui_manager.hpp"
#include "../src/label.hpp"
#include "../src/list_view.hpp"
#include "../src/panel.hpp"
#include "../src/slider.hpp"
#include "../src/text_input.hpp"
#include "../src/editor/editor_state.hpp"
#include "../src/editor/preview_window.hpp"
#include "../src/widget_factory.hpp"

extern "C" {
#include "../src/sdl_gui.h"
}

// ═══════════════════════════════════════════════════════════════════
// Point 5 — Lifetime: SlotMap/generation handles, WidgetFactory,
// editor diff, C-API boundary checks.
// ═══════════════════════════════════════════════════════════════════

TEST_CASE("Lifetime - ElementRef auto-nulls after destroy", "[lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("child ref dies with deleted parent") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        auto* label = new Label(manager, 10, 10, "hi", -1);
        panel->addChild(std::unique_ptr<GUIElement>(label));
        auto ref = manager.makeRef(label);
        REQUIRE(ref);
        Panel* panelRaw = panel.get();
        manager.addElement(std::move(panel));

        panelRaw->markForDeletion();
        manager.cleanup();
        manager.cleanup();

        REQUIRE(!ref);
        REQUIRE(ref.get() == nullptr);
        REQUIRE(!manager.isElementAlive(label));
    }

    SECTION("top-level ref dies after cleanup") {
        auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "x");
        auto ref = manager.makeRef(btn.get());
        Button* raw = btn.get();
        manager.addElement(std::move(btn));
        REQUIRE(ref);

        raw->markForDeletion();
        manager.cleanup();

        REQUIRE(!ref);
        REQUIRE(!manager.isElementAlive(raw));
    }

    SECTION("handle does not alias a new element (no ABA)") {
        auto first = std::make_unique<Panel>(manager, 0, 0, 50, 50);
        ElementHandle h = manager.getHandle(first.get());
        REQUIRE(h.valid());
        Panel* firstRaw = first.get();
        manager.addElement(std::move(first));
        firstRaw->markForDeletion();
        manager.cleanup();

        REQUIRE(!manager.isHandleAlive(h));
        REQUIRE(manager.resolve(h) == nullptr);

        // New element must get a resolvable handle even if it reuses memory.
        auto second = std::make_unique<Panel>(manager, 0, 0, 50, 50);
        ElementHandle h2 = manager.getHandle(second.get());
        manager.addElement(std::move(second));
        REQUIRE(manager.isHandleAlive(h2));
        REQUIRE(manager.resolve(h2) != nullptr);
        // Old handle stays dead regardless of address reuse.
        REQUIRE(!manager.isHandleAlive(h));
    }

    SECTION("isElementAlive false for never-registered and null") {
        REQUIRE(!manager.isElementAlive(nullptr));
        Panel stack(manager, 0, 0, 10, 10);  // ctor registers; dtor at scope end
        REQUIRE(manager.isElementAlive(&stack));
    }
}

TEST_CASE("Lifetime - focus and capture self-clear on destroy", "[lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("keyboard focus clears when target destroyed") {
        auto ti = std::make_unique<TextInput>(manager, 10, 10, 150, 30);
        TextInput* raw = ti.get();
        manager.addElement(std::move(ti));
        manager.setKeyboardFocus(raw);
        REQUIRE(manager.getKeyboardFocus() == raw);

        raw->markForDeletion();
        manager.cleanup();

        REQUIRE(manager.getKeyboardFocus() == nullptr);
        // No crash on event/render paths after the target is gone.
        SDL_Event motion = helper.createMouseMotion(5, 5);
        manager.processEvent(motion);
        manager.render();
    }

    SECTION("focus clears when child of deleted parent") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        auto* ti = new TextInput(manager, 10, 10, 150, 30);
        panel->addChild(std::unique_ptr<GUIElement>(ti));
        Panel* panelRaw = panel.get();
        manager.addElement(std::move(panel));
        manager.setKeyboardFocus(ti);
        REQUIRE(manager.getKeyboardFocus() == ti);

        panelRaw->markForDeletion();
        manager.cleanup();
        manager.cleanup();

        REQUIRE(manager.getKeyboardFocus() == nullptr);
    }

    SECTION("mouse capture clears when target destroyed") {
        auto btn = std::make_unique<Button>(manager, 10, 10, 100, 40, "c");
        Button* raw = btn.get();
        manager.addElement(std::move(btn));
        manager.captureMouse(raw);
        REQUIRE(manager.getMouseCapture() == raw);

        raw->markForDeletion();
        manager.cleanup();

        REQUIRE(manager.getMouseCapture() == nullptr);
        SDL_Event motion = helper.createMouseMotion(50, 50);
        REQUIRE_NOTHROW(manager.processEvent(motion));
    }

    SECTION("ElementRef in callback needs no manual guard") {
        auto label = std::make_unique<Label>(manager, 10, 10, "0", -1);
        auto ref = manager.makeRef(label.get());
        Label* raw = label.get();
        manager.addElement(std::move(label));

        // Simulate a menu-item lambda capturing the ref (old code needed
        // explicit isElementAlive guards; the handle does it now).
        auto action = [ref]() -> bool { return static_cast<bool>(ref); };
        REQUIRE(action());

        raw->markForDeletion();
        manager.cleanup();
        REQUIRE(!action());
    }
}

TEST_CASE("Lifetime - WidgetFactory single registry", "[lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("every known type constructs with the right ComponentType") {
        const std::pair<std::string_view, ComponentType> cases[] = {
            {"Panel", ComponentType::Panel},
            {"Button", ComponentType::Button},
            {"Label", ComponentType::Label},
            {"Checkbox", ComponentType::Checkbox},
            {"RadioButton", ComponentType::RadioButton},
            {"RadioGroup", ComponentType::RadioGroup},
            {"Slider", ComponentType::Slider},
            {"RangeSlider", ComponentType::RangeSlider},
            {"StringGrid", ComponentType::StringGrid},
            {"ListView", ComponentType::ListView},
            {"TextInput", ComponentType::TextInput},
            {"TextArea", ComponentType::TextArea},
            {"ComboBox", ComponentType::ComboBox},
            {"TabControl", ComponentType::TabControl},
            {"AnimatedImage", ComponentType::AnimatedImage},
            {"Canvas", ComponentType::Canvas},
            {"ProgressBar", ComponentType::ProgressBar},
            {"ScrollArea", ComponentType::ScrollArea},
            {"ArcContainer", ComponentType::ArcContainer},
        };
        for (auto [name, id] : cases) {
            INFO("type = " << name);
            REQUIRE(WidgetFactory::isKnownType(name));
            auto w = WidgetFactory::createBare(manager, name, 5, 5, 120, 40);
            REQUIRE(w);
            REQUIRE(w->getComponentTypeId() == id);
        }
    }

    SECTION("unknown type -> nullptr, not a silent default") {
        REQUIRE(!WidgetFactory::isKnownType("Nope"));
        REQUIRE(!WidgetFactory::isKnownType(""));
        WidgetProps props;
        REQUIRE(WidgetFactory::create(manager, "Nope", props) == nullptr);
        REQUIRE(WidgetFactory::createBare(manager, "Nope", 0, 0, 10, 10) == nullptr);
    }

    SECTION("scalar props shared by parser and preview") {
        WidgetProps props;
        props.x = 1;
        props.y = 2;
        props.w = 150;
        props.h = 30;
        props.text = "Hello";
        auto btn = WidgetFactory::create(manager, "Button", props);
        REQUIRE(btn);
        REQUIRE(btn->getComponentTypeId() == ComponentType::Button);

        WidgetProps lp;
        lp.text = "Item A";
        auto label = WidgetFactory::create(manager, "Label", lp);
        REQUIRE(label->getComponentTypeId() == ComponentType::Label);

        WidgetProps sp;
        sp.minVal = 10;
        sp.maxVal = 20;
        sp.value = 15;
        auto slider = WidgetFactory::create(manager, "Slider", sp);
        REQUIRE(slider->getComponentTypeId() == ComponentType::Slider);
        REQUIRE(static_cast<Slider*>(slider.get())->getValue() == 15);
    }

    SECTION("default sizes match the editor palette contract") {
        REQUIRE(WidgetFactory::defaultSize("Button") == std::pair<int, int>{120, 40});
        REQUIRE(WidgetFactory::defaultSize("Panel") == std::pair<int, int>{300, 200});
        REQUIRE(WidgetFactory::defaultSize("StringGrid") == std::pair<int, int>{400, 300});
        REQUIRE(WidgetFactory::defaultSize("Unknown") == std::pair<int, int>{100, 50});
    }

    SECTION("1px and layout-critical sizes survive the factory") {
        WidgetProps props;
        props.w = 1;
        props.h = 1;
        auto arc = WidgetFactory::create(manager, "ArcContainer", props);
        REQUIRE(arc);
        REQUIRE(arc->getComponentTypeId() == ComponentType::ArcContainer);
    }
}

TEST_CASE("Lifetime - editor diff preserves widgets in place", "[lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    EditorState state;
    PreviewWindow preview(manager, state, 800, 600);

    SECTION("prop edit updates the same widget (no recreate)") {
        size_t idx = state.addElement("Button", 100, 100);
        preview.syncAll();
        const std::string id = state.getElements()[idx].id;
        GUIElement* before = preview.findWidgetById(id);
        REQUIRE(before != nullptr);

        state.updateElementProperty(idx, "text", "Changed");
        preview.syncAll();

        GUIElement* after = preview.findWidgetById(id);
        REQUIRE(after == before);  // in place — focus/scroll/selection survive
    }

    SECTION("geometry edit keeps the widget") {
        size_t idx = state.addElement("Label", 10, 10);
        preview.syncAll();
        const std::string id = state.getElements()[idx].id;
        GUIElement* before = preview.findWidgetById(id);

        state.updateElementPosition(idx, 200, 150);
        state.updateElementSize(idx, 180, 40);
        preview.syncAll();

        REQUIRE(preview.findWidgetById(id) == before);
        REQUIRE(before->getX() == 200);
        REQUIRE(before->getWidth() == 180);
    }

    SECTION("delete removes the widget even with shifted indices") {
        size_t a = state.addElement("Button", 10, 10);
        size_t b = state.addElement("Label", 50, 50);
        preview.syncAll();
        const std::string idA = state.getElements()[a].id;
        const std::string idB = state.getElements()[b].id;
        REQUIRE(preview.findWidgetById(idA) != nullptr);
        REQUIRE(preview.findWidgetById(idB) != nullptr);

        state.deleteElement(a);  // shifts b's index — id map stays correct
        preview.syncAll();

        REQUIRE(preview.findWidgetById(idA) == nullptr);
        REQUIRE(preview.findWidgetById(idB) != nullptr);
        manager.cleanup();
    }

    SECTION("stale index after delete cannot remove the wrong widget") {
        size_t a = state.addElement("Button", 10, 10);
        state.addElement("Label", 50, 50);
        preview.syncAll();
        const std::string idB = state.getElements()[1].id;

        state.deleteElement(a);
        // onElementDeleted fires with the stale index 0 — must be harmless.
        preview.removeElementWidget(0);

        REQUIRE(preview.findWidgetById(idB) != nullptr);
        manager.cleanup();
    }

    SECTION("structural change recreates only that widget") {
        size_t idx = state.addElement("ComboBox", 10, 10);
        preview.syncAll();
        const std::string id = state.getElements()[idx].id;
        GUIElement* before = preview.findWidgetById(id);
        REQUIRE(before != nullptr);

        state.updateElementProperty(idx, "items", "a,b,c");
        preview.syncAll();
        GUIElement* after = preview.findWidgetById(id);
        REQUIRE(after != nullptr);
        REQUIRE(after != before);
        REQUIRE(static_cast<ComboBox*>(after)->getItemCount() == 3u);
        manager.cleanup();
    }
}

TEST_CASE("Lifetime - C-API boundary checks", "[lifetime]") {
    sdlgui_t gui = sdlgui_create("LifetimeTest", 800, 600, 0);
    REQUIRE(gui != nullptr);

    SECTION("type mismatch records an error instead of UB") {
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 10, 10, 100, 40, "Hi");
        REQUIRE(btn != nullptr);
        REQUIRE(std::string(sdlgui_last_error()) == "");

        // Slider getter on a Button: safe default + error, no crash.
        REQUIRE(sdlgui_slider_get_value(btn) == 0);
        REQUIRE(std::string(sdlgui_last_error()) != "");

        sdlgui_button_set_label(btn, "Yo");
        REQUIRE(std::string(sdlgui_last_error()) == "");
    }

    SECTION("caller-buffer getters copy without dangling") {
        sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 0, 0, "Hello", -1);
        char buf[64] = {};
        size_t n = sdlgui_label_get_text_buf(lbl, buf, sizeof(buf));
        REQUIRE(n == 5u);
        REQUIRE(std::string(buf) == "Hello");

        // Truncation is safe and NUL-terminated.
        char tiny[3] = {};
        n = sdlgui_label_get_text_buf(lbl, tiny, sizeof(tiny));
        REQUIRE(n == 2u);
        REQUIRE(std::string(tiny) == "He");

        // Wrong type: empty + error, buffer untouched-safe.
        sdlgui_element_t btn = sdlgui_button_create(gui, nullptr, 0, 0, 50, 20, "B");
        char buf2[8];
        std::memset(buf2, 'x', sizeof(buf2));
        REQUIRE(sdlgui_label_get_text_buf(btn, buf2, sizeof(buf2)) == 0u);
        REQUIRE(buf2[0] == '\0');
    }

    SECTION("generic factory create + unknown type") {
        sdlgui_element_t cb = sdlgui_create_widget(gui, nullptr, "Checkbox", 5, 5, 150, 25);
        REQUIRE(cb != nullptr);
        REQUIRE(std::string(sdlgui_element_get_type(cb)) == "Checkbox");
        REQUIRE(sdlgui_checkbox_is_checked(cb) == 0);

        sdlgui_element_t bad = sdlgui_create_widget(gui, nullptr, "Nope", 0, 0, 10, 10);
        REQUIRE(bad == nullptr);
        REQUIRE(std::string(sdlgui_last_error()) != "");
    }

    SECTION("is_alive tracks registration") {
        sdlgui_element_t lbl = sdlgui_label_create(gui, nullptr, 0, 0, "T", -1);
        REQUIRE(sdlgui_element_is_alive(gui, lbl) == 1);
        REQUIRE(sdlgui_element_is_alive(gui, nullptr) == 0);
        sdlgui_element_mark_for_deletion(lbl);
        sdlgui_cleanup(gui);
        REQUIRE(sdlgui_element_is_alive(gui, lbl) == 0);
    }

    sdlgui_destroy(gui);
}
