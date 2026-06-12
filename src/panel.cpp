#include "panel.hpp"
#include "SDL_rect.h"
#include "gui_manager.hpp"
#include "style.hpp"
#include "texture_manager.hpp"
#include "theme.hpp"

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
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && contains(event.button.x, event.button.y)) {
            m_is_dragging = true;
            auto absPos = getAbsolutePosition();
            m_drag_offset.x = event.button.x - absPos.x;
            m_drag_offset.y = event.button.y - absPos.y;
            m_manager.captureMouse(this);
            return true;
        }
    }
    
    // Obsługa przeciągania - sprawdzana PRZED hover, aby uniknąć lagów
    // Podczas drag używamy event.motion.x/y zamiast SDL_GetMouseState()
    if (m_is_dragging && event.type == SDL_MOUSEMOTION) {
        int parentX = m_parent ? m_parent->getAbsolutePosition().x : 0;
        int parentY = m_parent ? m_parent->getAbsolutePosition().y : 0;
        setPosition(event.motion.x - parentX - m_drag_offset.x, event.motion.y - parentY - m_drag_offset.y);
        return true;
    }

    if (m_is_dragging && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        m_is_dragging = false;
        m_manager.releaseMouse();
        return true;
    }

    // Hover check - tylko gdy NIE przeciągamy
    if (!m_is_dragging && event.type == SDL_MOUSEMOTION) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool currentlyHovered = contains(mouseX, mouseY);

        if (currentlyHovered && !m_isHovered) {
            m_isHovered = true;
            setState(ElementState::Hover);
        } else if (!currentlyHovered && m_isHovered) {
            m_isHovered = false;
            setState(ElementState::Normal);
        }
    }

    // Call parent to handle tooltip timer logic
    GUIElement::handleEvent(event);
    
    return false;
}

const char* Panel::getComponentType() const {
    return "Panel";
}

void Panel::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
}

void Panel::onMouseCaptureLost() {
    m_is_dragging = false;
}