#include "slider.hpp"
#include <algorithm> // Dla std::clamp

Slider::Slider(int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation)
    : GUIElement(x, y, width, height),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_currentValue(std::clamp(initialValue, minValue, maxValue)),
      m_orientation(orientation) { // Zainicjowanie orientacji
    
    // Określ rozmiar przycisków na podstawie orientacji suwaka
    int buttonSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();

    if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
        // Przyciski po lewej i prawej stronie
        m_decreaseButton = new Button(0, 0, buttonSize, getHeight());
        m_increaseButton = new Button(getWidth() - buttonSize, 0, buttonSize, getHeight());
    } else { // Pionowy suwak
        // Przyciski na górze i na dole
        m_decreaseButton = new Button(0, 0, getWidth(), buttonSize);
        m_increaseButton = new Button(0, getHeight() - buttonSize, getWidth(), buttonSize);
    }

    // Ustaw callbacki dla przycisków
    m_decreaseButton->setOnClickCallback([this](GUIElement* button) {
        m_currentValue = std::clamp(m_currentValue - 1, m_minValue, m_maxValue); // Zmniejsz wartość o 1
        if (m_onChange) {
            m_onChange(this); // Przekazujemy wskaźnik do Slidera
        }
    });

    m_increaseButton->setOnClickCallback([this](GUIElement* button) {
        m_currentValue = std::clamp(m_currentValue + 1, m_minValue, m_maxValue); // Zwiększ wartość o 1
        if (m_onChange) {
            m_onChange(this); // Przekazujemy wskaźnik do Slidera
        }
    });

    // Dodaj przyciski jako dzieci suwaka
    addChild(m_decreaseButton);
    addChild(m_increaseButton);
}

void Slider::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT && contains(e.button.x, e.button.y)) {
            m_isDragging = true;
            // Oblicz początkową wartość na podstawie pozycji kliknięcia
            int clickX = e.button.x - getAbsolutePosition().x;
            int clickY = e.button.y - getAbsolutePosition().y;
            
            if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
                float ratio = (float)clickX / getWidth();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            } else { // Pionowy suwak
                float ratio = (float)clickY / getHeight();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            }
            m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);
            if (m_onChange) {
                m_onChange(this);
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            m_isDragging = false;
        }
    } else if (e.type == SDL_MOUSEMOTION) {
        if (m_isDragging) {
            int mouseX = e.motion.x - getAbsolutePosition().x;
            int mouseY = e.motion.y - getAbsolutePosition().y;

            if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
                float ratio = (float)mouseX / getWidth();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            } else { // Pionowy suwak
                float ratio = (float)mouseY / getHeight();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            }
            m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);
            if (m_onChange) {
                m_onChange(this);
            }
        }
    }
    // Przekaż zdarzenie do przycisków
    m_decreaseButton->handleEvent(e);
    m_increaseButton->handleEvent(e);
}

void Slider::render(SDL_Renderer* renderer) {
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect sliderBarRect = {absPos.x, absPos.y, getWidth(), getHeight()};

    // Rysuj tło suwaka (np. szary prostokąt)
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &sliderBarRect);

    // Oblicz pozycję "kciuka" suwaka
    // Oblicz pozycję "kciuka" suwaka
    int thumbSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth(); // Rozmiar kciuka zależy od mniejszego wymiaru suwaka
    SDL_Rect thumbRect;

    if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
        int thumbX = absPos.x + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getWidth() - thumbSize));
        thumbRect = {thumbX, absPos.y, thumbSize, getHeight()};
    } else { // Pionowy suwak
        int thumbY = absPos.y + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getHeight() - thumbSize));
        thumbRect = {absPos.x, thumbY, getWidth(), thumbSize};
    }
    // Rysuj "kciuk" suwaka (np. niebieski prostokąt)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &thumbRect);

    // Renderuj dzieci (przyciski strzałek)
    for (GUIElement* child : m_children) {
        child->render(renderer);
    }
}