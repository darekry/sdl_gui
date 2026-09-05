#include "range_slider.hpp"
#include "constants.hpp"
#include "sdl_rect_helpers.hpp"


RangeSlider::RangeSlider(GUIManager& manager, int x, int y, int width, int height, int minValue, int maxValue, int lowerValue, int upperValue, Orientation orientation)
    : Panel(manager, x, y, width, height),
      m_orientation(orientation),
      m_minValue(minValue),
      m_maxValue(maxValue),
      m_lowerValue(std::clamp(lowerValue, minValue, maxValue)),
      m_upperValue(std::clamp(upperValue, minValue, maxValue)) {

    if (m_lowerValue > m_upperValue) {
        std::swap(m_lowerValue, m_upperValue);
    }

    setClipChildren(true);
}

void RangeSlider::updateValueFromMouse(int mouseX, int mouseY) {
    if (getWidth() <= 0 || getHeight() <= 0) {
        return;
    }

    auto absPos = getAbsolutePosition();

    if (m_activeThumb == ActiveThumb::Lower) {
        float ratio;
        if (m_orientation == Orientation::Horizontal) {
            float relativeMouseX = static_cast<float>(mouseX - absPos.x);
            ratio = std::clamp(relativeMouseX / static_cast<float>(getWidth()), 0.0f, 1.0f);
        } else {
            float relativeMouseY = static_cast<float>(mouseY - absPos.y);
            ratio = std::clamp(relativeMouseY / static_cast<float>(getHeight()), 0.0f, 1.0f);
        }
        int newValue = m_minValue + static_cast<int>(ratio * static_cast<float>(m_maxValue - m_minValue));
        newValue = std::min(newValue, m_upperValue);
        setLowerValue(newValue);
    } else if (m_activeThumb == ActiveThumb::Upper) {
        float ratio;
        if (m_orientation == Orientation::Horizontal) {
            float relativeMouseX = static_cast<float>(mouseX - absPos.x);
            ratio = std::clamp(relativeMouseX / static_cast<float>(getWidth()), 0.0f, 1.0f);
        } else {
            float relativeMouseY = static_cast<float>(mouseY - absPos.y);
            ratio = std::clamp(relativeMouseY / static_cast<float>(getHeight()), 0.0f, 1.0f);
        }
        int newValue = m_minValue + static_cast<int>(ratio * static_cast<float>(m_maxValue - m_minValue));
        newValue = std::max(newValue, m_lowerValue);
        setUpperValue(newValue);
    }
}

bool RangeSlider::handleEvent(const SDL_Event& e) {
    if (!m_enabled) return false;

    if (GUIElement::handleEvent(e)) {
        return true;
    }

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        auto absPos = getAbsolutePosition();
        SDL_Point mousePoint = {static_cast<int>(e.motion.x), static_cast<int>(e.motion.y)};
        int thumbSize = (m_orientation == Orientation::Horizontal) ? std::min(getHeight(), 20) : std::min(getWidth(), 20);

        if (m_activeThumb != ActiveThumb::None) {
            if (!contains(static_cast<int>(e.motion.x), static_cast<int>(e.motion.y))) {
                m_activeThumb = ActiveThumb::None;
                m_hoveredThumb = ActiveThumb::None;
                return false;
            }
            updateValueFromMouse(static_cast<int>(e.motion.x), static_cast<int>(e.motion.y));
            return true;
        }

        if (m_orientation == Orientation::Horizontal) {
            float lowerRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_lowerValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            int lowerX = absPos.x + static_cast<int>(lowerRatio * static_cast<float>(getWidth() - thumbSize));
            SDL_Rect lowerRect = {lowerX, absPos.y, thumbSize, getHeight()};
            if (SDL_PointInRect(&mousePoint, &lowerRect)) {
                m_hoveredThumb = ActiveThumb::Lower;
            } else {
                float upperRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_upperValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
                int upperX = absPos.x + static_cast<int>(upperRatio * static_cast<float>(getWidth() - thumbSize));
                SDL_Rect upperRect = {upperX, absPos.y, thumbSize, getHeight()};
                m_hoveredThumb = SDL_PointInRect(&mousePoint, &upperRect) ? ActiveThumb::Upper : ActiveThumb::None;
            }
        } else {
            float lowerRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_lowerValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            int lowerY = absPos.y + static_cast<int>(lowerRatio * static_cast<float>(getHeight() - thumbSize));
            SDL_Rect lowerRect = {absPos.x, lowerY, getWidth(), thumbSize};
            if (SDL_PointInRect(&mousePoint, &lowerRect)) {
                m_hoveredThumb = ActiveThumb::Lower;
            } else {
                float upperRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_upperValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
                int upperY = absPos.y + static_cast<int>(upperRatio * static_cast<float>(getHeight() - thumbSize));
                SDL_Rect upperRect = {absPos.x, upperY, getWidth(), thumbSize};
                m_hoveredThumb = SDL_PointInRect(&mousePoint, &upperRect) ? ActiveThumb::Upper : ActiveThumb::None;
            }
        }

    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        auto absPos = getAbsolutePosition();
        SDL_Point mousePoint = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)};
        int thumbSize = (m_orientation == Orientation::Horizontal) ? std::min(getHeight(), 20) : std::min(getWidth(), 20);

        if (m_orientation == Orientation::Horizontal) {
            float lowerRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_lowerValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            float upperRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_upperValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            int lowerX = absPos.x + static_cast<int>(lowerRatio * static_cast<float>(getWidth() - thumbSize));
            int upperX = absPos.x + static_cast<int>(upperRatio * static_cast<float>(getWidth() - thumbSize));
            SDL_Rect lowerRect = {lowerX, absPos.y, thumbSize, getHeight()};
            SDL_Rect upperRect = {upperX, absPos.y, thumbSize, getHeight()};

            if (SDL_PointInRect(&mousePoint, &lowerRect)) {
                m_activeThumb = ActiveThumb::Lower;
                m_hoveredThumb = ActiveThumb::Lower;
                return true;
            } else if (SDL_PointInRect(&mousePoint, &upperRect)) {
                m_activeThumb = ActiveThumb::Upper;
                m_hoveredThumb = ActiveThumb::Upper;
                return true;
            } else if (contains(mousePoint.x, mousePoint.y)) {
                float ratio = std::clamp(static_cast<float>(mousePoint.x - absPos.x) / static_cast<float>(getWidth()), 0.0f, 1.0f);
                int clickValue = m_minValue + static_cast<int>(ratio * static_cast<float>(m_maxValue - m_minValue));
                if (std::abs(clickValue - m_lowerValue) <= std::abs(clickValue - m_upperValue)) {
                    m_activeThumb = ActiveThumb::Lower;
                    m_hoveredThumb = ActiveThumb::Lower;
                    setLowerValue(clickValue);
                } else {
                    m_activeThumb = ActiveThumb::Upper;
                    m_hoveredThumb = ActiveThumb::Upper;
                    setUpperValue(clickValue);
                }
                return true;
            }
        } else {
            float lowerRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_lowerValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            float upperRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_upperValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
            int lowerY = absPos.y + static_cast<int>(lowerRatio * static_cast<float>(getHeight() - thumbSize));
            int upperY = absPos.y + static_cast<int>(upperRatio * static_cast<float>(getHeight() - thumbSize));
            SDL_Rect lowerRect = {absPos.x, lowerY, getWidth(), thumbSize};
            SDL_Rect upperRect = {absPos.x, upperY, getWidth(), thumbSize};

            if (SDL_PointInRect(&mousePoint, &lowerRect)) {
                m_activeThumb = ActiveThumb::Lower;
                m_hoveredThumb = ActiveThumb::Lower;
                return true;
            } else if (SDL_PointInRect(&mousePoint, &upperRect)) {
                m_activeThumb = ActiveThumb::Upper;
                m_hoveredThumb = ActiveThumb::Upper;
                return true;
            } else if (contains(mousePoint.x, mousePoint.y)) {
                float ratio = std::clamp(static_cast<float>(mousePoint.y - absPos.y) / static_cast<float>(getHeight()), 0.0f, 1.0f);
                int clickValue = m_minValue + static_cast<int>(ratio * static_cast<float>(m_maxValue - m_minValue));
                if (std::abs(clickValue - m_lowerValue) <= std::abs(clickValue - m_upperValue)) {
                    m_activeThumb = ActiveThumb::Lower;
                    m_hoveredThumb = ActiveThumb::Lower;
                    setLowerValue(clickValue);
                } else {
                    m_activeThumb = ActiveThumb::Upper;
                    m_hoveredThumb = ActiveThumb::Upper;
                    setUpperValue(clickValue);
                }
                return true;
            }
        }
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_activeThumb != ActiveThumb::None) {
            m_activeThumb = ActiveThumb::None;
            return true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_WHEEL && m_isHovered) {
        int delta = static_cast<int>(e.wheel.y) * m_wheelStep;
        if (delta != 0) {
            if (m_hoveredThumb == ActiveThumb::Lower || m_hoveredThumb == ActiveThumb::None) {
                setLowerValue(m_lowerValue + delta);
            } else {
                setUpperValue(m_upperValue + delta);
            }
            return true;
        }
    }
    return false;
}

ComponentType RangeSlider::getComponentTypeId() const {
    return ComponentType::RangeSlider;
}

void RangeSlider::setLowerValue(int value) {
    auto oldValue = m_lowerValue;
    m_lowerValue = std::clamp(value, m_minValue, m_upperValue);
    if (m_lowerValue != oldValue) {
        markDirty();
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void RangeSlider::setUpperValue(int value) {
    auto oldValue = m_upperValue;
    m_upperValue = std::clamp(value, m_lowerValue, m_maxValue);
    if (m_upperValue != oldValue) {
        markDirty();
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void RangeSlider::draw(SDL_Renderer* renderer) {
    Panel::draw(renderer);

    const auto& style = getComposedStyle(m_state);

    SDL_Color trackColor;
    if (style.backgroundColor) {
        trackColor = *style.backgroundColor;
        trackColor.r = std::max<uint8_t>(uint8_t{0}, trackColor.r - 20);
        trackColor.g = std::max<uint8_t>(uint8_t{0}, trackColor.g - 20);
        trackColor.b = std::max<uint8_t>(uint8_t{0}, trackColor.b - 20);
    } else {
        trackColor = {.r=200, .g=200, .b=200, .a=255};
    }
    SetDrawColor(renderer, trackColor);

    SDL_Rect trackRect;
    const int trackThickness = 4;
    if (m_orientation == Orientation::Horizontal) {
        trackRect = {0, getHeight() / 2 - trackThickness / 2, getWidth(), trackThickness};
    } else {
        trackRect = {getWidth() / 2 - trackThickness / 2, 0, trackThickness, getHeight()};
    }
    RenderFillRect(renderer, trackRect);

    int thumbSize = (m_orientation == Orientation::Horizontal) ? std::min(getHeight(), 20) : std::min(getWidth(), 20);

    float lowerRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_lowerValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;
    float upperRatio = (m_maxValue > m_minValue) ? static_cast<float>(m_upperValue - m_minValue) / static_cast<float>(m_maxValue - m_minValue) : 0.0f;

    const SDL_Color baseThumb = effectiveThumbColor(style);
    SDL_Color lowerThumbColor = (m_activeThumb == ActiveThumb::Lower || m_hoveredThumb == ActiveThumb::Lower)
        ? SDL_Color{180, 180, 180, 255}
        : baseThumb;
    SDL_Color upperThumbColor = (m_activeThumb == ActiveThumb::Upper || m_hoveredThumb == ActiveThumb::Upper)
        ? SDL_Color{180, 180, 180, 255}
        : baseThumb;

    if (m_orientation == Orientation::Horizontal) {
        int trackSpan = getWidth() - thumbSize;

        int lowerX = static_cast<int>(lowerRatio * static_cast<float>(trackSpan));
        SDL_Rect lowerThumbRect = {.x=lowerX, .y=getHeight() / 2 - thumbSize / 2, .w=thumbSize, .h=thumbSize};
        SetDrawColor(renderer, lowerThumbColor);
        RenderFillRect(renderer, lowerThumbRect);

        int upperX = static_cast<int>(upperRatio * static_cast<float>(trackSpan));
        SDL_Rect upperThumbRect = {.x=upperX, .y=getHeight() / 2 - thumbSize / 2, .w=thumbSize, .h=thumbSize};
        SetDrawColor(renderer, upperThumbColor);
        RenderFillRect(renderer, upperThumbRect);

        if (upperX > lowerX + thumbSize) {
            SDL_Color rangeColor;
            if (style.backgroundColor) {
                rangeColor = *style.backgroundColor;
                rangeColor.r = std::min<uint8_t>(uint8_t{255}, rangeColor.r + 50);
                rangeColor.g = std::min<uint8_t>(uint8_t{255}, rangeColor.g + 50);
                rangeColor.b = std::min<uint8_t>(uint8_t{255}, rangeColor.b + 50);
            } else {
                rangeColor = {.r=140, .g=160, .b=210, .a=255};
            }
            SetDrawColor(renderer, rangeColor);
            SDL_Rect rangeRect = {.x=lowerX + thumbSize, .y=getHeight() / 2 - trackThickness / 2, .w=upperX - lowerX - thumbSize, .h=trackThickness};
            RenderFillRect(renderer, rangeRect);
        }
    } else {
        int trackSpan = getHeight() - thumbSize;

        int lowerY = static_cast<int>(lowerRatio * static_cast<float>(trackSpan));
        SDL_Rect lowerThumbRect = {.x=getWidth() / 2 - thumbSize / 2, .y=lowerY, .w=thumbSize, .h=thumbSize};
        SetDrawColor(renderer, lowerThumbColor);
        RenderFillRect(renderer, lowerThumbRect);

        int upperY = static_cast<int>(upperRatio * static_cast<float>(trackSpan));
        SDL_Rect upperThumbRect = {.x=getWidth() / 2 - thumbSize / 2, .y=upperY, .w=thumbSize, .h=thumbSize};
        SetDrawColor(renderer, upperThumbColor);
        RenderFillRect(renderer, upperThumbRect);

        if (upperY > lowerY + thumbSize) {
            SDL_Color rangeColor;
            if (style.backgroundColor) {
                rangeColor = *style.backgroundColor;
                rangeColor.r = std::min<uint8_t>(uint8_t{255}, rangeColor.r + 50);
                rangeColor.g = std::min<uint8_t>(uint8_t{255}, rangeColor.g + 50);
                rangeColor.b = std::min<uint8_t>(uint8_t{255}, rangeColor.b + 50);
            } else {
                rangeColor = {.r=140, .g=160, .b=210, .a=255};
            }
            SetDrawColor(renderer, rangeColor);
            SDL_Rect rangeRect = {.x=getWidth() / 2 - trackThickness / 2, .y=lowerY + thumbSize, .w=trackThickness, .h=upperY - lowerY - thumbSize};
            RenderFillRect(renderer, rangeRect);
        }
    }
}
