#include "text_input.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
import std.compat;

TextInput::TextInput(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_text(""), m_textColor({0, 0, 0, 255}),
      m_backgroundColor({255, 255, 255, 255}), m_borderColor({0, 0, 0, 255}),
      m_locked(false), m_active(false) {
    m_cursor_pos = 0;
    m_text_offset_x = 0;
    m_show_cursor = false;
    m_cursor_blink_time = 0;
}

void TextInput::setText(std::string_view newText) {
    if (m_text != newText) {
        m_text = newText;
        m_cursor_pos = std::min(m_cursor_pos, m_text.length());
        updateTextTexture();
        update_text_offset();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

void TextInput::setText(std::string&& newText) {
    if (m_text != newText) {
        m_text = std::move(newText);
        m_cursor_pos = std::min(m_cursor_pos, m_text.length());
        updateTextTexture();
        update_text_offset();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

const std::string& TextInput::getText() const {
    return m_text;
}

void TextInput::setTextColor(const SDL_Color& color) {
    m_textColor = color;
    updateTextTexture();
}

void TextInput::setBackgroundColor(const SDL_Color& color) {
    m_backgroundColor = color;
}

void TextInput::setBorderColor(const SDL_Color& color) {
    m_borderColor = color;
}

void TextInput::setOnTextChanged(const std::function<void(TextInput*)>& callback) {
    m_onTextChanged = callback;
}

void TextInput::setOnEnterPressed(const std::function<void(TextInput*)>& callback) {
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
void TextInput::updateTextTexture() {
    if (m_text.empty()) {
        m_texture = nullptr;
        return;
    }
    SharedFont font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) {
        m_texture = nullptr;
        return;
    }
    m_texture = m_manager.getTextureManager().createTextureFromText(m_text, font, m_textColor);
}
void TextInput::update_text_offset() {
    SharedFont font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;

    int text_width = 0;
    TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &text_width, nullptr);

    int cursor_pos_x = text_width;
    int padding = 5;
    int visible_width = getWidth() - 2 * padding;

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
     if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }
    
    int total_text_width=0;
    TTF_SizeText(font.get(), m_text.c_str(), &total_text_width, nullptr);

    if (total_text_width < visible_width) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x > 0) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x < visible_width - total_text_width ) {
        m_text_offset_x = visible_width - total_text_width;
    }
}
void TextInput::render_cursor() {
    SharedFont font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;

    int cursor_x_pos = 0;
    if (m_cursor_pos > 0) {
        TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &cursor_x_pos, nullptr);
    }
    
    SDL_Rect cursor_rect = {
        getAbsolutePosition().x + 5 + cursor_x_pos + m_text_offset_x,
        getAbsolutePosition().y + (getHeight() - TTF_FontHeight(font.get())) / 2,
        2,
        TTF_FontHeight(font.get())
    };

    SDL_SetRenderDrawColor(m_manager.getRenderer(), m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    SDL_RenderFillRect(m_manager.getRenderer(), &cursor_rect);
}
void TextInput::draw() {
    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Rect rect = {getAbsolutePosition().x, getAbsolutePosition().y, getWidth(), getHeight()};

    SDL_SetRenderDrawColor(renderer, m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, m_borderColor.r, m_borderColor.g, m_borderColor.b, m_borderColor.a);
    SDL_RenderDrawRect(renderer, &rect);

    
    SDL_Rect clip_rect = { getAbsolutePosition().x + 5, getAbsolutePosition().y, getWidth() - 10, getHeight() };
    SDL_RenderSetClipRect(renderer, &clip_rect);
    
    if (m_texture) {
        int textWidth, textHeight;
        SDL_QueryTexture(m_texture.get(), nullptr, nullptr, &textWidth, &textHeight);
        
        SDL_Rect renderQuad = { getAbsolutePosition().x + 5 + m_text_offset_x, getAbsolutePosition().y + (getHeight() - textHeight) / 2, textWidth, textHeight };

        SDL_RenderCopy(renderer, m_texture.get(), nullptr, &renderQuad);
    }
        if (m_active && m_show_cursor) {
        render_cursor();
    }
    SDL_RenderSetClipRect(renderer, nullptr);

}
bool TextInput::handleEvent(const SDL_Event& e) {
if (m_locked || !m_enabled) {
    return false;
}

bool eventHandled = false;
if (e.type == SDL_MOUSEBUTTONDOWN) {
    if (contains(e.button.x, e.button.y)) {
         if (!m_active) {
            m_active = true;
            SDL_StartTextInput();
            m_show_cursor = true;
            m_cursor_blink_time = SDL_GetTicks();
            eventHandled = true;
        }
    } else {
         if (m_active) {
            m_active = false;
            SDL_StopTextInput();
            m_show_cursor = false;
        }
    }
} else if (m_active && e.type == SDL_TEXTINPUT) {
    m_text.insert(m_cursor_pos, e.text.text);
    m_cursor_pos += strlen(e.text.text);
    updateTextTexture();
    update_text_offset();
    if (m_onTextChanged) m_onTextChanged(this);
    eventHandled = true;
} else if (m_active && e.type == SDL_KEYDOWN) {
    if (e.key.keysym.sym == SDLK_BACKSPACE && m_cursor_pos > 0) {
        m_text.erase(m_cursor_pos - 1, 1);
        m_cursor_pos--;
        updateTextTexture();
        update_text_offset();
        if (m_onTextChanged) m_onTextChanged(this);
        eventHandled = true;
    } else if (e.key.keysym.sym == SDLK_RETURN) {
        if (m_onEnterPressed) m_onEnterPressed(this);
        m_active = false;
        SDL_StopTextInput();
        eventHandled = true;
    }else if (e.key.keysym.sym == SDLK_LEFT && m_cursor_pos > 0) {
        m_cursor_pos--;
        update_text_offset();
        eventHandled = true;
    } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursor_pos < m_text.length()) {
        m_cursor_pos++;
        update_text_offset();
        eventHandled = true;
    }

    if (eventHandled) {
         m_show_cursor = true;
         m_cursor_blink_time = SDL_GetTicks();
    }
}

if (m_active) {
    if (SDL_GetTicks() - m_cursor_blink_time > 500) {
        m_show_cursor = !m_show_cursor;
        m_cursor_blink_time = SDL_GetTicks();
    }
}


return eventHandled;
}