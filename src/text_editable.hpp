#pragma once

#include "gui.hpp"
#include "sdl_deleters.hpp"

/**
 * Abstract base class for text editing components (TextInput, TextArea).
 * Contains common selection state, methods, and clipboard handling.
 */
class TextEditable : public GUIElement {
public:
    TextEditable(GUIManager& manager, int x, int y, int w, int h);
    ~TextEditable() = default;

    // Selection methods (common implementation, char-index = UTF-8 characters)
    virtual bool hasSelection() const;
    virtual std::string getSelection() const;
    virtual void clearSelection();
    virtual void setSelection(size_t start, size_t end);
    
    // Text access
    virtual void setText(std::string_view text);
    const std::string& getText() const;
    
    // Callbacks
    void setOnTextChanged(const std::function<void(TextEditable*)>& callback);

    // Public clipboard API (keyboard shortcuts and the default context menu both use it)
    virtual bool copyToClipboard();
    virtual bool cutToClipboard();
    virtual bool pasteFromClipboard();
    virtual void selectAll();

    // Default right-click context menu (Cut/Copy/Paste/Select All, shared instance in GUIManager)
    void setContextMenuEnabled(bool enabled) { m_contextMenuEnabled = enabled; }
    [[nodiscard]] bool isContextMenuEnabled() const { return m_contextMenuEnabled; }

    // Locked state (read-only). Shared by TextInput and TextArea.
    virtual void setLocked(bool locked);
    virtual bool isLocked() const;

    // Focus handling
    void onFocusGained() override;
    void onFocusLost() override;

    // Binary search: char index at pixel x within text (shared by TextInput/TextArea)
    static size_t charIndexAtX(std::string_view text, TTF_Font* font, int x);

    bool canShareRenderCache() const override { return false; }

protected:
    // Text content
    std::string m_text;
    size_t m_cursorPos = 0;
    
    // Selection state
    size_t m_selectionStart = 0;
    size_t m_selectionEnd = 0;
    bool m_hasSelection = false;
    
    // Mouse drag state
    bool m_isDragging = false;
    size_t m_dragStartPos = 0;
    
    // Cursor blink
    Uint64 m_cursorBlinkTime = 0;
    bool m_showCursor = false;
    
    // Callback
    std::function<void(TextEditable*)> m_onTextChanged;

    // Locked state
    bool m_locked = false;

    // Default context menu toggle
    bool m_contextMenuEnabled = true;
    
    // Helper methods for subclasses
    void updateCursorBlink();
    void resetCursorBlink();

    // Builds the standard Cut/Copy/Paste/Select All items and shows the shared
    // context menu at the given window position (no-op when disabled).
    void showContextMenu(float x, float y);
    // Clipboard operations (called from handleEvent)
    bool handleClipboardCopy();
    bool handleClipboardPaste();
    bool handleClipboardCut();
    
    // Delete/Backspace with selection
    bool handleDeleteWithSelection();
    bool handleBackspaceWithSelection();
    
    // Typing with selection replacement
    void handleTextInputWithSelection(const char* text);
    
    // Remove the selected range and place the cursor at its start
    void deleteSelection();
    
    // Virtual methods for subclass-specific behavior
    virtual void updateTextOffset() = 0;
    virtual void refreshTextTexture() = 0;
    virtual void markNeedsUpdate() = 0;
};