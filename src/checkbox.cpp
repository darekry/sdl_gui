#include "checkbox.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

Checkbox::Checkbox(int x, int y, int w, int h)
    : GUIElement(x, y, w, h), m_isChecked(false), m_labelTexture(nullptr), m_onChange(nullptr)
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

void Checkbox::setLabel(SDL_Renderer* renderer, const std::string& text, std::shared_ptr<TTF_Font> font, SDL_Color color, TextureManager& textureManager) {
    (void)renderer; // Unused
    m_labelTexture = textureManager.createTextureFromText(text, font, color);
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

void Checkbox::render(SDL_Renderer* renderer) {
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect checkboxRect = {absPos.x, absPos.y, m_height, m_height};

    // Domyślny kolor ramki, można by dodać setBorderColor
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &checkboxRect);

    if (m_isChecked) {
        SDL_RenderDrawLine(renderer, absPos.x + 2, absPos.y + 2, absPos.x + m_height - 2, absPos.y + m_height - 2);
        SDL_RenderDrawLine(renderer, absPos.x + m_height - 2, absPos.y + 2, absPos.x + 2, absPos.y + m_height - 2);
    }

    if (m_labelTexture) {
        int labelWidth, labelHeight;
        SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &labelWidth, &labelHeight);
        SDL_Rect renderQuad = {absPos.x + m_height + 5, absPos.y + (m_height - labelHeight) / 2, labelWidth, labelHeight};
        SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
    }
}