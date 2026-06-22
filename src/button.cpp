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
    if (!m_enabled || !m_visible) {
        return false;
    }

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true;
        }
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT && contains(e.button.x, e.button.y)) {
        setState(ElementState::Pressed);
        m_manager.captureMouse(this);
        return true;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_state == ElementState::Pressed) {
            m_manager.releaseMouse();
            // Fix: Check mouse position before setting state
            if (contains(e.button.x, e.button.y)) {
                setState(ElementState::Hover);
                if (m_onClick) {
                    m_onClick(this);
                }
            } else {
                setState(ElementState::Normal);
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

    processButtonEvent(e);
    
    return false;
}

const char* Button::getComponentType() const {
    return "Button";
}

void Button::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
}