#include "button.hpp"
#include "gui_manager.hpp"

// Implementacja klasy Button
Button::Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label)
    : GUIElement(manager, x, y, width, height) {
    if (!label.empty()) {
        setLabel(label);
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

void Button::draw() {
    // Klasa bazowa zajmuje się całym rysowaniem na podstawie stylu
    GUIElement::draw();
}