#include "text_input.hpp"
#include <iostream> // For basic debugging output

TextInput::TextInput(int x, int y, int w, int h)
    : GUIElement(x, y, w, h), text(""), textColor({0, 0, 0, 255}), // Default text color black
      backgroundColor({255, 255, 255, 255}), // Default background color white
      borderColor({0, 0, 0, 255}), // Default border color black
      font(nullptr), locked(false), active(false), m_textTexture(nullptr) {
    // Constructor implementation
}

TextInput::~TextInput() {
    // Memory management for font and texture is now handled by shared_ptr
}

void TextInput::setText(const std::string& newText) {
    if (text != newText) {
        text = newText;
        m_renderedText = ""; // Clear rendered text to force re-render
        if (onTextChanged) {
            onTextChanged(this);
        }
    }
}

const std::string& TextInput::getText() const {
    return text;
}

void TextInput::setTextColor(SDL_Color color) {
    textColor = color;
    // Text color change requires re-rendering the texture
    m_renderedText = "";
}

void TextInput::setBackgroundColor(SDL_Color color) {
    backgroundColor = color;
}

void TextInput::setBorderColor(SDL_Color color) {
    borderColor = color;
}

void TextInput::setFont(std::shared_ptr<TTF_Font> newFont) {
    font = newFont;
    // Font change requires re-rendering the texture
    m_renderedText = "";
}

void TextInput::setOnTextChanged(std::function<void(TextInput*)> callback) {
    onTextChanged = callback;
}

void TextInput::setOnEnterPressed(std::function<void(TextInput*)> callback) {
    onEnterPressed = callback;
}

void TextInput::setLocked(bool isLocked) {
    locked = isLocked;
    if (locked) {
        active = false; // Deactivate if locked
    }
}

bool TextInput::isLocked() const {
    return locked;
}

void TextInput::render(SDL_Renderer* renderer) {
    SDL_Rect rect = {getAbsolutePosition().x, getAbsolutePosition().y, getWidth(), getHeight()};

    // Render background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Render border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    // Render text
    // Render text
    if (font && !text.empty()) {
        if (text != m_renderedText) {
            // Text has changed, re-render texture
            m_textTexture.reset(); // Release the old texture

            SDL_Surface* textSurface = TTF_RenderText_Solid(font.get(), text.c_str(), textColor);
            if (textSurface == nullptr) {
                SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            } else {
                m_textTexture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer, textSurface), SDLTextureDeleter());
                if (m_textTexture == nullptr) {
                    SDL_Log("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
                }
                SDL_FreeSurface(textSurface);
            }
            m_renderedText = text;
        }

        // Render the text texture
        if (m_textTexture) {
            int textWidth = 0;
            int textHeight = 0;
            SDL_QueryTexture(m_textTexture.get(), nullptr, nullptr, &textWidth, &textHeight);

            SDL_Rect renderQuad = { getAbsolutePosition().x, getAbsolutePosition().y, textWidth, textHeight };
            SDL_RenderCopy(renderer, m_textTexture.get(), nullptr, &renderQuad);
        }
    } else {
        // If text is empty or font is null, destroy the texture
        m_textTexture.reset();
        m_renderedText = "";
    }

    // Render children (if any)
    for (GUIElement* child : m_children) {
        child->render(renderer);
    }
}

void TextInput::handleEvent(SDL_Event& e) {
    if (locked) {
        return; // Ignore events if locked
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        if (contains(mouseX, mouseY)) {
            active = true; // Activate on click
        } else {
            active = false; // Deactivate if clicked outside
        }
    } else if (active && e.type == SDL_TEXTINPUT) {
        // Append new text
        text += e.text.text;
        m_renderedText = ""; // Mark for re-render
        if (onTextChanged) {
            onTextChanged(this);
        }
    } else if (active && e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && !text.empty()) {
            // Handle backspace
            text.pop_back();
            m_renderedText = ""; // Mark for re-render
            if (onTextChanged) {
                onTextChanged(this);
            }
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            // Handle Enter key
            if (onEnterPressed) {
                onEnterPressed(this);
            }
        }
    }

    // Pass event to children (if any) - though text input typically doesn't have children handling events
    for (GUIElement* child : m_children) {
        child->handleEvent(e);
    }
}