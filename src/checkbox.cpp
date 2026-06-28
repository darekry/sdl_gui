#include "checkbox.hpp"
#include "gui_manager.hpp"

Checkbox::Checkbox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_isChecked(false) {
    setCanGetKeyboardFocus(true);
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

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT && contains(e.button.x, e.button.y)) {
        setState(ElementState::Pressed);
        m_manager.captureMouse(this);
        m_manager.setKeyboardFocus(this);
        return true;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_state == ElementState::Pressed) {
            m_manager.releaseMouse();
            setState(ElementState::Hover);
            if (contains(e.button.x, e.button.y)) {
                setChecked(!m_isChecked);
            }
            return true;
        }
    }

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        if (m_state != ElementState::Pressed) {
            bool inside = contains(e.motion.x, e.motion.y);
            setState(inside ? ElementState::Hover : ElementState::Normal);
            processHoverTooltip(inside);
        }
    }

    if (hasKeyboardFocus()) {
        if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat && e.key.key == SDLK_SPACE) {
            setState(ElementState::Pressed);
            return true;
        }
        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_SPACE) {
            if (m_state == ElementState::Pressed) {
                setState(ElementState::Hover);
                setChecked(!m_isChecked);
            }
            return true;
        }
    }

    processButtonEvent(e);
    
    return false;
}

const char* Checkbox::getComponentType() const {
    return "Checkbox";
}
void Checkbox::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);

    if (m_isChecked) {
        const auto& style = getComposedStyle(m_state);
        if (style.textColor) {
            const auto& c = style.textColor.value();
            SetDrawColor(renderer, c);

            SDL_Rect checkRect = { 3, 3, m_width - 6, m_height - 6 };
            RenderLine(renderer, checkRect.x, checkRect.y + checkRect.h / 2, checkRect.x + checkRect.w / 2, checkRect.y + checkRect.h);
            RenderLine(renderer, checkRect.x + checkRect.w / 2, checkRect.y + checkRect.h, checkRect.x + checkRect.w, checkRect.y);
        }
    }
}