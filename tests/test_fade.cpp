#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "../src/gui_manager.hpp"
#include "../src/panel.hpp"

#include "std.hpp"

TEST_CASE("Fade - opacity clamp and parent-chain inheritance", "[fade][opacity]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
    auto child = std::make_unique<Panel>(manager, 10, 10, 50, 30);
    Panel* childRaw = child.get();
    panel->addChild(std::move(child));
    Panel* panelRaw = panel.get();
    manager.addElement(std::move(panel));

    SECTION("clamp to [0, 1]") {
        panelRaw->setOpacity(2.0f);
        REQUIRE(panelRaw->getOpacity() == 1.0f);
        panelRaw->setOpacity(-1.0f);
        REQUIRE(panelRaw->getOpacity() == 0.0f);
    }

    SECTION("effective opacity is the product over the parent chain") {
        panelRaw->setOpacity(0.5f);
        REQUIRE(childRaw->getEffectiveOpacity() == Catch::Approx(0.5f));
        childRaw->setOpacity(0.5f);
        REQUIRE(childRaw->getEffectiveOpacity() == Catch::Approx(0.25f));
        REQUIRE(panelRaw->getEffectiveOpacity() == Catch::Approx(0.5f));
    }
}

TEST_CASE("Fade - fadeOut with zero duration completes synchronously", "[fade]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
    Panel* raw = panel.get();
    manager.addElement(std::move(panel));

    bool done = false;
    raw->fadeOut(0, [&done]() { done = true; });
    REQUIRE(done);
    REQUIRE(raw->getOpacity() == 0.0f);
}

static void pump(GUIManager& manager, uint32_t budgetMs) {
    const Uint64 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < budgetMs) {
        manager.update();
        manager.cleanup();
        SDL_Delay(5);
    }
}

TEST_CASE("Fade - fadeOutAndDelete removes the element after the fade", "[fade]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
    Panel* raw = panel.get();
    auto ref = manager.makeRef(raw);
    manager.addElement(std::move(panel));

    bool midFadeSeen = false;
    raw->fadeOutAndDelete(60);
    // Pump until the element is gone (generous budget for loaded CI runners).
    const Uint64 start = SDL_GetTicks();
    while (ref && SDL_GetTicks() - start < 3000) {
        manager.update();
        manager.cleanup();
        if (ref && ref->getOpacity() < 1.0f) {
            midFadeSeen = true;
        }
        SDL_Delay(5);
    }
    REQUIRE(midFadeSeen); // opacity actually animated down, not snapped
    REQUIRE(!ref);        // markForDeletion + cleanup erased it
}

TEST_CASE("Fade - deleting the element mid-fade is safe (no crash, no callback)", "[fade]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
    Panel* raw = panel.get();
    auto ref = manager.makeRef(raw);
    manager.addElement(std::move(panel));

    bool done = false;
    raw->fadeOut(5000, [&done]() { done = true; });
    raw->markForDeletion();
    manager.cleanup();
    REQUIRE(!ref);

    pump(manager, 120); // leftover ticks must become no-ops, not UB
    REQUIRE(!done);
}
