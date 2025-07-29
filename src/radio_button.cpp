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
    auto previousState = m_currentState;
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        if (previousState == ElementState::Pressed && m_currentState == ElementState::Hover) {
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

void RadioButton::draw() {
    GUIElement::draw(); // Rysuje tło i (opcjonalnie) ramkę

    // Rysowanie "ptaszka" jeśli zaznaczony
    if (m_isSelected) {
        auto* renderer = m_manager.getRenderer();
        const auto style = getResolvedStyle();
        const auto absPos = getAbsolutePosition();

        if (style.textColor) { // Używamy koloru tekstu do rysowania kropki
            const auto& c = style.textColor.value();
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            drawFilledCircle(renderer, absPos.x + m_width / 2, absPos.y + m_height / 2, m_width / 4);
        }
    }
}