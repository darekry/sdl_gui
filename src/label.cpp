#include "label.hpp"
#include "SDL_log.h"
#include "gui_manager.hpp"


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
        int textWidth, textHeight;
        TTF_SizeText(font.get(), m_text.c_str(), &textWidth, &textHeight);
        setSize(textWidth, textHeight);
    }
}

Label::Label(GUIManager& manager, int x, int y, std::string_view text, int font_size)
    : GUIElement(manager, x, y, 0, 0), m_text(text), m_font_size(font_size) {
    recalculateSize();
    markDirty();
}

void Label::setText(std::string_view text) {
    if (m_text != text) {
        m_text = text;
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
        SDL_Log("no text color");

        return;
    }

    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or("assets/fonts/font.ttf"), font_size);
    
    if (!font) {
        SDL_Log("no font");
        return;
    }

    auto& textureManager = m_manager.getTextureManager();
    SharedTexture textTexture = textureManager.createTextureFromText(m_text, font, resolvedStyle.textColor.value());

    if (!textTexture) {
        SDL_Log("no shared texture");

        return;
    }

    SDL_Rect dstRect = {0, 0, m_width, m_height};
    SDL_RenderCopy(renderer, textTexture.get(), nullptr, &dstRect);
}