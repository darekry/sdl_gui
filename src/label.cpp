#include "label.hpp"
#include "gui_manager.hpp"


Label::Label(GUIManager& manager, int x, int y, std::string_view text, int font_size)
    : GUIElement(manager, x, y, 0, 0), m_text(text), m_font_size(font_size) {
    updateTexture();
}

void Label::setText(std::string_view text) {
    m_text = text;
    updateTexture();
}

void Label::updateTexture() {
    const auto resolvedStyle = getResolvedStyle();
    if (!resolvedStyle.textColor) {
        m_texture.reset();
        setSize(0, 0);
        return;
    }

    int font_size = m_font_size > 0 ? m_font_size : (resolvedStyle.fontSize.value_or(m_manager.getTheme().getDefaultStyle().fontSize.value_or(16)));
    
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont(resolvedStyle.fontName.value_or("assets/fonts/font.ttf"), font_size);
    if (font) {
        auto& textureManager = m_manager.getTextureManager();
        m_texture = textureManager.createTextureFromText(m_text, font, *resolvedStyle.textColor);
        if (m_texture) {
            int width, height;
            SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &width, &height);
            setSize(width, height);
        } else {
            setSize(0,0);
        }
    } else {
        setSize(0,0);
    }
}


void Label::draw() {
    if (m_style_dirty) {
        updateTexture();
        m_style_dirty = false;
    }
    // Rysuj tło/ramkę z klasy bazowej
    GUIElement::draw();

    // A teraz narysuj teksturę tekstu na wierzchu
    if (m_texture && isVisible()) {
        auto renderer = m_manager.getRenderer();
        auto absPos = getAbsolutePosition();
        SDL_Rect dstRect = { absPos.x, absPos.y, m_width, m_height };
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &dstRect);
    }
}