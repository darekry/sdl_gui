#include "radio_button.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "gui_manager.hpp"
import std.compat;

RadioButton::RadioButton(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_isSelected(false), m_group(nullptr), m_onChange(nullptr), m_labelTexture(nullptr)
{
}

void RadioButton::setSelected(bool selected, bool notifyGroup) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        if (m_group && selected && notifyGroup) {
            m_group->buttonSelected(this);
        }
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void RadioButton::setLabel(const std::string& text, int fontSize, SDL_Color color) {
    FontManager& fontManager = m_manager.getFontManager();
    SharedFont font = fontManager.loadFont("assets/fonts/font.ttf", fontSize);
    if(font) {
        m_labelTexture = m_manager.getTextureManager().createTextureFromText(text, font, color);
    }
}

void RadioButton::setGroup(RadioGroup* group) {
    m_group = group;
}

bool RadioButton::handleEvent(SDL_Event& e) {
    if (!m_enabled) return false;

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (contains(e.button.x, e.button.y)) {
            if (!m_isSelected) {
                setSelected(true);
            }
            return true;
        }
    }
    return false;
}

void RadioButton::render() {
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect radioRect = {absPos.x, absPos.y, m_height, m_height};

    // Domyślny kolor ramki
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &radioRect);

    if (m_isSelected) {
        SDL_Rect dotRect = {absPos.x + m_height / 4, absPos.y + m_height / 4, m_height / 2, m_height / 2};
        SDL_RenderFillRect(renderer, &dotRect);
    }

    if (m_labelTexture) {
        int labelWidth, labelHeight;
        SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &labelWidth, &labelHeight);
        SDL_Rect renderQuad = {absPos.x + m_height + 5, absPos.y + (m_height - labelHeight) / 2, labelWidth, labelHeight};
        SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
    }
}