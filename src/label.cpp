#include "label.hpp"
#include "gui_manager.hpp"
import std.compat;

Label::Label(GUIManager& manager, int x, int y, std::string_view text, int font_size, const SDL_Color& color)
    : GUIElement(manager, x, y, 0, 0), m_text(text), m_font_size(font_size), m_color(color) {
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont("assets/fonts/font.ttf", m_font_size);
    if (font) {
        auto& textureManager = m_manager.getTextureManager();
        m_texture = textureManager.createTextureFromText(m_text, font, m_color);
        int width, height;
        SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &width, &height);
        setSize(width, height);
    }
}

void Label::setText(std::string_view text) {
    m_text = text;
    auto& fontManager = m_manager.getFontManager();
    auto font = fontManager.loadFont("assets/fonts/font.ttf", m_font_size);
    if (font) {
        auto& textureManager = m_manager.getTextureManager();
        m_texture = textureManager.createTextureFromText(m_text, font, m_color);
        int width, height;
        SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &width, &height);
        setSize(width, height);
    }
}

void Label::draw() {
    // Label używa domyślnej implementacji draw() z GUIElement, która renderuje m_texture.
    GUIElement::draw();
}