#include "text_input.hpp"
#include "style.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "theme.hpp"
#include "theme.hpp"


TextInput::TextInput(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_text(""),
      m_locked(false), m_active(false) {
    m_cursor_pos = 0;
    m_text_offset_x = 0;
    m_show_cursor = false;
    m_cursor_blink_time = 0;
    markDirty();
}

void TextInput::setText(std::string_view newText) {
    if (m_text != newText) {
        m_text = newText;
        m_cursor_pos = std::min(m_cursor_pos, m_text.length());
        update_text_offset();
        markDirty();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

void TextInput::setText(std::string&& newText) {
    if (m_text != newText) {
        m_text = std::move(newText);
        m_cursor_pos = std::min(m_cursor_pos, m_text.length());
        update_text_offset();
        markDirty();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

const std::string& TextInput::getText() const {
    return m_text;
}

void TextInput::setTextColor(const SDL_Color& color) {
    GUIElement::setTextColor(ElementState::Normal, color);
    GUIElement::setTextColor(ElementState::Hover, color);
    GUIElement::setTextColor(ElementState::Pressed, color);
}

void TextInput::setBackgroundColor(const SDL_Color& color) {
    GUIElement::setBackgroundColor(ElementState::Normal, color);
    GUIElement::setBackgroundColor(ElementState::Hover, color);
    GUIElement::setBackgroundColor(ElementState::Pressed, color);
}

void TextInput::setBorderColor(const SDL_Color& color) {
    setBorder(ElementState::Normal, color, 1);
    setBorder(ElementState::Hover, color, 1);
    setBorder(ElementState::Pressed, color, 1);
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
    markDirty();
}
bool TextInput::isLocked() const {
    return m_locked;
}

void TextInput::update_text_offset() {
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;

    int text_width = 0;
    TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &text_width, nullptr);

    auto cursor_pos_x = text_width;
    auto padding = 5;
    auto visible_width = getWidth() - 2 * padding;

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


void TextInput::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    const auto& style = getComposedStyle(m_state);

    // Text
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!m_text.empty() && font && style.textColor) {
        auto text_texture = m_manager.getTextureManager().createTextureFromText(m_text, font, style.textColor.value());
        if (text_texture) {
            int textWidth = 0, textHeight = 0;
            SDL_QueryTexture(text_texture.get(), nullptr, nullptr, &textWidth, &textHeight);
            
            SDL_Rect clip_rect = { 5, 0, getWidth() - 10, getHeight() };
            SDL_RenderSetClipRect(renderer, &clip_rect);

            SDL_Rect renderQuad = { 5 + m_text_offset_x, (getHeight() - textHeight) / 2, textWidth, textHeight };
            SDL_RenderCopy(renderer, text_texture.get(), nullptr, &renderQuad);
            
            SDL_RenderSetClipRect(renderer, nullptr);
        }
    }

    // Cursor
    if (m_active && style.textColor) {
        if (SDL_GetTicks() - m_cursor_blink_time > 500) {
            m_show_cursor = !m_show_cursor;
            m_cursor_blink_time = SDL_GetTicks();
        }
        
        if (m_show_cursor) {
            int cursor_x_pos = 0;
            if (m_cursor_pos > 0 && font) {
                TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &cursor_x_pos, nullptr);
            }
            
            SDL_Rect cursor_rect = {
                5 + cursor_x_pos + m_text_offset_x,
                (getHeight() - TTF_FontHeight(font.get())) / 2,
                2,
                TTF_FontHeight(font.get())
            };
            auto color = style.textColor.value();
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &cursor_rect);
        }
    }
}

bool TextInput::handleEvent(const SDL_Event& e) {
    if (m_locked || !m_enabled) {
        return false;
    }
    auto eventHandled = false;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            if (!m_active) {
                m_active = true;
                SDL_StartTextInput();
                m_show_cursor = true;
                m_cursor_blink_time = SDL_GetTicks();
                markDirty();
                eventHandled = true;
            }
        } else {
            if (m_active) {
                m_active = false;
                SDL_StopTextInput();
                m_show_cursor = false;
                markDirty();
            }
        }
    } else if (m_active && e.type == SDL_TEXTINPUT) {
        m_text.insert(m_cursor_pos, e.text.text);
        m_cursor_pos += strlen(e.text.text);
        update_text_offset();
        markDirty();
        if (m_onTextChanged) m_onTextChanged(this);
        eventHandled = true;
    } else if (m_active && e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && m_cursor_pos > 0) {
            m_text.erase(m_cursor_pos - 1, 1);
            m_cursor_pos--;
            update_text_offset();
            markDirty();
            if (m_onTextChanged) m_onTextChanged(this);
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            if (m_onEnterPressed) m_onEnterPressed(this);
            m_active = false;
            SDL_StopTextInput();
            markDirty();
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_LEFT && m_cursor_pos > 0) {
            m_cursor_pos--;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursor_pos < m_text.length()) {
            m_cursor_pos++;
            update_text_offset();
            markDirty();
            eventHandled = true;
        }

        if (eventHandled) {
            m_show_cursor = true;
            m_cursor_blink_time = SDL_GetTicks();
            markDirty();
        }
    }

    if (m_active) {
        markDirty(); // Keep dirty for cursor blinking
    }

    return eventHandled;
}