#pragma once
#include "gui.hpp"

class Panel : public GUIElement {
public:
    Panel(GUIManager& manager, int x, int y, int width, int height);
    Panel(GUIManager& manager, SDL_Rect rect);
    bool handleEvent(const SDL_Event& event) override;
    void setDraggable(bool draggable);

    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;
    void onMouseCaptureLost() override;

    bool m_is_draggable = false;
    bool m_is_dragging = false;
    SDL_Point m_drag_offset;
};
