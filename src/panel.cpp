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

void Panel::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);

    const Style& resolvedStyle = getComposedStyle(m_state);

    // Rysuj teksturę
    if (resolvedStyle.texture.has_value()) {
        SDL_Rect destRect = {0, 0, m_width, m_height};
        SDL_RenderCopy(renderer, resolvedStyle.texture.value().get(), nullptr, &destRect);
    }
}