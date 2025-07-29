#include "text_area.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include <algorithm>

#include <string>
#include <sstream>
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

void TextArea::setWordWrap(bool enabled) {
    m_wordWrap = enabled;
    m_needs_texture_update = true;
}

bool TextArea::getWordWrap() const {
    return m_wordWrap;
}

void TextArea::draw() {
    // Rysowanie tła i ramki jest teraz obsługiwane przez styl
    GUIElement::draw();

    if (m_needs_texture_update) {
        recalculateLines();
        refreshTextures();
        m_needs_texture_update = false;
    }

    auto* renderer = m_manager.getRenderer();
    auto bounds = SDL_Rect{getAbsolutePosition().x, getAbsolutePosition().y, m_width, m_height};

    // Tekst
    // Tekst
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
     if (!font) {
        return;
    }

    auto clip_rect = SDL_Rect{bounds.x + 2, bounds.y, bounds.w - 4, bounds.h};
    SDL_RenderSetClipRect(renderer, &clip_rect);
    auto yOffset = 0;
    for (const auto& texture : m_line_textures) {
        if (texture) {
            auto destRect = SDL_Rect{bounds.x + 2 + m_text_offset_x, bounds.y + yOffset + 2, 0, 0};
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

    auto eventHandled = false;
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

    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    auto currentLine = std::string{};
    auto startPos = 0uz;
    auto endPos = 0uz;

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
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    const auto style = getResolvedStyle();
    SDL_Color color = style.textColor.value_or(SDL_Color{0,0,0,255});

    for (const auto& line : m_lines) {
        if (line.empty()) {
            m_line_textures.push_back(nullptr);
        } else {
            auto lineTexture = m_manager.getTextureManager().createTextureFromText(line, font, color);
            m_line_textures.push_back(lineTexture);
        }
    }
}

void TextArea::renderCursor() {
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    auto currentLineIndex = 0;
    auto posInLines = 0uz;
    auto tempPos = 0uz;

    for(size_t i = 0; i < m_lines.size(); ++i) {
        const auto lineLengthWithNewline = m_lines[i].length() + (i < m_lines.size() - 1 ? 1 : 0);
        if (m_cursorPos >= tempPos && m_cursorPos <= tempPos + lineLengthWithNewline) {
            currentLineIndex = i;
            posInLines = m_cursorPos - tempPos;
            break;
        }
        tempPos += lineLengthWithNewline;
    }

    auto x = 0, y = 0;
    const auto& lineContent = m_lines[currentLineIndex];
    if (posInLines > lineContent.length()) posInLines = lineContent.length();

    auto textBeforeCursor = lineContent.substr(0, posInLines);
    TTF_SizeText(font.get(), textBeforeCursor.c_str(), &x, nullptr);

    y = currentLineIndex * TTF_FontHeight(font.get());

    auto cursorRect = SDL_Rect{ getAbsolutePosition().x + 2 + x + m_text_offset_x, getAbsolutePosition().y + 2 + y, 2, TTF_FontHeight(font.get()) };
    const auto style = getResolvedStyle();
    SDL_Color color = style.textColor.value_or(SDL_Color{0,0,0,255});
    SDL_SetRenderDrawColor(m_manager.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_manager.getRenderer(), &cursorRect);
}


void TextArea::update_text_offset() {
    auto font = m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    if (!font) return;

    auto current_line_idx = 0;
    auto pos_in_lines = 0uz;
    auto temp_pos = 0uz;

    for (size_t i = 0; i < m_lines.size(); ++i) {
        auto line_len = m_lines[i].length() + (i < m_lines.size() - 1 ? 1 : 0);
        if (m_cursorPos >= temp_pos && m_cursorPos <= temp_pos + line_len) {
            current_line_idx = i;
            pos_in_lines = m_cursorPos - temp_pos;
            break;
        }
        temp_pos += line_len;
    }
    
    auto text_before_cursor = m_lines[current_line_idx].substr(0, pos_in_lines);
    auto cursor_pos_x=0;
    TTF_SizeText(font.get(), text_before_cursor.c_str(), &cursor_pos_x, nullptr);


    auto padding = 2;
    auto visible_width = getWidth() - 2 * padding;

    if (cursor_pos_x + m_text_offset_x > visible_width) {
        m_text_offset_x = visible_width - cursor_pos_x;
    }
     if (cursor_pos_x + m_text_offset_x < 0) {
        m_text_offset_x = -cursor_pos_x;
    }
    
    auto total_text_width=0;
    TTF_SizeText(font.get(), m_lines[current_line_idx].c_str(), &total_text_width, nullptr);

    if (total_text_width <= visible_width) {
        m_text_offset_x = 0;
    } else {
        m_text_offset_x = std::clamp(m_text_offset_x, visible_width - total_text_width, 0);
    }
}