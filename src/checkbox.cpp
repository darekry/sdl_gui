#include "checkbox.hpp"
#include "font_manager.hpp" // Potrzebne do renderowania tekstu
#include "texture_manager.hpp" // Potrzebne do zarządzania teksturami

Checkbox::Checkbox(int x, int y, int w, int h, const std::string& label)
    : GUIElement(x, y, w, h), m_isChecked(false), m_labelText(label),
      m_textColor({255, 255, 255, 255}), m_font(nullptr), m_labelTexture(nullptr),
      m_labelWidth(0), m_labelHeight(0)
{
    // Inicjalizacja specyficzna dla Checkboxa
}

Checkbox::~Checkbox() {
    // Destruktor
    // shared_ptr automatycznie zwolnią zasoby (czcionkę i teksturę)
}

void Checkbox::setChecked(bool checked) {
    if (m_isChecked != checked) {
        m_isChecked = checked;
        // Wywołaj callback, jeśli istnieje
        if (m_onChange) {
            m_onChange(this, m_isChecked);
        }
    }
}

void Checkbox::setLabel(const std::string& label) {
    m_labelText = label;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu lub można wymusić aktualizację tutaj
    // updateLabelTexture(renderer); // Potrzebny renderer, co może być problematyczne
}

void Checkbox::setFont(SharedFont font) {
    m_font = font;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu
}

void Checkbox::setTextColor(SDL_Color color) {
    m_textColor = color;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu
}

void Checkbox::handleEvent(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        // Sprawdź, czy kliknięcie było w obrębie widgetu
        if (e.button.button == SDL_BUTTON_LEFT &&
            e.button.x >= m_x && e.button.x < m_x + m_width &&
            e.button.y >= m_y && e.button.y < m_y + m_height) {

            // Przełącz stan
            setChecked(!m_isChecked);
        }
    }
}

void Checkbox::render(SDL_Renderer* renderer) {
    // Renderuj tło (opcjonalnie)
    // SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Ciemnoszary
    // SDL_Rect backgroundRect = {m_x, m_y, m_w, m_h};
    // SDL_RenderFillRect(renderer, &backgroundRect);

    // Renderuj ramkę Checkboxa
    SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    SDL_Rect checkboxRect = {m_x, m_y, m_height, m_height}; // Przyjmujemy, że Checkbox jest kwadratowy o boku równym wysokości widgetu
    SDL_RenderDrawRect(renderer, &checkboxRect);

    // Renderuj zaznaczenie, jeśli Checkbox jest zaznaczony
    if (m_isChecked) {
        // Proste zaznaczenie w kształcie X
        SDL_RenderDrawLine(renderer, m_x, m_y, m_x + m_height, m_y + m_height);
        SDL_RenderDrawLine(renderer, m_x + m_height, m_y, m_x, m_y + m_height);
    }

    // Renderuj etykietę tekstową
    if (!m_labelText.empty() && m_font) {
        // Sprawdź, czy tekstura etykiety wymaga aktualizacji
        if (!m_labelTexture || m_renderedText != m_labelText) {
             updateLabelTexture(renderer);
        }

        // Renderuj teksturę etykiety (jeśli istnieje)
        if (m_labelTexture) {
            SDL_Rect renderQuad = {m_x + m_height + 5, m_y + (m_height - m_labelHeight) / 2, m_labelWidth, m_labelHeight};
            SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
        }
    }
}

// Metoda pomocnicza do aktualizacji tekstury etykiety (wymaga dostępu do renderer i FontManager)
void Checkbox::updateLabelTexture(SDL_Renderer* renderer) {
    if (!m_labelText.empty() && m_font && renderer) {
        SDL_Surface* textSurface = TTF_RenderText_Solid(m_font.get(), m_labelText.c_str(), m_textColor);
        if (textSurface) {
            m_labelTexture.reset(SDL_CreateTextureFromSurface(renderer, textSurface), SDLTextureDeleter());
            m_labelWidth = textSurface->w;
            m_labelHeight = textSurface->h;
            SDL_FreeSurface(textSurface);
            m_renderedText = m_labelText;
        } else {
            std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        }
    } else {
        m_labelTexture = nullptr;
        m_labelWidth = 0;
        m_labelHeight = 0;
        m_renderedText = "";
    }
}