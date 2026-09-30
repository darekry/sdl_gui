#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/anchor.hpp"
#include "../src/layout.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/json_parser.hpp"
#include "../src/sgml_parser.hpp"

static Panel* addDocked(Panel* parent, GUIManager& manager,
                        int w, int h, Dock dock) {
    auto child = std::make_unique<Panel>(manager, 0, 0, w, h);
    child->setDock(dock);
    Panel* raw = child.get();
    parent->addChild(std::move(child));
    return raw;
}

TEST_CASE("DockLayout - top/bottom/left/right/fill chain", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    parent->setLayoutManager(std::make_unique<DockLayout>());
    Panel* parentRaw = parent.get();
    manager.addElement(std::move(parent));

    Panel* top = addDocked(parentRaw, manager, 100, 50, Dock::Top);
    Panel* bottom = addDocked(parentRaw, manager, 100, 40, Dock::Bottom);
    Panel* left = addDocked(parentRaw, manager, 60, 100, Dock::Left);
    Panel* fill = addDocked(parentRaw, manager, 100, 100, Dock::Fill);

    // Top: full width, own height at origin.
    REQUIRE(top->getX() == 0);
    REQUIRE(top->getY() == 0);
    REQUIRE(top->getWidth() == 400);
    REQUIRE(top->getHeight() == 50);
    // Bottom: full width, own height at bottom of remaining (300-40).
    REQUIRE(bottom->getX() == 0);
    REQUIRE(bottom->getY() == 260);
    REQUIRE(bottom->getWidth() == 400);
    REQUIRE(bottom->getHeight() == 40);
    // Left: own width, full remaining height (300-50-40=210).
    REQUIRE(left->getX() == 0);
    REQUIRE(left->getY() == 50);
    REQUIRE(left->getWidth() == 60);
    REQUIRE(left->getHeight() == 210);
    // Fill: remainder (400-60=340 x 210).
    REQUIRE(fill->getX() == 60);
    REQUIRE(fill->getY() == 50);
    REQUIRE(fill->getWidth() == 340);
    REQUIRE(fill->getHeight() == 210);
}

TEST_CASE("DockLayout - fill takes remainder, multiple fill chain", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("single fill after top") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        Panel* top = addDocked(raw, manager, 50, 30, Dock::Top);
        Panel* fill = addDocked(raw, manager, 50, 50, Dock::Fill);
        REQUIRE(top->getHeight() == 30);
        REQUIRE(fill->getY() == 30);
        REQUIRE(fill->getHeight() == 170);
        REQUIRE(fill->getWidth() == 200);
    }

    SECTION("second fill gets zero (chain, no overlap)") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 200, 200);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        Panel* first = addDocked(raw, manager, 50, 50, Dock::Fill);
        Panel* second = addDocked(raw, manager, 50, 50, Dock::Fill);
        REQUIRE(first->getWidth() == 200);
        REQUIRE(first->getHeight() == 200);
        REQUIRE(second->getWidth() == 0);
        REQUIRE(second->getHeight() == 0);
    }
}

TEST_CASE("DockLayout - hidden children skipped", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    parent->setLayoutManager(std::make_unique<DockLayout>());
    Panel* raw = parent.get();
    manager.addElement(std::move(parent));

    Panel* top = addDocked(raw, manager, 100, 50, Dock::Top);
    Panel* hidden = addDocked(raw, manager, 100, 40, Dock::Top);
    Panel* fill = addDocked(raw, manager, 100, 100, Dock::Fill);

    hidden->setVisible(false);
    raw->layoutChildren();

    REQUIRE(top->getY() == 0);
    REQUIRE(top->getHeight() == 50);
    // Hidden top (40px) must not eat space: fill starts right after first top.
    REQUIRE(fill->getY() == 50);
    REQUIRE(fill->getHeight() == 250);
}

TEST_CASE("DockLayout - spacing and padding", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    parent->setLayoutManager(std::make_unique<DockLayout>(4, 8, 8, 8, 8));
    Panel* raw = parent.get();
    manager.addElement(std::move(parent));

    Panel* top = addDocked(raw, manager, 100, 50, Dock::Top);
    Panel* fill = addDocked(raw, manager, 100, 100, Dock::Fill);

    // Remaining: 8..392 (384 wide), 8..292 (284 high).
    REQUIRE(top->getX() == 8);
    REQUIRE(top->getY() == 8);
    REQUIRE(top->getWidth() == 384);
    REQUIRE(top->getHeight() == 50);
    // Fill after top + spacing: y = 8+50+4 = 62, h = 284-50-4 = 230.
    REQUIRE(fill->getX() == 8);
    REQUIRE(fill->getY() == 62);
    REQUIRE(fill->getWidth() == 384);
    REQUIRE(fill->getHeight() == 230);
}

TEST_CASE("DockLayout - resize re-arranges", "[dock][layout][resize]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    parent->setLayoutManager(std::make_unique<DockLayout>());
    Panel* raw = parent.get();
    manager.addElement(std::move(parent));

    Panel* left = addDocked(raw, manager, 60, 100, Dock::Left);
    Panel* fill = addDocked(raw, manager, 100, 100, Dock::Fill);
    REQUIRE(fill->getWidth() == 340);

    raw->setSize(600, 500);
    REQUIRE(left->getHeight() == 500);
    REQUIRE(fill->getX() == 60);
    REQUIRE(fill->getWidth() == 540);
    REQUIRE(fill->getHeight() == 500);
}

TEST_CASE("DockLayout - None untouched, anchor still works", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("None without anchor stays where it was") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        auto free = std::make_unique<Panel>(manager, 42, 17, 50, 30);
        Panel* freeRaw = free.get();
        raw->addChild(std::move(free));

        REQUIRE(freeRaw->getX() == 42);
        REQUIRE(freeRaw->getY() == 17);
        REQUIRE(freeRaw->getWidth() == 50);
        REQUIRE(freeRaw->getHeight() == 30);
    }

    SECTION("None with anchor is placed by anchor") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        auto centered = std::make_unique<Panel>(manager, 0, 0, 50, 30);
        centered->setAnchor(Anchor::center());
        Panel* cRaw = centered.get();
        raw->addChild(std::move(centered));

        REQUIRE(cRaw->getX() == 175);
        REQUIRE(cRaw->getY() == 135);
    }

    SECTION("docked child ignores anchor") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        auto child = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        child->setAnchor(Anchor::center());
        child->setDock(Dock::Top);
        Panel* cRaw = child.get();
        raw->addChild(std::move(child));

        REQUIRE(cRaw->getX() == 0);
        REQUIRE(cRaw->getY() == 0);
        REQUIRE(cRaw->getWidth() == 400);
    }
}

TEST_CASE("DockLayout - setDock and addChild trigger re-layout", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setDock after add re-layouts parent") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        auto child = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        Panel* cRaw = child.get();
        raw->addChild(std::move(child));
        // None: untouched.
        REQUIRE(cRaw->getX() == 0);
        REQUIRE(cRaw->getY() == 0);

        cRaw->setDock(Dock::Bottom);
        REQUIRE(cRaw->getY() == 250);
        REQUIRE(cRaw->getWidth() == 400);
    }

    SECTION("addChild with dock pre-set is arranged immediately") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        parent->setLayoutManager(std::make_unique<DockLayout>());
        Panel* raw = parent.get();
        manager.addElement(std::move(parent));

        auto right = std::make_unique<Panel>(manager, 0, 0, 70, 20);
        right->setDock(Dock::Right);
        Panel* rRaw = right.get();
        raw->addChild(std::move(right));

        REQUIRE(rRaw->getX() == 330);
        REQUIRE(rRaw->getY() == 0);
        REQUIRE(rRaw->getWidth() == 70);
        REQUIRE(rRaw->getHeight() == 300);
    }
}

TEST_CASE("DockLayout - measure content size", "[dock][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    parent->setLayoutManager(std::make_unique<DockLayout>());
    Panel* raw = parent.get();
    manager.addElement(std::move(parent));

    addDocked(raw, manager, 100, 50, Dock::Top);
    addDocked(raw, manager, 60, 100, Dock::Left);
    addDocked(raw, manager, 100, 100, Dock::Fill);

    // Fill to reszta: 400-60=340 x 300-50=250, więc miara = kontener.
    LayoutSize size = raw->getLayoutManager()->measure(*raw, LayoutConstraints{});
    REQUIRE(size.width == 400);
    REQUIRE(size.height == 300);
}

TEST_CASE("DockLayout - parser json and xml", "[dock][layout][parser]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("json dock layout") {
        JsonParser parser(manager);
        auto root = parser.loadLayout("tests/data/dock_layout.json");
        REQUIRE(root != nullptr);
        REQUIRE(root->getLayoutManager() != nullptr);
        const auto& children = root->getChildren();
        REQUIRE(children.size() == 4);
        REQUIRE(children[0]->getDock() == Dock::Top);
        REQUIRE(children[1]->getDock() == Dock::Bottom);
        REQUIRE(children[2]->getDock() == Dock::Left);
        REQUIRE(children[3]->getDock() == Dock::Fill);
        // 400x300, pad 8, spacing 4 (same math as spacing test + bottom/left).
        REQUIRE(children[0]->getX() == 8);
        REQUIRE(children[0]->getY() == 8);
        REQUIRE(children[0]->getWidth() == 384);
        REQUIRE(children[1]->getY() == 252);
        REQUIRE(children[2]->getX() == 8);
        REQUIRE(children[2]->getY() == 62);
        REQUIRE(children[3]->getX() == 72);
        REQUIRE(children[3]->getY() == 62);
        REQUIRE(children[3]->getWidth() == 320);
        REQUIRE(children[3]->getHeight() == 186);
    }

    SECTION("xml dock layout matches json") {
        SGMLParser parser(manager);
        auto root = parser.loadLayout("tests/data/dock_layout.xml");
        REQUIRE(root != nullptr);
        REQUIRE(root->getLayoutManager() != nullptr);
        const auto& children = root->getChildren();
        REQUIRE(children.size() == 4);
        REQUIRE(children[0]->getDock() == Dock::Top);
        REQUIRE(children[3]->getDock() == Dock::Fill);
        REQUIRE(children[0]->getX() == 8);
        REQUIRE(children[3]->getX() == 72);
        REQUIRE(children[3]->getWidth() == 320);
        REQUIRE(children[3]->getHeight() == 186);
    }
}
