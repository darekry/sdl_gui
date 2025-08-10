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
        markDirty();
        if (m_onChange) {
            m_onChange(this, m_isChecked);
        }
    }
}

void Checkbox::setOnChange(OnChangeCallback callback) {
    m_onChange = std::move(callback);
}

bool Checkbox::handleEvent(const SDL_Event& e) {
    auto previousState = m_state;
    GUIElement::handleEvent(e);

    if (m_enabled && m_visible) {
        if (previousState == ElementState::Pressed && m_state == ElementState::Hover) {
            setChecked(!m_isChecked);
            return true;
        }
    }
    return false;
}

const char* Checkbox::getComponentType() const {
    return "Checkbox";
}
void Checkbox::draw(SDL_Renderer* renderer) {
    const auto& style = getComposedStyle(m_state);

    // 1. Rysowanie tła
    if (style.backgroundColor) {
        const auto& c = style.backgroundColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        SDL_RenderFillRect(renderer, &bgRect);
    }

    // 2. Rysowanie ramki
    if (style.borderColor) {
        const auto& c = style.borderColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect borderRect = {0, 0, m_width, m_height};
        SDL_RenderDrawRect(renderer, &borderRect);
    }
    // 3. Rysowanie "ptaszka", jeśli zaznaczony
    if (m_isChecked) {
        if (style.texture) {
            SDL_RenderCopy(renderer, (*style.texture).get(), nullptr, nullptr);
        } else if (style.textColor) {
            const auto& c = style.textColor.value();
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

            SDL_Rect checkRect = { 3, 3, m_width - 6, m_height - 6 };
            SDL_RenderDrawLine(renderer, checkRect.x, checkRect.y + checkRect.h / 2, checkRect.x + checkRect.w / 2, checkRect.y + checkRect.h);
            SDL_RenderDrawLine(renderer, checkRect.x + checkRect.w / 2, checkRect.y + checkRect.h, checkRect.x + checkRect.w, checkRect.y);
        }
    }
}