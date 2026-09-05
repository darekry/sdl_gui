#pragma once
#include "panel.hpp"
#include "slider.hpp"

class ProgressBar : public Panel {
public:
    ProgressBar(GUIManager& manager, int x, int y, int width, int height);

    float getValue() const { return m_value; }
    void setValue(float value);

    float getMin() const { return m_minValue; }
    float getMax() const { return m_maxValue; }
    void setMin(float min);
    void setMax(float max);
    void setRange(float min, float max);

    void setOrientation(Orientation orientation) { m_orientation = orientation; markDirty(); }
    Orientation getOrientation() const { return m_orientation; }

    void setShowText(bool show) { m_showText = show; markDirty(); }
    bool getShowText() const { return m_showText; }

    void setTextFormat(const std::string& format) { m_textFormat = format; markDirty(); }

    float* getValuePtr() { return &m_value; }

    ComponentType getComponentTypeId() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

    bool canShareRenderCache() const override { return false; }
private:
    float normalizedValue() const;

    Orientation m_orientation = Orientation::Horizontal;
    float m_value = 0.0f;
    float m_minValue = 0.0f;
    float m_maxValue = 100.0f;
    bool m_showText = true;
    std::string m_textFormat = "%.0f%%";
};
