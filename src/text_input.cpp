#include "text_input.hpp"
#include "style.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "theme.hpp"


TextInput::TextInput(GUIManager& manager, int x, int y, int w, int h)
    : TextEditable(manager, x, y, w, h), m_locked(false), m_text_offset_x(0) {
}

const char* TextInput::getComponentType() const {
    return "TextInput";
}

void TextInput::setOnEnterPressed(const std::function<void(TextInput*)>& callback) {
    m_onEnterPressed = callback;
}

void TextInput::setOnTextChanged(const std::function<void(TextInput*)>& callback) {
    m_onTextChanged = callback;
    TextEditable::setOnTextChanged([callback](TextEditable* te) {
        if (callback && te) {
            callback(static_cast<TextInput*>(te));
        }
    });
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

void TextInput::updateTextOffset() {
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;

    int text_width = 0;
    if (TTF_SizeText(font.get(), m_text.substr(0, m_cursorPos).c_str(), &text_width, nullptr) != 0) {
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

void TextInput::markNeedsUpdate() {
    markDirty();
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
    
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
    if (!font) return;
    
    int line_height = TTF_FontHeight(font.get());
    auto abs_pos = getAbsolutePosition();
    int padding = 5;
    
    // Set clip rect to match the visible text area
    SDL_Rect clip_rect = {
        abs_pos.x + padding,
        abs_pos.y,
        getWidth() - 2 * padding,
        getHeight()
    };
    SDL_RenderSetClipRect(renderer, &clip_rect);
    
    // Draw selection highlight
    if (hasSelection()) {
        size_t sel_start = std::min(m_selectionStart, m_selectionEnd);
        size_t sel_end = std::max(m_selectionStart, m_selectionEnd);
        
        int start_x = 0;
        int end_x = 0;
        
        if (sel_start > 0) {
            TTF_SizeText(font.get(), m_text.substr(0, sel_start).c_str(), &start_x, nullptr);
        }
        if (sel_end > 0) {
            TTF_SizeText(font.get(), m_text.substr(0, sel_end).c_str(), &end_x, nullptr);
        }
        
        SDL_Rect selection_rect = {
            abs_pos.x + padding + start_x + m_text_offset_x,
            abs_pos.y + (getHeight() - line_height) / 2,
            end_x - start_x,
            line_height
        };
        
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 180);
        SDL_RenderFillRect(renderer, &selection_rect);
    }
    
    SDL_RenderSetClipRect(renderer, nullptr);
    
    // Draw cursor
    updateCursorBlink();
    if (!m_showCursor) return;
    
    int cursor_x_pos = 0;
    if (m_cursorPos > 0) {
        if (TTF_SizeText(font.get(), m_text.substr(0, m_cursorPos).c_str(), &cursor_x_pos, nullptr) != 0) {
            LOG_DEBUG("TextInput: TTF_SizeText failed for cursor: %s", TTF_GetError());
            cursor_x_pos = 0;
        }
    }
    
    SDL_RenderSetClipRect(renderer, &clip_rect);
    
    SDL_Rect cursor_rect = {
        abs_pos.x + padding + cursor_x_pos + m_text_offset_x,
        abs_pos.y + (getHeight() - line_height) / 2,
        2,
        line_height
    };
    
    auto color = *style.textColor;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &cursor_rect);
    
    SDL_RenderSetClipRect(renderer, nullptr);
}

bool TextInput::handleEvent(const SDL_Event& e) {
    if (m_locked || !m_enabled) {
        return false;
    }

    // Mouse button down - start drag
    if (e.type == SDL_MOUSEBUTTONDOWN && contains(e.button.x, e.button.y)) {
        m_manager.setKeyboardFocus(this);
        
        auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int click_x = e.button.x - abs_pos.x - 5;
            
            size_t click_pos = 0;
            if (click_x <= 0 || m_text.empty()) {
                click_pos = 0;
            } else {
                // Binary search for click position
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
                    click_pos = left - 1;
                } else {
                    click_pos = left;
                }
            }
            
            m_isDragging = true;
            m_dragStartPos = click_pos;
            m_cursorPos = click_pos;
            clearSelection();
            
            updateTextOffset();
            resetCursorBlink();
            markDirty();
        }
        return true;
    }

    // Mouse motion - extend selection
    if (e.type == SDL_MOUSEMOTION && m_isDragging && hasKeyboardFocus()) {
        auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", 16);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int mouse_x = e.motion.x - abs_pos.x - 5;
            
            size_t mouse_pos = 0;
            if (mouse_x <= 0 || m_text.empty()) {
                mouse_pos = 0;
            } else {
                size_t left = 0;
                size_t right = m_text.length();
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    int text_width = 0;
                    TTF_SizeText(font.get(), m_text.substr(0, mid).c_str(), &text_width, nullptr);
                    if (text_width < mouse_x) {
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
                
                if (left > 0 && abs(mouse_x - width_at_right) < abs(mouse_x - width_at_left)) {
                    mouse_pos = left - 1;
                } else {
                    mouse_pos = left;
                }
            }
            
            m_cursorPos = mouse_pos;
            setSelection(m_dragStartPos, mouse_pos);
            updateTextOffset();
            resetCursorBlink();
            markDirty();
        }
        return true;
    }

    // Mouse button up - end drag
    if (e.type == SDL_MOUSEBUTTONUP && m_isDragging) {
        m_isDragging = false;
        return true;
    }

    if (!hasKeyboardFocus()) {
        return false;
    }

    bool eventHandled = false;
    
    if (e.type == SDL_TEXTINPUT) {
        handleTextInputWithSelection(e.text.text);
        eventHandled = true;
    } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.mod & KMOD_CTRL) {
            switch (e.key.keysym.sym) {
                case SDLK_c:
                    eventHandled = handleClipboardCopy();
                    break;
                case SDLK_v:
                    eventHandled = handleClipboardPaste();
                    break;
                case SDLK_x:
                    eventHandled = handleClipboardCut();
                    break;
                case SDLK_a:
                    setSelection(0, m_text.length());
                    m_cursorPos = m_text.length();
                    updateTextOffset();
                    markDirty();
                    eventHandled = true;
                    break;
            }
        }
        
        if (!eventHandled) {
            bool shiftPressed = (e.key.keysym.mod & KMOD_SHIFT);
            
            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                eventHandled = handleBackspaceWithSelection();
            } else if (e.key.keysym.sym == SDLK_DELETE) {
                eventHandled = handleDeleteWithSelection();
            } else if (e.key.keysym.sym == SDLK_RETURN) {
                if (m_onEnterPressed) {
                    m_onEnterPressed(this);
                }
                return true;
            } else if (e.key.keysym.sym == SDLK_LEFT && m_cursorPos > 0) {
                size_t new_pos = m_cursorPos - 1;
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = new_pos;
                } else {
                    clearSelection();
                }
                m_cursorPos = new_pos;
                updateTextOffset();
                eventHandled = true;
            } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursorPos < m_text.length()) {
                size_t new_pos = m_cursorPos + 1;
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = new_pos;
                } else {
                    clearSelection();
                }
                m_cursorPos = new_pos;
                updateTextOffset();
                eventHandled = true;
            } else if (e.key.keysym.sym == SDLK_HOME) {
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = 0;
                } else {
                    clearSelection();
                }
                m_cursorPos = 0;
                updateTextOffset();
                eventHandled = true;
            } else if (e.key.keysym.sym == SDLK_END) {
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = m_text.length();
                } else {
                    clearSelection();
                }
                m_cursorPos = m_text.length();
                updateTextOffset();
                eventHandled = true;
            }
        }
    }

    if (eventHandled) {
        resetCursorBlink();
    }

    return eventHandled;
}