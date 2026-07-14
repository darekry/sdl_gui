#pragma once
#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"


enum class Orientation : uint8_t {
    Horizontal,
    Vertical
};

class Slider : public Panel {
public:
    Slider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int initialValue, Orientation orientation);

    int getValue() const { return m_currentValue; }
    void setValue(int value);
    
    void setMin(int min) { m_minValue = min; setValue(m_currentValue); }
    void setMax(int max) { m_maxValue = max; setValue(m_currentValue); }
    void setRange(int min, int max) { m_minValue = min; m_maxValue = max; setValue(m_currentValue); }
    
    int getMin() const { return m_minValue; }
    int getMax() const { return m_maxValue; }

    using OnChangeCallback = std::function<void(GUIElement*)>;
    void setOnChangeCallback(OnChangeCallback callback) { m_onChange = std::move(callback); }

    void setWheelStep(int step) { m_wheelStep = step > 0 ? step : 1; }
    int getWheelStep() const { return m_wheelStep; }

    [[nodiscard]] Orientation getOrientation() const { return m_orientation; }

    Button* getDecrementButton();
    Button* getIncrementButton();

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

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
    int m_wheelStep = 1; // Wartość zmiany przy przewijaniu kółkiem myszy

    // Pola do zarządzania wewnętrznym układem
    int m_trackSize;
    int m_trackOffsetX = 0;
    int m_trackOffsetY = 0;

    std::unique_ptr<Button> m_decreaseButton;
    std::unique_ptr<Button> m_increaseButton;
};
