#include "text_area.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

import std.compat;

TextArea::TextArea(GUIManager& manager, int x, int y, int w, int h, const std::string& font_path, int font_size)
    : GUIElement(manager, x, y, w, h), m_font_path(font_path), m_font_size(font_size) {
    
    // Inicjalizacja, jeśli potrzebna
    m_manager.getFontManager().loadFont(m_font_path, m_font_size);
    m_needs_texture_update = true;
}

void TextArea::setText(const std::string& text) {
    m_text = text;
    m_cursorPos = std::min(m_cursorPos, m_text.length());
    m_needs_texture_update = true;
}

const std::string& TextArea::getText() const {
    return m_text;
}

void TextArea::setTextColor(SDL_Color color) {
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
void TextArea::render() {
    if (!m_visible) {
        return;
    }

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
        return; // Nie można renderować bez czcionki
    }

    int yOffset = 0;
    for (const auto& texture : m_line_textures) {
        if (texture) {
            SDL_Rect destRect = { bounds.x + 2, bounds.y + yOffset + 2, 0, 0 };
            SDL_QueryTexture(texture.get(), nullptr, nullptr, &destRect.w, &destRect.h);
            SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
        }
        yOffset += TTF_FontHeight(font.get());
    }

    if (m_showCursor) {
        renderCursor();
    }
}

bool TextArea::handleEvent(SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    bool eventHandled = false;
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(e.button.x, e.button.y)) {
            m_isHovered = true; // Użyjemy m_isHovered do decydowania o miganiu kursora
            SDL_StartTextInput();
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
            // TODO: Ustawienie pozycji kursora na podstawie kliknięcia
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
        eventHandled = true;
    } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && m_cursorPos > 0) {
            m_text.erase(m_cursorPos - 1, 1);
            m_cursorPos--;
            m_needs_texture_update = true;
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            m_text.insert(m_cursorPos, "\n");
            m_cursorPos++;
            m_needs_texture_update = true;
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_LEFT && m_cursorPos > 0) {
            m_cursorPos--;
            eventHandled = true;
        } else if (e.key.keysym.sym == SDLK_RIGHT && m_cursorPos < m_text.length()) {
            m_cursorPos++;
            eventHandled = true;
        }
        if (eventHandled) {
            m_showCursor = true;
            m_cursorBlinkTime = SDL_GetTicks();
        }
    }

    if (m_isHovered) {
        if (m_isHovered) {
            // Miganie kursora
            if (SDL_GetTicks() - m_cursorBlinkTime > 500) {
                m_showCursor = !m_showCursor;
                m_cursorBlinkTime = SDL_GetTicks();
            }
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
            m_line_textures.push_back(nullptr); // Pusta tekstura dla pustej linii
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
        size_t lineLengthWithNewline = m_lines[i].length() + (m_wordWrap ? 1 : 0); // Add 1 for the space that was used to wrap, or for a newline
        if (m_text.substr(tempPos + m_lines[i].length(), 1) == "\n") {
            lineLengthWithNewline = m_lines[i].length() + 1;
        }
    
    
        if (m_cursorPos >= tempPos && m_cursorPos < tempPos + lineLengthWithNewline) {
            currentLineIndex = i;
            posInLines = m_cursorPos - tempPos;
            break;
        }
        tempPos += lineLengthWithNewline;
    }
    // Handle cursor at the very end of the text
    if (m_cursorPos == m_text.length() && m_text.length() > 0) {
        bool cursorPlaced = false;
        if (!m_lines.empty()) {
            const auto& lastLine = m_lines.back();
            if (m_text.ends_with(lastLine)) {
                currentLineIndex = m_lines.size() - 1;
                posInLines = lastLine.length();
                cursorPlaced = true;
            }
        }
        if (!cursorPlaced) { // If text ends with newline, cursor should be on a new line
            currentLineIndex = m_lines.size();
            m_lines.push_back(""); // Add temporary empty line for cursor calculation
            posInLines = 0;
        }
    }


    int x, y;
    std::string lineContent = m_lines[currentLineIndex];
    if (posInLines > lineContent.length()) posInLines = lineContent.length();

    std::string textBeforeCursor = lineContent.substr(0, posInLines);
    TTF_SizeText(font.get(), textBeforeCursor.c_str(), &x, nullptr);

    y = currentLineIndex * TTF_FontHeight(font.get());

    SDL_Rect cursorRect = { getAbsolutePosition().x + 2 + x, getAbsolutePosition().y + 2 + y, 1, TTF_FontHeight(font.get()) };
    SDL_SetRenderDrawColor(m_manager.getRenderer(), m_textColor.r, m_textColor.g, m_textColor.b, m_textColor.a);
    SDL_RenderFillRect(m_manager.getRenderer(), &cursorRect);
}