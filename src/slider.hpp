#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "button.hpp"
#include "gui.hpp"
#include <functional>
#include <memory>

enum class Orientation {
    Horizontal,
    Vertical
};

class Slider : public GUIElement {
public:
    Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation);

    int getValue() const { return m_currentValue; }

    using OnChangeCallback = std::function<void(GUIElement*)>;
    void setOnChangeCallback(OnChangeCallback callback) { m_onChange = callback; }

    bool handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    Orientation m_orientation;
    int m_minValue;
    int m_maxValue;
    int m_currentValue;
    OnChangeCallback m_onChange;
    bool m_isDragging = false;

    std::unique_ptr<Button> m_decreaseButton;
    std::unique_ptr<Button> m_increaseButton;
};

#endif // SLIDER_HPP