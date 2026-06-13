#include "text_editable.hpp"
#include "gui_manager.hpp"
#include "utf8_utils.hpp"

TextEditable::TextEditable(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h) {
    setCanGetKeyboardFocus(true);
    markDirty();
}

void TextEditable::setText(std::string_view newText) {
    if (m_text != newText) {
        m_text = newText;
        m_cursorPos = std::min(m_cursorPos, utf8::charCount(m_text));
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
        m_cursorPos = std::min(m_cursorPos, utf8::charCount(m_text));
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
    SDL_StartTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
    m_showCursor = true;
    m_cursorBlinkTime = SDL_GetTicks();
}

void TextEditable::onFocusLost() {
    SDL_StopTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
    m_showCursor = false;
}

bool TextEditable::hasSelection() const {
    return m_hasSelection && m_selectionStart != m_selectionEnd;
}

std::string TextEditable::getSelection() const {
    if (!hasSelection()) return "";
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    return utf8::substrChars(m_text, start, end - start);
}

void TextEditable::clearSelection() {
    m_hasSelection = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

void TextEditable::setSelection(size_t start, size_t end) {
    size_t totalChars = utf8::charCount(m_text);
    m_selectionStart = std::min(start, totalChars);
    m_selectionEnd = std::min(end, totalChars);
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
            if (hasSelection()) {
                size_t startChar = std::min(m_selectionStart, m_selectionEnd);
                size_t endChar = std::max(m_selectionStart, m_selectionEnd);
                size_t startByte = utf8::charToByteIndex(m_text, startChar);
                size_t endByte = utf8::charToByteIndex(m_text, endChar);
                m_text.erase(startByte, endByte - startByte);
                m_cursorPos = startChar;
                clearSelection();
            }
            size_t cursorByte = utf8::charToByteIndex(m_text, m_cursorPos);
            m_text.insert(cursorByte, clipboard);
            m_cursorPos += utf8::charCount(clipboard);
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
        size_t startChar = std::min(m_selectionStart, m_selectionEnd);
        size_t endChar = std::max(m_selectionStart, m_selectionEnd);
        size_t startByte = utf8::charToByteIndex(m_text, startChar);
        size_t endByte = utf8::charToByteIndex(m_text, endChar);
        m_text.erase(startByte, endByte - startByte);
        m_cursorPos = startChar;
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
        size_t startChar = std::min(m_selectionStart, m_selectionEnd);
        size_t endChar = std::max(m_selectionStart, m_selectionEnd);
        size_t startByte = utf8::charToByteIndex(m_text, startChar);
        size_t endByte = utf8::charToByteIndex(m_text, endChar);
        m_text.erase(startByte, endByte - startByte);
        m_cursorPos = startChar;
        clearSelection();
    } else if (m_cursorPos < utf8::charCount(m_text)) {
        size_t cursorByte = utf8::charToByteIndex(m_text, m_cursorPos);
        size_t charLen = utf8::charByteLength(static_cast<unsigned char>(m_text[cursorByte]));
        m_text.erase(cursorByte, charLen);
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
        size_t startChar = std::min(m_selectionStart, m_selectionEnd);
        size_t endChar = std::max(m_selectionStart, m_selectionEnd);
        size_t startByte = utf8::charToByteIndex(m_text, startChar);
        size_t endByte = utf8::charToByteIndex(m_text, endChar);
        m_text.erase(startByte, endByte - startByte);
        m_cursorPos = startChar;
        clearSelection();
    } else if (m_cursorPos > 0) {
        size_t prevCharPos = m_cursorPos - 1;
        size_t prevBytePos = utf8::charToByteIndex(m_text, prevCharPos);
        size_t cursorByte = utf8::charToByteIndex(m_text, m_cursorPos);
        m_text.erase(prevBytePos, cursorByte - prevBytePos);
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
    if (hasSelection()) {
        size_t startChar = std::min(m_selectionStart, m_selectionEnd);
        size_t endChar = std::max(m_selectionStart, m_selectionEnd);
        size_t startByte = utf8::charToByteIndex(m_text, startChar);
        size_t endByte = utf8::charToByteIndex(m_text, endChar);
        m_text.erase(startByte, endByte - startByte);
        m_cursorPos = startChar;
        clearSelection();
    }
    size_t cursorByte = utf8::charToByteIndex(m_text, m_cursorPos);
    m_text.insert(cursorByte, text);
    m_cursorPos += utf8::charCount(text);
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    if (m_onTextChanged) m_onTextChanged(this);
}