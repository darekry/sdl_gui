#include "text_area.hpp"
#include "gui_manager.hpp"
#include "context_menu.hpp"
#include "texture_manager.hpp"
#include "utf8_utils.hpp"
#include "constants.hpp"

#include "std.hpp"

TextArea::TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size)
    : TextEditable(manager, x, y, w, h), m_font_path(font_path), m_font_size(font_size) {
    m_manager.getFontManager().loadFont(m_font_path.c_str(), m_font_size);
    m_needs_texture_update = true;
    m_text_offset_x = 0;
    m_lines.emplace_back("");
}

void TextArea::setText(std::string_view text) {
    TextEditable::setText(text);
}

void TextArea::setText(std::string&& text) {
    TextEditable::setText(std::string_view(text));
}

void TextArea::setText(const char* text) {
    TextEditable::setText(std::string_view(text ? text : ""));
}

void TextArea::setWordWrap(bool enabled) {
    if (m_wordWrap != enabled) {
        m_wordWrap = enabled;
        markNeedsUpdate();
    }
}

bool TextArea::getWordWrap() const {
    return m_wordWrap;
}

void TextArea::setOnTextChanged(const std::function<void(TextArea*)>& callback) {
    m_onTextChanged = callback;
    TextEditable::setOnTextChanged([callback](TextEditable* te) {
        if (callback && te) {
            callback(static_cast<TextArea*>(te));
        }
    });
}

void TextArea::setLocked(bool locked) {
    bool was = m_locked;
    TextEditable::setLocked(locked);
    if (locked && !was) {
        m_isHovered = false;
        setState(ElementState::Normal);
        m_showCursor = false;
    }
}

bool TextArea::isLocked() const {
    return TextEditable::isLocked();
}

void TextArea::updateTextOffset() {
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        return;
    }
    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }
    if (m_lines.empty()) {
        m_text_offset_x = 0;
        return;
    }

    size_t current_line_idx = getLineFromPosition(m_cursorPos);
    size_t col = getColumnFromPosition(m_cursorPos);
    const std::string& line = m_lines[current_line_idx];
    std::string text_before_cursor = utf8::substrChars(line, 0, col);
    int cursor_pos_x = 0;
    TTF_GetStringSize(font.get(), text_before_cursor.c_str(), text_before_cursor.length(), &cursor_pos_x, nullptr);

    int padding = 2;
    int visible_width = getWidth() - (2 * padding);

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
    if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }

    int total_text_width = 0;
    TTF_GetStringSize(font.get(), line.c_str(), line.length(), &total_text_width, nullptr);

    if (total_text_width <= visible_width) {
        m_text_offset_x = 0;
    } else {
        m_text_offset_x = std::clamp(m_text_offset_x, visible_width - total_text_width, 0);
    }
}

void TextArea::refreshTextTexture() {
    // Deferred: actual line textures are rebuilt lazily in draw()/renderOverlay().
    m_needs_texture_update = true;
}

void TextArea::markNeedsUpdate() {
    m_needs_texture_update = true;
    markDirty();
}

size_t TextArea::lineByteOffset(std::string_view line, size_t columnChars) {
    size_t total = utf8::charCount(line);
    return utf8::charToByteIndex(line, std::min(columnChars, total));
}

size_t TextArea::getLineFromPosition(size_t charPos) const {
    size_t temp = 0;
    for (size_t i = 0; i < m_lines.size(); ++i) {
        size_t lineChars = utf8::charCount(m_lines[i]);
        size_t lineLen = lineChars + (i + 1 < m_lines.size() ? 1 : 0); // +1 for '\n'
        if (charPos <= temp + lineLen || i + 1 == m_lines.size()) {
            return i;
        }
        temp += lineLen;
    }
    return m_lines.empty() ? 0 : m_lines.size() - 1;
}

size_t TextArea::getColumnFromPosition(size_t charPos) const {
    size_t temp = 0;
    for (size_t i = 0; i < m_lines.size(); ++i) {
        size_t lineChars = utf8::charCount(m_lines[i]);
        size_t lineLen = lineChars + (i + 1 < m_lines.size() ? 1 : 0);
        if (charPos <= temp + lineLen || i + 1 == m_lines.size()) {
            size_t col = (charPos > temp) ? (charPos - temp) : 0;
            return std::min(col, lineChars);
        }
        temp += lineLen;
    }
    return 0;
}

size_t TextArea::getPositionFromLineAndColumn(size_t line, size_t columnChars) const {
    return charPosFromRowColumn(line, columnChars);
}

size_t TextArea::charPosFromRowColumn(size_t row, size_t columnChars) const {
    if (m_lines.empty()) {
        return 0;
    }
    size_t pos = 0;
    for (size_t i = 0; i < row && i < m_lines.size(); ++i) {
        pos += utf8::charCount(m_lines[i]);
        if (i + 1 < m_lines.size()) {
            pos += 1; // '\n'
        }
    }
    if (row < m_lines.size()) {
        pos += std::min(columnChars, utf8::charCount(m_lines[row]));
    }
    return pos;
}

void TextArea::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);

    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        return;
    }

    auto clip_rect = SDL_Rect{2, 2, m_width - 4, m_height - 4};
    SDL_SetRenderClipRect(renderer, &clip_rect);

    int yOffset = m_scroll_offset_y;
    for (const auto& texture : m_line_textures) {
        if (texture) {
            int texW = TextureWidth(texture.get());
            int texH = TextureHeight(texture.get());
            SDL_Rect destRect = {2 + m_text_offset_x, yOffset + 2, texW, texH};

            if (destRect.y + destRect.h > 0 && destRect.y < m_height) {
                RenderTexture(renderer, texture.get(), destRect);
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

    // Mouse button down - start potential drag or position cursor
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && contains(e.button.x, e.button.y)) {
        // Right button: default text editing context menu instead of starting a drag
        if (e.button.button == SDL_BUTTON_RIGHT) {
            showContextMenu(e.button.x, e.button.y);
            return true;
        }

        setState(ElementState::Hover);
        m_isHovered = true;
        m_manager.setKeyboardFocus(this);
        resetCursorBlink();

        auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
        if (font) {
            if (m_needs_texture_update) {
                recalculateLines();
                refreshTextures();
                m_needs_texture_update = false;
            }
            auto abs_pos = getAbsolutePosition();
            int line_height = TTF_GetFontHeight(font.get());
            int click_y = static_cast<int>(e.button.y) - abs_pos.y - 2 + m_scroll_offset_y;
            int click_x = static_cast<int>(e.button.x) - abs_pos.x - 2 - m_text_offset_x;

            size_t row = static_cast<size_t>(std::max(0, click_y / line_height));
            row = std::min(row, m_lines.size() - 1);

            const auto& line = m_lines[row];
            size_t colChars = TextEditable::charIndexAtX(line, font.get(), click_x);
            m_cursorPos = charPosFromRowColumn(row, colChars);

            // Start drag selection
            m_isDragging = true;
            m_dragStartPos = m_cursorPos;
            clearSelection();

            updateTextOffset();
            markDirty();
        }
        return true;
    }
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && !contains(e.button.x, e.button.y)) {
        if (m_isHovered) {
            setState(ElementState::Normal);
            markDirty();
        }
        m_isHovered = false;
        m_isDragging = false;
        if (hasKeyboardFocus()) {
            m_manager.setKeyboardFocus(nullptr);
        }
        m_showCursor = false;
        return false;
    }

    // Mouse motion - extend selection during drag (focus-based, works outside bounds)
    if (e.type == SDL_EVENT_MOUSE_MOTION && m_isDragging && hasKeyboardFocus()) {
        auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
        if (font) {
            if (m_needs_texture_update) {
                recalculateLines();
                refreshTextures();
                m_needs_texture_update = false;
            }
            auto abs_pos = getAbsolutePosition();
            int line_height = TTF_GetFontHeight(font.get());
            int mouse_y = static_cast<int>(e.motion.y) - abs_pos.y - 2 + m_scroll_offset_y;
            int mouse_x = static_cast<int>(e.motion.x) - abs_pos.x - 2 - m_text_offset_x;

            size_t row = static_cast<size_t>(std::max(0, mouse_y / line_height));
            row = std::min(row, m_lines.size() - 1);

            const auto& line = m_lines[row];
            size_t colChars = TextEditable::charIndexAtX(line, font.get(), mouse_x);
            m_cursorPos = charPosFromRowColumn(row, colChars);

            setSelection(m_dragStartPos, m_cursorPos);
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
            }
            return true;
        }
        return false;
    }

    if (!hasKeyboardFocus()) {
        if (e.type == SDL_EVENT_MOUSE_MOTION && m_enabled && m_visible) {
            processHoverTooltip(contains(e.motion.x, e.motion.y));
        }
        processButtonEvent(e);
        return false;
    }

    bool eventHandled = false;

    if (e.type == SDL_EVENT_TEXT_INPUT) {
        handleTextInputWithSelection(e.text.text);
        eventHandled = true;
    } else if (e.type == SDL_EVENT_KEY_DOWN) {
        bool shiftPressed = (e.key.mod & SDL_KMOD_SHIFT) != 0;
        bool ctrlPressed = (e.key.mod & SDL_KMOD_CTRL) != 0;

        if (ctrlPressed && (e.key.key == SDLK_C || e.key.key == SDLK_V || e.key.key == SDLK_X ||
                            e.key.key == SDLK_A)) {
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
                    selectAll();
                    eventHandled = true;
                    break;
                default:
                    break;
            }
        } else if (e.key.key == SDLK_BACKSPACE) {
            eventHandled = handleBackspaceWithSelection();
        } else if (e.key.key == SDLK_DELETE) {
            eventHandled = handleDeleteWithSelection();
        } else if (e.key.key == SDLK_RETURN) {
            handleTextInputWithSelection("\n");
            eventHandled = true;
        } else if (e.key.key == SDLK_LEFT || e.key.key == SDLK_RIGHT) {
            size_t totalChars = utf8::charCount(m_text);
            size_t new_pos = m_cursorPos;
            if (e.key.key == SDLK_LEFT && m_cursorPos > 0) {
                new_pos = m_cursorPos - 1;
            } else if (e.key.key == SDLK_RIGHT && m_cursorPos < totalChars) {
                new_pos = m_cursorPos + 1;
            } else {
                new_pos = m_cursorPos; // at boundary: not handled
                eventHandled = false;
                if (e.type == SDL_EVENT_MOUSE_MOTION && m_enabled && m_visible) {
                    processHoverTooltip(contains(e.motion.x, e.motion.y));
                }
                processButtonEvent(e);
                return false;
            }
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
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_UP || e.key.key == SDLK_DOWN) {
            size_t current_line = getLineFromPosition(m_cursorPos);
            size_t current_column = getColumnFromPosition(m_cursorPos);
            bool canMove = (e.key.key == SDLK_UP && current_line > 0) ||
                           (e.key.key == SDLK_DOWN && current_line + 1 < m_lines.size());
            if (canMove) {
                size_t target = (e.key.key == SDLK_UP) ? current_line - 1 : current_line + 1;
                size_t new_pos = getPositionFromLineAndColumn(target, current_column);
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
                markDirty();
            }
            eventHandled = true;
        } else if (e.key.key == SDLK_HOME || e.key.key == SDLK_END) {
            size_t new_pos = m_cursorPos;
            if (ctrlPressed) {
                size_t totalChars = utf8::charCount(m_text);
                new_pos = (e.key.key == SDLK_HOME) ? 0 : totalChars;
            } else {
                size_t current_line = getLineFromPosition(m_cursorPos);
                new_pos = getPositionFromLineAndColumn(
                    current_line, (e.key.key == SDLK_HOME) ? 0 : utf8::charCount(m_lines[current_line]));
            }
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
            markDirty();
            eventHandled = true;
        } else if (e.key.key == SDLK_PAGEUP || e.key.key == SDLK_PAGEDOWN) {
            auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
            if (font) {
                int line_height = TTF_GetFontHeight(font.get());
                int visible_lines = std::max(1, m_height / line_height);

                size_t current_line = getLineFromPosition(m_cursorPos);
                size_t current_column = getColumnFromPosition(m_cursorPos);
                size_t target_line = current_line;
                if (e.key.key == SDLK_PAGEUP) {
                    target_line = (current_line > static_cast<size_t>(visible_lines))
                                      ? current_line - static_cast<size_t>(visible_lines)
                                      : 0;
                    m_scroll_offset_y += visible_lines * line_height;
                } else {
                    target_line =
                        std::min(current_line + static_cast<size_t>(visible_lines), m_lines.size() - 1);
                    m_scroll_offset_y -= visible_lines * line_height;
                }
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

                int max_scroll = static_cast<int>(m_lines.size()) * line_height - m_height;
                max_scroll = std::max(max_scroll, 0);
                m_scroll_offset_y = std::clamp(m_scroll_offset_y, -max_scroll, 0);
                updateTextOffset();
                markDirty();
            }
            eventHandled = true;
        }
        if (eventHandled) {
            resetCursorBlink();
            markDirty();
        }
    }

    if (e.type == SDL_EVENT_MOUSE_MOTION && m_enabled && m_visible) {
        processHoverTooltip(contains(e.motion.x, e.motion.y));
    }
    processButtonEvent(e);

    return eventHandled;
}

void TextArea::renderOverlay(SDL_Renderer* renderer) {
    if (!m_isHovered && !hasKeyboardFocus()) {
        return;
    }

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        return;
    }

    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }

    int line_height = TTF_GetFontHeight(font.get());
    auto abs_pos = getAbsolutePosition();

    // Set clip rect for selection and cursor rendering
    SDL_Rect clip_rect = {abs_pos.x + 2, abs_pos.y + 2, m_width - 4, m_height - 4};
    SDL_SetRenderClipRect(renderer, &clip_rect);

    // Draw selection highlight (multi-line support, char indices)
    if (hasSelection()) {
        size_t sel_start = std::min(m_selectionStart, m_selectionEnd);
        size_t sel_end = std::max(m_selectionStart, m_selectionEnd);

        size_t start_line = getLineFromPosition(sel_start);
        size_t end_line = getLineFromPosition(sel_end);
        size_t start_column = getColumnFromPosition(sel_start);
        size_t end_column = getColumnFromPosition(sel_end);

        for (size_t line_idx = start_line; line_idx <= end_line && line_idx < m_lines.size(); ++line_idx) {
            const auto& line = m_lines[line_idx];
            size_t lineChars = utf8::charCount(line);

            size_t line_sel_start = (line_idx == start_line) ? start_column : 0;
            size_t line_sel_end = (line_idx == end_line) ? end_column : lineChars;

            if (line_sel_start >= line_sel_end) {
                continue;
            }

            int start_x = 0;
            int end_x = 0;

            if (line_sel_start > 0) {
                std::string before = utf8::substrChars(line, 0, line_sel_start);
                TTF_GetStringSize(font.get(), before.c_str(), before.length(), &start_x, nullptr);
            }
            if (line_sel_end > 0) {
                std::string upto = utf8::substrChars(line, 0, line_sel_end);
                TTF_GetStringSize(font.get(), upto.c_str(), upto.length(), &end_x, nullptr);
            }

            int y_offset = static_cast<int>(line_idx) * line_height + m_scroll_offset_y;

            SDL_Rect selection_rect = {abs_pos.x + 2 + start_x + m_text_offset_x,
                                       abs_pos.y + 2 + y_offset, end_x - start_x, line_height};

            SetDrawColor(renderer, constants::kSelectionColor);
            RenderFillRect(renderer, selection_rect);
        }
    }

    // Draw cursor
    updateCursorBlink();

    if (!m_showCursor) {
        SDL_SetRenderClipRect(renderer, nullptr);
        return;
    }

    size_t currentLineIndex = getLineFromPosition(m_cursorPos);
    size_t posInChars = getColumnFromPosition(m_cursorPos);

    int x = 0;
    int y = 0;
    const auto& lineContent = m_lines[currentLineIndex];
    posInChars = std::min(posInChars, utf8::charCount(lineContent));

    std::string textBeforeCursor = utf8::substrChars(lineContent, 0, posInChars);
    TTF_GetStringSize(font.get(), textBeforeCursor.c_str(), textBeforeCursor.length(), &x, nullptr);

    y = static_cast<int>(currentLineIndex) * line_height + m_scroll_offset_y;

    auto cursorRect = SDL_Rect{abs_pos.x + 2 + x + m_text_offset_x, abs_pos.y + 2 + y, 2, line_height};

    const auto style = getComposedStyle(m_state);
    SDL_Color color = style.textColor.value_or(constants::kDefaultTextColor);
    SetDrawColor(renderer, color);
    RenderFillRect(renderer, cursorRect);

    // Reset clip rect
    SDL_SetRenderClipRect(renderer, nullptr);
}

void TextArea::recalculateLines() {
    m_lines.clear();
    if (m_text.empty()) {
        m_lines.emplace_back("");
        return;
    }

    // Split raw text into paragraphs (preserve empty lines, handle \r\n)
    std::vector<std::string> paragraphs;
    size_t start = 0;
    while (true) {
        size_t nl = m_text.find('\n', start);
        std::string part =
            (nl == std::string::npos) ? m_text.substr(start) : m_text.substr(start, nl - start);
        if (!part.empty() && part.back() == '\r') {
            part.pop_back();
        }
        paragraphs.push_back(std::move(part));
        if (nl == std::string::npos) {
            break;
        }
        start = nl + 1;
    }

    if (!m_wordWrap) {
        m_lines = std::move(paragraphs);
        if (m_lines.empty()) {
            m_lines.emplace_back("");
        }
        return;
    }

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        m_lines = std::move(paragraphs);
        return;
    }
    int avail = m_width - 4; // 4px padding (kept from previous implementation)
    if (avail <= 0) {
        m_lines = std::move(paragraphs);
        return;
    }

    auto textWidth = [&](const std::string& s) {
        int w = 0;
        TTF_GetStringSize(font.get(), s.c_str(), s.length(), &w, nullptr);
        return w;
    };

    for (const std::string& para : paragraphs) {
        if (para.empty()) {
            m_lines.emplace_back("");
            continue;
        }
        if (textWidth(para) <= avail) {
            m_lines.push_back(para);
            continue;
        }
        // Greedy wrap preserving all spaces: tokens = words + gaps
        // Walk the paragraph splitting on ' ' but keeping gap strings.
        size_t i = 0;
        std::string current;
        const size_t n = para.size();
        while (i < n) {
            // Collect gap (spaces)
            size_t gapStart = i;
            while (i < n && para[i] == ' ') {
                ++i;
            }
            std::string gap = para.substr(gapStart, i - gapStart);
            // Collect word (non-spaces)
            size_t wordStart = i;
            while (i < n && para[i] != ' ') {
                ++i;
            }
            std::string word = para.substr(wordStart, i - wordStart);
            if (word.empty()) {
                // Trailing spaces: append if fits, else push line
                std::string test = current + gap;
                if (textWidth(test) <= avail) {
                    current = std::move(test);
                } else {
                    if (!current.empty()) {
                        m_lines.push_back(current);
                    }
                    current.clear();
                    // trailing gap beyond width is dropped
                }
                break;
            }
            if (current.empty()) {
                // Line start: drop leading gap, place word (hard-break if needed)
                if (textWidth(word) <= avail) {
                    current = word;
                } else {
                    // Hard-break a too-long word by chars
                    size_t charOff = 0;
                    size_t totalChars = utf8::charCount(word);
                    while (charOff < totalChars) {
                        // binary search max fitting prefix
                        size_t lo = 1, hi = totalChars - charOff, best = 1;
                        while (lo <= hi) {
                            size_t mid = lo + (hi - lo) / 2;
                            std::string cand = utf8::substrChars(word, charOff, mid);
                            if (textWidth(cand) <= avail) {
                                best = mid;
                                lo = mid + 1;
                            } else {
                                hi = mid - 1;
                            }
                        }
                        std::string chunk = utf8::substrChars(word, charOff, best);
                        charOff += best;
                        if (charOff < totalChars) {
                            m_lines.push_back(chunk);
                        } else {
                            current = chunk;
                        }
                        if (best == 0) {
                            break; // safety
                        }
                    }
                }
            } else {
                std::string test = current + gap + word;
                if (textWidth(test) <= avail) {
                    current = std::move(test);
                } else {
                    m_lines.push_back(current);
                    // new line starts with word (gap collapsed at line break)
                    if (textWidth(word) <= avail) {
                        current = word;
                    } else {
                        current.clear();
                        size_t charOff = 0;
                        size_t totalChars = utf8::charCount(word);
                        while (charOff < totalChars) {
                            size_t lo = 1, hi = totalChars - charOff, best = 1;
                            while (lo <= hi) {
                                size_t mid = lo + (hi - lo) / 2;
                                std::string cand = utf8::substrChars(word, charOff, mid);
                                if (textWidth(cand) <= avail) {
                                    best = mid;
                                    lo = mid + 1;
                                } else {
                                    hi = mid - 1;
                                }
                            }
                            std::string chunk = utf8::substrChars(word, charOff, best);
                            charOff += best;
                            if (charOff < totalChars) {
                                m_lines.push_back(chunk);
                            } else {
                                current = chunk;
                            }
                            if (best == 0) {
                                break;
                            }
                        }
                    }
                }
            }
        }
        m_lines.push_back(current);
    }
    if (m_lines.empty()) {
        m_lines.emplace_back("");
    }
}

void TextArea::refreshTextures() {
    m_line_textures.clear();
    m_line_textures.reserve(m_lines.size());
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) {
        return;
    }

    const auto style = getComposedStyle(m_state);
    SDL_Color color = style.textColor.value_or(constants::kDefaultTextColor);

    for (const auto& line : m_lines) {
        if (line.empty()) {
            m_line_textures.push_back(nullptr);
        } else {
            m_line_textures.push_back(m_manager.getTextureManager().createTextureFromText(line, font, color));
        }
    }
}

ComponentType TextArea::getComponentTypeId() const {
    return ComponentType::TextArea;
}
