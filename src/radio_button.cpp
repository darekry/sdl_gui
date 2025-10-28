#include "radio_button.hpp"
#include "gui_manager.hpp"
#include "radio_group.hpp"

namespace {
    // Funkcja pomocnicza do rysowania wypełnionego okręgu
    void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
    }
}

RadioButton::RadioButton(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_isSelected(false) {
}

bool RadioButton::isSelected() const {
    return m_isSelected;
}

void RadioButton::setSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        markDirty(); // Oznacz jako "brudny" do przerysowania
        if (m_onChange) {
            m_onChange(this, m_isSelected);
        }
        if (m_isSelected && m_parent) {
             if (auto* group = dynamic_cast<RadioGroup*>(m_parent)) {
                 group->onButtonSelected(this);
            }
        }
    }
}

void RadioButton::setOnChange(OnChangeCallback callback) {
    m_onChange = std::move(callback);
}

bool RadioButton::handleEvent(const SDL_Event& e) {
    auto previousState = m_state;
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        if (previousState == ElementState::Pressed && m_state == ElementState::Hover) {
             if (!m_isSelected) {
                setSelected(true);
            }
            return true;
        }
    }
    
    return false;
}

const char* RadioButton::getComponentType() const {
    return "RadioButton";
}
void RadioButton::draw(SDL_Renderer* renderer) {
    const auto& style = getComposedStyle(m_state);
    
    if (style.backgroundColor) {
        const auto& c = style.backgroundColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        SDL_RenderFillRect(renderer, &bgRect);
    }

    if (style.borderColor) {
        const auto& c = style.borderColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect borderRect = {0, 0, m_width, m_height};
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    if (m_isSelected) {
        if (style.texture) {
            SDL_Rect indicatorRect = {0, 0, m_width, m_height};
            SDL_RenderCopy(renderer, style.texture->get(), nullptr, &indicatorRect);
        } else if (style.textColor) {
            const auto& c = style.textColor.value();
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            drawFilledCircle(renderer, m_width / 2, m_height / 2, m_height / 4);
        }
    }
}