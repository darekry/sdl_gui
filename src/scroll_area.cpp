#include "scroll_area.hpp"
#include "gui_manager.hpp"

ScrollArea::ScrollArea(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height) {
    m_contentWidth = width;
    m_contentHeight = height;

    auto viewport = std::make_unique<Panel>(manager, 0, 0, width, height);
    viewport->setClipChildren(true);
    m_viewport = viewport.get();

    auto content = std::make_unique<Panel>(manager, 0, 0, width, height);
    m_content = content.get();
    viewport->addChild(std::move(content));

    Panel::addChild(std::move(viewport));

    updateLayout();
}

void ScrollArea::setContent(std::unique_ptr<GUIElement> content) {
    if (!m_viewport || !m_content) return;

    auto* contentPanel = dynamic_cast<Panel*>(m_content);
    if (contentPanel) {
        contentPanel->clearChildren();
        if (content) {
            contentPanel->addChild(std::move(content));
        }
    }
    markDirty();
}

void ScrollArea::setContentSize(int width, int height) {
    m_contentWidth = width;
    m_contentHeight = height;
    if (m_content) {
        m_content->setSize(width, height);
    }
    updateSliderRanges();
    markDirty();
}

void ScrollArea::setScrollEnabled(bool vertical, bool horizontal) {
    m_vScrollEnabled = vertical;
    m_hScrollEnabled = horizontal;
    updateLayout();
    markDirty();
}

void ScrollArea::setVerticalScroll(bool enabled) {
    m_vScrollEnabled = enabled;
    updateLayout();
    markDirty();
}

void ScrollArea::setHorizontalScroll(bool enabled) {
    m_hScrollEnabled = enabled;
    updateLayout();
    markDirty();
}

void ScrollArea::setScrollOffset(int x, int y) {
    m_scrollX = x;
    m_scrollY = y;
    if (m_content) {
        m_content->setPosition(-m_scrollX, -m_scrollY);
    }
    if (m_vSlider) m_vSlider->setValue(y);
    if (m_hSlider) m_hSlider->setValue(x);
    markDirty();
}

void ScrollArea::updateLayout() {
    int sliderW = m_sliderSize;
    int sliderH = m_sliderSize;

    bool showV = m_vScrollEnabled;
    bool showH = m_hScrollEnabled;

    int vpW = m_width - (showV ? sliderW : 0);
    int vpH = m_height - (showH ? sliderH : 0);

    if (m_viewport) {
        m_viewport->setPosition(0, 0);
        m_viewport->setSize(vpW, vpH);
    }

    if (showV) {
        if (!m_vSlider) {
            auto vs = std::make_unique<Slider>(m_manager,
                m_width - sliderW, 0, sliderW, vpH,
                0, 1000, 0, Orientation::Vertical);
            m_vSlider = vs.get();
            m_vSlider->setOnChangeCallback([this](GUIElement*) {
                m_scrollY = m_vSlider->getValue();
                if (m_content) m_content->setPosition(-m_scrollX, -m_scrollY);
            });
            Panel::addChild(std::move(vs));
        } else {
            m_vSlider->setPosition(m_width - sliderW, 0);
            m_vSlider->setSize(sliderW, vpH);
            m_vSlider->setVisible(true);
        }
    } else if (m_vSlider) {
        m_vSlider->setVisible(false);
    }

    if (showH) {
        if (!m_hSlider) {
            auto hs = std::make_unique<Slider>(m_manager,
                0, m_height - sliderH, vpW, sliderH,
                0, 1000, 0, Orientation::Horizontal);
            m_hSlider = hs.get();
            m_hSlider->setOnChangeCallback([this](GUIElement*) {
                m_scrollX = m_hSlider->getValue();
                if (m_content) m_content->setPosition(-m_scrollX, -m_scrollY);
            });
            Panel::addChild(std::move(hs));
        } else {
            m_hSlider->setPosition(0, m_height - sliderH);
            m_hSlider->setSize(vpW, sliderH);
            m_hSlider->setVisible(true);
        }
    } else if (m_hSlider) {
        m_hSlider->setVisible(false);
    }

    updateSliderRanges();
}

void ScrollArea::updateSliderRanges() {
    int vpW = m_viewport ? m_viewport->getWidth() : m_width;
    int vpH = m_viewport ? m_viewport->getHeight() : m_height;

    int maxX = std::max(0, m_contentWidth - vpW);
    int maxY = std::max(0, m_contentHeight - vpH);

    if (m_vSlider) {
        m_vSlider->setRange(0, maxY);
        m_scrollY = std::min(m_scrollY, maxY);
        m_vSlider->setValue(m_scrollY);
    }

    if (m_hSlider) {
        m_hSlider->setRange(0, maxX);
        m_scrollX = std::min(m_scrollX, maxX);
        m_hSlider->setValue(m_scrollX);
    }
}

bool ScrollArea::handleEvent(const SDL_Event& e) {
    if (!m_visible || !m_enabled) return false;

    if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        int mx = static_cast<int>(e.wheel.mouse_x);
        int my = static_cast<int>(e.wheel.mouse_y);
        if (contains(mx, my)) {
            if (m_vSlider && m_vSlider->isVisible()) {
                int step = 60;
                int delta = (e.wheel.y > 0) ? -step : step;
                int newVal = std::clamp(m_scrollY + delta, 0,
                    std::max(0, m_contentHeight - (m_viewport ? m_viewport->getHeight() : 0)));
                setScrollOffset(m_scrollX, newVal);
            } else if (m_hSlider && m_hSlider->isVisible()) {
                int step = 60;
                int delta = (e.wheel.y > 0) ? -step : step;
                int newVal = std::clamp(m_scrollX + delta, 0,
                    std::max(0, m_contentWidth - (m_viewport ? m_viewport->getWidth() : 0)));
                setScrollOffset(newVal, m_scrollY);
            }
            return true;
        }
    }

    return Panel::handleEvent(e);
}

void ScrollArea::onParentResize(int parentWidth, int parentHeight) {
    Panel::onParentResize(parentWidth, parentHeight);
    updateLayout();
}
