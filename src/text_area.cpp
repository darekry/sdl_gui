#include "text_area.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "sdl_deleters.hpp"

import std.compat;

TextArea::TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size)
    : GUIElement(manager, x, y, w, h), m_font_path(font_path), m_font_size(font_size) {
    
    m_manager.getFontManager().loadFont(m_font_path.c_str(), m_font_size);
    m_needs_texture_update = true;
    m_text_offset_x = 0;
    m_lines.push_back("");  // Zainicjalizuj z pustą linią, aby uniknąć SIGSEGV w update_text_offset()
}

void TextArea::setText(std::string_view text) {
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, (m_text.length()));
    m_needs_texture_update = true;
    markDirty();
    if (m_onTextChanged) { m_onTextChanged(this); }
}

void TextArea::setText(std::string&& text) {
    m_text = std::move(text);
    m_cursorPos = std::min(m_cursorPos, (m_text.length()));
    m_needs_texture_update = true;
    markDirty();
    if (m_onTextChanged) { m_onTextChanged(this); }
}

void TextArea::setText(const char* text) {
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, (m_text.length()));
    m_needs_texture_update = true;
    markDirty();
    if (m_onTextChanged) { m_onTextChanged(this); }
}

const std::string& TextArea::getText() const {
    return m_text;
}

void TextArea::setWordWrap(bool enabled) {
    m_wordWrap = enabled;
    m_needs_texture_update = true;
}

bool TextArea::getWordWrap() const {
    return m_wordWrap;
}

void TextArea::setOnTextChanged(const std::function<void(TextArea*)>& callback) {
    m_onTextChanged = callback;
}

void TextArea::setLocked(bool locked) {
    m_locked = locked;
    if (m_locked) {
        m_isHovered = false;
        setState(ElementState::Normal);
        SDL_StopTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
        m_showCursor = false;
    }
    markDirty();
}

bool TextArea::isLocked() const {
    return m_locked;
}

bool TextArea::hasSelection() const {
    return m_hasSelection && m_selectionStart != m_selectionEnd;
}

std::string TextArea::getSelection() const {
    if (!hasSelection()) return "";
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    return m_text.substr(start, end - start);
}

void TextArea::clearSelection() {
    m_hasSelection = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

void TextArea::setSelection(size_t start, size_t end) {
    m_selectionStart = std::min(start, m_text.length());
    m_selectionEnd = std::min(end, m_text.length());
    m_hasSelection = (m_selectionStart != m_selectionEnd);
}

void TextArea::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);

    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    auto clip_rect = SDL_Rect{2, 2, m_width - 4, m_height - 4};
    SDL_SetRenderClipRect(renderer, &clip_rect);

    int yOffset = m_scroll_offset_y;
    for (const auto& texture : m_line_textures) {
        if (texture) {
            int texW, texH;
            {  float _fw=0,_fh=0; SDL_GetTextureSize(texture.get(), &_fw, &_fh); texW=static_cast<int>(_fw); texH=static_cast<int>(_fh); }
            SDL_Rect destRect = {2 + m_text_offset_x, yOffset + 2, texW, texH};

            if (destRect.y + destRect.h > 0 && destRect.y < m_height) {
                 { SDL_FRect _dr = {static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h)}; SDL_RenderTexture(renderer, texture.get(), nullptr, &_dr); }
            }
        }
        yOffset += TTF_GetFontHeight(font.get());
    }

    SDL_SetRenderClipRect(renderer, nullptr);
}
bool TextArea::handleEvent(const SDL_Event& e) {
    if (m_locked || !m_enabled || !m_visible) {
        return false;
    }

    auto eventHandled = false;
    
    // Mouse button down - start potential drag or position cursor
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && contains(e.button.x, e.button.y)) {
        setState(ElementState::Hover);
        m_isHovered = true;
        SDL_StartTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
        m_showCursor = true;
        m_cursorBlinkTime = SDL_GetTicks();
        
        auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int line_height = TTF_GetFontHeight(font.get());
            int click_y = static_cast<int>(e.button.y) - abs_pos.y - 2 + m_scroll_offset_y;
            int click_x = static_cast<int>(e.button.x) - abs_pos.x - 2 - m_text_offset_x;
            
            size_t row = static_cast<size_t>(std::max(0, click_y / line_height));
            row = std::min(row, m_lines.size() - 1);
            
            const auto& line = m_lines[row];
            size_t pos_in_line = 0;
            
            if (click_x <= 0) {
                pos_in_line = 0;
            } else if (line.empty()) {
                pos_in_line = 0;
            } else {
                size_t left = 0;
                size_t right = line.length();
                std::string workingBuffer;
                workingBuffer.reserve(line.size());
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    workingBuffer.assign(line.data(), mid);
                    int text_width = 0;
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &text_width, nullptr);
                    if (text_width < click_x) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                
                int width_at_left = 0;
                int width_at_prev = 0;
                workingBuffer.assign(line.data(), left);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_left, nullptr);
                if (left > 0) {
                    workingBuffer.assign(line.data(), left - 1);
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_prev, nullptr);
                }
                
                if (left > 0 && abs(click_x - width_at_prev) < abs(click_x - width_at_left)) {
                    pos_in_line = left - 1;
                } else {
                    pos_in_line = left;
                }
            }
            
            size_t abs_pos_in_text = 0;
            for (size_t i = 0; i < row; ++i) {
                abs_pos_in_text += m_lines[i].length() + 1;
            }
            abs_pos_in_text += pos_in_line;
            m_cursorPos = abs_pos_in_text;
            
            // Start drag selection
            m_isDragging = true;
            m_dragStartPos = m_cursorPos;
            clearSelection();
            
            update_text_offset();
            markDirty();
            eventHandled = true;
        }
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !contains(e.button.x, e.button.y)) {
        if (m_isHovered) {
            setState(ElementState::Normal);
            markDirty();
        }
        m_isHovered = false;
        m_isDragging = false;
        SDL_StopTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
        m_showCursor = false;
    }
    
    // Mouse motion - extend selection during drag
    if (e.type == SDL_EVENT_MOUSE_MOTION && m_isDragging && m_isHovered) {
        auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
        if (font) {
            auto abs_pos = getAbsolutePosition();
            int line_height = TTF_GetFontHeight(font.get());
            int mouse_y = static_cast<int>(e.motion.y) - abs_pos.y - 2 + m_scroll_offset_y;
            int mouse_x = static_cast<int>(e.motion.x) - abs_pos.x - 2 - m_text_offset_x;
            
            size_t row = static_cast<size_t>(std::max(0, mouse_y / line_height));
            row = std::min(row, m_lines.size() - 1);
            
            const auto& line = m_lines[row];
            size_t pos_in_line = 0;
            
            if (mouse_x <= 0) {
                pos_in_line = 0;
            } else if (line.empty()) {
                pos_in_line = 0;
            } else {
                size_t left = 0;
                size_t right = line.length();
                std::string workingBuffer;
                workingBuffer.reserve(line.size());
                while (left < right) {
                    size_t mid = left + (right - left) / 2;
                    workingBuffer.assign(line.data(), mid);
                    int text_width = 0;
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &text_width, nullptr);
                    if (text_width < mouse_x) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                
                int width_at_left = 0;
                int width_at_prev = 0;
                workingBuffer.assign(line.data(), left);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_left, nullptr);
                if (left > 0) {
                    workingBuffer.assign(line.data(), left - 1);
                    TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &width_at_prev, nullptr);
                }
                
                if (left > 0 && abs(mouse_x - width_at_prev) < abs(mouse_x - width_at_left)) {
                    pos_in_line = left - 1;
                } else {
                    pos_in_line = left;
                }
            }
            
            size_t abs_pos_in_text = 0;
            for (size_t i = 0; i < row; ++i) {
                abs_pos_in_text += m_lines[i].length() + 1;
            }
            abs_pos_in_text += pos_in_line;
            m_cursorPos = abs_pos_in_text;
            
            setSelection(m_dragStartPos, m_cursorPos);
            update_text_offset();
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
            markDirty();
            eventHandled = true;
        }
    }
    
    // Mouse button up - end drag
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && m_isDragging) {
        m_isDragging = false;
        eventHandled = true;
    }
    
    if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        if (m_isHovered) {
            auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
            if (font) {
                int line_height = TTF_GetFontHeight(font.get());
                m_scroll_offset_y += static_cast<int>(e.wheel.y) * line_height;

                int max_scroll = static_cast<int>(m_lines.size() * static_cast<size_t>(line_height)) - m_height;
                max_scroll = std::max(max_scroll, 0);

                m_scroll_offset_y = std::clamp(m_scroll_offset_y, -max_scroll, 0);
                markDirty();
                eventHandled = true;
            }
        }
    } else if (e.type == SDL_EVENT_TEXT_INPUT && m_isHovered) {
        // Replace selection if exists, otherwise insert at cursor
        if (hasSelection()) {
            size_t start = std::min(m_selectionStart, m_selectionEnd);
            size_t end = std::max(m_selectionStart, m_selectionEnd);
            m_text.erase(start, end - start);
            m_cursorPos = start;
            clearSelection();
        }
        m_text.insert(static_cast<size_t>(m_cursorPos), e.text.text);
        m_cursorPos += (strlen(e.text.text));
        m_needs_texture_update = true;
        update_text_offset();
        markDirty();
        if (m_onTextChanged) { m_onTextChanged(this); }
        eventHandled = true;
    } else if (e.type == SDL_EVENT_KEY_DOWN && m_isHovered) {
        bool shiftPressed = (e.key.mod & SDL_KMOD_SHIFT);
        
        if (e.key.key == SDLK_BACKSPACE && m_cursorPos > 0) {
            if (hasSelection()) {
                size_t start = std::min(m_selectionStart, m_selectionEnd);
                size_t end = std::max(m_selectionStart, m_selectionEnd);
                m_text.erase(start, end - start);
                m_cursorPos = start;
                clearSelection();
            } else {
                m_text.erase(m_cursorPos - 1, 1);
                m_cursorPos--;
            }
            m_needs_texture_update = true; 
            update_text_offset();
            markDirty();
            if (m_onTextChanged) { m_onTextChanged(this); }
            eventHandled = true;
        } else if (e.key.key == SDLK_RETURN) {
            // Replace selection if exists
            if (hasSelection()) {
                size_t start = std::min(m_selectionStart, m_selectionEnd);
                size_t end = std::max(m_selectionStart, m_selectionEnd);
                m_text.erase(start, end - start);
                m_cursorPos = start;
                clearSelection();
            }
            m_text.insert(m_cursorPos, "\n");
            m_cursorPos++;
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            if (m_onTextChanged) { m_onTextChanged(this); }
            eventHandled = true;
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
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_RIGHT && m_cursorPos < m_text.length()) {
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
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_UP) {
            
            size_t current_line = getLineFromPosition(m_cursorPos);
            if (current_line > 0) {
                size_t current_column = getColumnFromPosition(m_cursorPos);
                size_t new_pos = getPositionFromLineAndColumn(current_line - 1, current_column);
                
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
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_DOWN) {
            
            size_t current_line = getLineFromPosition(m_cursorPos);
            if (current_line < m_lines.size() - 1) {
                size_t current_column = getColumnFromPosition(m_cursorPos);
                size_t new_pos = getPositionFromLineAndColumn(current_line + 1, current_column);
                
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
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_HOME) {
            bool ctrlPressed = (e.key.mod & SDL_KMOD_CTRL);

            
            if (ctrlPressed) {
                size_t new_pos = 0;
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
            } else {
                size_t current_line = getLineFromPosition(m_cursorPos);
                size_t line_start = getPositionFromLineAndColumn(current_line, 0);
                
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = line_start;
                } else {
                    clearSelection();
                }
                m_cursorPos = line_start;
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_END) {
            bool ctrlPressed = (e.key.mod & SDL_KMOD_CTRL);

            
            if (ctrlPressed) {
                size_t new_pos = m_text.length();
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
            } else {
                size_t current_line = getLineFromPosition(m_cursorPos);
                size_t line_end = getPositionFromLineAndColumn(current_line, m_lines[current_line].length());
                
                if (shiftPressed) {
                    if (!m_hasSelection) {
                        m_selectionStart = m_cursorPos;
                        m_hasSelection = true;
                    }
                    m_selectionEnd = line_end;
                } else {
                    clearSelection();
                }
                m_cursorPos = line_end;
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_PAGEUP) {

            
            auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
            if (font) {
                int line_height = TTF_GetFontHeight(font.get());
                int visible_lines = std::max(1, m_height / line_height);
                
                size_t current_line = getLineFromPosition(m_cursorPos);
                size_t current_column = getColumnFromPosition(m_cursorPos);
                size_t target_line = current_line > static_cast<size_t>(visible_lines) 
                    ? current_line - static_cast<size_t>(visible_lines) : 0;
                size_t new_pos = getPositionFromLineAndColumn(target_line, current_column);
                
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
                
                m_scroll_offset_y += visible_lines * line_height;
                int max_scroll = static_cast<int>(m_lines.size()) * line_height - m_height;
                max_scroll = std::max(max_scroll, 0);
                m_scroll_offset_y = std::clamp(m_scroll_offset_y, -max_scroll, 0);
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_PAGEDOWN) {

            
            auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
            if (font) {
                int line_height = TTF_GetFontHeight(font.get());
                int visible_lines = std::max(1, m_height / line_height);
                
                size_t current_line = getLineFromPosition(m_cursorPos);
                size_t current_column = getColumnFromPosition(m_cursorPos);
                size_t target_line = std::min(current_line + static_cast<size_t>(visible_lines), m_lines.size() - 1);
                size_t new_pos = getPositionFromLineAndColumn(target_line, current_column);
                
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
                
                m_scroll_offset_y -= visible_lines * line_height;
                int max_scroll = static_cast<int>(m_lines.size()) * line_height - m_height;
                max_scroll = std::max(max_scroll, 0);
                m_scroll_offset_y = std::clamp(m_scroll_offset_y, -max_scroll, 0);
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_DELETE) {
            if (hasSelection()) {
                size_t start = std::min(m_selectionStart, m_selectionEnd);
                size_t end = std::max(m_selectionStart, m_selectionEnd);
                m_text.erase(start, end - start);
                m_cursorPos = start;
                clearSelection();
            } else if (m_cursorPos < m_text.length()) {
                m_text.erase(m_cursorPos, 1);
            }
            m_needs_texture_update = true;
            update_text_offset();
            markDirty();
            if (m_onTextChanged) { m_onTextChanged(this); }
            eventHandled = true;
        } else if (e.key.mod & SDL_KMOD_CTRL) {
            switch (e.key.key) {
                case SDLK_C:
                    if (hasSelection()) {
                        SDL_SetClipboardText(getSelection().c_str());
                        eventHandled = true;
                    }
                    break;
                case SDLK_V:
                    if (SDL_HasClipboardText()) {
                        char* clipboard = SDL_GetClipboardText();
                        if (clipboard) {
                            if (hasSelection()) {
                                size_t start = std::min(m_selectionStart, m_selectionEnd);
                                m_text.erase(start, m_selectionEnd - m_selectionStart);
                                m_cursorPos = start;
                                clearSelection();
                            }
                            size_t insert_len = strlen(clipboard);
                            m_text.insert(m_cursorPos, clipboard);
                            m_cursorPos += insert_len;
                            m_needs_texture_update = true;
                            update_text_offset();
                            markDirty();
                            if (m_onTextChanged) { m_onTextChanged(this); }
                            SDL_free(clipboard);
                            eventHandled = true;
                        }
                    }
                    break;
                case SDLK_X:
                    if (hasSelection()) {
                        SDL_SetClipboardText(getSelection().c_str());
                        size_t start = std::min(m_selectionStart, m_selectionEnd);
                        size_t end = std::max(m_selectionStart, m_selectionEnd);
                        m_text.erase(start, end - start);
                        m_cursorPos = start;
                        clearSelection();
                        m_needs_texture_update = true;
                        update_text_offset();
                        markDirty();
                        if (m_onTextChanged) { m_onTextChanged(this); }
                        eventHandled = true;
                    }
                    break;
                case SDLK_A:
                    // Select All
                    setSelection(0, m_text.length());
                    m_cursorPos = m_text.length();
                    m_needs_texture_update = true;
                    update_text_offset();
                    markDirty();
                    eventHandled = true;
                    break;
            }
        }
        if (eventHandled) {
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
            markDirty();
        }
    }

    // Call parent to handle tooltip timer logic
    GUIElement::handleEvent(e);
    
    return eventHandled;
}

void TextArea::renderOverlay(SDL_Renderer* renderer) {
    if (!m_isHovered) return;
    
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }
    
    int line_height = TTF_GetFontHeight(font.get());
    auto abs_pos = getAbsolutePosition();
    
    // Set clip rect for selection and cursor rendering
    SDL_Rect clip_rect = {
        abs_pos.x + 2,
        abs_pos.y + 2,
        m_width - 4,
        m_height - 4
    };
    SDL_SetRenderClipRect(renderer, &clip_rect);
    
    // Draw selection highlight (multi-line support)
    if (hasSelection()) {
        size_t sel_start = std::min(m_selectionStart, m_selectionEnd);
        size_t sel_end = std::max(m_selectionStart, m_selectionEnd);
        
        size_t start_line = getLineFromPosition(sel_start);
        size_t end_line = getLineFromPosition(sel_end);
        size_t start_column = getColumnFromPosition(sel_start);
        size_t end_column = getColumnFromPosition(sel_end);
        
        for (size_t line_idx = start_line; line_idx <= end_line && line_idx < m_lines.size(); ++line_idx) {
            const auto& line = m_lines[line_idx];
            
            size_t line_sel_start = (line_idx == start_line) ? start_column : 0;
            size_t line_sel_end = (line_idx == end_line) ? end_column : line.length();
            
            if (line_sel_start >= line_sel_end) continue;
            
            int start_x = 0;
            int end_x = 0;
            
            // Use reusable buffer to avoid allocations
            std::string workingBuffer;
            workingBuffer.reserve(line.size());
            
            if (line_sel_start > 0 && line_sel_start <= line.length()) {
                workingBuffer.assign(line.data(), line_sel_start);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &start_x, nullptr);
            }
            if (line_sel_end > 0 && line_sel_end <= line.length()) {
                workingBuffer.assign(line.data(), line_sel_end);
                TTF_GetStringSize(font.get(), workingBuffer.c_str(), workingBuffer.length(), &end_x, nullptr);
            }
            
            int y_offset = static_cast<int>(line_idx) * line_height + m_scroll_offset_y;
            
            SDL_Rect selection_rect = {
                abs_pos.x + 2 + start_x + m_text_offset_x,
                abs_pos.y + 2 + y_offset,
                end_x - start_x,
                line_height
            };
            
            // Semi-transparent blue selection highlight
            SDL_SetRenderDrawColor(renderer, 100, 150, 255, 180);
            { SDL_FRect _fr = {static_cast<float>(selection_rect.x), static_cast<float>(selection_rect.y), static_cast<float>(selection_rect.w), static_cast<float>(selection_rect.h)}; SDL_RenderFillRect(renderer, &_fr); }
        }
    }
    
    // Draw cursor
    if (SDL_GetTicks() - m_cursorBlinkTime > 500) {
        m_showCursor = !m_showCursor;
        m_cursorBlinkTime = SDL_GetTicks();
    }
    
    if (!m_showCursor) {
        SDL_SetRenderClipRect(renderer, nullptr);
        return;
    }

    auto currentLineIndex = getLineFromPosition(m_cursorPos);
    size_t posInLines = getColumnFromPosition(m_cursorPos);
    
    auto x = 0;
    auto y = 0;
    const auto& lineContent = m_lines[currentLineIndex];
    posInLines = std::min(posInLines, lineContent.length());

    auto textBeforeCursor = lineContent.substr(0, posInLines);
    TTF_GetStringSize(font.get(), textBeforeCursor.c_str(), textBeforeCursor.length(), &x, nullptr);

    y = static_cast<int>(currentLineIndex) * line_height + m_scroll_offset_y;

    auto cursorRect = SDL_Rect{ abs_pos.x + 2 + x + m_text_offset_x, abs_pos.y + 2 + y, 2, line_height };
    
    const auto style = getComposedStyle(m_state);
    SDL_Color color = style.textColor.value_or(SDL_Color{0,0,0,255});
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    { SDL_FRect _fr = {static_cast<float>(cursorRect.x), static_cast<float>(cursorRect.y), static_cast<float>(cursorRect.w), static_cast<float>(cursorRect.h)}; SDL_RenderFillRect(renderer, &_fr); }
    
    // Reset clip rect
    SDL_SetRenderClipRect(renderer, nullptr);
}

void TextArea::recalculateLines() {
    m_lines.clear();
    if (m_text.empty()) {
        m_lines.emplace_back("");
        return;
    }

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    auto currentLine = std::string{};
    auto startPos = 0UZ;
    auto endPos = 0UZ;

    while ((endPos = m_text.find('\n', startPos)) != std::string::npos) {
        m_lines.push_back(m_text.substr(startPos, endPos - startPos));
        startPos = endPos + 1;
    }
    auto remainingText = m_text.substr(startPos);
    
    if (!m_wordWrap) {
        m_lines.push_back(remainingText);
        return;
    }

    auto word = std::string{};
    auto stream = std::istringstream(remainingText);
    
    currentLine.clear();
    while (stream >> word) {
        auto testLine = currentLine.empty() ? word : (currentLine + " " + word);
        auto width = 0;
        TTF_GetStringSize(font.get(), testLine.c_str(), testLine.length(), &width, nullptr);
    
        if (width > m_width - 4) { // 4px padding
            m_lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    m_lines.push_back(currentLine);
}

void TextArea::refreshTextures() {
    m_line_textures.clear();
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    const auto style = getComposedStyle(m_state);
    SDL_Color color = style.textColor.value_or(SDL_Color{0,0,0,255});

    for (const auto& line : m_lines) {
        if (line.empty()) {
            m_line_textures.push_back(nullptr);
        } else {
            SDL_Surface* surface = TTF_RenderText_Blended(font.get(), line.c_str(), line.length(), color);
            if (!surface) {
                LOG_DEBUG("TextArea: TTF_RenderText_Blended failed: %s", SDL_GetError());
                m_line_textures.push_back(nullptr);
                continue;
            }
            SDL_Texture* texture = SDL_CreateTextureFromSurface(m_manager.getRenderer(), surface);
            SDL_DestroySurface(surface);
            
            if (!texture) {
                LOG_DEBUG("TextArea: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
                m_line_textures.push_back(nullptr);
                continue;
            }
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            m_line_textures.push_back(SharedTexture(texture, SDLTextureDeleter()));
        }
    }
}

const char* TextArea::getComponentType() const {
    return "TextArea";
}

void TextArea::update_text_offset() {
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        return;
    }

    size_t current_line_idx = 0;
    auto pos_in_lines = 0uz;
    auto temp_pos = 0uz;

    for (size_t i = 0; i < m_lines.size(); ++i) {
        auto line_len = static_cast<size_t>(m_lines[i].length()) + (i < m_lines.size() - 1 ? 1UZ : 0UZ);
        if (m_cursorPos >= temp_pos && m_cursorPos <= temp_pos + line_len) {
            current_line_idx = i;
            pos_in_lines = m_cursorPos - temp_pos;
            break;
        }
        temp_pos += line_len;
    }
    
    auto text_before_cursor = m_lines[current_line_idx].substr(0, pos_in_lines);
    auto cursor_pos_x=0;
    TTF_GetStringSize(font.get(), text_before_cursor.c_str(), text_before_cursor.length(), &cursor_pos_x, nullptr);


    auto padding = 2;
    auto visible_width = getWidth() - (2 * padding);

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
     if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }
    
    auto total_text_width=0;
    TTF_GetStringSize(font.get(), m_lines[current_line_idx].c_str(), m_lines[current_line_idx].length(), &total_text_width, nullptr);

    if (total_text_width <= visible_width) {
        m_text_offset_x = 0;
    } else {
        m_text_offset_x = std::clamp(m_text_offset_x, visible_width - total_text_width, 0);
    }
}

size_t TextArea::getLineFromPosition(size_t pos) const {
    auto temp_pos = 0uz;
    for (size_t i = 0; i < m_lines.size(); ++i) {
        auto line_len = static_cast<size_t>(m_lines[i].length()) + (i < m_lines.size() - 1 ? 1UZ : 0UZ);
        if (pos >= temp_pos && pos <= temp_pos + line_len) {
            return i;
        }
        temp_pos += line_len;
    }
    return m_lines.size() > 0 ? m_lines.size() - 1 : 0;
}

size_t TextArea::getColumnFromPosition(size_t pos) const {
    auto temp_pos = 0uz;
    for (size_t i = 0; i < m_lines.size(); ++i) {
        auto line_len = static_cast<size_t>(m_lines[i].length()) + (i < m_lines.size() - 1 ? 1UZ : 0UZ);
        if (pos >= temp_pos && pos <= temp_pos + line_len) {
            return pos - temp_pos;
        }
        temp_pos += line_len;
    }
    return 0;
}

size_t TextArea::getPositionFromLineAndColumn(size_t line, size_t column) const {
    if (m_lines.empty()) return 0;
    
    auto pos = 0uz;
    for (size_t i = 0; i < line && i < m_lines.size(); ++i) {
        pos += m_lines[i].length();
        if (i < m_lines.size() - 1) pos += 1;
    }
    
    if (line < m_lines.size()) {
        column = std::min(column, m_lines[line].length());
        pos += column;
    }
    
    return pos;
}