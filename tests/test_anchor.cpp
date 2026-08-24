#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/anchor.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("Anchor - presets", "[anchor]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setWindowSize(800, 600);

    SECTION("topLeft keeps size, offsets by margin") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::topLeft(10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 10);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("topRight anchors to right edge") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::topRight(10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 690);
        REQUIRE(panel->getY() == 10);
    }

    SECTION("bottomRight anchors to bottom-right corner") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::bottomRight(10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 690);
        REQUIRE(panel->getY() == 540);
    }

    SECTION("center centers the element in the parent") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::center());
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 350);
        REQUIRE(panel->getY() == 275);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("fill stretches with uniform margin") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::fill(10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 10);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 580);
    }

    SECTION("bottomBar full width, fixed height from bottom") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 50);
        panel->setAnchor(Anchor::bottomBar(50, 10, 10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 500);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("topBar full width, fixed height from top") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 60);
        panel->setAnchor(Anchor::topBar(60, 10, 10));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 60);
    }

    SECTION("leftSidebar fixed width, full height with margins") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
        panel->setAnchor(Anchor::leftSidebar(200, 60, 70));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 0);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 200);
        REQUIRE(panel->getHeight() == 470);
    }

    SECTION("rightSidebar fixed width, full height with margins") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
        panel->setAnchor(Anchor::rightSidebar(200, 60, 70));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 600);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 200);
        REQUIRE(panel->getHeight() == 470);
    }

    SECTION("horizontalStretch keeps height, stretches width") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::horizontalStretch(5, 5));
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 5);
        REQUIRE(panel->getWidth() == 790);
        REQUIRE(panel->getHeight() == 50);
    }
}

TEST_CASE("Anchor - coordinate encoding", "[anchor]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("values in 0-1 are percentages") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor{0.25f, -1, -1, -1});
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 200);
    }

    SECTION("values above 1 are exact pixels") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor{100, -1, -1, -1});
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 100);
    }

    SECTION("0.5 centers on the given axis") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor{0.5f, 0.5f, -1, -1});
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 350);
        REQUIRE(panel->getY() == 275);
    }

    SECTION("both edges set stretches the element") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor{0, -1, 0, -1});
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 0);
        REQUIRE(panel->getWidth() == 800);
    }

    SECTION("no anchor leaves position untouched") {
        auto panel = std::make_unique<Panel>(manager, 42, 17, 100, 50);
        panel->setAnchor(Anchor::none());
        panel->applyAnchor(800, 600);
        REQUIRE(panel->getX() == 42);
        REQUIRE(panel->getY() == 17);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }
}

TEST_CASE("Anchor - resize re-applies anchors", "[anchor][resize]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setWindowSize(800, 600);

    auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
    panel->setAnchor(Anchor::center());
    manager.addElement(std::move(panel));

    manager.handleResize(1000, 700);
    int winW = 0, winH = 0;
    manager.getWindowSize(winW, winH);
    REQUIRE(winW == 1000);
    REQUIRE(winH == 700);

    auto* centerPanel = manager.findElementAt(500, 350);
    REQUIRE(centerPanel != nullptr);
    REQUIRE(centerPanel->getX() == 450);
    REQUIRE(centerPanel->getY() == 325);
}

TEST_CASE("Anchor - children propagate with parent size", "[anchor][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
    auto child = std::make_unique<Panel>(manager, 0, 0, 50, 30);
    child->setAnchor(Anchor::center());
    parent->addChild(std::move(child));
    Panel* parentRaw = parent.get();
    manager.addElement(std::move(parent));

    parentRaw->updateLayout(400, 300);
    auto* childPanel = parentRaw->findElementAt(200, 150);
    REQUIRE(childPanel != nullptr);
    REQUIRE(childPanel->getX() == 175);
    REQUIRE(childPanel->getY() == 135);
}

TEST_CASE("Anchor - applied immediately on add, without resize", "[anchor][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setWindowSize(800, 600);

    SECTION("top-level element is anchored by addElement") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::center());
        Panel* raw = panel.get();
        manager.addElement(std::move(panel));

        REQUIRE(raw->getX() == 350);
        REQUIRE(raw->getY() == 275);
    }

    SECTION("anchored child is applied by addChild") {
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        Panel* parentRaw = parent.get();
        manager.addElement(std::move(parent));

        auto child = std::make_unique<Panel>(manager, 0, 0, 50, 30);
        child->setAnchor(Anchor::bottomRight(10));
        Panel* childRaw = child.get();
        parentRaw->addChild(std::move(child));

        REQUIRE(childRaw->getX() == 400 - 10 - 50);
        REQUIRE(childRaw->getY() == 300 - 10 - 30);
    }

    SECTION("anchored subtree of a dialog added after startup") {
        auto dialog = std::make_unique<Panel>(manager, 0, 0, 520, 380);
        dialog->setAnchor(Anchor::center());
        auto button = std::make_unique<Panel>(manager, 0, 0, 76, 28);
        button->setAnchor(Anchor{-1, -1, 12, 34});
        Panel* buttonRaw = button.get();
        dialog->addChild(std::move(button));
        Panel* dialogRaw = dialog.get();
        manager.addElement(std::move(dialog));

        const int dialogX = (800 - 520) / 2;
        const int dialogY = (600 - 380) / 2;
        REQUIRE(dialogRaw->getX() == dialogX);
        REQUIRE(dialogRaw->getY() == dialogY);
        REQUIRE(buttonRaw->getX() == 520 - 12 - 76);
        REQUIRE(buttonRaw->getY() == 380 - 34 - 28);
    }
}
