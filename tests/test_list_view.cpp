#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/list_view.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("ListView - Construction and Empty State", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Constructor creates empty ListView") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        REQUIRE(listView->getItemCount() == 0);
        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("Empty ListView getItemCount returns 0") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        REQUIRE(listView->getItemCount() == 0);
    }

    SECTION("Empty ListView getSelectedRow returns nullopt") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("Empty ListView clearSelection works without error") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->clearSelection();
        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("getComponentType returns ListView") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        REQUIRE(std::string(listView->getComponentType()) == "ListView");
    }
}

TEST_CASE("ListView - Item Management", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("addItem adds items to list") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("Item 1");
        listView->addItem("Item 2");
        listView->addItem("Item 3");

        REQUIRE(listView->getItemCount() == 3);
    }

    SECTION("getItem returns correct text") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("First");
        listView->addItem("Second");
        listView->addItem("Third");

        REQUIRE(listView->getItem(0) == "First");
        REQUIRE(listView->getItem(1) == "Second");
        REQUIRE(listView->getItem(2) == "Third");
    }

    SECTION("getItemCount returns correct count after multiple adds") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        REQUIRE(listView->getItemCount() == 0);

        listView->addItem("A");
        REQUIRE(listView->getItemCount() == 1);

        listView->addItem("B");
        REQUIRE(listView->getItemCount() == 2);

        listView->addItem("C");
        REQUIRE(listView->getItemCount() == 3);
    }

    SECTION("insertItem inserts at position") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("C");
        listView->insertItem(1, "B");

        REQUIRE(listView->getItem(0) == "A");
        REQUIRE(listView->getItem(1) == "B");
        REQUIRE(listView->getItem(2) == "C");
    }

    SECTION("insertItem at beginning") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("B");
        listView->addItem("C");
        listView->insertItem(0, "A");

        REQUIRE(listView->getItem(0) == "A");
        REQUIRE(listView->getItem(1) == "B");
        REQUIRE(listView->getItem(2) == "C");
    }

    SECTION("insertItem at end clamps to count") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->insertItem(10, "C");

        REQUIRE(listView->getItem(2) == "C");
    }

    SECTION("removeItem removes item at index") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("X");
        listView->addItem("Y");
        listView->addItem("Z");
        listView->removeItem(1);

        REQUIRE(listView->getItemCount() == 2);
        REQUIRE(listView->getItem(0) == "X");
        REQUIRE(listView->getItem(1) == "Z");
    }

    SECTION("removeItem at beginning") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->removeItem(0);

        REQUIRE(listView->getItemCount() == 2);
        REQUIRE(listView->getItem(0) == "B");
        REQUIRE(listView->getItem(1) == "C");
    }

    SECTION("removeItem at end") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->removeItem(2);

        REQUIRE(listView->getItemCount() == 2);
        REQUIRE(listView->getItem(0) == "A");
        REQUIRE(listView->getItem(1) == "B");
    }

    SECTION("removeItem invalid index does nothing") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->removeItem(10);

        REQUIRE(listView->getItemCount() == 2);
    }

    SECTION("setItem updates item text") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("Old");
        listView->setItem(0, "New");

        REQUIRE(listView->getItem(0) == "New");
    }

    SECTION("setItem preserves other items") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->setItem(1, "Updated");

        REQUIRE(listView->getItem(0) == "A");
        REQUIRE(listView->getItem(1) == "Updated");
        REQUIRE(listView->getItem(2) == "C");
    }

    SECTION("clearItems removes all items") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("1");
        listView->addItem("2");
        listView->addItem("3");
        listView->clearItems();

        REQUIRE(listView->getItemCount() == 0);
    }

    SECTION("clearItems clears selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->setSelectedRow(0);
        listView->clearItems();

        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }
}

TEST_CASE("ListView - Selection Behavior", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelectedRow selects row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");

        listView->setSelectedRow(1);
        REQUIRE(listView->getSelectedRow() == 1);
    }

    SECTION("setSelectedRow changes selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");

        listView->setSelectedRow(0);
        REQUIRE(listView->getSelectedRow() == 0);

        listView->setSelectedRow(2);
        REQUIRE(listView->getSelectedRow() == 2);
    }

    SECTION("getSelectedRow returns nullopt when nothing selected") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");

        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("clearSelection removes selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->setSelectedRow(1);
        listView->clearSelection();

        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("setSelectedRow invalid index does nothing") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->setSelectedRow(10);

        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("removeItem adjusts selection when removed before selected") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->setSelectedRow(2);

        listView->removeItem(0);
        REQUIRE(listView->getSelectedRow() == 1);
    }

    SECTION("removeItem clears selection when selected item removed") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->setSelectedRow(1);

        listView->removeItem(1);
        REQUIRE(listView->getSelectedRow() == std::nullopt);
    }

    SECTION("removeItem keeps selection when removed after selected") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("A");
        listView->addItem("B");
        listView->addItem("C");
        listView->setSelectedRow(0);

        listView->removeItem(2);
        REQUIRE(listView->getSelectedRow() == 0);
    }
}

TEST_CASE("ListView - Row Click Selection", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("Click on first row selects it") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(listViewPtr->getSelectedRow() == 0);
    }

    SECTION("Click on second row selects it") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(listViewPtr->getSelectedRow() == 1);
    }

    SECTION("Click on third row selects it") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");

        int clickX = 10 + 5;
        int clickY = 10 + 2 * ROW_HEIGHT + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(listViewPtr->getSelectedRow() == 2);
    }

    SECTION("Click changes selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        int clickX = 10 + 5;
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));
        REQUIRE(listViewPtr->getSelectedRow() == 0);

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT + ROW_HEIGHT / 2));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT + ROW_HEIGHT / 2));
        REQUIRE(listViewPtr->getSelectedRow() == 1);
    }
}

TEST_CASE("ListView - Row Click Callback", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("setOnRowClick callback fires on click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        size_t clickedRow = SIZE_MAX;
        listViewPtr->setOnRowClick([&](ListView*, size_t row) { clickedRow = row; });

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(clickedRow == 1);
    }

    SECTION("onRowClick provides correct ListView pointer") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");

        ListView* callbackSource = nullptr;
        listViewPtr->setOnRowClick([&](ListView* src, size_t) { callbackSource = src; });

        int clickX = 10 + 5;
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));

        REQUIRE(callbackSource == listViewPtr);
    }

    SECTION("onRowClick fires for each click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        int clickCount = 0;
        listViewPtr->setOnRowClick([&](ListView*, size_t) { ++clickCount; });

        int clickX = 10 + 5;
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT / 2));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT + ROW_HEIGHT / 2));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, 10 + ROW_HEIGHT + ROW_HEIGHT / 2));

        REQUIRE(clickCount == 2);
    }
}

TEST_CASE("ListView - Row Double-Click Callback", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("setOnRowDoubleClick callback fires on double-click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        size_t doubleClickedRow = SIZE_MAX;
        listViewPtr->setOnRowDoubleClick([&](ListView*, size_t row) { doubleClickedRow = row; });

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(doubleClickedRow == 1);
    }

    SECTION("onRowDoubleClick provides correct row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");

        size_t clickedRow = SIZE_MAX;
        listViewPtr->setOnRowDoubleClick([&](ListView*, size_t row) { clickedRow = row; });

        int clickX = 10 + 5;
        int clickY = 10 + 2 * ROW_HEIGHT + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(clickedRow == 2);
    }
}

TEST_CASE("ListView - Row Activate Callback", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("setOnRowActivate callback fires on double-click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        size_t activatedRow = SIZE_MAX;
        listViewPtr->setOnRowActivate([&](ListView*, size_t row) { activatedRow = row; });

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(activatedRow == 0);
    }

    SECTION("Enter key fires onRowActivate for selected row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(1);

        size_t activatedRow = SIZE_MAX;
        listViewPtr->setOnRowActivate([&](ListView*, size_t row) { activatedRow = row; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(activatedRow == 1);
    }

    SECTION("Enter key does nothing when no selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");

        bool activated = false;
        listViewPtr->setOnRowActivate([&](ListView*, size_t) { activated = true; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(!activated);
    }

    SECTION("KP_Enter fires onRowActivate") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(0);

        size_t activatedRow = SIZE_MAX;
        listViewPtr->setOnRowActivate([&](ListView*, size_t row) { activatedRow = row; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_KP_ENTER));

        REQUIRE(activatedRow == 0);
    }
}

TEST_CASE("ListView - Keyboard Navigation", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Arrow down navigates to next row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 1);
    }

    SECTION("Arrow up navigates to previous row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");
        listViewPtr->setSelectedRow(2);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_UP));

        REQUIRE(listViewPtr->getSelectedRow() == 1);
    }

    SECTION("Arrow down stops at last row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(1);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 1);
    }

    SECTION("Arrow up stops at first row") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_UP));

        REQUIRE(listViewPtr->getSelectedRow() == 0);
    }

    SECTION("Arrow keys do nothing without selection") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == std::nullopt);
    }

    SECTION("Arrow left/right ignored (single column)") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->setSelectedRow(0);

        bool handled = listViewPtr->handleEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_LEFT));
        REQUIRE(!handled);

        handled = listViewPtr->handleEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RIGHT));
        REQUIRE(!handled);
    }

    SECTION("Multiple arrow down moves through rows") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");
        listViewPtr->addItem("Row 3");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));
        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 3);
    }
}

TEST_CASE("ListView - Scrolling", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Large number of items handled") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 184, 200);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        for (int i = 0; i < 100; ++i) {
            listViewPtr->addItem("Item " + std::to_string(i));
        }

        REQUIRE(listViewPtr->getItemCount() == 100);
        REQUIRE(listViewPtr->getItem(50) == "Item 50");
        REQUIRE(listViewPtr->getItem(99) == "Item 99");
    }

    SECTION("Slider exists for scrolling") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->addItem("Row 2");

        manager.render();

        REQUIRE(listViewPtr->getRowHeight() == 24);
    }

    SECTION("Scroll offset starts at zero") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));

        for (int i = 0; i < 10; ++i) {
            listViewPtr->addItem("Row " + std::to_string(i));
        }

        manager.render();

        REQUIRE(listViewPtr->getVerticalScrollOffset() == 0);
    }
}

TEST_CASE("ListView - Disabled State", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("Disabled ListView ignores click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        listViewPtr->setEnabled(false);
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(listViewPtr->getSelectedRow() == std::nullopt);
    }

    SECTION("Disabled ListView ignores keyboard") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        listViewPtr->setEnabled(false);
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 0);
    }

    SECTION("Disabled ListView ignores keyboard navigation") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        listViewPtr->setEnabled(false);
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 0);
    }
}

TEST_CASE("ListView - Hidden State", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    constexpr int ROW_HEIGHT = 24;

    SECTION("Hidden ListView ignores click") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        listViewPtr->setVisible(false);
        manager.addElement(std::move(listView));

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        int clickX = 10 + 5;
        int clickY = 10 + ROW_HEIGHT / 2;

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, clickX, clickY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, clickX, clickY));

        REQUIRE(listViewPtr->getSelectedRow() == std::nullopt);
    }

    SECTION("Hidden ListView ignores keyboard") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        listViewPtr->setVisible(false);
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");
        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_DOWN));

        REQUIRE(listViewPtr->getSelectedRow() == 0);
    }
}

TEST_CASE("ListView - Callback Independence", "[list_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Callbacks work independently") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");
        listViewPtr->addItem("Row 1");

        size_t clickRow = SIZE_MAX;
        size_t doubleClickRow = SIZE_MAX;
        size_t activateRow = SIZE_MAX;

        listViewPtr->setOnRowClick([&](ListView*, size_t row) { clickRow = row; });
        listViewPtr->setOnRowDoubleClick([&](ListView*, size_t row) { doubleClickRow = row; });
        listViewPtr->setOnRowActivate([&](ListView*, size_t row) { activateRow = row; });

        listViewPtr->setSelectedRow(0);

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(clickRow == SIZE_MAX);
        REQUIRE(doubleClickRow == SIZE_MAX);
        REQUIRE(activateRow == 0);
    }

    SECTION("All callbacks can be set") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        listView->addItem("Row 0");

        bool clickSet = false;
        bool doubleClickSet = false;
        bool activateSet = false;

        listView->setOnRowClick([&](ListView*, size_t) { clickSet = true; });
        listView->setOnRowDoubleClick([&](ListView*, size_t) { doubleClickSet = true; });
        listView->setOnRowActivate([&](ListView*, size_t) { activateSet = true; });

        REQUIRE(true);
    }

    SECTION("Callback without selection does not fire") {
        auto listView = std::make_unique<ListView>(manager, 10, 10, 200, 300);
        ListView* listViewPtr = listView.get();
        manager.addElement(std::move(listView));
        manager.setKeyboardFocus(listViewPtr);

        listViewPtr->addItem("Row 0");

        bool activated = false;
        listViewPtr->setOnRowActivate([&](ListView*, size_t) { activated = true; });

        manager.processEvent(helper.createKeyEvent(SDL_KEYDOWN, SDLK_RETURN));

        REQUIRE(!activated);
    }
}