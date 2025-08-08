#include "button.hpp"
#include "gui_manager.hpp"

// Implementacja klasy Button
#include "label.hpp"

Button::Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label)
    : GUIElement(manager, x, y, width, height) {
    if (!label.empty()) {
        auto label_elem = std::make_unique<Label>(manager, 0, 0, label);
        int label_width, label_height;
        label_elem->getSize(label_width, label_height);
        label_elem->setPosition((width - label_width) / 2, (height - label_height) / 2);
        addChild(std::move(label_elem));
    }
}

void Button::setOnClickCallback(OnClickCallback callback) {
    m_onClick = std::move(callback);
}

void Button::setOnMouseOverCallback(OnMouseOverCallback callback) {
    m_onMouseOver = std::move(callback);
}

bool Button::handleEvent(const SDL_Event& e) {
    auto previousState = m_currentState;
    GUIElement::handleEvent(e); // Pozwól klasie bazowej zaktualizować stan

    if (m_enabled && m_visible) {
        // Wywołaj callback onClick, jeśli stan zmienił się na Hover po wciśnięciu
        if (previousState == ElementState::Pressed && m_currentState == ElementState::Hover) {
            if (m_onClick) {
                m_onClick(this);
                return true; // Zdarzenie obsłużone
            }
        }
    }
    
    return false;
}

const char* Button::getComponentType() const {
    return "Button";
}

void Button::draw(SDL_Renderer* renderer) {
    const Style& resolvedStyle = getResolvedStyle();

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