/**
 * @file example_resize.cpp
 * @brief Demonstrates Anchor system for responsive layout
 * 
 * This example shows how to use anchors to create responsive GUI elements
 * that automatically adjust when the window is resized.
 * 
 * Anchor system features:
 * - Declarative positioning - set once, works forever
 * - Automatic resize handling - no manual updateLayout() calls
 * - Multiple positioning modes: fixed, centered, stretched, corner-anchored
 * 
 * Test scenarios:
 * - Panel anchored to center (maintains center position)
 * - Button anchored to bottom-right corner
 * - Panel that stretches horizontally at bottom
 * - Labels at various positions
 */
#include "panel.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "label.hpp"
#include "gui.hpp"
#include "style.hpp"
#include "sdl_app.hpp"
#include "anchor.hpp"

#include "std.hpp"

int main(int, char**)
{
    try {
        // Create RESIZABLE window
        SDLApp app("Anchor System Demo - Resize the window!", 800, 600, true);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        
        // Set initial window size in GUIManager
        int windowWidth, windowHeight;
        app.getWindowSize(windowWidth, windowHeight);
        guiManager.setWindowSize(windowWidth, windowHeight);
        
        // ====================================================================
        // Create elements with anchors - positions update automatically!
        // ====================================================================
        
        // 1. Top-left fixed label - using preset anchor
        auto topLeftLabel = std::make_unique<Label>(guiManager, 10, 10, "Fixed Top-Left (Anchor::topLeft)");
        topLeftLabel->setAnchor(Anchor::topLeft(10));
        guiManager.addElement(std::move(topLeftLabel));
        
        // 2. Top-right corner label
        auto topRightLabel = std::make_unique<Label>(guiManager, 0, 10, "Top-Right Corner");
        topRightLabel->setAnchor(Anchor::topRight(10));
        Style statusStyle;
        statusStyle.textColor = SDL_Color{80, 80, 120, 255};
        topRightLabel->setStyle(ElementState::Normal, statusStyle);
        guiManager.addElement(std::move(topRightLabel));
        
        // 3. Centered panel - stays in center regardless of window size
        auto centerPanel = std::make_unique<Panel>(guiManager, 350, 280, 200, 100);
        centerPanel->setDraggable(true);
        centerPanel->setAnchor(Anchor::center()); // Will stay centered!
        Style panelStyle;
        panelStyle.backgroundColor = SDL_Color{100, 149, 237, 200};
        panelStyle.borderColor = SDL_Color{255, 255, 255, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 8;
        centerPanel->setStyle(ElementState::Normal, panelStyle);
        guiManager.addElement(std::move(centerPanel));
        
        // 4. Bottom-right close button
        auto closeBtn = std::make_unique<Button>(guiManager, 0, 0, 80, 30, "Close");
        closeBtn->setAnchor(Anchor::bottomRight(10)); // 10px from bottom-right corner
        closeBtn->setOnClickCallback([](GUIElement*) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_EVENT_QUIT;
            SDL_PushEvent(&quitEvent);
        });
        guiManager.addElement(std::move(closeBtn));
        
        // 5. Bottom stretch panel - fills entire width, 50px from bottom
        auto bottomBar = std::make_unique<Panel>(guiManager, 0, 0, 800, 50);
        // Anchor: fill horizontally, 50px from bottom edge
        // left=0 (from left edge), right=0 (from right edge), bottom=50 (height from bottom)
        bottomBar->setAnchor(Anchor::bottomBar(50, 10, 10));
        Style barStyle;
        barStyle.backgroundColor = SDL_Color{50, 50, 60, 220};
        barStyle.borderColor = SDL_Color{100, 100, 120, 255};
        barStyle.borderWidth = 1;
        barStyle.borderRadius = 4;
        bottomBar->setStyle(ElementState::Normal, barStyle);
        guiManager.addElement(std::move(bottomBar));
        
        // 6. Left sidebar - full height, 200px width
        auto sidebar = std::make_unique<Panel>(guiManager, 0, 0, 200, 600);
        sidebar->setAnchor(Anchor::leftSidebar(200, 60, 70)); // 200px wide, margins top/bottom
        Style sidebarStyle;
        sidebarStyle.backgroundColor = SDL_Color{40, 44, 52, 230};
        sidebarStyle.borderColor = SDL_Color{60, 64, 72, 255};
        sidebarStyle.borderWidth = 1;
        sidebar->setStyle(ElementState::Normal, sidebarStyle);
        guiManager.addElement(std::move(sidebar));
        
        // 7. Size indicator label - centered at top
        auto sizeLabel = std::make_unique<Label>(guiManager, 0, 40, "Window: 800 x 600");
        // Horizontal center, 40px from top
        sizeLabel->setAnchor(Anchor{
            .left = 0.5f,   // Center horizontally (50%)
            .top = 40.0f,   // 40px from top
            .right = -1,    // Not set
            .bottom = -1    // Not set
        });
        Label* sizeLabelPtr = sizeLabel.get();
        auto sizeLabelRef = guiManager.makeRef(sizeLabelPtr);
        guiManager.addElement(std::move(sizeLabel));
        
        // Resize callback to update size display (custom logic on top of anchors)
        guiManager.setResizeCallback([sizeLabelRef](int w, int h) {
            if (sizeLabelRef) {
                sizeLabelRef->setText("Window: " + std::to_string(w) + " x " + std::to_string(h));
            }
        });
        
        // ====================================================================
        // Main loop - resize handling is automatic!
        // ====================================================================
        
        bool quit = false;
        SDL_Event e;
        
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_EVENT_QUIT:
                        quit = true;
                        break;
                        
                    case SDL_EVENT_WINDOW_RESIZED:
                        guiManager.handleResize(e.window.data1, e.window.data2);
                        break;
                        
                    default:
                        guiManager.processEvent(e);
                        break;
                }
            }
            
            guiManager.update();
            
            // Render
            SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
            SDL_RenderClear(renderer);
            
            guiManager.render();
            
            SDL_RenderPresent(renderer);
        }
        
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}