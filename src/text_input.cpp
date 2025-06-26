#include "text_input.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"

TextInput::TextInput(int x, int y, int w, int h)
    : GUIElement(x, y, w, h), text(""), textColor({0, 0, 0, 255}), // Default text color black
      backgroundColor({255, 255, 255, 255}), // Default background color white
      borderColor({0, 0, 0, 255}), // Default border color black
      locked(false), active(false), m_textTexture(nullptr) {
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
    if (!getGUIManager()) return;
    SDL_Renderer* actual_renderer = getGUIManager()->getRenderer();
    FontManager* fontManager = getGUIManager()->getFontManager();
    if (!actual_renderer || !fontManager) return;

    SDL_Rect rect = {getAbsolutePosition().x, getAbsolutePosition().y, getWidth(), getHeight()};

    // Render background
    SDL_SetRenderDrawColor(actual_renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(actual_renderer, &rect);

    // Render border
    SDL_SetRenderDrawColor(actual_renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderDrawRect(actual_renderer, &rect);

    // Render text
    SharedFont font = fontManager->loadFont("assets/fonts/font.ttf", 16); // Load default font
    if (font && !text.empty()) {
        if (text != m_renderedText) {
            // Text has changed, re-render texture
            m_textTexture.reset(); // Release the old texture

            SDL_Surface* textSurface = TTF_RenderText_Solid(font.get(), text.c_str(), textColor);
            if (textSurface == nullptr) {
                SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            } else {
                m_textTexture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(actual_renderer, textSurface), SDLTextureDeleter());
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

            SDL_Rect renderQuad = { getAbsolutePosition().x + 5, getAbsolutePosition().y + 5, textWidth, textHeight }; // Add some padding
            SDL_RenderCopy(actual_renderer, m_textTexture.get(), nullptr, &renderQuad);
        }
    } else {
        // If text is empty or font is null, destroy the texture
        m_textTexture.reset();
        m_renderedText = "";
    }

    // Render children (if any)
    for (auto& child : m_children) {
        child->render(actual_renderer);
    }
}

bool TextInput::handleEvent(SDL_Event& e) {
    if (locked || !m_enabled) {
        return false;
    }

    bool eventHandled = false;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            if (!active) {
                active = true;
                SDL_StartTextInput();
                eventHandled = true;
            }
        } else {
            if (active) {
                active = false;
                SDL_StopTextInput();
            }
        }
    } else if (active && e.type == SDL_TEXTINPUT) {
        text += e.text.text;
        m_renderedText = "";
        if (onTextChanged) onTextChanged(this);
        eventHandled = true;
    } else if (active && e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && !text.empty()) {
            text.pop_back();
            m_renderedText = "";
            if (onTextChanged) onTextChanged(this);
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            if (onEnterPressed) onEnterPressed(this);
            active = false;
            SDL_StopTextInput();
            eventHandled = true;
        }
    }

    // Zdarzenie jest "konsumowane" tylko jeśli pole tekstowe jest aktywne
    // lub zostało właśnie aktywowane. W przeciwnym razie pozwalamy na propagację.
    return eventHandled;
}