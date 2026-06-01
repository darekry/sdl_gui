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
    if (!m_enabled || !m_visible) {
        return false;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && contains(e.button.x, e.button.y)) {
        setState(ElementState::Pressed);
        m_manager.captureMouse(this);
        return true;
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_state == ElementState::Pressed) {
            m_manager.releaseMouse();
            setState(ElementState::Hover);
            if (contains(e.button.x, e.button.y)) {
                setChecked(!m_isChecked);
            }
            return true;
        }
    }

    if (e.type == SDL_MOUSEMOTION) {
        if (m_state != ElementState::Pressed) {
            if (contains(e.motion.x, e.motion.y)) {
                setState(ElementState::Hover);
            } else {
                setState(ElementState::Normal);
            }
        }
    }

    // Call parent to handle tooltip timer logic
    GUIElement::handleEvent(e);
    
    return false;
}

const char* Checkbox::getComponentType() const {
    return "Checkbox";
}
void Checkbox::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    const auto& style = getComposedStyle(m_state);

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