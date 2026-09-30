#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/composite/color_dialog.hpp"
#include "../src/slider.hpp"
#include "../src/text_input.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"

#include "std.hpp"

static std::vector<Slider*> collectSliders(ColorDialog* dlg) {
    std::vector<Slider*> out;
    for (const auto& child : dlg->getChildren()) {
        if (auto* s = dynamic_cast<Slider*>(child.get())) {
            out.push_back(s);
        }
    }
    return out;  // R, G, B, H, S, V in construction order
}

static TextInput* findHexInput(ColorDialog* dlg) {
    for (const auto& child : dlg->getChildren()) {
        // The HEX field is the only TextInput in the dialog.
        if (auto* ti = dynamic_cast<TextInput*>(child.get())) {
            return ti;
        }
    }
    return nullptr;
}

static Button* findButtonByText(ColorDialog* dlg, std::string_view text) {
    for (const auto& child : dlg->getChildren()) {
        if (auto* b = dynamic_cast<Button*>(child.get())) {
            if (b->getText() == text) return b;
        }
    }
    return nullptr;
}

static void clickAt(TestHelper& helper, GUIManager& manager, GUIElement* e) {
    SDL_Point p = e->getAbsolutePosition();
    const int cx = p.x + e->getWidth() / 2;
    const int cy = p.y + e->getHeight() / 2;
    manager.processEvent(helper.createMouseMotion(cx, cy));
    manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, cx, cy));
    manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, cx, cy));
}

TEST_CASE("ColorDialog - RGB/HSV conversion roundtrip", "[color][dialog]") {
    SECTION("primaries and grays are exact") {
        struct Case { uint8_t r, g, b; int h, s, v; };
        const Case cases[] = {
            {255, 0, 0, 0, 100, 100}, {0, 255, 0, 120, 100, 100},
            {0, 0, 255, 240, 100, 100}, {255, 255, 0, 60, 100, 100},
            {0, 255, 255, 180, 100, 100}, {255, 0, 255, 300, 100, 100},
            {0, 0, 0, 0, 0, 0}, {255, 255, 255, 0, 0, 100},
        };
        for (const auto& c : cases) {
            int h = -1, s = -1, v = -1;
            ColorDialog::rgbToHsv(c.r, c.g, c.b, h, s, v);
            REQUIRE(h == c.h);
            REQUIRE(s == c.s);
            REQUIRE(v == c.v);
            uint8_t r = 0, g = 0, b = 0;
            ColorDialog::hsvToRgb(h, s, v, r, g, b);
            REQUIRE(r == c.r);
            REQUIRE(g == c.g);
            REQUIRE(b == c.b);
        }
    }

    SECTION("arbitrary colors roundtrip within +-5 (int-math quantization)") {
        // Each mapping quantizes (8-bit <-> degrees/percent); 1 deg of hue
        // alone shifts a channel by up to ~4 LSB at full saturation.
        const SDL_Color colors[] = {
            {123, 45, 67, 255}, {10, 200, 30, 255}, {250, 128, 64, 255},
            {16, 32, 48, 255}, {200, 200, 100, 255}, {128, 128, 128, 255},
        };
        for (const auto& c : colors) {
            int h = 0, s = 0, v = 0;
            ColorDialog::rgbToHsv(c.r, c.g, c.b, h, s, v);
            uint8_t r = 0, g = 0, b = 0;
            ColorDialog::hsvToRgb(h, s, v, r, g, b);
            REQUIRE(std::abs(static_cast<int>(r) - c.r) <= 5);
            REQUIRE(std::abs(static_cast<int>(g) - c.g) <= 5);
            REQUIRE(std::abs(static_cast<int>(b) - c.b) <= 5);
        }
    }

    SECTION("HEX parse accepts #RRGGBB and RRGGBB, rejects garbage") {
        REQUIRE(ColorDialog::parseHex("#FF8000") == SDL_Color{255, 128, 0, 255});
        REQUIRE(ColorDialog::parseHex("00ff00") == SDL_Color{0, 255, 0, 255});
        REQUIRE(!ColorDialog::parseHex("zzz").has_value());
        REQUIRE(!ColorDialog::parseHex("#12345").has_value());
        REQUIRE(!ColorDialog::parseHex("").has_value());
        REQUIRE(!ColorDialog::parseHex("#GGGGGG").has_value());
    }
}

TEST_CASE("ColorDialog - sync guard and slider wiring", "[color][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    ColorDialog* dlg = ColorDialog::create(manager, "Pick", {255, 0, 0, 255});
    REQUIRE(dlg != nullptr);
    REQUIRE(dlg->isOverlay());
    REQUIRE(dlg->getColor() == SDL_Color{255, 0, 0, 255});
    REQUIRE(dlg->getInitial() == SDL_Color{255, 0, 0, 255});

    auto sliders = collectSliders(dlg);
    REQUIRE(sliders.size() == 6);
    // Red: R=255, H=0, S=100, V=100.
    REQUIRE(sliders[0]->getValue() == 255);
    REQUIRE(sliders[1]->getValue() == 0);
    REQUIRE(sliders[3]->getValue() == 0);
    REQUIRE(sliders[4]->getValue() == 100);
    REQUIRE(sliders[5]->getValue() == 100);
    REQUIRE(findHexInput(dlg)->getText() == "#FF0000");

    SECTION("moving a slider re-syncs everything (no infinite loop)") {
        sliders[1]->setValue(128);  // G: red -> orange
        REQUIRE(dlg->getColor() == SDL_Color{255, 128, 0, 255});
        REQUIRE(findHexInput(dlg)->getText() == "#FF8000");
        // H follows (~30deg for 255,128,0).
        REQUIRE(sliders[3]->getValue() == 30);
    }

    SECTION("moving an HSV slider recolors from HSV") {
        sliders[3]->setValue(120);  // H: red -> green
        SDL_Color c = dlg->getColor();
        REQUIRE(c.r == 0);
        REQUIRE(c.g == 255);
        REQUIRE(c.b == 0);
        REQUIRE(sliders[0]->getValue() == 0);
        REQUIRE(sliders[1]->getValue() == 255);
    }

    SECTION("setColor pushes to all widgets") {
        dlg->setColor({0, 0, 255, 255});
        REQUIRE(sliders[2]->getValue() == 255);
        REQUIRE(sliders[0]->getValue() == 0);
        REQUIRE(sliders[3]->getValue() == 240);
        REQUIRE(findHexInput(dlg)->getText() == "#0000FF");
        REQUIRE(dlg->getColor() == SDL_Color{0, 0, 255, 255});
    }

    manager.setKeyboardFocus(nullptr);
    dlg->close();
    manager.cleanup();
}

TEST_CASE("ColorDialog - HEX input drives the color, bad input ignored", "[color][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    ColorDialog* dlg = ColorDialog::create(manager, "Pick", {255, 0, 0, 255});
    TextInput* hex = findHexInput(dlg);
    REQUIRE(hex != nullptr);

    hex->setText(std::string{"#00FF00"});
    REQUIRE(dlg->getColor() == SDL_Color{0, 255, 0, 255});

    hex->setText(std::string{"not-a-color"});
    REQUIRE(dlg->getColor() == SDL_Color{0, 255, 0, 255});
    REQUIRE(dlg->isOpen());

    manager.setKeyboardFocus(nullptr);
    dlg->close();
    manager.cleanup();
}

TEST_CASE("ColorDialog - preset click, OK and Cancel", "[color][dialog]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("preset click sets the color") {
        ColorDialog* dlg = ColorDialog::create(manager, "Pick", {255, 0, 0, 255});
        // First preset is black.
        Button* first = nullptr;
        for (const auto& child : dlg->getChildren()) {
            if (auto* b = dynamic_cast<Button*>(child.get())) {
                if (b->getText().empty()) { first = b; break; }
            }
        }
        REQUIRE(first != nullptr);
        clickAt(helper, manager, first);
        REQUIRE(dlg->getColor() == SDL_Color{0, 0, 0, 255});
        dlg->close();
        manager.cleanup();
    }

    SECTION("OK reports the color and closes") {
        SDL_Color got{0, 0, 0, 0};
        bool fired = false;
        ColorDialog* dlg = ColorDialog::create(manager, "Pick", {10, 20, 30, 255},
            [&got, &fired](SDL_Color c) { got = c; fired = true; });
        dlg->setColor({1, 2, 3, 255});
        Button* ok = findButtonByText(dlg, "OK");
        REQUIRE(ok != nullptr);
        clickAt(helper, manager, ok);
        REQUIRE(fired);
        REQUIRE(got == SDL_Color{1, 2, 3, 255});
        REQUIRE(!dlg->isOpen());
        manager.cleanup();
    }

    SECTION("Cancel closes without callback") {
        bool fired = false;
        ColorDialog* dlg = ColorDialog::create(manager, "Pick", {10, 20, 30, 255},
            [&fired](SDL_Color) { fired = true; });
        Button* cancel = findButtonByText(dlg, "Cancel");
        REQUIRE(cancel != nullptr);
        clickAt(helper, manager, cancel);
        REQUIRE(!fired);
        REQUIRE(!dlg->isOpen());
        manager.cleanup();
    }

    SECTION("Enter confirms (ModalDialog key path)") {
        SDL_Color got{0, 0, 0, 0};
        ColorDialog* dlg = ColorDialog::create(manager, "Pick", {7, 8, 9, 255},
            [&got](SDL_Color c) { got = c; });
        manager.setKeyboardFocus(dlg);
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN));
        REQUIRE(got == SDL_Color{7, 8, 9, 255});
        REQUIRE(!dlg->isOpen());
        manager.cleanup();
    }
}
