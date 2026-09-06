#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "panel.hpp"
#include "button.hpp"

// Regression: widgets must actually render pixels (ScopedRenderTarget clip restore
// used to enable a 0x0 clip rect and blank the whole frame).
TEST_CASE("Rendering produces opaque widget pixels", "[render][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    SDL_Color bg = {45, 48, 58, 255};
    auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 100);
    panel->setBackgroundColor(ElementState::Normal, bg);
    panel->setBorderRadius(ElementState::Normal, 8);
    manager.addElement(std::move(panel));

    auto btn = std::make_unique<Button>(manager, 30, 130, 120, 40, "Test");
    btn->setBackgroundColor(ElementState::Normal, {200, 100, 50, 255});
    manager.addElement(std::move(btn));

    manager.update();
    manager.cleanup();
    manager.render();

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        Uint8* p = (Uint8*)surf->pixels;
        auto result = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
        SDL_DestroySurface(surf);
        return result;
    };

    SECTION("panel center matches its background color") {
        auto px = readPixel(110, 60);
        REQUIRE(px[0] == bg.r);
        REQUIRE(px[1] == bg.g);
        REQUIRE(px[2] == bg.b);
    }

    SECTION("button is opaque and colored (pixel away from label text)") {
        auto px = readPixel(45, 145);
        REQUIRE(px[3] > 200);
        REQUIRE(px[0] == 200);
        REQUIRE(px[1] == 100);
    }

    SECTION("empty area stays transparent (no clip bleed)") {
        auto px = readPixel(250, 200);
        REQUIRE(px[3] == 0);
    }
}
