#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/anchor.hpp"
#include "../src/layout.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"
#include "../src/composite/dialog_box.hpp"

// TestHelper daje managera z Viewport 800x600 wstrzykiwanym w konstruktorze —
// żaden test nie woła setWindowSize() ani handleResize() przed asercjami
// pozycji (kotwice aplikowane od razu przy addElement/addChild).

TEST_CASE("Anchor - presets", "[anchor]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("topLeft keeps size, offsets by margin") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::topLeft(10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 10);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("topRight anchors to right edge") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::topRight(10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 690);
        REQUIRE(panel->getY() == 10);
    }

    SECTION("bottomRight anchors to bottom-right corner") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::bottomRight(10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 690);
        REQUIRE(panel->getY() == 540);
    }

    SECTION("center centers the element in the parent") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::center());
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 350);
        REQUIRE(panel->getY() == 275);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("fill stretches with uniform margin") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::fill(10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 10);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 580);
    }

    SECTION("bottomBar full width, fixed height from bottom") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 50);
        panel->setAnchor(Anchor::bottomBar(50, 10, 10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 500);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("topBar full width, fixed height from top") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 800, 60);
        panel->setAnchor(Anchor::topBar(60, 10, 10));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 10);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 780);
        REQUIRE(panel->getHeight() == 60);
    }

    SECTION("leftSidebar fixed width, full height with margins") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
        panel->setAnchor(Anchor::leftSidebar(60, 70));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 0);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 200);
        REQUIRE(panel->getHeight() == 470);
    }

    SECTION("rightSidebar fixed width, full height with margins") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
        panel->setAnchor(Anchor::rightSidebar(60, 70));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 600);
        REQUIRE(panel->getY() == 60);
        REQUIRE(panel->getWidth() == 200);
        REQUIRE(panel->getHeight() == 470);
    }

    SECTION("horizontalStretch keeps height, stretches width") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::horizontalStretch(5, 5));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 5);
        REQUIRE(panel->getWidth() == 790);
        REQUIRE(panel->getHeight() == 50);
    }
}

TEST_CASE("Anchor - pixel-exact margins, no magic floats", "[anchor]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("1px margin is reachable (old 1.0 meant 100% of parent)") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::topLeft(1));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 1);
        REQUIRE(panel->getY() == 1);
    }

    SECTION("fill(1) insets by exactly one pixel") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::fill(1));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 1);
        REQUIRE(panel->getY() == 1);
        REQUIRE(panel->getWidth() == 798);
        REQUIRE(panel->getHeight() == 598);
    }

    SECTION("center is an enum variant, not a 0.5 float comparison") {
        const Anchor a = Anchor::center();
        REQUIRE(a.h == HAnchor::Center);
        REQUIRE(a.v == VAnchor::Center);
        auto panel = std::make_unique<Panel>(manager, 0, 0, 101, 51);
        panel->setAnchor(a);
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == (800 - 101) / 2);
        REQUIRE(panel->getY() == (600 - 51) / 2);
    }

    SECTION("pinned() escape hatch with separate margins") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::pinned(HAnchor::Right, VAnchor::Bottom, 0, 0, 12, 34));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 800 - 12 - 100);
        REQUIRE(panel->getY() == 600 - 34 - 50);
    }

    SECTION("at() fixes position without stretching") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::at(16, 8));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 16);
        REQUIRE(panel->getY() == 8);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }

    SECTION("both stretch edges fill the axis") {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::horizontalStretch(0, 0));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 0);
        REQUIRE(panel->getWidth() == 800);
    }

    SECTION("no anchor leaves position untouched") {
        auto panel = std::make_unique<Panel>(manager, 42, 17, 100, 50);
        panel->setAnchor(Anchor::none());
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 42);
        REQUIRE(panel->getY() == 17);
        REQUIRE(panel->getWidth() == 100);
        REQUIRE(panel->getHeight() == 50);
    }
}

TEST_CASE("Anchor - resize re-applies anchors", "[anchor][resize]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

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
        button->setAnchor(Anchor::bottomRightAt(12, 34));
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

TEST_CASE("Layout - regressions", "[anchor][layout][regression]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("1px margin renders at 1px, not 100%") {
        // Dawny silnik: 1.0 <= 1.0 → gałąź procentowa → left = 800*1.0 = 800.
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::fill(1));
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 1);
        REQUIRE(panel->getY() == 1);
        REQUIRE(panel->getWidth() == 798);
        REQUIRE(panel->getHeight() == 598);
    }

    SECTION("center uses current size, no m_original history") {
        // Dawny silnik liczył center z m_originalW/H zapamiętanych raz przy
        // setAnchor — po późniejszym setSize środek się rozjeżdżał.
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::center());
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 350);
        panel->setSize(200, 50);
        panel->updateLayout(800, 600);
        REQUIRE(panel->getX() == 300);
        REQUIRE(panel->getWidth() == 200);
    }

    SECTION("setSize on anchor-less parent re-arranges anchored children") {
        // Dawny onSizeChanged() bazy był no-opem: dzieci z anchorami zostawały
        // na starych pozycjach aż do kolejnego resize okna.
        auto parent = std::make_unique<Panel>(manager, 0, 0, 400, 300);
        auto child = std::make_unique<Panel>(manager, 0, 0, 50, 30);
        child->setAnchor(Anchor::center());
        Panel* childRaw = child.get();
        parent->addChild(std::move(child));
        Panel* parentRaw = parent.get();
        manager.addElement(std::move(parent));

        REQUIRE(childRaw->getX() == 175);
        REQUIRE(childRaw->getY() == 135);

        parentRaw->setSize(600, 500);
        REQUIRE(childRaw->getX() == 275);
        REQUIRE(childRaw->getY() == 235);
    }

    SECTION("resize propagates through anchor-less parent") {
        // Dawny handleResize() pomijał top-level bez anchorów, więc ich
        // zakotwiczone dzieci nie dostawały updateLayout().
        auto parent = std::make_unique<Panel>(manager, 10, 10, 400, 300);
        auto child = std::make_unique<Panel>(manager, 0, 0, 50, 30);
        child->setAnchor(Anchor::center());
        Panel* childRaw = child.get();
        parent->addChild(std::move(child));
        Panel* parentRaw = parent.get();
        manager.addElement(std::move(parent));

        manager.handleResize(1024, 768);
        REQUIRE(parentRaw->getX() == 10);
        REQUIRE(parentRaw->getY() == 10);
        REQUIRE(parentRaw->getWidth() == 400);
        REQUIRE(parentRaw->getHeight() == 300);
        REQUIRE(childRaw->getX() == 175);
        REQUIRE(childRaw->getY() == 135);
    }

    SECTION("anchored element never sits at (0,0) before first resize") {
        // Viewport wstrzykiwany w konstruktorze: addElement aplikuje kotwicę
        // od razu. Żadnego handleResize() w tym teście.
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 50);
        panel->setAnchor(Anchor::center());
        Panel* raw = panel.get();
        manager.addElement(std::move(panel));

        REQUIRE((raw->getX() != 0 || raw->getY() != 0));
        REQUIRE(raw->getX() == 350);
        REQUIRE(raw->getY() == 275);
    }

    SECTION("non-positive resize keeps the NonZero viewport") {
        manager.handleResize(0, 0);
        int winW = 0, winH = 0;
        manager.getWindowSize(winW, winH);
        REQUIRE(winW == 800);
        REQUIRE(winH == 600);
    }
}

TEST_CASE("StackLayout - dialog button strip", "[anchor][layout]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("confirm buttons form a centered strip at the bottom") {
        auto dialog = DialogBox::createConfirm(manager, "Sure?", "Tak", "Nie");
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));

        // "Tak"/"Nie" → min width 80 each, spacing 10 → total 170, centered.
        const auto& children = raw->getChildren();
        REQUIRE(children.size() == 3);  // message label + 2 buttons
        auto* yesBtn = children[1].get();
        auto* noBtn = children[2].get();
        REQUIRE(yesBtn->getX() == (400 - 170) / 2);
        REQUIRE(noBtn->getX() == (400 - 170) / 2 + 80 + 10);
        REQUIRE(yesBtn->getY() == 150 - 35 - 15);
        REQUIRE(noBtn->getY() == 150 - 35 - 15);
    }

    SECTION("button strip re-centers after dialog resize") {
        auto dialog = DialogBox::createAlert(manager, "Hi!", "OK");
        DialogBox* raw = dialog.get();
        manager.addElement(std::move(dialog));

        raw->setSize(500, 200);
        const auto& children = raw->getChildren();
        REQUIRE(children.size() == 2);  // message label + 1 button
        auto* okBtn = children[1].get();
        REQUIRE(okBtn->getX() == (500 - 80) / 2);
        REQUIRE(okBtn->getY() == 200 - 35 - 15);
    }
}
