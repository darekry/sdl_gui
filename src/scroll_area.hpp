#pragma once

#include "panel.hpp"
#include "slider.hpp"

#include "std.hpp"

class ScrollArea : public Panel {
public:
    ScrollArea(GUIManager& manager, int x, int y, int width, int height);

    void setContent(std::unique_ptr<GUIElement> content);
    [[nodiscard]] GUIElement* getContent() const { return m_content; }

    void setContentSize(int width, int height);
    void setScrollEnabled(bool vertical, bool horizontal);
    void setVerticalScroll(bool enabled);
    void setHorizontalScroll(bool enabled);

    [[nodiscard]] int getScrollOffsetX() const { return m_scrollX; }
    [[nodiscard]] int getScrollOffsetY() const { return m_scrollY; }
    void setScrollOffset(int x, int y);

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override { return "ScrollArea"; }
    void onParentResize(int parentWidth, int parentHeight) override;

private:
    void updateLayout();
    void updateSliderRanges();

    Panel* m_viewport = nullptr;
    GUIElement* m_content = nullptr;
    Slider* m_vSlider = nullptr;
    Slider* m_hSlider = nullptr;

    int m_sliderSize = 16;
    int m_scrollX = 0;
    int m_scrollY = 0;
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    bool m_vScrollEnabled = true;
    bool m_hScrollEnabled = false;
};
