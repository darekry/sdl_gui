#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/text_area.hpp"
#include "../src/context_menu.hpp"
#include "../src/gui_manager.hpp"
#include "../src/theme.hpp"

static TextArea* makeArea(GUIManager& manager) {
    auto ta = std::make_unique<TextArea>(manager, 100, 100, 300, 200, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));
    area->setText("line1\nline2\nline3");
    return area;
}

static void renderAll(GUIManager& manager) {
    manager.update();
    manager.cleanup();
    manager.render();
}

TEST_CASE("BISECT V1 - setText + render", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    makeArea(manager);
    renderAll(manager);
    CHECK(true);
}

TEST_CASE("BISECT V2 - V1 + selectAll + render", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    auto* area = makeArea(manager);
    area->selectAll();
    renderAll(manager);
    CHECK(true);
}

TEST_CASE("BISECT V3 - V2 + LMB + render", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    auto* area = makeArea(manager);
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 150, 130));
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 150, 130));
    area->selectAll();
    renderAll(manager);
    CHECK(true);
}

TEST_CASE("BISECT V5 - verbatim copy of failing pixel test", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());

    auto ta = std::make_unique<TextArea>(manager, 100, 100, 300, 200, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));
    area->setText("line1\nline2\nline3");

    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 150, 130));
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 150, 130));
    area->selectAll();
    REQUIRE(area->hasSelection());

    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
    REQUIRE(manager.isContextMenuVisible());

    manager.update();
    manager.cleanup();
    manager.render();
    CHECK(true);
}

TEST_CASE("BISECT V6 - debug geometry and focus", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());

    auto ta = std::make_unique<TextArea>(manager, 100, 100, 300, 200, "assets/fonts/font.ttf", 16);
    TextArea* area = ta.get();
    manager.addElement(std::move(ta));
    area->setText("line1\nline2\nline3");

    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 150, 130));
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 150, 130));
    area->selectAll();

    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));

    auto* menu = manager.getContextMenu();
    REQUIRE(menu != nullptr);
    printf("DBG menu visible=%d pos=(%.1f,%.1f) size=(%.1f,%.1f)\n",
           (int)manager.isContextMenuVisible(), (double)menu->getX(), (double)menu->getY(),
           (double)menu->getWidth(), (double)menu->getHeight());
    printf("DBG focus=%s (area=%p)\n",
           manager.getKeyboardFocus() == area ? "TextArea" : "OTHER",
           (void*)area);

    renderAll(manager);

    auto sample = [&](int x, int y, const char* tag) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        if (!surf) { printf("DBG %s: no surf\n", tag); return; }
        Uint8* px = static_cast<Uint8*>(surf->pixels);
        printf("DBG %s (%d,%d): %d,%d,%d\n", tag, x, y, px[0], px[1], px[2]);
        SDL_DestroySurface(surf);
    };
    sample(208, 162, "menu-item-pad");
    sample(120, 115, "selected-text");
    sample(50, 50, "background");
    CHECK(true);
}

TEST_CASE("BISECT V4 - V3 + RMB menu + render", "[bisect]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    auto* area = makeArea(manager);
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_LEFT, 150, 130));
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_UP, SDL_BUTTON_LEFT, 150, 130));
    area->selectAll();
    manager.processEvent(helper.createMouseEvent(SDL_EVENT_MOUSE_BUTTON_DOWN, SDL_BUTTON_RIGHT, 200, 150));
    REQUIRE(manager.isContextMenuVisible());
    renderAll(manager);
    CHECK(true);
}
