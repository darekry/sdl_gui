#include "panel.hpp"
#include "gui_manager.hpp"

// Implementacja klasy Panel
Panel::Panel(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
}

void Panel::setDraggable(bool draggable) {
    m_is_draggable = draggable;
}

bool Panel::handleEvent(const SDL_Event& event) {
    if (!m_visible) {
        return false;
    }

    if (GUIElement::handleEvent(event)) {
        return true;
    }

    if (m_is_draggable) {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            auto mousePos = SDL_Point{event.button.x, event.button.y};
            auto absPos = getAbsolutePosition();
            auto panelRect = SDL_Rect{absPos.x, absPos.y, m_width, m_height};

            if (SDL_PointInRect(&mousePos, &panelRect)) {
                m_is_dragging = true;
                m_drag_offset.x = mousePos.x - absPos.x;
                m_drag_offset.y = mousePos.y - absPos.y;
                return true;
            }
        } else if (event.type == SDL_MOUSEMOTION && m_is_dragging) {
            auto mouseX = 0;
            auto mouseY = 0;
            SDL_GetMouseState(&mouseX, &mouseY);
            setPosition(mouseX - m_drag_offset.x - (m_parent ? m_parent->getAbsolutePosition().x : 0),
                        mouseY - m_drag_offset.y - (m_parent ? m_parent->getAbsolutePosition().y : 0));
            return true;
        } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            if (m_is_dragging) {
                m_is_dragging = false;
                return true;
            }
        }
    }

    return false;
}

const char* Panel::getComponentType() const {
    return "Panel";
}

void Panel::draw() {
    // Ta metoda jest teraz pusta, ponieważ całe rysowanie jest obsługiwane
    // przez GUIElement::draw() i system motywów.
    // Zachowujemy ją, aby klasy pochodne mogły ją nadpisać w razie potrzeby.
}