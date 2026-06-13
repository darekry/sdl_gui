#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/combobox.hpp"
#include "../src/gui_manager.hpp"

namespace {
constexpr int kComboX = 50;
constexpr int kComboY = 40;
constexpr int kComboWidth = 160;
constexpr int kComboHeight = 24;
constexpr int kItemHeight = 30;
}

TEST_CASE("ComboBox - Initial State", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Empty ComboBox has no selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getSelectedIndex() == -1);
        REQUIRE(cb->getSelectedItem() == "");
        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("ComboBox with items has first item selected by default") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("getSelectedIndex returns 0 after adding first item") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->addItem("Only");

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "Only");
    }
}

TEST_CASE("ComboBox - Dropdown Behavior", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click on collapsed ComboBox expands dropdown") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        REQUIRE_FALSE(cb->isExpanded());

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE(cb->isExpanded());
    }

    SECTION("Click on expanded ComboBox collapses dropdown") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE(cb->isExpanded());

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("isExpanded returns true when expanded") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE(cb->isExpanded() == true);
    }

    SECTION("Dropdown shows all items after expansion") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("Item1");
        cb->addItem("Item2");
        cb->addItem("Item3");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        REQUIRE(cb->isExpanded());
    }
}

TEST_CASE("ComboBox - Item Selection", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click on dropdown item selects that item") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        int itemX = kComboX + 10;
        int itemY = kComboY + kComboHeight + kItemHeight / 2 + kItemHeight;
        manager.processEvent(helper.createMouseMotion(itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, itemX, itemY));

        REQUIRE(cb->getSelectedIndex() == 1);
        REQUIRE(cb->getSelectedItem() == "Second");
    }

    SECTION("Selection changes and on_selection_changed fires") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        int callbackIndex = -2;
        std::string callbackText;
        cb->on_selection_changed = [&](int index, const std::string& text) {
            callbackIndex = index;
            callbackText = text;
        };

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        int itemX = kComboX + 10;
        int itemY = kComboY + kComboHeight + kItemHeight / 2 + kItemHeight;
        manager.processEvent(helper.createMouseMotion(itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, itemX, itemY));

        REQUIRE(callbackIndex == 1);
        REQUIRE(callbackText == "Second");
    }

    SECTION("getSelectedIndex returns new index after selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        REQUIRE(cb->getSelectedIndex() == 0);

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        int itemX = kComboX + 10;
        int itemY = kComboY + kComboHeight + kItemHeight / 2 + kItemHeight * 2;
        manager.processEvent(helper.createMouseMotion(itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, itemX, itemY));

        REQUIRE(cb->getSelectedIndex() == 2);
    }

    SECTION("getSelectedItem returns selected text after selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        REQUIRE(cb->getSelectedItem() == "First");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        int itemX = kComboX + 10;
        int itemY = kComboY + kComboHeight + kItemHeight / 2 + kItemHeight * 2;
        manager.processEvent(helper.createMouseMotion(itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, itemX, itemY));

        REQUIRE(cb->getSelectedItem() == "Third");
    }

    SECTION("Dropdown collapses after selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();
        REQUIRE(cb->isExpanded());

        int itemX = kComboX + 10;
        int itemY = kComboY + kComboHeight + kItemHeight / 2;
        manager.processEvent(helper.createMouseMotion(itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, itemX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, itemX, itemY));

        REQUIRE_FALSE(cb->isExpanded());
    }
}

TEST_CASE("ComboBox - Click Outside", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Click outside expanded dropdown collapses it") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE(cb->isExpanded());

        int outsideX = kComboX - 20;
        int outsideY = kComboY - 20;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, outsideX, outsideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, outsideX, outsideY));

        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("Selection unchanged when clicking outside") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE(cb->isExpanded());

        int outsideX = kComboX - 20;
        int outsideY = kComboY - 20;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, outsideX, outsideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, outsideX, outsideY));

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }

    SECTION("Click to the right of dropdown collapses it") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE(cb->isExpanded());

        int outsideX = kComboX + kComboWidth + 50;
        int outsideY = kComboY + kComboHeight + 10;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, outsideX, outsideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, outsideX, outsideY));

        REQUIRE_FALSE(cb->isExpanded());
    }
}

TEST_CASE("ComboBox - Adding Items", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("addItem with string literal adds item") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->addItem("Item1");
        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "Item1");
    }

    SECTION("addItem with std::string adds item") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        std::string item = "Item1";
        cb->addItem(item);
        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "Item1");
    }

    SECTION("addItem with string&& adds item") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->addItem(std::string("Item1"));
        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "Item1");
    }

    SECTION("Multiple items can be added") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");
        cb->addItem("Fourth");

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }

    SECTION("Adding items after empty initialization works") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getSelectedIndex() == -1);

        cb->addItem("First");

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }
}

TEST_CASE("ComboBox - Programmatic Selection", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelectedIndex changes selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        cb->setSelectedIndex(1);

        REQUIRE(cb->getSelectedIndex() == 1);
        REQUIRE(cb->getSelectedItem() == "Second");
    }

    SECTION("setSelectedIndex fires on_selection_changed callback") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        int callbackIndex = -2;
        std::string callbackText;
        cb->on_selection_changed = [&](int index, const std::string& text) {
            callbackIndex = index;
            callbackText = text;
        };

        cb->setSelectedIndex(2);

        REQUIRE(callbackIndex == 2);
        REQUIRE(callbackText == "Third");
    }

    SECTION("setSelectedIndex to same value does not fire callback") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        int callbackCount = 0;
        cb->on_selection_changed = [&](int, const std::string&) {
            callbackCount++;
        };

        cb->setSelectedIndex(0);

        REQUIRE(callbackCount == 0);
        REQUIRE(cb->getSelectedIndex() == 0);
    }

    SECTION("setSelectedIndex with -1 does not change selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        cb->setSelectedIndex(-1);

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }

    SECTION("setSelectedIndex with invalid index does not change selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        cb->setSelectedIndex(100);

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }

    SECTION("setSelectedIndex with negative value does not change selection") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        cb->setSelectedIndex(-5);

        REQUIRE(cb->getSelectedIndex() == 0);
    }

    SECTION("setSelectedIndex with valid index updates correctly") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        cb->setSelectedIndex(0);
        REQUIRE(cb->getSelectedIndex() == 0);

        cb->setSelectedIndex(2);
        REQUIRE(cb->getSelectedIndex() == 2);

        cb->setSelectedIndex(1);
        REQUIRE(cb->getSelectedIndex() == 1);
    }
}

TEST_CASE("ComboBox - Empty ComboBox", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Empty ComboBox has getSelectedIndex returns -1") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getSelectedIndex() == -1);
    }

    SECTION("Empty ComboBox getSelectedItem returns empty string") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getSelectedItem() == "");
    }

    SECTION("Click on empty ComboBox can expand") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE(cb->isExpanded());
    }

    SECTION("Clicking on empty expanded ComboBox collapses it") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE(cb->isExpanded());

        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("setSelectedIndex on empty ComboBox does nothing") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        cb->setSelectedIndex(0);

        REQUIRE(cb->getSelectedIndex() == -1);
    }
}

TEST_CASE("ComboBox - Disabled State", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Disabled ComboBox ignores clicks to expand") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->setEnabled(false);

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("Disabled ComboBox cannot expand dropdown") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->setEnabled(false);

        REQUIRE_FALSE(cb->isExpanded());

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("Disabled ComboBox selection unchanged after click") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->setEnabled(false);

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
    }

    SECTION("setEnabled(true) allows interaction again") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->setEnabled(false);

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        REQUIRE_FALSE(cb->isExpanded());

        cb->setEnabled(true);
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));

        REQUIRE(cb->isExpanded());
    }
}

TEST_CASE("ComboBox - Multiple Selection Changes", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Multiple selection changes fire callback each time") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");
        cb->addItem("Third");

        int callbackCount = 0;
        cb->on_selection_changed = [&](int, const std::string&) {
            callbackCount++;
        };

        cb->setSelectedIndex(1);
        cb->setSelectedIndex(2);
        cb->setSelectedIndex(0);

        REQUIRE(callbackCount == 3);
    }

    SECTION("Selecting same item multiple times via UI") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));
        cb->addItem("First");
        cb->addItem("Second");

        int callbackCount = 0;
        cb->on_selection_changed = [&](int, const std::string&) {
            callbackCount++;
        };

        int insideX = kComboX + 5;
        int insideY = kComboY + 5;
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, insideY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, insideY));
        manager.render();

        int itemY = kComboY + kComboHeight + kItemHeight / 2;
        manager.processEvent(helper.createMouseMotion(insideX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, insideX, itemY));
        manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, insideX, itemY));

        REQUIRE(callbackCount == 0);
    }
}

TEST_CASE("ComboBox - Position and Size", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("ComboBox position is set correctly") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getX() == kComboX);
        REQUIRE(cb->getY() == kComboY);
    }

    SECTION("ComboBox size is set correctly") {
        auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
        ComboBox* cb = combo.get();
        manager.addElement(std::move(combo));

        REQUIRE(cb->getWidth() == kComboWidth);
        REQUIRE(cb->getHeight() == kComboHeight);
    }
}