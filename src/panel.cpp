#include "panel.hpp"
#include <SDL3/SDL_rect.h>
#include "gui_manager.hpp"

Panel::Panel(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
}

Panel::Panel(GUIManager& manager, SDL_Rect rect)
    : GUIElement(manager, rect.x, rect.y, rect.w, rect.h) {
}


void Panel::setDraggable(bool draggable) {
    m_is_draggable = draggable;
}

bool Panel::handleSelf(const SDL_Event& event) {
    if (!m_visible) {
        return false;
    }

    // Drag logic - only active if no child handled the event
    // (children already ran in propagateToChildren before this hook).
    if (m_is_draggable) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT && contains(event.button.x, event.button.y)) {
            m_is_dragging = true;
            auto absPos = getAbsolutePosition();
            m_drag_offset.x = static_cast<int>(event.button.x) - absPos.x;
            m_drag_offset.y = static_cast<int>(event.button.y) - absPos.y;
            m_manager.captureMouse(this);
            return true;
        }
    }
    
    // Drag handling - checked BEFORE hover to avoid lag
    if (m_is_dragging && event.type == SDL_EVENT_MOUSE_MOTION) {
        int parentX = m_parent ? m_parent->getAbsolutePosition().x : 0;
        int parentY = m_parent ? m_parent->getAbsolutePosition().y : 0;
        setPosition(static_cast<int>(event.motion.x) - parentX - m_drag_offset.x, static_cast<int>(event.motion.y) - parentY - m_drag_offset.y);
        return true;
    }

    if (m_is_dragging && event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        m_is_dragging = false;
        m_manager.releaseMouse();
        return true;
    }

    // Hover + press state: shared base implementation (was a hand-rolled
    // hover block + processButtonEvent here).
    return GUIElement::handleSelf(event);
}

ComponentType Panel::getComponentTypeId() const {
    return ComponentType::Panel;
}

void Panel::onMouseCaptureLost() {
    m_is_dragging = false;
}