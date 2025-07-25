#include "checkbox.hpp"
#include "gui_manager.hpp"

Checkbox::Checkbox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_isChecked(false) {
}

bool Checkbox::isChecked() const {
    return m_isChecked;
}

void Checkbox::setChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (m_onChange) {
            m_onChange(this, m_isChecked);
        }
    }
}

void Checkbox::setOnChange(OnChangeCallback callback) {
    m_onChange = std::move(callback);
}

bool Checkbox::handleEvent(const SDL_Event& e) {
    auto previousState = m_currentState;
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        if (previousState == ElementState::Pressed && m_currentState == ElementState::Hover) {
            setChecked(!m_isChecked);
            return true;
        }
    }
    return false;
}

const char* Checkbox::getComponentType() const {
    return "Checkbox";
}

void Checkbox::draw() {
    GUIElement::draw(); // Rysuje tło i ramkę

    if (m_isChecked) {
        auto* renderer = m_manager.getRenderer();
        const auto style = getResolvedStyle();
        const auto absPos = getAbsolutePosition();

        if (style.textColor) { // Używamy koloru tekstu do rysowania "ptaszka"
            const auto& c = style.textColor.value();
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            // Rysowanie "ptaszka"
            SDL_RenderDrawLine(renderer, absPos.x + 3, absPos.y + m_height / 2, absPos.x + m_height / 2, absPos.y + m_height - 3);
            SDL_RenderDrawLine(renderer, absPos.x + m_height / 2, absPos.y + m_height - 3, absPos.x + m_height - 3, absPos.y + 3);
        }
    }
}