#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "gui.hpp"
#include "gui.hpp" // Klasa Button jest zdefiniowana w gui.hpp
#include <functional>

enum class Orientation {
    Horizontal,
    Vertical
};

class Slider : public GUIElement {
public:
    // Konstruktor
    Slider(int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation);

    // Metoda do pobierania aktualnej wartości
    int getValue() const { return m_currentValue; }

    // Typ callbacka dla zmiany wartości
    // Typ callbacka dla zmiany wartości
    using OnChangeCallback = std::function<void(GUIElement*)>;

    // Metoda do przypisywania callbacka
    void setOnChangeCallback(OnChangeCallback callback) { m_onChange = callback; }

    // Przesłonięte metody do obsługi zdarzeń i renderowania
    void handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    Orientation m_orientation; // Dodano pole do przechowywania orientacji
    int m_minValue;
    int m_maxValue;
    int m_currentValue;
    OnChangeCallback m_onChange;
    bool m_isDragging = false; // Do obsługi przeciągania

    Button* m_decreaseButton; // Przycisk do zmniejszania wartości
    Button* m_increaseButton; // Przycisk do zwiększania wartości
};

#endif // SLIDER_HPP