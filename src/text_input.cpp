#include "text_input.hpp"
#include "style.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "theme.hpp"


TextInput::TextInput(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h), m_text(""), m_locked(false), m_cursor_pos(0), m_text_offset_x(0), m_show_cursor(false), m_cursor_blink_time(0) {
    setCanGetKeyboardFocus(true);
    markDirty();
}

void TextInput::setText(std::string_view newText) {
    if (m_text != newText) {
        m_text = newText;
        m_cursor_pos = std::min(m_cursor_pos, m_text.length());
        update_text_offset();
        refreshTextTexture();
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
        refreshTextTexture();
        markDirty();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

const char* TextInput::getComponentType() const {
    return "TextInput";
}

const std::string& TextInput::getText() const {
    return m_text;
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
        if (hasKeyboardFocus()) {
            m_manager.setKeyboardFocus(nullptr);
        }
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
    if (TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &text_width, nullptr) != 0) {
        LOG_DEBUG("TextInput: TTF_SizeText failed: %s", TTF_GetError());
        text_width = 0;
    }

    auto cursor_pos_x = text_width;
    auto padding = 5;
    auto visible_width = getWidth() - 2 * padding;

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
    if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }

    int total_text_width = 0;
    if (TTF_SizeText(font.get(), m_text.c_str(), &total_text_width, nullptr) != 0) {
        LOG_DEBUG("TextInput: TTF_SizeText failed: %s", TTF_GetError());
        total_text_width = 0;
    }

    if (total_text_width < visible_width) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x > 0) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x < visible_width - total_text_width) {
        m_text_offset_x = visible_width - total_text_width;
    }
}

void TextInput::refreshTextTexture() {
    m_textTexture.reset();
    
    if (m_text.empty()) return;
    
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;
    
    const auto& style = getComposedStyle(m_state);
    if (!style.textColor) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font.get(), m_text.c_str(), *style.textColor);
    if (!surface) {
        LOG_DEBUG("TextInput: TTF_RenderUTF8_Blended failed: %s", TTF_GetError());
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_manager.getRenderer(), surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        LOG_DEBUG("TextInput: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return;
    }
    
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    m_textTexture = SharedTexture(texture, SDLTextureDeleter());
}

void TextInput::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    
    if (!m_text.empty() && m_textTexture) {
        int textWidth = 0, textHeight = 0;
        SDL_QueryTexture(m_textTexture.get(), nullptr, nullptr, &textWidth, &textHeight);
        
        SDL_Rect clip_rect = {5, 0, getWidth() - 10, getHeight()};
        SDL_RenderSetClipRect(renderer, &clip_rect);
        
        SDL_Rect renderQuad = {5 + m_text_offset_x, (getHeight() - textHeight) / 2, textWidth, textHeight};
        SDL_RenderCopy(renderer, m_textTexture.get(), nullptr, &renderQuad);
        
        SDL_RenderSetClipRect(renderer, nullptr);
    }
}

void TextInput::renderOverlay(SDL_Renderer* renderer) {
    if (!hasKeyboardFocus()) return;
    
    const auto& style = getComposedStyle(m_state);
    if (!style.textColor) return;
    
    if (SDL_GetTicks() - m_cursor_blink_time > 500) {
        m_show_cursor = !m_show_cursor;
        m_cursor_blink_time = SDL_GetTicks();
    }
    
    if (!m_show_cursor) return;
    
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;
    
    int cursor_x_pos = 0;
    if (m_cursor_pos > 0) {
        if (TTF_SizeText(font.get(), m_text.substr(0, m_cursor_pos).c_str(), &cursor_x_pos, nullptr) != 0) {
            LOG_DEBUG("TextInput: TTF_SizeText failed for cursor: %s", TTF_GetError());
            cursor_x_pos = 0;
        }
    }
    
    auto abs_pos = getAbsolutePosition();
    SDL_Rect cursor_rect = {
        abs_pos.x + 5 + cursor_x_pos + m_text_offset_x,
        abs_pos.y + (getHeight() - TTF_FontHeight(font.get())) / 2,
        2,
        TTF_FontHeight(font.get())
    };
    
    auto color = *style.textColor;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &cursor_rect);
}

void TextInput::onFocusGained() {
    SDL_StartTextInput();
    m_show_cursor = true;
    m_cursor_blink_time = SDL_GetTicks();
}

void TextInput::onFocusLost() {
    SDL_StopTextInput();
    m_show_cursor = false;
}

bool TextInput::handleEvent(const SDL_Event& e) {
    if (m_locked || !m_enabled) {
        return false;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && contains(e.button.x, e.button.y)) {
        m_manager.setKeyboardFocus(this);
        
        auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int click_x = e.button.x - abs_pos.x - 5;
            
            if (click_x <= 0) {
                m_cursor_pos = 0;
            } else if (m_text.empty()) {
                m_cursor_pos = 0;
            } else {
                size_t left = 0;
                size_t right = m_text.length();
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    int text_width = 0;
                    TTF_SizeText(font.get(), m_text.substr(0, mid).c_str(), &text_width, nullptr);
                    if (text_width < click_x) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                
                int width_at_left = 0;
                int width_at_right = 0;
                TTF_SizeText(font.get(), m_text.substr(0, left).c_str(), &width_at_left, nullptr);
                if (left > 0) {
                    TTF_SizeText(font.get(), m_text.substr(0, left - 1).c_str(), &width_at_right, nullptr);
                }
                
                if (left > 0 && abs(click_x - width_at_right) < abs(click_x - width_at_left)) {
                    m_cursor_pos = left - 1;
                } else {
                    m_cursor_pos = left;
                }
            }
            
            update_text_offset();
            m_show_cursor = true;
            m_cursor_blink_time = SDL_GetTicks();
            markDirty();
        }
        return true;
    }

    if (!hasKeyboardFocus()) {
        return false;
    }

    bool eventHandled = false;
    if (e.type == SDL_TEXTINPUT) {
        m_text.insert(m_cursor_pos, e.text.text);
        m_cursor_pos += strlen(e.text.text);
        update_text_offset();
        refreshTextTexture();
        markDirty();
        if (m_onTextChanged) m_onTextChanged(this);
        eventHandled = true;
    } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && m_cursor_pos > 0) {
            m_text.erase(m_cursor_pos - 1, 1);
            m_cursor_pos--;
            update_text_offset();
            refreshTextTexture();
            markDirty();
            if (m_onTextChanged) m_onTextChanged(this);
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            // Call callback BEFORE any other operations
            // Callback may destroy this TextInput (e.g., StringGrid::stopEditing)
            // so we must return immediately after to prevent use-after-free
            if (m_onEnterPressed) {
                m_onEnterPressed(this);
            }
            return true;  // Return immediately - this object may be destroyed
        } else if (e.key.keysym.sym == SDLK_LEFT && m_cursor_pos > 0) {
            m_cursor_pos--;
            update_text_offset();
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursor_pos < m_text.length()) {
            m_cursor_pos++;
            update_text_offset();
            eventHandled = true;
        }
    }

    if (eventHandled) {
        m_show_cursor = true;
        m_cursor_blink_time = SDL_GetTicks();
    }

    return eventHandled;
}