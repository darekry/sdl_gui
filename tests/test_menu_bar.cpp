#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"

#include "../src/gui_manager.hpp"
#include "../src/menu_bar.hpp"
#include "../src/menu_popup.hpp"
#include "../src/text_input.hpp"

namespace {

void clickAt(GUIManager& manager, TestHelper& helper, int x, int y) {
    manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, x, y));
    manager.processEvent(helper.createMouseButton(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, x, y));
}

SDL_Point popupItemCenter(MenuPopup* popup, size_t index) {
    const SDL_Point origin = popup->getAbsolutePosition();
    const SDL_Rect item = popup->getItemRect(index);
    return SDL_Point{origin.x + item.x + item.w / 2, origin.y + item.y + item.h / 2};
}

MenuBar* makeBar(GUIManager& manager, int x = 0, int width = 800) {
    auto bar = std::make_unique<MenuBar>(manager, x, 0, width);
    MenuBar* raw = bar.get();
    manager.addElement(std::move(bar));
    return raw;
}

MenuBar* makeCivilizationBar(GUIManager& manager, bool* saved, bool* quit) {
    MenuBar* bar = makeBar(manager);
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&Save Game", [saved]() { *saved = true; }).setShortcut("Ctrl+S");
    file.addSeparator();
    file.addItem("&Tax Rate").setShortcut("Ctrl+T");
    file.addItem("&Luxuries Rate").setShortcut("Ctrl+L");
    file.addItem("&Find City...").setShortcut("Ctrl+F");
    file.addSubmenu("&Options")
        .addItem("&Instant Advice").setChecked(true);
    file.addSeparator();
    file.addItem("&Revolution").setShortcut("Ctrl+R");
    file.addItem("&Retire");
    file.addItem("&Quit", [quit]() { *quit = true; }).setShortcut("Ctrl+Q");
    bar->addMenu("&Edit").addItem("&Undo").setShortcut("Ctrl+Z");
    return bar;
}

} // namespace

TEST_CASE("Menu text helpers - mnemonics and shortcuts", "[menu][model]") {
    SECTION("display text strips lone ampersands, keeps doubled ones") {
        REQUIRE(menu_text::displayText("&File") == "File");
        REQUIRE(menu_text::displayText("E&xit") == "Exit");
        REQUIRE(menu_text::displayText("Save && Quit") == "Save & Quit");
        REQUIRE(menu_text::displayText("No mnemonic") == "No mnemonic");
    }

    SECTION("mnemonic character and its display index") {
        REQUIRE(menu_text::mnemonic("&File") == 'F');
        REQUIRE(menu_text::mnemonic("E&xit") == 'x');
        REQUIRE(menu_text::mnemonic("&&literal") == '\0');
        REQUIRE(menu_text::mnemonicDisplayIndex("&File") == 0);
        REQUIRE(menu_text::mnemonicDisplayIndex("E&xit") == 1);
        REQUIRE(menu_text::mnemonicDisplayIndex("Save && Quit") == std::string::npos);
    }

    SECTION("shortcut strings parse into key + modifiers") {
        SDL_Keycode key = SDLK_UNKNOWN;
        SDL_Keymod mod = SDL_KMOD_NONE;

        REQUIRE(menu_text::parseShortcutString("Ctrl+S", key, mod));
        REQUIRE(key == SDLK_S);
        REQUIRE((mod & SDL_KMOD_CTRL) != 0);
        REQUIRE((mod & (SDL_KMOD_SHIFT | SDL_KMOD_ALT)) == 0);

        REQUIRE(menu_text::parseShortcutString("Ctrl+Shift+Q", key, mod));
        REQUIRE(key == SDLK_Q);
        REQUIRE((mod & SDL_KMOD_CTRL) != 0);
        REQUIRE((mod & SDL_KMOD_SHIFT) != 0);

        REQUIRE(menu_text::parseShortcutString("F10", key, mod));
        REQUIRE(key == SDLK_F10);
        REQUIRE(mod == SDL_KMOD_NONE);

        REQUIRE(menu_text::parseShortcutString("Esc", key, mod));
        REQUIRE(key == SDLK_ESCAPE);

        // Overlong F-key numbers are rejected, never accumulated into int overflow.
        REQUIRE_FALSE(menu_text::parseShortcutString("F99999999999", key, mod));
    }

    SECTION("MenuItem builder sets display string, command and check state") {
        MenuItem item("&Save");
        item.setShortcut("Ctrl+S").setCommand("file.save").setChecked(true);
        REQUIRE(item.displayText() == "Save");
        REQUIRE(item.shortcut == "Ctrl+S");
        REQUIRE(item.shortcutKey == SDLK_S);
        REQUIRE(item.command == "file.save");
        REQUIRE(item.check == MenuCheck::Check);
        REQUIRE(item.checked);

        item.setRadio(false);
        REQUIRE(item.check == MenuCheck::Radio);
        REQUIRE_FALSE(item.checked);
    }
}

TEST_CASE("MenuBar - tree and title geometry", "[menu][bar]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&New");
    file.addSeparator();
    file.addItem("&Open");
    bar->addMenu("&Edit");

    REQUIRE(bar->getMenuCount() == 2);
    REQUIRE(bar->getHeight() > 0);
    REQUIRE(bar->getTitleRect(0).w > 0);
    REQUIRE(bar->getTitleRect(1).w > 0);
    REQUIRE(bar->getTitleRect(0).x < bar->getTitleRect(1).x);

    REQUIRE(file.itemCount() == 3);
    REQUIRE(file.itemAt(1) != nullptr);
    REQUIRE(file.itemAt(1)->separator);
    REQUIRE_FALSE(bar->getPopupFor(&file) == nullptr);

    // Popups exist for every node with children, including nested submenus.
    MenuItem& options = file.addSubmenu("&Options");
    options.addItem("&Sound");
    REQUIRE_FALSE(bar->getPopupFor(&options) == nullptr);
}

TEST_CASE("MenuBar - mouse open / activate / close", "[menu][bar][mouse]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    bool saved = false;
    bool quit = false;
    MenuBar* bar = makeCivilizationBar(manager, &saved, &quit);
    REQUIRE(manager.getMenuBar() == bar);

    auto titlePoint = [&](size_t index) {
        const SDL_Rect rect = bar->getTitleRect(index);
        const SDL_Point absolute = bar->getAbsolutePosition();
        return SDL_Point{absolute.x + rect.x + rect.w / 2, absolute.y + rect.h / 2};
    };

    SECTION("clicking a title opens its popup below the bar") {
        const SDL_Point filePoint = titlePoint(0);
        clickAt(manager, helper, filePoint.x, filePoint.y);

        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenMenuIndex() == 0);
        REQUIRE(bar->getOpenPopupDepth() == 1);

        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);
        REQUIRE(popup->isOpen());
        REQUIRE(popup->getY() >= bar->getAbsolutePosition().y + bar->getHeight());
        REQUIRE(manager.getKeyboardFocus() == popup);

        // File has 9 rows: Save, sep, Tax, Luxuries, Find, Options, sep, Revolution, Retire, Quit.
        REQUIRE(popup->itemCount() == 10);
    }

    SECTION("clicking an item fires its action and closes the menu") {
        const SDL_Point filePoint = titlePoint(0);
        clickAt(manager, helper, filePoint.x, filePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);

        const SDL_Point save = popupItemCenter(popup, 0);
        clickAt(manager, helper, save.x, save.y);

        REQUIRE(saved);
        REQUIRE_FALSE(bar->isMenuOpen());
        REQUIRE_FALSE(popup->isOpen());
    }

    SECTION("disabled and separator rows do not fire or close") {
        MenuItem& file = *bar->getMenu(0);
        bool fired = false;
        MenuItem& disabled = file.addItem("Disabled", [&fired]() { fired = true; }, false);
        REQUIRE_FALSE(disabled.enabled);

        const SDL_Point filePoint = titlePoint(0);
        clickAt(manager, helper, filePoint.x, filePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);

        const SDL_Point disabledPoint = popupItemCenter(popup, 10);
        clickAt(manager, helper, disabledPoint.x, disabledPoint.y);
        REQUIRE_FALSE(fired);
        REQUIRE(bar->isMenuOpen());

        const SDL_Point separatorPoint = popupItemCenter(popup, 1);
        clickAt(manager, helper, separatorPoint.x, separatorPoint.y);
        REQUIRE(bar->isMenuOpen());
    }

    SECTION("clicking the same title closes the menu (toggle)") {
        const SDL_Point filePoint = titlePoint(0);
        clickAt(manager, helper, filePoint.x, filePoint.y);
        REQUIRE(bar->isMenuOpen());

        clickAt(manager, helper, filePoint.x, filePoint.y);
        REQUIRE_FALSE(bar->isMenuOpen());
    }

    SECTION("clicking another title switches the open menu") {
        const SDL_Point filePoint = titlePoint(0);
        const SDL_Point editPoint = titlePoint(1);
        clickAt(manager, helper, filePoint.x, filePoint.y);
        REQUIRE(bar->getOpenMenuIndex() == 0);

        clickAt(manager, helper, editPoint.x, editPoint.y);
        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenMenuIndex() == 1);
        REQUIRE(bar->getOpenPopupDepth() == 1);
    }

    SECTION("clicking outside closes without firing an action") {
        const SDL_Point filePoint = titlePoint(0);
        clickAt(manager, helper, filePoint.x, filePoint.y);
        REQUIRE(bar->isMenuOpen());

        clickAt(manager, helper, 700, 500);
        REQUIRE_FALSE(bar->isMenuOpen());
        REQUIRE_FALSE(saved);
    }
}

TEST_CASE("MenuBar - check and radio items", "[menu][bar][check]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    MenuItem& view = bar->addMenu("&View");
    int toggles = 0;
    MenuItem& grid = view.addItem("&Grid", [&toggles]() { ++toggles; }).setChecked(true);
    MenuItem& snap = view.addItem("&Snap", [&toggles]() { ++toggles; }).setChecked(false);
    MenuItem& small = view.addItem("&Small", [&toggles]() { ++toggles; }).setRadio(true);
    MenuItem& large = view.addItem("&Large", [&toggles]() { ++toggles; }).setRadio(false);

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    const SDL_Point titlePoint{absolute.x + title.x + title.w / 2, absolute.y + title.h / 2};

    SECTION("check item toggles every activation") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);

        SDL_Point point = popupItemCenter(popup, 0);
        clickAt(manager, helper, point.x, point.y);
        REQUIRE_FALSE(grid.checked);
        REQUIRE_FALSE(snap.checked);
        REQUIRE(toggles == 1);
        REQUIRE_FALSE(bar->isMenuOpen());

        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        popup = bar->getTopPopup();
        point = popupItemCenter(popup, 0);
        clickAt(manager, helper, point.x, point.y);
        REQUIRE(grid.checked);
        REQUIRE(toggles == 2);
    }

    SECTION("radio item clears its siblings in the same popup") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);

        SDL_Point point = popupItemCenter(popup, 3); // Large
        clickAt(manager, helper, point.x, point.y);
        REQUIRE(large.checked);
        REQUIRE_FALSE(small.checked);
        REQUIRE(toggles == 1);
    }
}

TEST_CASE("MenuBar - submenus", "[menu][bar][submenu]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    int soundClicks = 0;
    MenuItem& file = bar->addMenu("&File");
    MenuItem& options = file.addSubmenu("&Options");
    options.addItem("&Sound", [&soundClicks]() { ++soundClicks; }).setChecked(true);
    options.addItem("&Music");

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    const SDL_Point titlePoint{absolute.x + title.x + title.w / 2, absolute.y + title.h / 2};

    SECTION("hovering a submenu parent opens the child popup") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* top = bar->getTopPopup();
        REQUIRE(top != nullptr);

        const SDL_Point optionsPoint = popupItemCenter(top, 0);
        manager.processEvent(helper.createMouseMotion(optionsPoint.x, optionsPoint.y));

        REQUIRE(bar->getOpenPopupDepth() == 2);
        MenuPopup* child = bar->getTopPopup();
        REQUIRE(child != nullptr);
        REQUIRE(child != top);
        REQUIRE(child->isOpen());
        REQUIRE(child->getX() >= top->getX());

        const SDL_Point soundPoint = popupItemCenter(child, 0);
        clickAt(manager, helper, soundPoint.x, soundPoint.y);
        REQUIRE(soundClicks == 1);
        REQUIRE_FALSE(bar->isMenuOpen());
    }

    SECTION("Escape closes one level, the second Escape closes everything") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* top = bar->getTopPopup();
        const SDL_Point optionsPoint = popupItemCenter(top, 0);
        manager.processEvent(helper.createMouseMotion(optionsPoint.x, optionsPoint.y));
        REQUIRE(bar->getOpenPopupDepth() == 2);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));
        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenPopupDepth() == 1);
        REQUIRE(manager.getKeyboardFocus() == bar->getTopPopup());

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));
        REQUIRE_FALSE(bar->isMenuOpen());
    }

}

TEST_CASE("MenuBar - submenu flips at the right window edge", "[menu][bar][submenu][position]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto edgeBar = std::make_unique<MenuBar>(manager, 640, 0, 160);
    MenuBar* edge = edgeBar.get();
    manager.addElement(std::move(edgeBar));
    MenuItem& file = edge->addMenu("&File");
    file.addSubmenu("&Options").addItem("&Sound");

    const SDL_Rect title = edge->getTitleRect(0);
    const SDL_Point absolute = edge->getAbsolutePosition();
    clickAt(manager, helper, absolute.x + title.x + title.w / 2, absolute.y + title.h / 2);

    MenuPopup* top = edge->getTopPopup();
    REQUIRE(top != nullptr);
    const SDL_Point optionsPoint = popupItemCenter(top, 0);
    manager.processEvent(helper.createMouseMotion(optionsPoint.x, optionsPoint.y));

    REQUIRE(edge->getOpenPopupDepth() == 2);
    MenuPopup* child = edge->getTopPopup();
    REQUIRE(child != top);
    REQUIRE(child->getX() < top->getX());
    REQUIRE(child->getX() >= 0);
}

TEST_CASE("MenuBar - keyboard navigation", "[menu][bar][keyboard]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    int action = 0;
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&A", [&action]() { action = 1; }, false);
    file.addSeparator();
    file.addItem("&B", [&action]() { action = 2; });
    file.addItem("&C", [&action]() { action = 3; });
    bar->addMenu("&Edit").addItem("Undo");

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    const SDL_Point titlePoint{absolute.x + title.x + title.w / 2, absolute.y + title.h / 2};

    SECTION("Down skips disabled rows and separators, Enter activates") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);
        REQUIRE(popup->getHoveredIndex() == -1);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN));
        REQUIRE(popup->getHoveredIndex() == 2); // B (A disabled, separator skipped)

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN));
        REQUIRE(popup->getHoveredIndex() == 3); // C

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RETURN));
        REQUIRE(action == 3);
        REQUIRE_FALSE(bar->isMenuOpen());
    }

    SECTION("mnemonic letters activate items while the menu is open") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_C));
        REQUIRE(action == 3);
        REQUIRE_FALSE(bar->isMenuOpen());

        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_B, SDL_KMOD_ALT));
        REQUIRE(action == 2);
        REQUIRE_FALSE(bar->isMenuOpen());
    }

    SECTION("Right/Left switch top-level menus while open") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        REQUIRE(bar->getOpenMenuIndex() == 0);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_RIGHT));
        REQUIRE(bar->getOpenMenuIndex() == 1);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LEFT));
        REQUIRE(bar->getOpenMenuIndex() == 0);
    }
}

TEST_CASE("MenuBar - Win32 accelerators (Alt/F10/shortcuts)", "[menu][bar][winapi]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    bool saved = false;
    bool quit = false;
    MenuBar* bar = makeCivilizationBar(manager, &saved, &quit);

    SECTION("Alt+F opens the File menu") {
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LALT, SDL_KMOD_ALT));
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_F, SDL_KMOD_ALT));

        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenMenuIndex() == 0);
        REQUIRE(bar->getTopPopup() != nullptr);
    }

    SECTION("Alt+E opens the Edit menu") {
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LALT, SDL_KMOD_ALT));
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_E, SDL_KMOD_ALT));

        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenMenuIndex() == 1);
    }

    SECTION("F10 enters menu mode, Down opens the highlighted menu") {
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_F10));
        REQUIRE(bar->isMenuMode());
        REQUIRE(manager.getKeyboardFocus() == bar);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_DOWN));
        REQUIRE(bar->isMenuOpen());
        REQUIRE(bar->getOpenMenuIndex() == 0);
    }

    SECTION("application shortcuts fire while menus are closed") {
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_S, SDL_KMOD_CTRL));
        REQUIRE(saved);
        REQUIRE_FALSE(bar->isMenuOpen());

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_Q, SDL_KMOD_CTRL));
        REQUIRE(quit);
    }

    SECTION("shortcuts do not fire while a menu is open") {
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LALT, SDL_KMOD_ALT));
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_F, SDL_KMOD_ALT));
        REQUIRE(bar->isMenuOpen());

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_S, SDL_KMOD_CTRL));
        REQUIRE_FALSE(saved);
    }

    SECTION("closing restores focus to the widget focused before the menu") {
        auto input = std::make_unique<TextInput>(manager, 20, 80, 200, 30);
        TextInput* field = input.get();
        manager.addElement(std::move(input));

        clickAt(manager, helper, 30, 90);
        REQUIRE(manager.getKeyboardFocus() == field);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_LALT, SDL_KMOD_ALT));
        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_F, SDL_KMOD_ALT));
        REQUIRE(bar->isMenuOpen());
        REQUIRE(manager.getKeyboardFocus() != field);

        manager.processEvent(helper.createKeyEvent(SDL_EVENT_KEY_DOWN, SDLK_ESCAPE));
        REQUIRE_FALSE(bar->isMenuOpen());
        REQUIRE(manager.getKeyboardFocus() == field);
    }
}

TEST_CASE("MenuBar - clearMenus detaches popup models", "[menu][bar][lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&Open");

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    clickAt(manager, helper, absolute.x + title.x + title.w / 2, absolute.y + title.h / 2);

    MenuPopup* popup = bar->getTopPopup();
    REQUIRE(popup != nullptr);
    REQUIRE(popup->itemCount() == 1);

    bar->clearMenus();
    REQUIRE(bar->getMenuCount() == 0);

    // The popup outlives the MenuItem tree until cleanup(), but its model
    // pointer must be detached so accessors never touch freed memory.
    REQUIRE(popup->itemCount() == 0);
    REQUIRE(popup->itemAt(0) == nullptr);
    popup->refresh();
    manager.cleanup();
}

TEST_CASE("MenuBar - menu mutation from an action is deferred safely", "[menu][bar][lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&Add Menu", [bar]() {
        MenuItem& added = bar->addMenu("&Added");
        added.addItem("&Child");
    });

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    clickAt(manager, helper, absolute.x + title.x + title.w / 2, absolute.y + title.h / 2);
    MenuPopup* popup = bar->getTopPopup();
    REQUIRE(popup != nullptr);
    const SDL_Point item = popupItemCenter(popup, 0);
    clickAt(manager, helper, item.x, item.y);

    // The action ran inside GUIManager::processEvent(); the popup it created
    // is appended only after propagation, so the event iterator stays valid.
    REQUIRE(bar->getMenuCount() == 2);
    REQUIRE(bar->getPopupFor(bar->getMenu(1)) != nullptr);
    manager.cleanup();
}

TEST_CASE("MenuBar - command handler and render", "[menu][bar][render]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    MenuBar* bar = makeBar(manager);
    std::string lastCommand;
    bar->setCommandHandler([&lastCommand](std::string_view command) {
        lastCommand = std::string(command);
    });
    MenuItem& file = bar->addMenu("&File");
    file.addItem("&Command").setCommand("file.command");

    const SDL_Rect title = bar->getTitleRect(0);
    const SDL_Point absolute = bar->getAbsolutePosition();
    const SDL_Point titlePoint{absolute.x + title.x + title.w / 2, absolute.y + title.h / 2};

    SECTION("items without action dispatch their command id") {
        clickAt(manager, helper, titlePoint.x, titlePoint.y);
        MenuPopup* popup = bar->getTopPopup();
        const SDL_Point point = popupItemCenter(popup, 0);
        clickAt(manager, helper, point.x, point.y);

        REQUIRE(lastCommand == "file.command");
        REQUIRE_FALSE(bar->isMenuOpen());
    }

    SECTION("bar and open popup render opaque pixels") {
        manager.handleResize(320, 240);
        manager.update();
        manager.cleanup();
        manager.render();

        auto readPixel = [&](int x, int y) {
            SDL_Rect r{x, y, 1, 1};
            SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
            REQUIRE(surf != nullptr);
            auto result = readbackPixel(surf);
            SDL_DestroySurface(surf);
            return result;
        };

        const SDL_Rect barRect = bar->getTitleRect(0);
        const SDL_Point barAbs = bar->getAbsolutePosition();
        // Right of the last title: menu-bar face color (Win95 default theme).
        auto barPixel = readPixel(320 - 5, barAbs.y + bar->getHeight() / 2);
        REQUIRE(barPixel[3] > 200);
        REQUIRE(barPixel[0] == 192);
        REQUIRE(barPixel[1] == 192);
        REQUIRE(barPixel[2] == 192);

        clickAt(manager, helper, barAbs.x + barRect.x + barRect.w / 2,
                barAbs.y + barRect.h / 2);
        REQUIRE(bar->isMenuOpen());
        manager.update();
        manager.cleanup();
        manager.render();

        MenuPopup* popup = bar->getTopPopup();
        REQUIRE(popup != nullptr);
        REQUIRE(popup->isOpen());
        REQUIRE(popup->isVisible());
        const SDL_Point popupAbs = popup->getAbsolutePosition();
        // Right padding of the first row, away from text/shortcut/arrow.
        auto popupPixel = readPixel(popupAbs.x + popup->getWidth() - 3,
                                    popupAbs.y + 10);
        REQUIRE(popupPixel[3] > 200);
        REQUIRE(popupPixel[0] == 255);
        REQUIRE(popupPixel[1] == 255);
        REQUIRE(popupPixel[2] == 255);
    }
}
