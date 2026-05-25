#include "text_editable.hpp"
#include "gui_manager.hpp"

TextEditable::TextEditable(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h) {
    setCanGetKeyboardFocus(true);
    markDirty();
}

void TextEditable::setText(std::string_view newText) {
    if (m_text != newText) {
        m_text = newText;
        m_cursorPos = std::min(m_cursorPos, m_text.length());
        updateTextOffset();
        refreshTextTexture();
        markNeedsUpdate();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

void TextEditable::setText(std::string&& newText) {
    if (m_text != newText) {
        m_text = std::move(newText);
        m_cursorPos = std::min(m_cursorPos, m_text.length());
        updateTextOffset();
        refreshTextTexture();
        markNeedsUpdate();
        if (m_onTextChanged) {
            m_onTextChanged(this);
        }
    }
}

const std::string& TextEditable::getText() const {
    return m_text;
}

void TextEditable::setOnTextChanged(const std::function<void(TextEditable*)>& callback) {
    m_onTextChanged = callback;
}

void TextEditable::onFocusGained() {
    SDL_StartTextInput();
    m_showCursor = true;
    m_cursorBlinkTime = SDL_GetTicks();
}

void TextEditable::onFocusLost() {
    SDL_StopTextInput();
    m_showCursor = false;
}

bool TextEditable::hasSelection() const {
    return m_hasSelection && m_selectionStart != m_selectionEnd;
}

std::string TextEditable::getSelection() const {
    if (!hasSelection()) return "";
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    return m_text.substr(start, end - start);
}

void TextEditable::clearSelection() {
    m_hasSelection = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

void TextEditable::setSelection(size_t start, size_t end) {
    m_selectionStart = std::min(start, m_text.length());
    m_selectionEnd = std::min(end, m_text.length());
    m_hasSelection = (m_selectionStart != m_selectionEnd);
}

void TextEditable::updateCursorBlink() {
    if (SDL_GetTicks() - m_cursorBlinkTime > 500) {
        m_showCursor = !m_showCursor;
        m_cursorBlinkTime = SDL_GetTicks();
    }
}

void TextEditable::resetCursorBlink() {
    m_showCursor = true;
    m_cursorBlinkTime = SDL_GetTicks();
}

bool TextEditable::handleClipboardCopy() {
    if (hasSelection()) {
        SDL_SetClipboardText(getSelection().c_str());
        return true;
    }
    return false;
}

bool TextEditable::handleClipboardPaste() {
    if (SDL_HasClipboardText()) {
        char* clipboard = SDL_GetClipboardText();
        if (clipboard) {
            // Replace selection if exists
            if (hasSelection()) {
                size_t start = std::min(m_selectionStart, m_selectionEnd);
                size_t len = std::max(m_selectionStart, m_selectionEnd) - start;
                m_text.erase(start, len);
                m_cursorPos = start;
                clearSelection();
            }
            size_t paste_len = strlen(clipboard);
            m_text.insert(m_cursorPos, clipboard);
            m_cursorPos += paste_len;
            updateTextOffset();
            refreshTextTexture();
            markNeedsUpdate();
            if (m_onTextChanged) m_onTextChanged(this);
            SDL_free(clipboard);
            return true;
        }
    }
    return false;
}

bool TextEditable::handleClipboardCut() {
    if (hasSelection()) {
        SDL_SetClipboardText(getSelection().c_str());
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t len = std::max(m_selectionStart, m_selectionEnd) - start;
        m_text.erase(start, len);
        m_cursorPos = start;
        clearSelection();
        updateTextOffset();
        refreshTextTexture();
        markNeedsUpdate();
        if (m_onTextChanged) m_onTextChanged(this);
        return true;
    }
    return false;
}

bool TextEditable::handleDeleteWithSelection() {
    if (hasSelection()) {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t len = std::max(m_selectionStart, m_selectionEnd) - start;
        m_text.erase(start, len);
        m_cursorPos = start;
        clearSelection();
    } else if (m_cursorPos < m_text.length()) {
        m_text.erase(m_cursorPos, 1);
    } else {
        return false;
    }
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    if (m_onTextChanged) m_onTextChanged(this);
    return true;
}

bool TextEditable::handleBackspaceWithSelection() {
    if (hasSelection()) {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t len = std::max(m_selectionStart, m_selectionEnd) - start;
        m_text.erase(start, len);
        m_cursorPos = start;
        clearSelection();
    } else if (m_cursorPos > 0) {
        m_text.erase(m_cursorPos - 1, 1);
        m_cursorPos--;
    } else {
        return false;
    }
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    if (m_onTextChanged) m_onTextChanged(this);
    return true;
}

void TextEditable::handleTextInputWithSelection(const char* text) {
    // Replace selection if exists
    if (hasSelection()) {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t len = std::max(m_selectionStart, m_selectionEnd) - start;
        m_text.erase(start, len);
        m_cursorPos = start;
        clearSelection();
    }
    m_text.insert(m_cursorPos, text);
    m_cursorPos += strlen(text);
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    if (m_onTextChanged) m_onTextChanged(this);
}