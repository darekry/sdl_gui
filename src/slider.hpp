#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"
import std.compat;

enum class Orientation : uint8_t {
    Horizontal,
    Vertical
};

class Slider : public Panel {
public:
    Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation);

    int getValue() const { return m_currentValue; }

    using OnChangeCallback = std::function<void(GUIElement*)>;
    void setOnChangeCallback(OnChangeCallback callback) { m_onChange = std::move(callback); }

    Button* getDecrementButton();
    Button* getIncrementButton();

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw() override;

private:
    void updateValueFromMouse(int mouseX, int mouseY);

    Button* decrementButton = nullptr;
    Button* incrementButton = nullptr;

    Orientation m_orientation;
    int m_minValue;
    int m_maxValue;
    int m_currentValue;
    OnChangeCallback m_onChange;
    bool m_isDragging = false;

    // Pola do zarządzania wewnętrznym układem
    int m_trackSize;
    int m_trackOffsetX = 0;
    int m_trackOffsetY = 0;

    std::unique_ptr<Button> m_decreaseButton;
    std::unique_ptr<Button> m_increaseButton;
};

#endif // SLIDER_HPP