#include "radio_button.hpp"
#include "font_manager.hpp" // Potrzebne do renderowania tekstu
#include "texture_manager.hpp" // Potrzebne do zarządzania teksturami
#include <iostream> // Dla std::cerr

RadioButton::RadioButton(int x, int y, int w, int h, const std::string& label)
    : GUIElement(x, y, w, h), m_isSelected(false), m_labelText(label),
      m_textColor({255, 255, 255, 255}), m_font(nullptr), m_group(nullptr),
      m_labelTexture(nullptr), m_labelWidth(0), m_labelHeight(0)
{
    // Inicjalizacja specyficzna dla RadioButtona
}

RadioButton::~RadioButton() {
    // Destruktor
    // shared_ptr automatycznie zwolnią zasoby (czcionkę i teksturę)
    // Nie zwalniamy wskaźnika m_group, ponieważ grupa zarządza swoim cyklem życia
}

void RadioButton::setSelected(bool selected, bool notifyGroup) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        // Jeśli przycisk jest zaznaczany, należy do grupy i mamy ją powiadomić
        if (m_group && selected && notifyGroup) {
            m_group->buttonSelected(this);
        }
        // Wywołaj callback, jeśli istnieje
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void RadioButton::setLabel(const std::string& label) {
    m_labelText = label;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu
}

void RadioButton::setFont(SharedFont font) {
    m_font = font;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu
}

void RadioButton::setTextColor(SDL_Color color) {
    m_textColor = color;
    // Tekstura etykiety zostanie zaktualizowana przy następnym renderowaniu
}

void RadioButton::setGroup(RadioGroup* group) {
    m_group = group;
}

void RadioButton::handleEvent(SDL_Event& e) {
    // Reaguj na puszczenie lewego przycisku myszy w obrębie widgetu
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        // Sprawdź, czy kliknięcie było w obrębie widgetu
        if (e.button.button == SDL_BUTTON_LEFT &&
            e.button.x >= m_x && e.button.x < m_x + m_width &&
            e.button.y >= m_y && e.button.y < m_y + m_height) {
            // Jeśli przycisk nie jest jeszcze zaznaczony i należy do grupy, powiadom grupę
            if (!m_isSelected && m_group) {
                m_group->buttonSelected(this);
            } else if (!m_isSelected && !m_group) {
                // Jeśli nie należy do grupy, po prostu zaznacz go
                setSelected(true);
            }
            // Jeśli jest już zaznaczony, kliknięcie nic nie robi (Radio Buttony nie odznaczają się same)
        }
    }
}

void RadioButton::render(SDL_Renderer* renderer)  {
    // Renderuj tło (opcjonalnie)
    // SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Ciemnoszary
    // SDL_Rect backgroundRect = {m_x, m_y, m_width, m_height};
    // SDL_RenderFillRect(renderer, &backgroundRect);

    // Renderuj okrąg RadioButtona
    SDL_SetRenderDrawColor(renderer, m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    // Proste rysowanie okręgu (wymaga funkcji rysującej okręgi, której SDL2 natywnie nie ma)
    // Na potrzeby przykładu, narysujemy kwadrat jako placeholder
    SDL_Rect radioRect = {m_x, m_y, m_height, m_height}; // Przyjmujemy, że RadioButton jest kwadratowy o boku równym wysokości widgetu
    SDL_RenderDrawRect(renderer, &radioRect);


    // Renderuj kropkę zaznaczenia, jeśli RadioButton jest zaznaczony
    if (m_isSelected) {
        // Prosta kropka (mniejszy kwadrat w środku)
        SDL_Rect dotRect = {m_x + m_height / 4, m_y + m_height / 4, m_height / 2, m_height / 2};
        SDL_RenderFillRect(renderer, &dotRect);
    }

    // Renderuj etykietę tekstową
    if (!m_labelText.empty() && m_font && renderer) {
        // Sprawdź, czy tekstura etykiety wymaga aktualizacji lub nie istnieje
        if (!m_labelTexture || m_renderedText != m_labelText) {
             m_labelTexture = createTextTexture(renderer, m_font, m_labelText, m_textColor);
             if (m_labelTexture) {
                 SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &m_labelWidth, &m_labelHeight);
                 m_renderedText = m_labelText;
             } else {
                 m_labelWidth = 0;
                 m_labelHeight = 0;
                 m_renderedText = "";
             }
        }

        // Renderuj teksturę etykiety (jeśli istnieje)
        if (m_labelTexture) {
            SDL_Rect renderQuad = {m_x + m_height + 5, m_y + (m_height - m_labelHeight) / 2, m_labelWidth, m_labelHeight};
            SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
        }
    } else {
        // Jeśli tekst jest pusty, czcionka jest nullptr lub renderer jest nullptr, zwolnij teksturę
        m_labelTexture = nullptr;
        m_labelWidth = 0;
        m_labelHeight = 0;
        m_renderedText = "";
    }
}