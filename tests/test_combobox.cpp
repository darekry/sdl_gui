#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/combobox.hpp"
#include "../src/gui_manager.hpp"

namespace {
constexpr int kComboX = 50;
constexpr int kComboY = 40;
constexpr int kComboWidth = 160;
constexpr int kComboHeight = 24;
constexpr int kItemHeight = 30; // must match implementation detail in combobox.cpp
}

TEST_CASE("ComboBox behaviour", "[combobox]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto combo = std::make_unique<ComboBox>(manager, kComboX, kComboY, kComboWidth, kComboHeight);
    ComboBox* cb = combo.get();
    manager.addElement(std::move(combo));

    cb->addItem("First");
    cb->addItem("Second");
    cb->addItem("Third");

    SECTION("First item becomes selected by default") {
        REQUIRE(cb->getSelectedIndex() == 0);
        REQUIRE(cb->getSelectedItem() == "First");
        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("Selecting a new option updates selection and collapses the dropdown") {
        int callbackIndex = -1;
        std::string callbackLabel;
        cb->on_selection_changed = [&](int index, const std::string& label) {
            callbackIndex = index;
            callbackLabel = label;
        };

        const int insideMainX = kComboX + 5;
        const int insideMainY = kComboY + 5;
        manager.processEvent(helper.createMouseMotion(insideMainX, insideMainY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, insideMainX, insideMainY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, insideMainX, insideMainY));
        REQUIRE(cb->isExpanded());

        // Ensure dropdown children are created
        manager.render();

        const int optionX = kComboX + 10;
        const int optionY = kComboY + kComboHeight + kItemHeight / 2 + kItemHeight; // second entry
        manager.processEvent(helper.createMouseMotion(optionX, optionY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, optionX, optionY));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, optionX, optionY));

        REQUIRE(callbackIndex == 1);
        REQUIRE(callbackLabel == "Second");
        REQUIRE(cb->getSelectedIndex() == 1);
        REQUIRE(cb->getSelectedItem() == "Second");
        REQUIRE_FALSE(cb->isExpanded());
    }

    SECTION("Clicking outside while expanded collapses the dropdown without changing selection") {
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, kComboX + 5, kComboY + 5));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, kComboX + 5, kComboY + 5));
        REQUIRE(cb->isExpanded());

        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, kComboX - 20, kComboY - 20));
        manager.processEvent(helper.createMouseButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, kComboX - 20, kComboY - 20));
        REQUIRE_FALSE(cb->isExpanded());
        REQUIRE(cb->getSelectedIndex() == 0);
    }
}
