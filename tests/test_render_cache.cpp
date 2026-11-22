#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "label.hpp"
#include "panel.hpp"

// Regression: render cache must be shared between identical elements
// (same type, size, state, composed style) - one entry in TextureManager,
// not one texture per element. State transitions (hover) must reuse
// existing entries instead of re-rendering per element.
TEST_CASE("Render cache is shared between identical elements", "[render][cache]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(640, 480);

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
        // Weak cache: b3's Normal entry is released (and erased) when it moves
        // to its distinct Hover entry, so the count does not grow by one per
        // state transition — only live holders count.
        auto hover3 = helper.createMouseMotion(300, 30);
        manager.processEvent(hover3);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 2);
        REQUIRE(b3ptr->getState() == ElementState::Hover);

        auto hover1 = helper.createMouseMotion(30, 30);
        manager.processEvent(hover1);
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 3);
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

// Weak cache lifetime: the widget is the holder. Entries disappear with the
// last holder (explicit release), never via a periodic prune.
TEST_CASE("Render cache entries follow widget lifetime", "[render][cache][lifetime]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(640, 480);
    auto& texManager = manager.getTextureManager();

    SECTION("last holder destruction releases the shared entry") {
        auto p1 = std::make_unique<Panel>(manager, 10, 10, 60, 30);
        p1->setBackgroundColor(ElementState::Normal, SDL_Color{45, 48, 58, 255});
        auto p2 = std::make_unique<Panel>(manager, 100, 10, 60, 30);
        p2->setBackgroundColor(ElementState::Normal, SDL_Color{45, 48, 58, 255});
        Panel* raw1 = p1.get();
        Panel* raw2 = p2.get();
        manager.addElement(std::move(p1));
        manager.addElement(std::move(p2));
        manager.render();

        const size_t afterRender = texManager.getRenderCacheSize();
        REQUIRE(afterRender == 1);

        auto detached1 = manager.detachElement(raw1);
        REQUIRE(detached1);
        detached1.reset();
        REQUIRE(texManager.getRenderCacheSize() == 1); // raw2 still holds it

        auto detached2 = manager.detachElement(raw2);
        REQUIRE(detached2);
        detached2.reset();
        REQUIRE(texManager.getRenderCacheSize() == 0); // last holder gone
    }

    SECTION("dynamic label text releases the old key") {
        auto label = std::make_unique<Label>(manager, 10, 10, "first", 16);
        Label* raw = label.get();
        manager.addElement(std::move(label));
        manager.render();
        const size_t oldEntry = texManager.getRenderCacheSize();
        REQUIRE(oldEntry == 1);

        raw->setText("second");
        manager.render();
        // New key is live, old (sole holder) is released: the map does not grow.
        REQUIRE(texManager.getRenderCacheSize() == 1);
    }

    SECTION("GUIManager::update does not prune expired entries") {
        auto p1 = std::make_unique<Panel>(manager, 10, 10, 60, 30);
        Panel* raw1 = p1.get();
        manager.addElement(std::move(p1));
        manager.render();
        REQUIRE(texManager.getRenderCacheSize() == 1);

        auto detached = manager.detachElement(raw1);
        detached.reset(); // entry released explicitly by ~GUIElement
        // Re-create an expired-but-unreleased entry through the public API.
        auto held = texManager.renderCache(0xabcdefull, 20, 20, [](SDL_Renderer*) {});
        REQUIRE(held);
        const size_t before = texManager.getRenderCacheSize();
        held.reset();
        manager.update();
        REQUIRE(texManager.getRenderCacheSize() == before); // no sweep
        texManager.releaseRenderCache(0xabcdefull);
        REQUIRE(texManager.getRenderCacheSize() == 0);
    }
}
