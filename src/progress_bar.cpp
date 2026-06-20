#include "progress_bar.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "gui.hpp"
#include "std.hpp"
namespace {
    constexpr float kEpsilon = 1e-6f;
}

ProgressBar::ProgressBar(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height) {
}

float ProgressBar::normalizedValue() const {
    if (m_maxValue <= m_minValue) {
        return 0.0f;
    }
    return std::clamp((m_value - m_minValue) / (m_maxValue - m_minValue), 0.0f, 1.0f);
}

void ProgressBar::setValue(float value) {
    float clamped = std::clamp(value, m_minValue, m_maxValue);
    if (std::abs(m_value - clamped) > kEpsilon) {
        m_value = clamped;
        markDirty();
    }
}

void ProgressBar::setMin(float min) {
    if (std::abs(m_minValue - min) > kEpsilon) {
        m_minValue = min;
        if (m_maxValue < m_minValue) {
            m_maxValue = m_minValue + 1.0f;
        }
        setValue(m_value);
    }
}

void ProgressBar::setMax(float max) {
    if (std::abs(m_maxValue - max) > kEpsilon) {
        m_maxValue = max;
        if (m_maxValue < m_minValue) {
            m_minValue = m_maxValue - 1.0f;
        }
        setValue(m_value);
    }
}

void ProgressBar::setRange(float min, float max) {
    if (std::abs(m_minValue - min) > kEpsilon || std::abs(m_maxValue - max) > kEpsilon) {
        m_minValue = min;
        m_maxValue = max;
        if (m_maxValue < m_minValue) {
            m_maxValue = m_minValue + 1.0f;
        }
        setValue(m_value);
    }
}

const char* ProgressBar::getComponentType() const {
    return "ProgressBar";
}

void ProgressBar::draw(SDL_Renderer* renderer) {
    Panel::draw(renderer);

    const auto& style = getComposedStyle(m_state);

    float ratio = normalizedValue();

    if (ratio > 0.0f) {
        SDL_Color fillColor;
        if (style.borderColor) {
            fillColor = *style.borderColor;
        } else {
            fillColor = {0, 120, 215, 255};
        }

        int borderW = style.borderWidth.value_or(0);

        SDL_Rect fillRect;
        if (m_orientation == Orientation::Horizontal) {
            fillRect = {
                borderW,
                borderW,
                static_cast<int>(static_cast<float>(m_width - 2 * borderW) * ratio),
                m_height - 2 * borderW
            };
        } else {
            int fillHeight = static_cast<int>(static_cast<float>(m_height - 2 * borderW) * ratio);
            fillRect = {
                borderW,
                m_height - borderW - fillHeight,
                m_width - 2 * borderW,
                fillHeight
            };
        }

        int borderRadius = style.borderRadius.value_or(0);
        if (borderRadius > 0) {
            int maxRadius = (m_orientation == Orientation::Horizontal)
                ? (fillRect.w - 1) / 2
                : (fillRect.h - 1) / 2;
            int effectiveRadius = std::min(borderRadius, maxRadius);
            SDL_FRect ffillRect = {
                static_cast<float>(fillRect.x), static_cast<float>(fillRect.y),
                static_cast<float>(fillRect.w), static_cast<float>(fillRect.h)
            };
            SDL_FColor ffillColor = {
                fillColor.r / 255.0f, fillColor.g / 255.0f,
                fillColor.b / 255.0f, fillColor.a / 255.0f
            };
            drawRoundedFilledRect(renderer, ffillRect, static_cast<float>(effectiveRadius), ffillColor);
        } else {
            SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
            { SDL_FRect _fr = {static_cast<float>(fillRect.x), static_cast<float>(fillRect.y), static_cast<float>(fillRect.w), static_cast<float>(fillRect.h)}; SDL_RenderFillRect(renderer, &_fr); }
        }
    }

    if (m_showText) {
        double pct = static_cast<double>(ratio) * 100.0;
        char buffer[32] = {};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
        snprintf(buffer, sizeof(buffer), m_textFormat.c_str(), pct);
#pragma clang diagnostic pop

        auto& fontManager = m_manager.getFontManager();
        int fontSize = style.fontSize.value_or(16);
        auto fontPath = style.fontName.value_or("assets/fonts/font.ttf");
        auto font = fontManager.loadFont(fontPath, fontSize);

        if (font) {
            auto& textureManager = m_manager.getTextureManager();
            auto textColor = style.textColor.value_or(SDL_Color{0, 0, 0, 255});
            auto textTex = textureManager.createTextureFromText(buffer, font, textColor);

            if (textTex) {
                int tw = 0, th = 0;
                {  float _fw=0,_fh=0; SDL_GetTextureSize(textTex.get(), &_fw, &_fh); tw=static_cast<int>(_fw); th=static_cast<int>(_fh); }
                SDL_Rect dstRect = {
                    (m_width - tw) / 2,
                    (m_height - th) / 2,
                    tw,
                    th
                };
                { SDL_FRect _dr = {static_cast<float>(dstRect.x), static_cast<float>(dstRect.y), static_cast<float>(dstRect.w), static_cast<float>(dstRect.h)}; SDL_RenderTexture(renderer, textTex.get(), nullptr, &_dr); }
            }
        }
    }
}
