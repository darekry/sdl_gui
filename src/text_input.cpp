#include "text_input.hpp"
#include <SDL3/SDL_blendmode.h>
#include "style.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "theme.hpp"
#include "utf8_utils.hpp"
#include "constants.hpp"


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
    auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
    if (!font) return;

    int text_width = 0;
    std::string textBeforeCursor = utf8::substrChars(m_text, 0, m_cursorPos);
    if (!TTF_GetStringSize(font.get(), textBeforeCursor.c_str(), textBeforeCursor.length(), &text_width, nullptr)) {
        LOG_DEBUG("TextInput: TTF_GetStringSize failed: %s", SDL_GetError());
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
    if (!TTF_GetStringSize(font.get(), m_text.c_str(), m_text.length(), &total_text_width, nullptr)) {
        LOG_DEBUG("TextInput: TTF_GetStringSize failed: %s", SDL_GetError());
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
    
    auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
    if (!font) return;
    
    const auto& style = getComposedStyle(m_state);
    if (!style.textColor) return;
    
    SDL_Surface* surface = TTF_RenderText_Blended(font.get(), m_text.c_str(), m_text.length(), *style.textColor);
    if (!surface) {
        LOG_DEBUG("TextInput: TTF_RenderText_Blended failed: %s", SDL_GetError());
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_manager.getRenderer(), surface);
    SDL_DestroySurface(surface);
    
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
        int textWidth = TextureWidth(m_textTexture.get());
        int textHeight = TextureHeight(m_textTexture.get());
        
        SDL_Rect clip_rect = {5, 0, getWidth() - 10, getHeight()};
        SDL_SetRenderClipRect(renderer, &clip_rect);
        
        SDL_Rect renderQuad = {5 + m_text_offset_x, (getHeight() - textHeight) / 2, textWidth, textHeight};
        RenderTexture(renderer, m_textTexture.get(), renderQuad);
        
        SDL_SetRenderClipRect(renderer, nullptr);
    }
}

void TextInput::renderOverlay(SDL_Renderer* renderer) {
    if (!hasKeyboardFocus()) return;
    
    const auto& style = getComposedStyle(m_state);
    if (!style.textColor) return;
    
    auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
    if (!font) return;
    
    int line_height = TTF_GetFontHeight(font.get());
    auto abs_pos = getAbsolutePosition();
    int padding = 5;
    
    // Set clip rect to match the visible text area
    SDL_Rect clip_rect = {
        abs_pos.x + padding,
        abs_pos.y,
        getWidth() - 2 * padding,
        getHeight()
    };
    SDL_SetRenderClipRect(renderer, &clip_rect);
    
    // Draw selection highlight
    if (hasSelection()) {
        size_t sel_start = std::min(m_selectionStart, m_selectionEnd);
        size_t sel_end = std::max(m_selectionStart, m_selectionEnd);
        
        int start_x = 0;
        int end_x = 0;
        
        if (sel_start > 0) {
            std::string textBeforeSel = utf8::substrChars(m_text, 0, sel_start);
            TTF_GetStringSize(font.get(), textBeforeSel.c_str(), textBeforeSel.length(), &start_x, nullptr);
        }
        if (sel_end > 0) {
            std::string textBeforeEnd = utf8::substrChars(m_text, 0, sel_end);
            TTF_GetStringSize(font.get(), textBeforeEnd.c_str(), textBeforeEnd.length(), &end_x, nullptr);
        }
        
        SDL_Rect selection_rect = {
            abs_pos.x + padding + start_x + m_text_offset_x,
            abs_pos.y + (getHeight() - line_height) / 2,
            end_x - start_x,
            line_height
        };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SetDrawColor(renderer, constants::kSelectionColor);
        RenderFillRect(renderer, selection_rect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
    
    SDL_SetRenderClipRect(renderer, nullptr);
    
    // Draw cursor
    updateCursorBlink();
    if (!m_showCursor) return;
    
    int cursor_x_pos = 0;
    if (m_cursorPos > 0) {
        std::string textBeforeCursor = utf8::substrChars(m_text, 0, m_cursorPos);
        if (!TTF_GetStringSize(font.get(), textBeforeCursor.c_str(), textBeforeCursor.length(), &cursor_x_pos, nullptr)) {
            LOG_DEBUG("TextInput: TTF_GetStringSize failed for cursor: %s", SDL_GetError());
            cursor_x_pos = 0;
        }
    }
    
    SDL_SetRenderClipRect(renderer, &clip_rect);
    
    SDL_Rect cursor_rect = {
        abs_pos.x + padding + cursor_x_pos + m_text_offset_x,
        abs_pos.y + (getHeight() - line_height) / 2,
        2,
        line_height
    };
    
    auto color = *style.textColor;
    SetDrawColor(renderer, color);
    RenderFillRect(renderer, cursor_rect);
    
    SDL_SetRenderClipRect(renderer, nullptr);
}

bool TextInput::handleEvent(const SDL_Event& e) {
    if (m_locked || !m_enabled) {
        return false;
    }

    // Mouse button down - start drag
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && contains(e.button.x, e.button.y)) {
        m_manager.setKeyboardFocus(this);
        
        auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int click_x = static_cast<int>(e.button.x) - abs_pos.x - 5;
            
size_t click_pos = 0;
            if (click_x <= 0 || m_text.empty()) {
                click_pos = 0;
            } else {
                // Reusable buffer to avoid allocations in binary search
                std::string workingBuffer;
                workingBuffer.reserve(m_text.size());
                
                size_t totalChars = utf8::charCount(m_text);
                size_t left = 0;
                size_t right = totalChars;
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    workingBuffer = utf8::substrChars(m_text, 0, mid);
                    int text_width = 0;
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &text_width, nullptr);
                    if (text_width < click_x) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                
                int width_at_left = 0;
                int width_at_right = 0;
                workingBuffer = utf8::substrChars(m_text, 0, left);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_left, nullptr);
                if (left > 0) {
                    workingBuffer = utf8::substrChars(m_text, 0, left - 1);
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_right, nullptr);
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
    if (e.type == SDL_EVENT_MOUSE_MOTION && m_isDragging && hasKeyboardFocus()) {
        auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, 16);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int mouse_x = static_cast<int>(e.motion.x) - abs_pos.x - 5;
            
size_t mouse_pos = 0;
            if (mouse_x <= 0 || m_text.empty()) {
                mouse_pos = 0;
            } else {
                // Reusable buffer to avoid allocations in binary search
                std::string workingBuffer;
                workingBuffer.reserve(m_text.size());
                
                size_t totalChars = utf8::charCount(m_text);
                size_t left = 0;
                size_t right = totalChars;
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    workingBuffer = utf8::substrChars(m_text, 0, mid);
                    int text_width = 0;
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &text_width, nullptr);
                    if (text_width < mouse_x) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                
                int width_at_left = 0;
                int width_at_right = 0;
                workingBuffer = utf8::substrChars(m_text, 0, left);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_left, nullptr);
                if (left > 0) {
                    workingBuffer = utf8::substrChars(m_text, 0, left - 1);
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_right, nullptr);
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
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && m_isDragging) {
        m_isDragging = false;
        return true;
    }

    if (!hasKeyboardFocus()) {
        return false;
    }

    bool eventHandled = false;
    
    if (e.type == SDL_EVENT_TEXT_INPUT) {
        handleTextInputWithSelection(e.text.text);
        eventHandled = true;
    } else if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.mod & SDL_KMOD_CTRL) {
            switch (e.key.key) {
                case SDLK_C:
                    eventHandled = handleClipboardCopy();
                    break;
                case SDLK_V:
                    eventHandled = handleClipboardPaste();
                    break;
                case SDLK_X:
                    eventHandled = handleClipboardCut();
                    break;
                case SDLK_A:
                    setSelection(0, utf8::charCount(m_text));
                    m_cursorPos = utf8::charCount(m_text);
                    updateTextOffset();
                    markDirty();
                    eventHandled = true;
                    break;
            }
        }
        
        if (!eventHandled) {
            bool shiftPressed = (e.key.mod & SDL_KMOD_SHIFT);
            
            if (e.key.key == SDLK_BACKSPACE) {
                eventHandled = handleBackspaceWithSelection();
            } else if (e.key.key == SDLK_DELETE) {
                eventHandled = handleDeleteWithSelection();
            } else if (e.key.key == SDLK_RETURN) {
                if (m_onEnterPressed) {
                    m_onEnterPressed(this);
                }
                return true;
            } else if (e.key.key == SDLK_LEFT && m_cursorPos > 0) {
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
            } else if (e.key.key == SDLK_RIGHT && m_cursorPos < utf8::charCount(m_text)) {
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
            } else if (e.key.key == SDLK_HOME) {
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
            } else if (e.key.key == SDLK_END) {
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = utf8::charCount(m_text);
                } else {
                    clearSelection();
                }
                m_cursorPos = utf8::charCount(m_text);
                updateTextOffset();
                eventHandled = true;
            }
        }
    }

    if (eventHandled) {
        resetCursorBlink();
    }

    // Call parent to handle tooltip timer logic
    GUIElement::handleEvent(e);
    
    return eventHandled;
}