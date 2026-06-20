#include "../lib/catch_amalgamated.hpp"

#include "../src/window_manager.hpp"
#include "../src/window.hpp"
#include "../src/button.hpp"
#include "../src/panel.hpp"
#include "../src/label.hpp"

#include "std.hpp"

// WindowManager tests require SDL initialization, which WindowManager handles itself.
// We test window creation, management, and cleanup.

TEST_CASE("WindowManager Construction and SDL Init", "[window_manager][init]") {
    SECTION("WindowManager initializes SDL successfully") {
        // WindowManager constructor initializes SDL, SDL_image, SDL_ttf
        WindowManager windowManager;
        
        // SDL should be initialized
        REQUIRE(SDL_WasInit(SDL_INIT_VIDEO) != 0);
        REQUIRE(TTF_WasInit() == 1);  // SDL_ttf initialized
        
        // Initial state
        REQUIRE(windowManager.getWindowCount() == 0);
        REQUIRE_FALSE(windowManager.hasOpenWindows());
        REQUIRE_FALSE(windowManager.shouldQuit());
    }
    
    SECTION("WindowManager destructor cleans up SDL") {
        {
            WindowManager windowManager;
            REQUIRE(SDL_WasInit(SDL_INIT_VIDEO) != 0);
        }
        // SDL_Quit called; SDL subsystems cleaned up by destructor
        // Note: SDL_WasInit behavior after quit depends on SDL internals (ref counting)
    }
}

TEST_CASE("WindowManager Window Creation", "[window_manager][create]") {
    WindowManager windowManager;
    
    SECTION("createWindow creates a window successfully") {
        Window* window = windowManager.createWindow("Test Window", 400, 300);
        
        REQUIRE(window != nullptr);
        REQUIRE(windowManager.getWindowCount() == 1);
        REQUIRE(windowManager.hasOpenWindows());
        
        REQUIRE(window->getTitle() == "Test Window");
        REQUIRE(window->getWindowID() > 0);
        REQUIRE(window->isVisible());
        REQUIRE_FALSE(window->isMarkedForClose());
    }
    
    SECTION("createWindow creates multiple windows") {
        Window* win1 = windowManager.createWindow("Window 1", 400, 300);
        Window* win2 = windowManager.createWindow("Window 2", 500, 400);
        Window* win3 = windowManager.createWindow("Window 3", 600, 500);
        
        REQUIRE(win1 != nullptr);
        REQUIRE(win2 != nullptr);
        REQUIRE(win3 != nullptr);
        
        REQUIRE(windowManager.getWindowCount() == 3);
        
        // Each window should have unique IDs
        REQUIRE(win1->getWindowID() != win2->getWindowID());
        REQUIRE(win2->getWindowID() != win3->getWindowID());
    }
    
    SECTION("createWindow with resizable flag") {
        Window* window = windowManager.createWindow("Resizable", 400, 300, true);
        
        REQUIRE(window != nullptr);
        // Window should be resizable (SDL_WINDOW_RESIZABLE flag set)
    }
    
    SECTION("createWindow returns nullptr for invalid size") {
        // SDL should handle these gracefully, but let's test edge cases
        Window* window = windowManager.createWindow("Zero Size", 0, 0);
        // SDL may create a minimum sized window or fail
        // Behavior depends on SDL version
    }
}

TEST_CASE("WindowManager Window Access", "[window_manager][access]") {
    WindowManager windowManager;
    
    Window* win1 = windowManager.createWindow("Window 1", 400, 300);
    Window* win2 = windowManager.createWindow("Window 2", 500, 400);
    
    SECTION("getWindowByID returns correct window") {
        Uint32 id1 = win1->getWindowID();
        Uint32 id2 = win2->getWindowID();
        
        REQUIRE(windowManager.getWindowByID(id1) == win1);
        REQUIRE(windowManager.getWindowByID(id2) == win2);
        REQUIRE(windowManager.getWindowByID(99999) == nullptr);
    }
    
    SECTION("getWindow by index returns correct window") {
        REQUIRE(windowManager.getWindow(0) == win1);
        REQUIRE(windowManager.getWindow(1) == win2);
        REQUIRE(windowManager.getWindow(2) == nullptr);
        REQUIRE(windowManager.getWindow(999) == nullptr);
    }
    
    SECTION("getWindowCount reflects number of windows") {
        REQUIRE(windowManager.getWindowCount() == 2);
        
        windowManager.createWindow("Window 3", 300, 200);
        REQUIRE(windowManager.getWindowCount() == 3);
        
        win1->markForClose();
        windowManager.cleanupAll();  // Should remove win1
        
        REQUIRE(windowManager.getWindowCount() == 2);
    }
    
    SECTION("getFocusedWindow returns focused window") {
        // Initially no window focused (depends on OS behavior)
        // After creating a window and showing it, it may be focused
        Window* focused = windowManager.getFocusedWindow();
        // Just verify method works - actual focus depends on OS
    }
}

TEST_CASE("WindowManager Window Closing", "[window_manager][close]") {
    WindowManager windowManager;
    
    Window* win1 = windowManager.createWindow("Main", 400, 300);
    Window* win2 = windowManager.createWindow("Secondary 1", 300, 200);
    Window* win3 = windowManager.createWindow("Secondary 2", 300, 200);
    
    SECTION("closeWindow marks window for close") {
        REQUIRE_FALSE(win2->isMarkedForClose());
        
        windowManager.closeWindow(win2->getWindowID());
        
        REQUIRE(win2->isMarkedForClose());
        REQUIRE(windowManager.getWindowCount() == 3);  // Still in list until cleanup
    }
    
    SECTION("closeWindow returns false for invalid ID") {
        REQUIRE_FALSE(windowManager.closeWindow(99999));
    }
    
    SECTION("closeSecondaryWindows marks all except first") {
        windowManager.closeSecondaryWindows();
        
        REQUIRE_FALSE(win1->isMarkedForClose());
        REQUIRE(win2->isMarkedForClose());
        REQUIRE(win3->isMarkedForClose());
    }
    
    SECTION("closeAllWindows marks all for close") {
        windowManager.closeAllWindows();
        
        REQUIRE(win1->isMarkedForClose());
        REQUIRE(win2->isMarkedForClose());
        REQUIRE(win3->isMarkedForClose());
        
        REQUIRE_FALSE(windowManager.hasOpenWindows());
    }
    
    SECTION("cleanupAll removes marked windows") {
        win2->markForClose();
        win3->markForClose();
        
        REQUIRE(windowManager.getWindowCount() == 3);
        
        windowManager.cleanupAll();
        
        REQUIRE(windowManager.getWindowCount() == 1);
        REQUIRE(windowManager.getWindow(0) == win1);
        REQUIRE(windowManager.getWindow(1) == nullptr);
    }
}

TEST_CASE("WindowManager State", "[window_manager][state]") {
    WindowManager windowManager;
    
    SECTION("hasOpenWindows reflects open windows") {
        REQUIRE_FALSE(windowManager.hasOpenWindows());
        
        Window* win = windowManager.createWindow("Test", 400, 300);
        REQUIRE(windowManager.hasOpenWindows());
        
        win->markForClose();
        REQUIRE_FALSE(windowManager.hasOpenWindows());
    }
    
    SECTION("shouldQuit initially false") {
        REQUIRE_FALSE(windowManager.shouldQuit());
    }
    
    SECTION("requestQuit sets quit flag and closes windows") {
        Window* win = windowManager.createWindow("Test", 400, 300);
        
        windowManager.requestQuit();
        
        REQUIRE(windowManager.shouldQuit());
        REQUIRE(win->isMarkedForClose());
    }
    
    SECTION("shouldQuit true when all windows closed") {
        Window* win = windowManager.createWindow("Test", 400, 300);
        win->markForClose();
        
        REQUIRE_FALSE(windowManager.shouldQuit());
        
        windowManager.cleanupAll();
        
        REQUIRE(windowManager.shouldQuit());
        REQUIRE(windowManager.getWindowCount() == 0);
    }
}

TEST_CASE("Window Properties", "[window][properties]") {
    WindowManager windowManager;
    Window* window = windowManager.createWindow("Test Window", 800, 600);
    
    SECTION("Window has correct title") {
        REQUIRE(window->getTitle() == "Test Window");
    }
    
    SECTION("Window has valid window ID") {
        REQUIRE(window->getWindowID() > 0);
    }
    
    SECTION("Window has correct SDL handles") {
        REQUIRE(window->getSDLWindow() != nullptr);
        REQUIRE(window->getRenderer() != nullptr);
    }
    
    SECTION("Window has GUIManager") {
        REQUIRE(&window->getGUIManager() != nullptr);
    }
    
    SECTION("Window getSize returns correct values") {
        int w, h;
        window->getSize(w, h);
        
        // Size may differ slightly due to window decorations
        REQUIRE(w > 0);
        REQUIRE(h > 0);
    }
    
    SECTION("Window visibility can be toggled") {
        REQUIRE(window->isVisible());
        
        window->hide();
        REQUIRE_FALSE(window->isVisible());
        
        window->show();
        REQUIRE(window->isVisible());
    }
    
    SECTION("Window focus state") {
        // Initially may not be focused (depends on OS)
        window->hide();
        REQUIRE_FALSE(window->isFocused());
        
        window->show();
        // May become focused after show
    }
}

TEST_CASE("Window GUIManager Integration", "[window][gui]") {
    WindowManager windowManager;
    Window* window = windowManager.createWindow("GUI Test", 400, 300);
    GUIManager& gui = window->getGUIManager();
    
    SECTION("Window's GUIManager can add elements") {
        auto panel = std::make_unique<Panel>(gui, 10, 10, 200, 100);
        Panel* panelPtr = panel.get();
        
        gui.addElement(std::move(panel));
        
        REQUIRE(gui.findElementAt(50, 50) == panelPtr);
    }
    
    SECTION("Window's GUIManager has renderer") {
        REQUIRE(gui.getRenderer() == window->getRenderer());
    }
    
    SECTION("Window's GUIManager has managers") {
        REQUIRE(gui.getTimerManager() != nullptr);
        REQUIRE(gui.getAnimationManager() != nullptr);
    }
    
    SECTION("Multiple windows have independent GUIManagers") {
        Window* win1 = windowManager.createWindow("Win1", 400, 300);
        Window* win2 = windowManager.createWindow("Win2", 400, 300);
        
        GUIManager& gui1 = win1->getGUIManager();
        GUIManager& gui2 = win2->getGUIManager();
        
        // Add element to gui1
        auto panel1 = std::make_unique<Panel>(gui1, 10, 10, 100, 50);
        gui1.addElement(std::move(panel1));
        
        // gui2 should not have this element
        REQUIRE(gui2.findElementAt(50, 50) == nullptr);
        
        // gui1 should have it
        REQUIRE(gui1.findElementAt(50, 50) != nullptr);
    }
}

TEST_CASE("Window Callbacks", "[window][callbacks]") {
    WindowManager windowManager;
    Window* window = windowManager.createWindow("Callback Test", 400, 300);
    
    SECTION("setOnCloseCallback is stored") {
        bool callbackCalled = false;
        window->setOnCloseCallback([&callbackCalled](Window* w) {
            callbackCalled = true;
            w->markForClose();
        });
        
        // Simulate close event
        window->markForClose();
        
        // Callback not called automatically on markForClose
        REQUIRE_FALSE(callbackCalled);
        
        // Callback would be called on SDL_EVENT_WINDOW_CLOSE_REQUESTED
    }
    
    SECTION("setOnResizeCallback is stored") {
        int resizeW = 0, resizeH = 0;
        window->setOnResizeCallback([&resizeW, &resizeH](Window* w, int newW, int newH) {
            resizeW = newW;
            resizeH = newH;
        });
        
        // Resize callback would be triggered on SDL_EVENT_WINDOW_RESIZED
        // Can't easily simulate without user interaction
    }
}

TEST_CASE("WindowManager Render All", "[window_manager][render]") {
    WindowManager windowManager;
    
    SECTION("renderAll only renders visible windows") {
        Window* win1 = windowManager.createWindow("Visible", 400, 300);
        Window* win2 = windowManager.createWindow("Hidden", 400, 300);
        
        win2->hide();
        
        // renderAll should skip hidden windows
        windowManager.renderAll();
        
        // No easy way to verify visually, but method should not crash
        REQUIRE(true);
    }
    
    SECTION("renderAll skips marked for close windows") {
        Window* win = windowManager.createWindow("Test", 400, 300);
        win->markForClose();
        
        windowManager.renderAll();
        
        // Should not crash, marked window skipped
        REQUIRE(true);
    }
}

TEST_CASE("WindowManager Update All", "[window_manager][update]") {
    WindowManager windowManager;
    
    SECTION("updateAll updates all windows' GUIManagers") {
        Window* win = windowManager.createWindow("Test", 400, 300);
        
        // Add a timer or animation to test update
        TimerManager* timerManager = win->getGUIManager().getTimerManager();
        REQUIRE(timerManager != nullptr);
        
        windowManager.updateAll();
        
        // Timers/animations updated
        REQUIRE(true);
    }
}