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
    const Style& resolvedStyle = getComposedStyle(m_state);

    // Rysuj tło
    if (resolvedStyle.backgroundColor.has_value()) {
        const auto& color = resolvedStyle.backgroundColor.value();
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        SDL_RenderFillRect(renderer, &bgRect);
    }

    // Rysuj teksturę
    if (resolvedStyle.texture.has_value()) {
        SDL_Rect destRect = {0, 0, m_width, m_height};
        SDL_RenderCopy(renderer, resolvedStyle.texture.value().get(), nullptr, &destRect);
    }
    
    // Rysuj ramkę
    if (resolvedStyle.borderColor.has_value() && resolvedStyle.borderWidth.has_value()) {
        const auto& color = resolvedStyle.borderColor.value();
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect borderRect = {0, 0, m_width, m_height};
        for (int i = 0; i < resolvedStyle.borderWidth.value(); ++i) {
            SDL_RenderDrawRect(renderer, &borderRect);
            borderRect.x++;
            borderRect.y++;
            borderRect.w -= 2;
            borderRect.h -= 2;
        }
    }
}