#include "text_area.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

import std.compat;

TextArea::TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size)
    : GUIElement(manager, x, y, w, h), m_font_path(font_path), m_font_size(font_size) {
    
    m_manager.getFontManager().loadFont(m_font_path.c_str(), m_font_size);
    m_needs_texture_update = true;
    m_text_offset_x = 0;
}

void TextArea::setText(std::string_view text) {
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, m_text.length());
    m_needs_texture_update = true;
}

void TextArea::setText(std::string&& text) {
    m_text = std::move(text);
    m_cursorPos = std::min(m_cursorPos, m_text.length());
    m_needs_texture_update = true;
}

void TextArea::setText(const char* text) {
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, m_text.length());
    m_needs_texture_update = true;
}

const std::string& TextArea::getText() const {
    return m_text;
}

void TextArea::setTextColor(const SDL_Color& color) {
    m_textColor = color;
    m_needs_texture_update = true;
}
void TextArea::setWordWrap(bool enabled) {
    m_wordWrap = enabled;
    m_needs_texture_update = true;
}

bool TextArea::getWordWrap() const {
    return m_wordWrap;
}

void TextArea::draw() {
    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }

    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Rect bounds = { getAbsolutePosition().x, getAbsolutePosition().y, m_width, m_height };

    // Tło
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &bounds);

    // Ramka
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &bounds);

    // Tekst
    SharedFont font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
     if (!font) {
        return;
    }

    SDL_Rect clip_rect = { bounds.x + 2, bounds.y, bounds.w - 4, bounds.h };
    SDL_RenderSetClipRect(renderer, &clip_rect);
    int yOffset = 0;
    for (const auto& texture : m_line_textures) {
        if (texture) {
            SDL_Rect destRect = { bounds.x + 2 + m_text_offset_x, bounds.y + yOffset + 2, 0, 0 };
            SDL_QueryTexture(texture.get(), nullptr, nullptr, &destRect.w, &destRect.h);
             SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
        }
        yOffset += TTF_FontHeight(font.get());
    }

    if (m_showCursor) {
        renderCursor();
    }
    SDL_RenderSetClipRect(renderer, nullptr);
}
bool TextArea::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    bool eventHandled = false;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            m_isHovered = true;
            SDL_StartTextInput();
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
            eventHandled = true;
        } else {
            m_isHovered = false;
            SDL_StopTextInput();
            m_showCursor = false;
        }
    } else if (e.type == SDL_TEXTINPUT && m_isHovered) {
        m_text.insert(m_cursorPos, e.text.text);
        m_cursorPos += strlen(e.text.text);
        m_needs_texture_update = true;
        update_text_offset();
        eventHandled = true;
    } else if (e.type == SDL_KEYDOWN && m_isHovered) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && m_cursorPos > 0) {
            m_text.erase(m_cursorPos - 1, 1);
            m_cursorPos--;
            m_needs_texture_update = true;
            update_text_offset();
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            m_text.insert(m_cursorPos, "\n");
            m_cursorPos++;
            m_needs_texture_update = true;
             update_text_offset();
           eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_LEFT && m_cursorPos > 0) {
            m_cursorPos--;
            update_text_offset();
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursorPos < m_text.length()) {
            m_cursorPos++;
            update_text_offset();
            eventHandled = true;
        }
        if (eventHandled) {
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
        }
    }

    if (m_isHovered) {
        if (SDL_GetTicks() - m_cursorBlinkTime > 500) {
            m_showCursor = !m_showCursor;
            m_cursorBlinkTime = SDL_GetTicks();
        }
    }

    return eventHandled;
}

void TextArea::recalculateLines() {
    m_lines.clear();
    if (m_text.empty()) {
        m_lines.push_back("");
        return;
    }

    SharedFont font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    std::string currentLine;
    size_t startPos = 0;
    size_t endPos = 0;

    while ((endPos = m_text.find('\n', startPos)) != std::string::npos) {
        m_lines.push_back(m_text.substr(startPos, endPos - startPos));
        startPos = endPos + 1;
    }
    std::string remainingText = m_text.substr(startPos);
    
    if (!m_wordWrap) {
        m_lines.push_back(remainingText);
        return;
    }

    std::string word;
    std::istringstream stream(remainingText);
    
    currentLine.clear();
    while (stream >> word) {
        std::string testLine = currentLine.empty() ? word : (currentLine + " " + word);
        int width;
        TTF_SizeText(font.get(), testLine.c_str(), &width, nullptr);
    
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
    SharedFont font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    for (const auto& line : m_lines) {
        if (line.empty()) {
            m_line_textures.push_back(nullptr);
        } else {
            SharedTexture lineTexture = m_manager.getTextureManager().createTextureFromText(line, font, m_textColor);
            m_line_textures.push_back(lineTexture);
        }
    }
}

void TextArea::renderCursor() {
    SharedFont font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    int currentLineIndex = 0;
    size_t posInLines = 0;
    size_t tempPos = 0;

    for(size_t i = 0; i < m_lines.size(); ++i) {
        size_t lineLengthWithNewline = m_lines[i].length() + (i < m_lines.size() - 1 ? 1 : 0);
        if (m_cursorPos >= tempPos && m_cursorPos <= tempPos + lineLengthWithNewline) {
            currentLineIndex = i;
            posInLines = m_cursorPos - tempPos;
            break;
        }
        tempPos += lineLengthWithNewline;
    }

    int x, y;
    std::string lineContent = m_lines[currentLineIndex];
    if (posInLines > lineContent.length()) posInLines = lineContent.length();

    std::string textBeforeCursor = lineContent.substr(0, posInLines);
    TTF_SizeText(font.get(), textBeforeCursor.c_str(), &x, nullptr);

    y = currentLineIndex * TTF_FontHeight(font.get());

    SDL_Rect cursorRect = { getAbsolutePosition().x + 2 + x + m_text_offset_x, getAbsolutePosition().y + 2 + y, 2, TTF_FontHeight(font.get()) };
    SDL_SetRenderDrawColor(m_manager.getRenderer(), m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    SDL_RenderFillRect(m_manager.getRenderer(), &cursorRect);
}


void TextArea::update_text_offset() {
    SharedFont font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    int current_line_idx = 0;
    size_t pos_in_lines = 0;
    size_t temp_pos = 0;

    for (size_t i = 0; i < m_lines.size(); ++i) {
        size_t line_len = m_lines[i].length() + (i < m_lines.size() - 1 ? 1 : 0);
        if (m_cursorPos >= temp_pos && m_cursorPos <= temp_pos + line_len) {
            current_line_idx = i;
            pos_in_lines = m_cursorPos - temp_pos;
            break;
        }
        temp_pos += line_len;
    }
    
    std::string text_before_cursor = m_lines[current_line_idx].substr(0, pos_in_lines);
    int cursor_pos_x=0;
    TTF_SizeText(font.get(), text_before_cursor.c_str(), &cursor_pos_x, nullptr);


    int padding = 2;
    int visible_width = getWidth() - 2 * padding;

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
     if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }
    
    int total_text_width=0;
    TTF_SizeText(font.get(), m_lines[current_line_idx].c_str(), &total_text_width, nullptr);

    if (total_text_width < visible_width) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x > 0) {
        m_text_offset_x = 0;
    } else if (m_text_offset_x < visible_width - total_text_width ) {
        m_text_offset_x = visible_width - total_text_width;
    }
}