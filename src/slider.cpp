#include "slider.hpp"
#include "gui_manager.hpp"
import std.compat;

Slider::Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation)
    : Panel(manager, x, y, width, height),
      m_orientation(orientation),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_currentValue(std::clamp(initialValue, minValue, maxValue)) {
    // Ustawienie koloru tła jest dziedziczone z Panel
    setBackgroundColor({150, 150, 150, 255});
    setClipChildren(false);
    // Określ rozmiar przycisków na podstawie orientacji suwaka
    auto buttonSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();

    if (m_orientation == Orientation::Horizontal) { // Poziomy suwak
        m_decreaseButton = std::make_unique<Button>(manager, -buttonSize, 0, buttonSize, getHeight());
        m_increaseButton = std::make_unique<Button>(manager, getWidth(), 0, buttonSize, getHeight());
    } else { // Pionowy suwak
        m_decreaseButton = std::make_unique<Button>(manager, 0, -buttonSize, getWidth(), buttonSize);
        m_increaseButton = std::make_unique<Button>(manager, 0, getHeight(), getWidth(), buttonSize);
    }
    
    decrementButton = m_decreaseButton.get();
    incrementButton = m_increaseButton.get();

    decrementButton->setLabel("<", 12, {255, 255, 255, 255});
    incrementButton->setLabel(">", 12, {255, 255, 255, 255});
 
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
    // Krok 3a: Standardowe sprawdzenie widoczności.
    if (!isVisible()) return;

    SDL_RenderSetClipRect(m_manager.getRenderer(), nullptr);

    // Krok 3b: Pobierz wskaźnik na renderer.
    auto* renderer = m_manager.getRenderer();
    auto absPos = getAbsolutePosition();
    auto rect = SDL_Rect{absPos.x, absPos.y, getWidth(), getHeight()};

    // Krok 3c: Rysuj tło Slidera.
    SDL_SetRenderDrawColor(renderer, m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Krok 3d: Rysuj "kciuk" Slidera.
    SDL_Rect thumbRect;
    int thumbSize = (m_orientation == Orientation::Horizontal) ? getHeight() : getWidth();
    
    if (m_orientation == Orientation::Horizontal) {
        int thumbX = absPos.x + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getWidth() - thumbSize));
        thumbRect = {.x=thumbX, .y=absPos.y, .w=thumbSize, .h=getHeight()};
    } else {
        int thumbY = absPos.y + (int)(((float)(m_currentValue - m_minValue) / (m_maxValue - m_minValue)) * (getHeight() - thumbSize));
        thumbRect = {.x=absPos.x, .y=thumbY, .w=getWidth(), .h=thumbSize};
    }

    // Użyj koloru `foregroundColor` dla kciuka.
    SDL_SetRenderDrawColor(renderer, m_foregroundColor.r, m_foregroundColor.g, m_foregroundColor.b, m_foregroundColor.a);
    SDL_RenderFillRect(renderer, &thumbRect);
    
    // Krok 3e: Rysuj obramowanie.
    if (m_borderWidth > 0) {
        SDL_SetRenderDrawColor(renderer, m_borderColor.r, m_borderColor.g, m_borderColor.b, m_borderColor.a);
        for (int i = 0; i < m_borderWidth; ++i) {
            SDL_Rect borderRect = {rect.x + i, rect.y + i, rect.w - 2 * i, rect.h - 2 * i};
            SDL_RenderDrawRect(renderer, &borderRect);
        }
    }

    // Krok 3f: Rysowanie dzieci (przycisków) jest obsługiwane przez GUIElement::render() po wywołaniu tej metody.
    // Nie ma potrzeby dodawać tutaj pętli.
}

Button* Slider::getDecrementButton() {
    return decrementButton;
}

Button* Slider::getIncrementButton() {
    return incrementButton;
}