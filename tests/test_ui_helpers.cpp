#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/ui_helpers.hpp"
#include "../src/slider.hpp"
#include "../src/range_slider.hpp"
#include "../src/label.hpp"
#include "../src/panel.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ui_helpers linkLabel", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("prefix/suffix variant refreshes immediately and on change") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 30, 0, 100, 42,
                                               Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "stale", 14);
        Slider* s = slider.get();
        Label* l = label.get();
        manager.addElement(std::move(slider));
        manager.addElement(std::move(label));

        linkLabel(*s, *l, "v=", "px");
        REQUIRE(l->getText() == "v=42px"); // immediate refresh, no event needed
        s->setValue(7);
        REQUIRE(l->getText() == "v=7px");
    }

    SECTION("formatter variant") {
        auto slider = std::make_unique<Slider>(manager, 0, 0, 200, 30, 0, 120, 90,
                                               Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "", 14);
        Slider* s = slider.get();
        Label* l = label.get();
        manager.addElement(std::move(slider));
        manager.addElement(std::move(label));

        linkLabel(*s, *l, [](int v) { return std::to_string(v / 60) + "m"; });
        REQUIRE(l->getText() == "1m");
        s->setValue(120);
        REQUIRE(l->getText() == "2m");
    }

    SECTION("linkRangeLabel default and formatter variants") {
        auto range = std::make_unique<RangeSlider>(manager, 0, 0, 200, 30, 0, 100, 20, 80,
                                                   Orientation::Horizontal);
        auto range2 = std::make_unique<RangeSlider>(manager, 0, 100, 200, 30, 0, 100, 20, 80,
                                                    Orientation::Horizontal);
        auto label = std::make_unique<Label>(manager, 0, 40, "", 14);
        auto label2 = std::make_unique<Label>(manager, 0, 140, "", 14);
        RangeSlider* r = range.get();
        RangeSlider* r2 = range2.get();
        Label* l = label.get();
        Label* l2 = label2.get();
        manager.addElement(std::move(range));
        manager.addElement(std::move(range2));
        manager.addElement(std::move(label));
        manager.addElement(std::move(label2));

        linkRangeLabel(*r, *l);
        REQUIRE(l->getText() == "[20, 80]");
        linkRangeLabel(*r2, *l2, [](int lo, int hi) {
            return std::to_string(lo) + ".." + std::to_string(hi);
        });
        r->setLowerValue(10);
        r2->setLowerValue(10);
        REQUIRE(l->getText() == "[10, 80]");
        REQUIRE(l2->getText() == "10..80");
    }
}

TEST_CASE("ui_helpers generators", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("gridPanels builds a positioned panel matrix") {
        auto board = std::make_unique<Panel>(manager, 0, 0, 400, 400);
        Panel* boardPtr = board.get();
        manager.addElement(std::move(board));

        auto grid = gridPanels(*boardPtr, 3, 2, 50, 40, 5, 10, 20);
        REQUIRE(grid.size() == 2);
        REQUIRE(grid[0].size() == 3);
        REQUIRE(grid[1][2] != nullptr);
        // cell (2,1): ox + 2*(50+5), oy + 1*(40+5), relative to parent
        REQUIRE(grid[1][2]->getX() == 10 + 2 * 55);
        REQUIRE(grid[1][2]->getY() == 20 + 1 * 45);
        int w = 0, h = 0;
        grid[0][0]->getSize(w, h);
        REQUIRE(w == 50);
        REQUIRE(h == 40);
    }

    SECTION("faceGrid builds buttons with centered faces") {
        auto grid = faceGrid(manager, 2, 2, 100, 100, 10, 0, 0, 32);
        REQUIRE(grid.size() == 2);
        FaceCell c = grid[0][1];
        REQUIRE(c.button != nullptr);
        REQUIRE(c.face != nullptr);
        REQUIRE(c.button->getX() == 1 * (100 + 10));
        REQUIRE(c.button->getY() == 0);
        // Face is centered for its (empty) initial size: position is
        // non-negative and inside the button.
        REQUIRE(c.face->getX() >= 0);
        REQUIRE(c.face->getY() >= 0);
        REQUIRE(c.face->getX() <= 100);
        REQUIRE(c.face->getY() <= 100);
        c.face->setText("A");
        REQUIRE(c.face->getText() == "A");
    }

    SECTION("makeStrip places widgets along a pitch") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 600, 100);
        Panel* p = panel.get();
        manager.addElement(std::move(panel));

        auto items = makeStrip<Slider>(*p, 4, 10, 5, 60, 30, 70, 0,
                                       [](GUIManager& m, int x, int y, int w, int h, int i) {
                                           return std::make_unique<Slider>(m, x, y, w, h, 0, 100,
                                                                           i * 10, Orientation::Horizontal);
                                       });
        REQUIRE(items.size() == 4);
        for (int i = 0; i < 4; ++i) {
            REQUIRE(items[i]->getX() == 10 + i * 70);
            REQUIRE(items[i]->getY() == 5);
            REQUIRE(items[i]->getValue() == i * 10);
        }
    }

    SECTION("shuffledPairs returns each symbol exactly twice") {
        auto deck = shuffledPairs(8);
        REQUIRE(deck.size() == 16);
        std::vector<int> counts(8, 0);
        for (int s : deck) {
            REQUIRE(s >= 0);
            REQUIRE(s < 8);
            ++counts[static_cast<size_t>(s)];
        }
        for (int c : counts) REQUIRE(c == 2);
    }
}

TEST_CASE("ui_helpers tracked", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("manager overload fuses makeRef and addElement") {
        auto [panel, ref] = addTracked(manager, std::make_unique<Panel>(manager, 5, 5, 50, 50));
        REQUIRE(panel != nullptr);
        REQUIRE(static_cast<bool>(ref));
        REQUIRE(ref.get() == panel);
        REQUIRE(panel->getX() == 5);
    }

    SECTION("parent overload attaches as child with a live ref") {
        auto [board, boardRef] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 200, 200));
        auto [child, childRef] = addTracked(*board, std::make_unique<Label>(manager, 10, 10, "hi", 14));
        REQUIRE(static_cast<bool>(childRef));
        REQUIRE(childRef.get() == child);
        REQUIRE(child->getText() == "hi");
        (void)boardRef;
    }
}

TEST_CASE("ui_helpers card", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("styleCard defaults reproduce the canonical dark look") {
        auto [panel, ref] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 100, 100));
        styleCard(*panel);
        Style s = panel->getComposedStyle(ElementState::Normal);
        REQUIRE(s.backgroundColor == SDL_Color{50, 52, 64, 255});
        REQUIRE(s.borderColor == SDL_Color{98, 114, 164, 255});
        REQUIRE(s.borderWidth == 2);
        REQUIRE(s.borderRadius == 10);
        (void)ref;
    }

    SECTION("styleCard custom colors override the defaults") {
        auto [panel, ref] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 100, 100));
        styleCard(*panel, {28, 30, 42, 255}, {98, 114, 164, 255}, 2, 8);
        Style s = panel->getComposedStyle(ElementState::Normal);
        REQUIRE(s.backgroundColor == SDL_Color{28, 30, 42, 255});
        REQUIRE(s.borderRadius == 8);
        (void)ref;
    }

    SECTION("addDarkPanel attaches styled panels (parent + manager)") {
        auto [board, boardRef] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 300, 300));
        Panel* child = addDarkPanel(*board, 10, 10, 80, 60);
        Panel* top = addDarkPanel(manager, 100, 100, 80, 60);
        REQUIRE(child->getX() == 10);
        REQUIRE(top->getX() == 100);
        REQUIRE(child->getComposedStyle(ElementState::Normal).backgroundColor ==
                SDL_Color{50, 52, 64, 255});
        REQUIRE(top->getComposedStyle(ElementState::Normal).backgroundColor ==
                SDL_Color{50, 52, 64, 255});
        (void)boardRef;
    }
}

TEST_CASE("ui_helpers builders", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("addButton wires text, geometry and click callback") {
        bool clicked = false;
        Button* b = addButton(manager, 10, 20, 120, 40, "Fire",
                              [&](GUIElement*) { clicked = true; });
        REQUIRE(b->getText() == "Fire");
        REQUIRE(b->getX() == 10);
        REQUIRE(b->getY() == 20);
        SDL_Event down = helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN,
                                                 SDL_BUTTON_LEFT, 70, 40);
        SDL_Event up = helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP,
                                               SDL_BUTTON_LEFT, 70, 40);
        manager.processEvent(down);
        manager.processEvent(up);
        REQUIRE(clicked);
    }

    SECTION("addLabel applies text and optional color") {
        Label* plain = addLabel(manager, 5, 5, "plain", 16);
        Label* white = addLabel(manager, 5, 30, "bright", 16, SDL_Color{255, 255, 255, 255});
        REQUIRE(plain->getText() == "plain");
        REQUIRE(white->getText() == "bright");
        REQUIRE(white->getComposedStyle(ElementState::Normal).textColor ==
                SDL_Color{255, 255, 255, 255});
    }

    SECTION("addLabeledCheckbox places the caption right of the box") {
        auto [board, boardRef] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 300, 100));
        LabeledCheckbox lc = addLabeledCheckbox(*board, 20, 70, "Check me!", 24, 16);
        REQUIRE(lc.box != nullptr);
        REQUIRE(lc.label != nullptr);
        int w = 0, h = 0;
        lc.box->getSize(w, h);
        REQUIRE(w == 24);
        REQUIRE(h == 24);
        REQUIRE(lc.label->getX() == 20 + 24 + 8);
        REQUIRE(lc.label->getText() == "Check me!");
        (void)boardRef;
    }
}

TEST_CASE("ui_helpers grid", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("makeGrid forwards cell coords to the factory") {
        auto board = std::make_unique<Panel>(manager, 0, 0, 400, 400);
        Panel* boardPtr = board.get();
        manager.addElement(std::move(board));

        auto grid = makeGrid<Button>(*boardPtr, 3, 2, 10, 20, 50, 40, 5, 5,
                                     [](GUIManager& m, int c, int r, int x, int y, int w, int h) {
                                         return std::make_unique<Button>(
                                             m, x, y, w, h, std::to_string(c + 10 * r));
                                     });
        REQUIRE(grid.size() == 2);
        REQUIRE(grid[0].size() == 3);
        // cell (2,1): ox + 2*(50+5), oy + 1*(40+5); caption encodes (col,row)
        REQUIRE(grid[1][2]->getX() == 10 + 2 * 55);
        REQUIRE(grid[1][2]->getY() == 20 + 1 * 45);
        REQUIRE(grid[1][2]->getText() == "12");
    }

    SECTION("makeGrid manager overload attaches top-level") {
        auto grid = makeGrid<Panel>(manager, 2, 1, 0, 0, 30, 30, 0, 0,
                                    [](GUIManager& m, int, int, int x, int y, int w, int h) {
                                        return std::make_unique<Panel>(m, x, y, w, h);
                                    });
        REQUIRE(grid.size() == 1);
        REQUIRE(grid[0][1]->getX() == 30);
    }
}

TEST_CASE("ui_helpers sliders", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("onAnyChange runs refresh immediately and on any slider") {
        auto s1 = std::make_unique<Slider>(manager, 0, 0, 100, 20, 0, 100, 10,
                                           Orientation::Horizontal);
        auto s2 = std::make_unique<Slider>(manager, 0, 30, 100, 20, 0, 100, 20,
                                           Orientation::Horizontal);
        Slider* a = s1.get();
        Slider* b = s2.get();
        manager.addElement(std::move(s1));
        manager.addElement(std::move(s2));

        int calls = 0;
        int sum = 0;
        onAnyChange({a, b}, [&]() {
            ++calls;
            sum = a->getValue() + b->getValue();
        });
        REQUIRE(calls == 1); // immediate run — no stale readout
        REQUIRE(sum == 30);
        a->setValue(50);
        REQUIRE(calls == 2);
        REQUIRE(sum == 70);
        b->setValue(0);
        REQUIRE(calls == 3);
        REQUIRE(sum == 50);
    }

    SECTION("bindSliderValue mirrors into int and float variables") {
        auto s1 = std::make_unique<Slider>(manager, 0, 0, 100, 20, 0, 100, 25,
                                           Orientation::Horizontal);
        auto s2 = std::make_unique<Slider>(manager, 0, 30, 100, 20, 0, 100, 50,
                                           Orientation::Horizontal);
        Slider* a = s1.get();
        Slider* b = s2.get();
        manager.addElement(std::move(s1));
        manager.addElement(std::move(s2));

        int speed = 0;
        float damping = 0.0f;
        bindSliderValue(*a, speed);
        bindSliderValue(*b, damping, 0.01f);
        REQUIRE(speed == 25); // immediate sync
        REQUIRE(damping == Catch::Approx(0.5f));
        a->setValue(60);
        b->setValue(80);
        REQUIRE(speed == 60);
        REQUIRE(damping == Catch::Approx(0.8f));
    }
}

TEST_CASE("ui_helpers status bars", "[ui_helpers]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("manager bars span the screen width at top/bottom") {
        StatusBar top = makeTopBar(manager, 800, 28, "carrier", 14);
        StatusBar bottom = makeBottomBar(manager, 800, 600, 28, "ready", 14);
        REQUIRE(top.bar->getY() == 0);
        REQUIRE(bottom.bar->getY() == 600 - 28);
        int w = 0, h = 0;
        top.bar->getSize(w, h);
        REQUIRE(w == 800);
        REQUIRE(h == 28);
        bottom.bar->getSize(w, h);
        REQUIRE(w == 800);
        REQUIRE(top.label->getText() == "carrier");
        REQUIRE(bottom.label->getText() == "ready");
    }

    SECTION("parent bars derive geometry from the parent") {
        auto [win, winRef] = addTracked(manager, std::make_unique<Panel>(manager, 0, 0, 400, 300));
        StatusBar top = makeTopBar(*win, 24, "title", 14);
        StatusBar bottom = makeBottomBar(*win, 24, "hint", 14);
        REQUIRE(top.bar->getY() == 0);
        REQUIRE(bottom.bar->getY() == 300 - 24);
        int w = 0, h = 0;
        top.bar->getSize(w, h);
        REQUIRE(w == 400);
        (void)winRef;
    }
}
