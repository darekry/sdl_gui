#include "panel.hpp"
#include <SDL3/SDL_rect.h>
#include "gui_manager.hpp"

// Implementacja klasy Panel
Panel::Panel(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
}

Panel::Panel(GUIManager& manager, SDL_Rect rect)
    : GUIElement(manager, rect.x, rect.y, rect.w, rect.h) {
}


void Panel::setDraggable(bool draggable) {
    m_is_draggable = draggable;
}

bool Panel::handleEvent(const SDL_Event& event) {
    if (!m_visible) {
        return false;
    }

    // Przekaż zdarzenie do dzieci. Jeśli któreś je obsłuży, nie rób nic więcej.
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleEvent(event)) {
            return true;
        }
    }

    // Logika przeciągania - aktywowana tylko jeśli żadne dziecko nie obsłużyło zdarzenia
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
    
    // Obsługa przeciągania - sprawdzana PRZED hover, aby uniknąć lagów
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

    // Hover check - tylko gdy NIE przeciągamy
    if (!m_is_dragging && event.type == SDL_EVENT_MOUSE_MOTION) {
        int mouseX = static_cast<int>(event.motion.x);
        int mouseY = static_cast<int>(event.motion.y);
        bool currentlyHovered = contains(mouseX, mouseY);

        if (currentlyHovered && !m_isHovered) {
            m_isHovered = true;
            setState(ElementState::Hover);
        } else if (!currentlyHovered && m_isHovered) {
            m_isHovered = false;
            setState(ElementState::Normal);
        }
        processHoverTooltip(currentlyHovered);
    }
    
    processButtonEvent(event);
    
    return false;
}

const char* Panel::getComponentType() const {
    return "Panel";
}

void Panel::onMouseCaptureLost() {
    m_is_dragging = false;
}