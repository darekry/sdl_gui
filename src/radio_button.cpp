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

void RadioButton::setLabel(std::string_view text, int fontSize, const SDL_Color& color) {
    auto& fontManager = m_manager.getFontManager();
    SharedFont font = fontManager.loadFont("assets/fonts/font.ttf", fontSize);
    if(font) {
        m_labelTexture = m_manager.getTextureManager().createTextureFromText(text, font, color);
    }
}

void RadioButton::setGroup(RadioGroup* group) {
    m_group = group;
}

bool RadioButton::handleEvent(const SDL_Event& e) {
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

void RadioButton::draw() {
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect radioRect = {absPos.x, absPos.y, m_height, m_height};

    if (m_isHovered) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    }
    SDL_RenderFillRect(renderer, &radioRect);
    
    // Domyślny kolor ramki
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &radioRect);

    if (m_isSelected) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect dotRect = {absPos.x + 4, absPos.y + 4, m_height - 8, m_height - 8};
        SDL_RenderFillRect(renderer, &dotRect);
    }

    if (m_labelTexture) {
        int labelWidth, labelHeight;
        SDL_QueryTexture(m_labelTexture.get(), nullptr, nullptr, &labelWidth, &labelHeight);
        SDL_Rect renderQuad = {absPos.x + m_height + 5, absPos.y + (m_height - labelHeight) / 2, labelWidth, labelHeight};
        SDL_RenderCopy(renderer, m_labelTexture.get(), nullptr, &renderQuad);
    }
}