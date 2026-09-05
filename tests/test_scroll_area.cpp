#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/scroll_area.hpp"
#include "../src/label.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ScrollArea - construction", "[scroll_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("constructor creates viewport and vertical slider") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        REQUIRE(areaPtr->getContent() != nullptr);
        REQUIRE(areaPtr->getScrollOffsetX() == 0);
        REQUIRE(areaPtr->getScrollOffsetY() == 0);
    }

    SECTION("default scroll state") {
        ScrollArea area(manager, 0, 0, 300, 200);
        REQUIRE(area.getComponentTypeId() == ComponentType::ScrollArea);
    }

    SECTION("content size defaults to area size") {
        ScrollArea area(manager, 0, 0, 300, 200);
        // with default content size, slider range is 0
        REQUIRE(area.getScrollOffsetY() == 0);
    }
}

TEST_CASE("ScrollArea - content and scrolling", "[scroll_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setContentSize grows scroll range") {
        auto area = std::make_unique<ScrollArea>(manager, 0, 0, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(600, 800);
        // viewport height = 200 (no h scrollbar); max scroll = 800 - 200 = 600
        areaPtr->setScrollOffset(0, 600);
        REQUIRE(areaPtr->getScrollOffsetY() == 600);
    }

    SECTION("setScrollOffset moves content") {
        auto area = std::make_unique<ScrollArea>(manager, 0, 0, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(600, 800);
        areaPtr->setScrollOffset(100, 200);
        REQUIRE(areaPtr->getScrollOffsetX() == 100);
        REQUIRE(areaPtr->getScrollOffsetY() == 200);
    }

    SECTION("setContent adds a widget as content") {
        auto area = std::make_unique<ScrollArea>(manager, 0, 0, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        auto label = std::make_unique<Label>(manager, 0, 0, "Hello");
        Label* labelPtr = label.get();
        areaPtr->setContent(std::move(label));

        // content widget lives inside the ScrollArea's content panel
        REQUIRE(areaPtr->getContent() != nullptr);
        REQUIRE(labelPtr->getParent() != nullptr);
    }

    SECTION("setContent(nullptr) is safe") {
        ScrollArea area(manager, 0, 0, 300, 200);
        area.setContent(nullptr);
        REQUIRE(area.getContent() != nullptr);
    }
}

TEST_CASE("ScrollArea - wheel scrolling", "[scroll_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("wheel over area scrolls down") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(300, 800);

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 60);
    }

    SECTION("wheel up scrolls up") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(300, 800);
        areaPtr->setScrollOffset(0, 120);

        SDL_Event e = helper.createMouseWheel(1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 60);
    }

    SECTION("wheel outside area is ignored") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(300, 800);

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 500.0f;
        e.wheel.mouse_y = 500.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 0);
    }

    SECTION("wheel clamps at the bottom") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(300, 800);
        areaPtr->setScrollOffset(0, 595); // max is 600

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 600);
    }
}

TEST_CASE("ScrollArea - scroll enable toggling", "[scroll_area]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("disabling vertical scroll stops wheel handling") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setContentSize(300, 800);
        areaPtr->setVerticalScroll(false);

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 0);
    }

    SECTION("enabling horizontal scroll gives it the wheel") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setVerticalScroll(false);
        areaPtr->setHorizontalScroll(true);
        areaPtr->setContentSize(800, 300);

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetX() == 60);
    }

    SECTION("both scrollbars enabled keeps vertical priority") {
        auto area = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
        ScrollArea* areaPtr = area.get();
        manager.addElement(std::move(area));

        areaPtr->setHorizontalScroll(true);
        areaPtr->setContentSize(800, 800);

        SDL_Event e = helper.createMouseWheel(-1);
        e.wheel.mouse_x = 100.0f;
        e.wheel.mouse_y = 100.0f;
        manager.processEvent(e);

        REQUIRE(areaPtr->getScrollOffsetY() == 60);
        REQUIRE(areaPtr->getScrollOffsetX() == 0);
    }
}
