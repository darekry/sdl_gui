#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "checkbox.hpp"

// Regression: render cache must be shared between identical elements
// (same type, size, state, composed style) - one entry in TextureManager,
// not one texture per element. State transitions (hover) must reuse
// existing entries instead of re-rendering per element.
TEST_CASE("Render cache is shared between identical elements", "[render][cache]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.setWindowSize(640, 480);

    auto makeButton = [&](int x, bool distinct = false) {
        auto b = std::make_unique<Button>(manager, x, 10, 120, 40, "");
        if (distinct) {
            b->setBorder(ElementState::Normal, {200, 50, 50, 255}, 3);
        }
        return b;
    };

    auto b1 = makeButton(10);
    auto b2 = makeButton(140);
    auto b3 = makeButton(270, true);
    auto* b1ptr = b1.get();
    auto* b2ptr = b2.get();
    auto* b3ptr = b3.get();
    manager.addElement(std::move(b1));
    manager.addElement(std::move(b2));
    manager.addElement(std::move(b3));

    manager.update();
    manager.cleanup();
    manager.render();

    auto& texManager = manager.getTextureManager();

    SECTION("identical buttons share one render cache entry") {
        REQUIRE(texManager.getRenderCacheSize() == 2);
    }

    SECTION("hover creates one shared Hover entry, not per element") {
        auto hover1 = helper.createMouseMotion(30, 30);
        manager.processEvent(hover1);
        manager.render();
        REQUIRE(b1ptr->getState() == ElementState::Hover);
        REQUIRE(texManager.getRenderCacheSize() == 3);

        auto hover2 = helper.createMouseMotion(160, 30);
        manager.processEvent(hover2);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 3);
        REQUIRE(b1ptr->getState() == ElementState::Normal);
        REQUIRE(b2ptr->getState() == ElementState::Hover);

        auto hover1Again = helper.createMouseMotion(30, 30);
        manager.processEvent(hover1Again);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 3);
    }

    SECTION("different style means separate entry") {
        auto hover3 = helper.createMouseMotion(300, 30);
        manager.processEvent(hover3);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 3);
        REQUIRE(b3ptr->getState() == ElementState::Hover);

        auto hover1 = helper.createMouseMotion(30, 30);
        manager.processEvent(hover1);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 4);
        REQUIRE(b1ptr->getState() == ElementState::Hover);
    }

    SECTION("opt-out widgets bypass the shared cache") {
        auto c1 = std::make_unique<Checkbox>(manager, 10, 100, 24, 24);
        auto c2 = std::make_unique<Checkbox>(manager, 40, 100, 24, 24);
        manager.addElement(std::move(c1));
        manager.addElement(std::move(c2));
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 2);
    }
}
