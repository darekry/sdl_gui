#pragma once
#include "panel.hpp"
#include "slider.hpp"

class RangeSlider : public Panel {
public:
    RangeSlider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int lowerValue, int upperValue, Orientation orientation);

    int getLowerValue() const { return m_lowerValue; }
    int getUpperValue() const { return m_upperValue; }
    void setLowerValue(int value);
    void setUpperValue(int value);

    void setMin(int min) { m_minValue = min; setLowerValue(m_lowerValue); setUpperValue(m_upperValue); }
    void setMax(int max) { m_maxValue = max; setLowerValue(m_lowerValue); setUpperValue(m_upperValue); }
    void setRange(int min, int max) { m_minValue = min; m_maxValue = max; setLowerValue(m_lowerValue); setUpperValue(m_upperValue); }

    int getMin() const { return m_minValue; }
    int getMax() const { return m_maxValue; }

    using OnChangeCallback = std::function<void(GUIElement*)>;
    void setOnChangeCallback(OnChangeCallback callback) { m_onChange = std::move(callback); }

    void setWheelStep(int step) { m_wheelStep = step > 0 ? step : 1; }
    int getWheelStep() const { return m_wheelStep; }

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

private:
    enum class ActiveThumb : uint8_t {
        None,
        Lower,
        Upper
    };

    void updateValueFromMouse(int mouseX, int mouseY);

    Orientation m_orientation;
    int m_minValue;
    int m_maxValue;
    int m_lowerValue;
    int m_upperValue;
    OnChangeCallback m_onChange;
    ActiveThumb m_activeThumb = ActiveThumb::None;
    ActiveThumb m_hoveredThumb = ActiveThumb::None;
    int m_wheelStep = 1;
};
