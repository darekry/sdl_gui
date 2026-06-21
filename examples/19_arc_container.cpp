/**
 * @file 19_arc_container.cpp
 * @brief Demonstrates ArcContainer for circular/radial layouts
 * 
 * This example shows how to use ArcContainer for game-style radial menus
 * with buttons positioned along an arc and rotated labels that "radiate"
 * from the center.
 * 
 * Features:
 * - 12 buttons in a full circle (clock-like arrangement)
 * - Labels rotated to point outward from center
 * - Hit testing only within the arc region
 * - Rotation support via SDL_RenderTextureRotated
 */

#include <SDL3/SDL_render.h>
#include "arc_container.hpp"
#include "gui_manager.hpp"
#include "button.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "style.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("ArcContainer Demo - Radial Menu", 500, 500, true);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        
        Style btnStyle;
        btnStyle.backgroundColor = SDL_Color{70, 130, 180, 255};
        btnStyle.borderColor = SDL_Color{255, 255, 255, 255};
        btnStyle.borderWidth = 2;
        btnStyle.borderRadius = 8;
        btnStyle.fontSize = 14;
        
        auto arc = std::make_unique<ArcContainer>(guiManager, 250, 250, 180, 0, 360);
        
        auto centerPanel = std::make_unique<Panel>(guiManager, 220, 220, 60, 60);
        Style centerStyle;
        centerStyle.backgroundColor = SDL_Color{50, 50, 70, 230};
        centerStyle.borderColor = SDL_Color{100, 149, 237, 255};
        centerStyle.borderWidth = 3;
        centerStyle.borderRadius = 30;
        centerPanel->setStyle(ElementState::Normal, centerStyle);
        
        auto innerLabel = std::make_unique<Label>(guiManager, 15, 20, "OK");
        centerPanel->addChild(std::move(innerLabel));
        auto centerPanelRef = guiManager.makeRef(centerPanel.get());
        guiManager.addElement(std::move(centerPanel));
        
        for (int i = 0; i < 12; ++i) {
            float angle = static_cast<float>(i) * 30.0f;
            auto btn = std::make_unique<Button>(guiManager, 0, 0, 50, 30, 
                                                 std::to_string(i));
            btn->setStyle(ElementState::Normal, btnStyle);
            btn->setOnClickCallback([i, centerPanelRef](GUIElement*) {
                double newRotation = 30.0 * (i );
                if (centerPanelRef) centerPanelRef->setRotation(newRotation);
            });
            arc->addChildAtAngle(std::move(btn), angle-90, true, 00);
        }
        
        guiManager.addElement(std::move(arc));
        
        auto infoLabel = std::make_unique<Label>(guiManager, 10, 460, 
            "Click buttons on the arc - hit-test works on arc shape!");
        guiManager.addElement(std::move(infoLabel));
        
        bool quit = false;
        SDL_Event e;
        
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                } else {
                    guiManager.processEvent(e);
                }
            }
            
            guiManager.update();
            
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