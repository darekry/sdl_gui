#include "checkbox.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "gui_manager.hpp"
import std.compat;

Checkbox::Checkbox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_isChecked(false), m_labelTexture(nullptr), m_onChange(nullptr)
{
}

void Checkbox::setChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (m_onChange) {
            m_onChange(this, m_isChecked);
        }
    }
}

void Checkbox::setLabel(const std::string& text, int fontSize, SDL_Color color) {
    FontManager& fontManager = m_manager.getFontManager();
    SharedFont font = fontManager.loadFont("assets/fonts/font.ttf", fontSize);
    if(font) {
        TextureManager& textureManager = m_manager.getTextureManager();
        m_labelTexture = textureManager.createTextureFromText(text, font, color);
    }
}


bool Checkbox::handleEvent(SDL_Event& e) {
    if (!m_enabled) return false;

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (contains(e.button.x, e.button.y)) {
            setChecked(!m_isChecked);
            return true; // Zdarzenie obsłużone
        }
    }
    return false;
}

void Checkbox::draw() {
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect checkboxRect = {absPos.x, absPos.y, m_height, m_height};

    if (m_isHovered) {
       SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }
    else {
       SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    SDL_RenderFillRect(renderer, &checkboxRect);

    // Domyślny kolor ramki
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &checkboxRect);

    if (m_isChecked) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawLine(renderer, absPos.x + 3, absPos.y + m_height / 2, absPos.x + m_height / 2, absPos.y + m_height - 3);
        SDL_RenderDrawLine(renderer, absPos.x + m_height / 2, absPos.y + m_height - 3, absPos.x + m_height - 3, absPos.y + 3);
    }

    if (m_labelTexture) {
        int labelWidth, labelHeight;
        SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &labelWidth, &labelHeight);
        SDL_Rect renderQuad = {absPos.x + m_height + 5, absPos.y + (m_height - labelHeight) / 2, labelWidth, labelHeight};
        SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
    }
}