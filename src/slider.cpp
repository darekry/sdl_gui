#include "slider.hpp"
#include "gui_manager.hpp"
import std.compat;

Slider::Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation)
    : GUIElement(manager, x, y, width, height),
      m_orientation(orientation),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_currentValue(std::clamp(initialValue, minValue, maxValue)) {
    
    // Określ rozmiar przycisków na podstawie orientacji suwaka
    auto buttonSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();

    if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
        m_decreaseButton = std::make_unique<Button>(manager, -buttonSize, 0, buttonSize, getHeight());
        m_increaseButton = std::make_unique<Button>(manager, getWidth(), 0, buttonSize, getHeight());
    } else { // Pionowy suwak
        m_decreaseButton = std::make_unique<Button>(manager, 0, -buttonSize, getWidth(), buttonSize);
        m_increaseButton = std::make_unique<Button>(manager, 0, getHeight(), getWidth(), buttonSize);
    }
 
      // Ustaw callbacki dla przycisków
      m_decreaseButton->setOnClickCallback([this](GUIElement*) {
          auto oldValue = m_currentValue;
          m_currentValue = std::clamp(m_currentValue - 1, m_minValue, m_maxValue);
          if (m_onChange && m_currentValue != oldValue) {
              m_onChange(this);
          }
      });
 
      m_increaseButton->setOnClickCallback([this](GUIElement*) {
          auto oldValue = m_currentValue;
          m_currentValue = std::clamp(m_currentValue + 1, m_minValue, m_maxValue);
          if (m_onChange && m_currentValue != oldValue) {
              m_onChange(this);
          }
      });
 
      // Dodaj przyciski jako dzieci suwaka, przenosząc własność
      addChild(std::move(m_decreaseButton));
      addChild(std::move(m_increaseButton));
  }

bool Slider::handleEvent(const SDL_Event& e) {
    if (!m_enabled) {
        return false;
    }

    // Najpierw przekaż zdarzenie do dzieci (przycisków)
    if (GUIElement::handleEvent(e)) {
        return true; // Jeśli dziecko obsłużyło zdarzenie, zakończ
    }

    // Następnie obsłuż logikę przeciągania suwaka
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT && contains(e.button.x, e.button.y)) {
            m_isDragging = true;
            auto oldValue = m_currentValue;
            auto mouseX = e.button.x - getAbsolutePosition().x;
            auto mouseY = e.button.y - getAbsolutePosition().y;
            if (m_orientation == Orientation::Horizontal) {
                auto ratio = static_cast<float>(mouseX) / getWidth();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            } else {
                auto ratio = static_cast<float>(mouseY) / getHeight();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            }
            m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);
            if (m_onChange && m_currentValue != oldValue) {
                m_onChange(this);
            }
            return true;
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.button == SDL_BUTTON_LEFT && m_isDragging) {
            m_isDragging = false;
            return true;
        }
    } else if (e.type == SDL_MOUSEMOTION) {
        if (m_isDragging) {
            auto oldValue = m_currentValue;
            auto mouseX = e.motion.x - getAbsolutePosition().x;
            auto mouseY = e.motion.y - getAbsolutePosition().y;
            if (m_orientation == Orientation::Horizontal) {
                auto ratio = static_cast<float>(mouseX) / getWidth();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            } else {
                auto ratio = static_cast<float>(mouseY) / getHeight();
                m_currentValue = m_minValue + ratio * (m_maxValue - m_minValue);
            }
            m_currentValue = std::clamp(m_currentValue, m_minValue, m_maxValue);
            if (m_onChange && m_currentValue != oldValue) {
                m_onChange(this);
            }
            return true;
        }
    }

    return false;
}

void Slider::draw() {
    auto* renderer = m_manager.getRenderer();
    auto absPos = getAbsolutePosition();
    auto sliderBarRect = SDL_Rect{absPos.x, absPos.y, getWidth(), getHeight()};

    // Rysuj tło suwaka
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderFillRect(renderer, &sliderBarRect);

    // Oblicz pozycję "kciuka" suwaka
    auto thumbSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();
    auto thumbRect = SDL_Rect{};

    if (m_orientation == Orientation::Horizontal) {
        auto thumbX = absPos.x + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getWidth() - thumbSize));
        thumbRect = {.x=thumbX, .y=absPos.y, .w=thumbSize, .h=getHeight()};
    } else {
        auto thumbY = absPos.y + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getHeight() - thumbSize));
        thumbRect = {.x=absPos.x, .y=thumbY, .w=getWidth(), .h=thumbSize};
    }
    // Rysuj "kciuk" suwaka
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &thumbRect);

    // Dzieci są renderowane automatycznie
}