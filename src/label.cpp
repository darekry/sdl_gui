#include "label.hpp"
#include "gui_manager.hpp"
#include "gui.hpp"
#include "constants.hpp"


void Label::recalculateSize() {
    if (m_text.empty()) {
        setSize(0, 0);
        return;
    }
    const auto& resolvedStyle = getComposedStyle(m_state);
    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or(constants::kDefaultFontPath), font_size);
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
        LOG_DEBUG("no text color");
        return;
    }

    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));

    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or(constants::kDefaultFontPath), font_size);
    if (!font) {
        LOG_DEBUG("no font");
        return;
    }

    auto textTexture = m_manager.getTextureManager().createTextureFromText(m_text, font, resolvedStyle.textColor.value());
    if (!textTexture) {
        LOG_DEBUG("no shared texture");
        return;
    }

    SDL_Rect dstRect = {0, 0, m_width, m_height};
    RenderTexture(renderer, textTexture.get(), dstRect);
}