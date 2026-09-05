#include "radio_button.hpp"
#include "gui_manager.hpp"
#include "radio_group.hpp"

namespace {
    void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    RenderPoint(renderer, centerX + x, centerY + y);
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
        markDirty();
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

ComponentType RadioButton::getComponentTypeId() const {
    return ComponentType::RadioButton;
}
void RadioButton::draw(SDL_Renderer* renderer) {
    const auto& style = getComposedStyle(m_state);

    if (style.backgroundColor) {
        const auto& c = style.backgroundColor.value();
        SetDrawColor(renderer, c);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        RenderFillRect(renderer, bgRect);
    }

    // Bevel vs plain border: one shared renderer (see drawResolvedBorder).
    // style.texture stays the selection indicator, not the background.
    drawResolvedBorder(renderer, SDL_Rect{0, 0, m_width, m_height}, style, supportsBevel());

    if (m_isSelected) {
        if (style.texture) {
            SDL_Rect indicatorRect = {0, 0, m_width, m_height};
            RenderTexture(renderer, style.texture->get(), indicatorRect);
        } else if (style.textColor) {
            const auto& c = style.textColor.value();
            SetDrawColor(renderer, c);
            drawFilledCircle(renderer, m_width / 2, m_height / 2, m_height / 4);
        }
    }
}