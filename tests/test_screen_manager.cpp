#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/screen_manager.hpp"
#include "../src/screen.hpp"
#include "../src/button.hpp"
#include "../src/panel.hpp"
#include "../src/label.hpp"

import std.compat;

// === Test Screens ===

class TestScreen : public Screen {
public:
    explicit TestScreen(const std::string& name) : m_name(name) {}
    
    std::string getName() const override { return m_name; }
    
    void onEnter(GUIManager& manager) override {
        m_enterCount++;
        m_lastManager = &manager;
        
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 100);
        m_panel = panel.get();
        manager.addElement(std::move(panel));
    }
    
    void onExit(GUIManager& manager) override {
        m_exitCount++;
        if (m_panel) {
            m_panel->markForDeletion();
        }
        m_panel = nullptr;
    }
    
    bool handleEvent(GUIManager& manager, const SDL_Event& e) override {
        m_eventCount++;
        return false;
    }
    
    void update(GUIManager& manager) override {
        m_updateCount++;
    }
    
    void render(GUIManager& manager, SDL_Renderer* renderer) override {
        m_renderCount++;
    }
    
    bool wantsPreProcessEvent() const override { return true; }
    
    int m_enterCount = 0;
    int m_exitCount = 0;
    int m_eventCount = 0;
    int m_updateCount = 0;
    int m_renderCount = 0;
    GUIManager* m_lastManager = nullptr;
    Panel* m_panel = nullptr;
    
private:
    std::string m_name;
};

class PreProcessScreen : public Screen {
public:
    std::string getName() const override { return "PreProcessScreen"; }
    
    bool wantsPreProcessEvent() const override { return true; }
    
    void onEnter(GUIManager& manager) override {
        auto panel = std::make_unique<Panel>(manager, 0, 0, 100, 100);
        manager.addElement(std::move(panel));
    }
    
    void onExit(GUIManager& manager) override {}
    
    bool handleEvent(GUIManager& manager, const SDL_Event& e) override {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            m_escPressed = true;
            return true;  // Consume event
        }
        return false;
    }
    
    void update(GUIManager& manager) override {}
    void render(GUIManager& manager, SDL_Renderer* renderer) override {}
    
    bool m_escPressed = false;
};

// === ScreenManager Tests ===

TEST_CASE("ScreenManager Construction", "[screen_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    
    SECTION("ScreenManager constructs with GUIManager reference") {
        ScreenManager screenManager(manager);
        REQUIRE(screenManager.getCurrentScreenName() == "");
        REQUIRE(screenManager.getCurrentScreen() == nullptr);
        REQUIRE(screenManager.getStackDepth() == 0);
    }
}

TEST_CASE("ScreenManager Screen Addition", "[screen_manager][add]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("addScreen adds screen successfully") {
        auto screen = std::make_unique<TestScreen>("test1");
        REQUIRE(screenManager.addScreen("test1", std::move(screen)));
        REQUIRE(screenManager.hasScreen("test1"));
        REQUIRE(screenManager.getScreen("test1") != nullptr);
    }
    
    SECTION("addScreen rejects duplicate name") {
        auto screen1 = std::make_unique<TestScreen>("test1");
        auto screen2 = std::make_unique<TestScreen>("test1");
        
        REQUIRE(screenManager.addScreen("test1", std::move(screen1)));
        REQUIRE_FALSE(screenManager.addScreen("test1", std::move(screen2)));
        REQUIRE(screenManager.getScreen("test1") != nullptr);
    }
    
    SECTION("addScreen with multiple screens") {
        screenManager.addScreen("menu", std::make_unique<TestScreen>("menu"));
        screenManager.addScreen("game", std::make_unique<TestScreen>("game"));
        screenManager.addScreen("settings", std::make_unique<TestScreen>("settings"));
        
        REQUIRE(screenManager.hasScreen("menu"));
        REQUIRE(screenManager.hasScreen("game"));
        REQUIRE(screenManager.hasScreen("settings"));
        REQUIRE_FALSE(screenManager.hasScreen("pause"));
    }
}

TEST_CASE("ScreenManager Screen Removal", "[screen_manager][remove]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("removeScreen removes existing screen") {
        screenManager.addScreen("test", std::make_unique<TestScreen>("test"));
        REQUIRE(screenManager.hasScreen("test"));
        
        REQUIRE(screenManager.removeScreen("test"));
        REQUIRE_FALSE(screenManager.hasScreen("test"));
    }
    
    SECTION("removeScreen fails for non-existent screen") {
        REQUIRE_FALSE(screenManager.removeScreen("nonexistent"));
    }
    
    SECTION("removeScreen fails for current screen") {
        screenManager.addScreen("test", std::make_unique<TestScreen>("test"));
        screenManager.changeScreen("test");
        
        REQUIRE_FALSE(screenManager.removeScreen("test"));
        REQUIRE(screenManager.hasScreen("test"));
    }
    
    SECTION("removeScreen fails for screen in stack") {
        screenManager.addScreen("base", std::make_unique<TestScreen>("base"));
        screenManager.addScreen("overlay", std::make_unique<TestScreen>("overlay"));
        screenManager.changeScreen("base");
        screenManager.pushScreen("overlay");
        
        REQUIRE_FALSE(screenManager.removeScreen("base"));
        REQUIRE_FALSE(screenManager.removeScreen("overlay"));
    }
}

TEST_CASE("ScreenManager Screen Switching", "[screen_manager][change]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("changeScreen activates screen and calls lifecycle") {
        auto screen = std::make_unique<TestScreen>("test");
        TestScreen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        
        REQUIRE(screenManager.changeScreen("test"));
        REQUIRE(screenManager.getCurrentScreenName() == "test");
        REQUIRE(screenPtr->m_enterCount == 1);
        REQUIRE(screenPtr->m_exitCount == 0);
    }
    
    SECTION("changeScreen calls onExit on previous screen") {
        auto screen1 = std::make_unique<TestScreen>("screen1");
        auto screen2 = std::make_unique<TestScreen>("screen2");
        TestScreen* ptr1 = screen1.get();
        TestScreen* ptr2 = screen2.get();
        
        screenManager.addScreen("screen1", std::move(screen1));
        screenManager.addScreen("screen2", std::move(screen2));
        
        screenManager.changeScreen("screen1");
        REQUIRE(ptr1->m_enterCount == 1);
        REQUIRE(ptr1->m_exitCount == 0);
        
        screenManager.changeScreen("screen2");
        REQUIRE(ptr2->m_enterCount == 1);
        REQUIRE(ptr1->m_exitCount == 1);
    }
    
    SECTION("changeScreen fails for non-existent screen") {
        REQUIRE_FALSE(screenManager.changeScreen("nonexistent"));
    }
    
    SECTION("changeScreen clears stack before switching") {
        screenManager.addScreen("base", std::make_unique<TestScreen>("base"));
        screenManager.addScreen("overlay", std::make_unique<TestScreen>("overlay"));
        screenManager.addScreen("other", std::make_unique<TestScreen>("other"));
        
        TestScreen* basePtr = static_cast<TestScreen*>(screenManager.getScreen("base"));
        TestScreen* overlayPtr = static_cast<TestScreen*>(screenManager.getScreen("overlay"));
        
        screenManager.changeScreen("base");
        screenManager.pushScreen("overlay");
        
        REQUIRE(screenManager.getStackDepth() == 2);
        REQUIRE(basePtr->m_exitCount == 0);  // Not exited, just overlayed
        REQUIRE(overlayPtr->m_enterCount == 1);
        
        screenManager.changeScreen("other");
        REQUIRE(screenManager.getStackDepth() == 1);
        REQUIRE(overlayPtr->m_exitCount == 1);  // Overlay exited
    }
}

TEST_CASE("ScreenManager Stack Operations", "[screen_manager][stack]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("pushScreen adds overlay without exiting base") {
        auto base = std::make_unique<TestScreen>("base");
        auto overlay = std::make_unique<TestScreen>("overlay");
        TestScreen* basePtr = base.get();
        TestScreen* overlayPtr = overlay.get();
        
        screenManager.addScreen("base", std::move(base));
        screenManager.addScreen("overlay", std::move(overlay));
        
        screenManager.changeScreen("base");
        REQUIRE(basePtr->m_enterCount == 1);
        
        screenManager.pushScreen("overlay");
        REQUIRE(screenManager.getStackDepth() == 2);
        REQUIRE(screenManager.getCurrentScreenName() == "overlay");
        REQUIRE(overlayPtr->m_enterCount == 1);
        REQUIRE(basePtr->m_exitCount == 0);  // Base not exited
    }
    
    SECTION("pushScreen fails for same screen at top") {
        screenManager.addScreen("test", std::make_unique<TestScreen>("test"));
        screenManager.changeScreen("test");
        
        REQUIRE_FALSE(screenManager.pushScreen("test"));
        REQUIRE(screenManager.getStackDepth() == 1);
    }
    
    SECTION("popScreen removes overlay and returns to base") {
        auto base = std::make_unique<TestScreen>("base");
        auto overlay = std::make_unique<TestScreen>("overlay");
        TestScreen* basePtr = base.get();
        TestScreen* overlayPtr = overlay.get();
        
        screenManager.addScreen("base", std::move(base));
        screenManager.addScreen("overlay", std::move(overlay));
        
        screenManager.changeScreen("base");
        screenManager.pushScreen("overlay");
        
        std::string popped = screenManager.popScreen();
        REQUIRE(popped == "overlay");
        REQUIRE(screenManager.getStackDepth() == 1);
        REQUIRE(screenManager.getCurrentScreenName() == "base");
        REQUIRE(overlayPtr->m_exitCount == 1);
        REQUIRE(basePtr->m_enterCount == 2);  // Re-entered after pop
    }
    
    SECTION("popScreen on empty stack returns empty string") {
        REQUIRE(screenManager.popScreen() == "");
        REQUIRE(screenManager.getStackDepth() == 0);
    }
    
    SECTION("clearStack removes all screens") {
        screenManager.addScreen("base", std::make_unique<TestScreen>("base"));
        screenManager.addScreen("overlay1", std::make_unique<TestScreen>("overlay1"));
        screenManager.addScreen("overlay2", std::make_unique<TestScreen>("overlay2"));
        
        screenManager.changeScreen("base");
        screenManager.pushScreen("overlay1");
        screenManager.pushScreen("overlay2");
        
        REQUIRE(screenManager.getStackDepth() == 3);
        
        screenManager.clearStack();
        REQUIRE(screenManager.getStackDepth() == 0);
        REQUIRE(screenManager.getCurrentScreen() == nullptr);
    }
}

TEST_CASE("ScreenManager Event Handling", "[screen_manager][events]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("handleEvent passes to current screen") {
        auto screen = std::make_unique<TestScreen>("test");
        TestScreen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        screenManager.changeScreen("test");
        
        SDL_Event e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_SPACE);
        screenManager.handleEvent(e);
        
        REQUIRE(screenPtr->m_eventCount == 1);
    }
    
    SECTION("handleEvent with no screen returns false") {
        SDL_Event e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_SPACE);
        REQUIRE_FALSE(screenManager.handleEvent(e));
    }
    
    SECTION("wantsPreProcessEvent intercepts before GUIManager") {
        auto screen = std::make_unique<PreProcessScreen>();
        PreProcessScreen* screenPtr = screen.get();
        screenManager.addScreen("preprocess", std::move(screen));
        screenManager.changeScreen("preprocess");
        
        SDL_Event e = helper.createKeyEvent(SDL_KEYDOWN, SDLK_ESCAPE);
        bool handled = screenManager.handleEvent(e);
        
        REQUIRE(handled);
        REQUIRE(screenPtr->m_escPressed);
    }
}

TEST_CASE("ScreenManager Update/Render/Cleanup", "[screen_manager][update]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("update calls screen update and GUIManager update") {
        auto screen = std::make_unique<TestScreen>("test");
        TestScreen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        screenManager.changeScreen("test");
        
        screenManager.update();
        REQUIRE(screenPtr->m_updateCount == 1);
    }
    
    SECTION("render calls screen render and GUIManager render") {
        auto screen = std::make_unique<TestScreen>("test");
        TestScreen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        screenManager.changeScreen("test");
        
        screenManager.render(helper.getRenderer());
        REQUIRE(screenPtr->m_renderCount == 1);
    }
    
    SECTION("render with stack renders from bottom to top") {
        auto base = std::make_unique<TestScreen>("base");
        auto overlay = std::make_unique<TestScreen>("overlay");
        TestScreen* basePtr = base.get();
        TestScreen* overlayPtr = overlay.get();
        
        screenManager.addScreen("base", std::move(base));
        screenManager.addScreen("overlay", std::move(overlay));
        
        screenManager.changeScreen("base");
        screenManager.pushScreen("overlay");
        
        screenManager.render(helper.getRenderer());
        
        // Base rendered first, then overlay
        REQUIRE(basePtr->m_renderCount == 1);
        REQUIRE(overlayPtr->m_renderCount == 1);
    }
    
    SECTION("cleanup calls GUIManager cleanup") {
        auto screen = std::make_unique<TestScreen>("test");
        screenManager.addScreen("test", std::move(screen));
        screenManager.changeScreen("test");
        
        // Mark the panel for deletion
        TestScreen* screenPtr = static_cast<TestScreen*>(screenManager.getCurrentScreen());
        if (screenPtr && screenPtr->m_panel) {
            screenPtr->m_panel->markForDeletion();
        }
        
        screenManager.cleanup();
        // After cleanup, panel should be removed
    }
}

TEST_CASE("ScreenManager Getters", "[screen_manager][getters]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    ScreenManager screenManager(manager);
    
    SECTION("getScreen returns correct screen") {
        auto screen = std::make_unique<TestScreen>("test");
        Screen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        
        REQUIRE(screenManager.getScreen("test") == screenPtr);
        REQUIRE(screenManager.getScreen("nonexistent") == nullptr);
    }
    
    SECTION("getCurrentScreen returns current active screen") {
        auto screen = std::make_unique<TestScreen>("test");
        Screen* screenPtr = screen.get();
        screenManager.addScreen("test", std::move(screen));
        screenManager.changeScreen("test");
        
        REQUIRE(screenManager.getCurrentScreen() == screenPtr);
    }
    
    SECTION("getCurrentScreenName returns correct name") {
        screenManager.addScreen("menu", std::make_unique<TestScreen>("menu"));
        screenManager.addScreen("game", std::make_unique<TestScreen>("game"));
        
        REQUIRE(screenManager.getCurrentScreenName() == "");
        
        screenManager.changeScreen("menu");
        REQUIRE(screenManager.getCurrentScreenName() == "menu");
        
        screenManager.changeScreen("game");
        REQUIRE(screenManager.getCurrentScreenName() == "game");
    }
    
    SECTION("getStackDepth reflects stack size") {
        screenManager.addScreen("s1", std::make_unique<TestScreen>("s1"));
        screenManager.addScreen("s2", std::make_unique<TestScreen>("s2"));
        screenManager.addScreen("s3", std::make_unique<TestScreen>("s3"));
        
        REQUIRE(screenManager.getStackDepth() == 0);
        
        screenManager.changeScreen("s1");
        REQUIRE(screenManager.getStackDepth() == 1);
        
        screenManager.pushScreen("s2");
        REQUIRE(screenManager.getStackDepth() == 2);
        
        screenManager.pushScreen("s3");
        REQUIRE(screenManager.getStackDepth() == 3);
        
        screenManager.popScreen();
        REQUIRE(screenManager.getStackDepth() == 2);
        
        screenManager.clearStack();
        REQUIRE(screenManager.getStackDepth() == 0);
    }
}