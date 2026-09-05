#include "text_editable.hpp"
#include "gui_manager.hpp"
#include "context_menu.hpp"
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



const std::string& TextEditable::getText() const {
    return m_text;
}

void TextEditable::setOnTextChanged(const std::function<void(TextEditable*)>& callback) {
    m_onTextChanged = callback;
}

void TextEditable::setLocked(bool locked) {
    m_locked = locked;
    if (m_locked && hasKeyboardFocus()) {
        m_manager.setKeyboardFocus(nullptr);
    }
    markDirty();
}

bool TextEditable::isLocked() const {
    return m_locked;
}

bool TextEditable::copyToClipboard() {
    return handleClipboardCopy();
}

bool TextEditable::cutToClipboard() {
    return handleClipboardCut();
}

bool TextEditable::pasteFromClipboard() {
    return handleClipboardPaste();
}

void TextEditable::selectAll() {
    size_t totalChars = utf8::charCount(m_text);
    setSelection(0, totalChars);
    m_cursorPos = totalChars;
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    markDirty();
}

void TextEditable::showContextMenu(float x, float y) {
    if (!m_contextMenuEnabled) {
        return;
    }

    // Alive-guarded actions: the shared menu can outlive this widget.
    TextEditable* self = this;
    GUIManager& mgr = m_manager;

    std::vector<ContextMenuItem> items;
    items.emplace_back("Cut", [self, &mgr]() {
        if (mgr.isElementAlive(self)) self->cutToClipboard();
    }, hasSelection());
    items.emplace_back("Copy", [self, &mgr]() {
        if (mgr.isElementAlive(self)) self->copyToClipboard();
    }, hasSelection());
    items.emplace_back(true);
    items.emplace_back("Paste", [self, &mgr]() {
        if (mgr.isElementAlive(self)) self->pasteFromClipboard();
    }, SDL_HasClipboardText());
    items.emplace_back(true);
    items.emplace_back("Select All", [self, &mgr]() {
        if (mgr.isElementAlive(self)) self->selectAll();
    }, !m_text.empty());

    mgr.showContextMenu(items, x, y);
}

void TextEditable::onFocusGained() {
    GUIElement::onFocusGained();
    SDL_StartTextInput(SDL_GetRenderWindow(m_manager.getRenderer()));
    m_showCursor = true;
    m_cursorBlinkTime = SDL_GetTicks();
}

void TextEditable::onFocusLost() {
    GUIElement::onFocusLost();
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

void TextEditable::deleteSelection() {
    if (!hasSelection()) return;
    size_t startChar = std::min(m_selectionStart, m_selectionEnd);
    size_t endChar = std::max(m_selectionStart, m_selectionEnd);
    size_t startByte = utf8::charToByteIndex(m_text, startChar);
    size_t endByte = utf8::charToByteIndex(m_text, endChar);
    m_text.erase(startByte, endByte - startByte);
    m_cursorPos = startChar;
    clearSelection();
}

size_t TextEditable::charIndexAtX(std::string_view text, TTF_Font* font, int x) {
    if (x <= 0 || text.empty()) {
        return 0;
    }

    // Reusable buffer to avoid allocations in binary search
    std::string workingBuffer;
    workingBuffer.reserve(text.size());

    size_t totalChars = utf8::charCount(text);
    size_t left = 0;
    size_t right = totalChars;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        workingBuffer = utf8::substrChars(text, 0, mid);
        int text_width = 0;
        TTF_GetStringSize(font, workingBuffer.c_str(), workingBuffer.length(), &text_width, nullptr);
        if (text_width < x) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    int width_at_left = 0;
    int width_at_right = 0;
    workingBuffer = utf8::substrChars(text, 0, left);
    TTF_GetStringSize(font, workingBuffer.c_str(), workingBuffer.length(), &width_at_left, nullptr);
    if (left > 0) {
        workingBuffer = utf8::substrChars(text, 0, left - 1);
        TTF_GetStringSize(font, workingBuffer.c_str(), workingBuffer.length(), &width_at_right, nullptr);
    }

    if (left > 0 && std::abs(x - width_at_right) < std::abs(x - width_at_left)) {
        return left - 1;
    }
    return left;
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
            deleteSelection();
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
        deleteSelection();
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
        deleteSelection();
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
        deleteSelection();
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
    deleteSelection();
    size_t cursorByte = utf8::charToByteIndex(m_text, m_cursorPos);
    m_text.insert(cursorByte, text);
    m_cursorPos += utf8::charCount(text);
    updateTextOffset();
    refreshTextTexture();
    markNeedsUpdate();
    if (m_onTextChanged) m_onTextChanged(this);
}