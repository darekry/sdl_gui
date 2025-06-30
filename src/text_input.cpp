#include "text_input.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

TextInput::TextInput(int x, int y, int w, int h)
    : GUIElement(x, y, w, h), m_text(""), m_textColor({0, 0, 0, 255}),
      m_backgroundColor({255, 255, 255, 255}), m_borderColor({0, 0, 0, 255}),
      m_locked(false), m_active(false) {
}

void TextInput::setText(const std::string& newText) {
    if (m_text != newText) {
        m_text = newText;
        if (m_guiManager) {
            updateTextTexture(*m_guiManager->getTextureManager());
        }
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

const std::string& TextInput::getText() const {
    return m_text;
}

void TextInput::setTextColor(SDL_Color color) {
    m_textColor = color;
    if (m_guiManager) {
        updateTextTexture(*m_guiManager->getTextureManager());
    }
}

void TextInput::setBackgroundColor(SDL_Color color) {
    m_backgroundColor = color;
}

void TextInput::setBorderColor(SDL_Color color) {
    m_borderColor = color;
}

void TextInput::setOnTextChanged(std::function<void(TextInput*)> callback) {
    m_onTextChanged = callback;
}

void TextInput::setOnEnterPressed(std::function<void(TextInput*)> callback) {
    m_onEnterPressed = callback;
}

void TextInput::setLocked(bool isLocked) {
    m_locked = isLocked;
    if (m_locked) {
        m_active = false;
    }
}

bool TextInput::isLocked() const {
    return m_locked;
}

void TextInput::updateTextTexture(TextureManager& textureManager) {
    if (m_text.empty() || !m_guiManager || !m_guiManager->getFontManager()) {
        m_texture = nullptr;
        return;
    }
    SharedFont font = m_guiManager->getFontManager()->loadFont("assets/fonts/font.ttf", 16);
    if (!font) {
        m_texture = nullptr;
        return;
    }
    m_texture = textureManager.createTextureFromText(m_text, font, m_textColor);
}

void TextInput::render(SDL_Renderer* renderer) {
    SDL_Rect rect = {getAbsolutePosition().x, getAbsolutePosition().y, getWidth(), getHeight()};

    SDL_SetRenderDrawColor(renderer, m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, m_borderColor.r, m_borderColor.g, m_borderColor.b, m_borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    if (m_texture) {
        int textWidth, textHeight;
        SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &textWidth, &textHeight);
        SDL_Rect renderQuad = { getAbsolutePosition().x + 5, getAbsolutePosition().y + 5, textWidth, textHeight };
        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad);
    }
    
   // GUIElement::render(renderer);
}

bool TextInput::handleEvent(SDL_Event& e) {
    if (m_locked || !m_enabled || !m_guiManager) {
        return false;
    }

    bool eventHandled = false;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            if (!m_active) {
                m_active = true;
                SDL_StartTextInput();
                eventHandled = true;
            }
        } else {
            if (m_active) {
                m_active = false;
                SDL_StopTextInput();
            }
        }
    } else if (m_active && e.type == SDL_TEXTINPUT) {
        m_text += e.text.text;
        updateTextTexture(*m_guiManager->getTextureManager());
        if (m_onTextChanged) m_onTextChanged(this);
        eventHandled = true;
    } else if (m_active && e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && !m_text.empty()) {
            m_text.pop_back();
            updateTextTexture(*m_guiManager->getTextureManager());
            if (m_onTextChanged) m_onTextChanged(this);
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            if (m_onEnterPressed) m_onEnterPressed(this);
            m_active = false;
            SDL_StopTextInput();
            eventHandled = true;
        }
    }

    return eventHandled;
}