#pragma once

#include "panel.hpp"

/**
 * @file custom_control.hpp
 * @brief CustomControl - owner-draw panel for app-rendered content.
 *
 * A focusable Panel that always renders through the direct path. The app
 * installs a DrawPhase::Replace hook (C: sdlgui_element_set_draw_hook) and/or
 * Before/After hooks; the callback gets the real SDL_Renderer, the local rect
 * {0,0,w,h} and the ancestors' clip translated to local coordinates.
 *
 * Without a Replace hook the default Panel background/border is drawn, so a
 * Before/After-only control composes with the normal widget chrome.
 */
class CustomControl : public Panel {
public:
    CustomControl(GUIManager& manager, int x, int y, int width, int height);

    ComponentType getComponentTypeId() const override;

protected:
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;
};
