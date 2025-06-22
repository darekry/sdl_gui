#include "checkbox.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

Checkbox::Checkbox(int x, int y, int w, int h, const std::string& label)
    : GUIElement(x, y, w, h), m_isChecked(false), m_labelText(label),
      m_textColor({255, 255, 255, 255}), m_font(nullptr), m_onChange(nullptr),
      m_labelTexture(nullptr), m_labelWidth(0), m_labelHeight(0)
{
}

Checkbox::~Checkbox() {
}

void Checkbox::setChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        if (m_onChange) {
            m_onChange(this, m_isChecked);
        }
    }
}

void Checkbox::setLabel(const std::string& label) {
    m_labelText = label;
    m_renderedText = ""; // Wymuś aktualizację tekstury
}

void Checkbox::setFont(SharedFont font) {
    m_font = font;
    m_renderedText = "";
}

void Checkbox::setTextColor(SDL_Color color) {
    m_textColor = color;
    m_renderedText = "";
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

    SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    SDL_RenderDrawRect(renderer, &checkboxRect);

    if (m_isChecked) {
        SDL_RenderDrawLine(renderer, absPos.x + 2, absPos.y + 2, absPos.x + m_height - 2, absPos.y + m_height - 2);
        SDL_RenderDrawLine(renderer, absPos.x + m_height - 2, absPos.y + 2, absPos.x + 2, absPos.y + m_height - 2);
    }

    if (!m_labelText.empty() && m_font && renderer) {
        if (!m_labelTexture || m_renderedText != m_labelText) {
             m_labelTexture = createTextTexture(renderer, m_font, m_labelText, m_textColor);
             if (m_labelTexture) {
                 SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &m_labelWidth, &m_labelHeight);
                 m_renderedText = m_labelText;
             }
        }

        if (m_labelTexture) {
            SDL_Rect renderQuad = {absPos.x + m_height + 5, absPos.y + (m_height - m_labelHeight) / 2, m_labelWidth, m_labelHeight};
            SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
        }
    }
}