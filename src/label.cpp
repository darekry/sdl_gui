#include "label.hpp"
#include <SDL3/SDL_log.h>
#include "gui_manager.hpp"
#include "gui.hpp"


void Label::recalculateSize() {
    if (m_text.empty()) {
        setSize(0, 0);
        return;
    }
    const auto& resolvedStyle = getComposedStyle(m_state);
    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or("assets/fonts/font.ttf"), font_size);
    if (font) {
        int textWidth = 0, textHeight = 0;
        if (!TTF_GetStringSize(font.get(), m_text.c_str(), m_text.length(), &textWidth, &textHeight)) {
            LOG_DEBUG("Label: TTF_GetStringSize failed: %s", SDL_GetError());
            textWidth = textHeight = 0;
        }
        setSize(textWidth, textHeight);
    }
}

Label::Label(GUIManager& manager, int x, int y, std::string_view text, int font_size)
    : GUIElement(manager, x, y, 0, 0), m_text(text), m_font_size(font_size), m_textTextureDirty(true) {
    recalculateSize();
    markDirty();
}

void Label::setText(std::string_view text) {
    if (m_text != text) {
        m_text = text;
        m_textTextureDirty = true;
        recalculateSize();
        markDirty();
    }
}

void Label::draw(SDL_Renderer* renderer) {
    if (m_text.empty()) {
        return;
    }

    const auto& resolvedStyle = getComposedStyle(m_state);
    if (!resolvedStyle.textColor.has_value()) {
        LOG_DEBUG("no text color");
        return;
    }

    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    SDL_Color currentColor = resolvedStyle.textColor.value();

    bool needsRecreate = m_textTextureDirty ||
                         m_cachedTextContent != m_text ||
                         m_cachedFontSize != font_size ||
                         (m_cachedTextColor.r != currentColor.r ||
                          m_cachedTextColor.g != currentColor.g ||
                          m_cachedTextColor.b != currentColor.b ||
                          m_cachedTextColor.a != currentColor.a);

    if (needsRecreate) {
        auto& fontManager = m_manager.getFontManager();
        auto font = fontManager.loadFont(resolvedStyle.fontName.value_or("assets/fonts/font.ttf"), font_size);

        if (!font) {
            LOG_DEBUG("no font");
            return;
        }

        auto& textureManager = m_manager.getTextureManager();
        m_cachedTextTexture = textureManager.createTextureFromText(m_text, font, currentColor);

        if (!m_cachedTextTexture) {
            LOG_DEBUG("no shared texture");
            return;
        }

        m_cachedTextContent = m_text;
        m_cachedTextColor = currentColor;
        m_cachedFontSize = font_size;
        m_textTextureDirty = false;
    }

    if (!m_cachedTextTexture) {
        return;
    }

    SDL_Rect dstRect = {0, 0, m_width, m_height};
    ({ SDL_FRect _dr = {static_cast<float>(dstRect.x), static_cast<float>(dstRect.y), static_cast<float>(dstRect.w), static_cast<float>(dstRect.h)}; SDL_RenderTexture(renderer, m_cachedTextTexture.get(), nullptr, &_dr); });
}