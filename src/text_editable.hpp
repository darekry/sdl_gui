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

    // Selection methods (common implementation)
    bool hasSelection() const;
    std::string getSelection() const;
    void clearSelection();
    void setSelection(size_t start, size_t end);
    
    // Text access
    virtual void setText(std::string_view text);
    const std::string& getText() const;
    
    // Callbacks
    void setOnTextChanged(const std::function<void(TextEditable*)>& callback);
    
    // Focus handling
    void onFocusGained() override;
    void onFocusLost() override;
    
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
    
    // Helper methods for subclasses
    void updateCursorBlink();
    void resetCursorBlink();
    
    // Clipboard operations (called from handleEvent)
    bool handleClipboardCopy();
    bool handleClipboardPaste();
    bool handleClipboardCut();
    
    // Delete/Backspace with selection
    bool handleDeleteWithSelection();
    bool handleBackspaceWithSelection();
    
    // Typing with selection replacement
    void handleTextInputWithSelection(const char* text);
    
    // Virtual methods for subclass-specific behavior
    virtual void updateTextOffset() = 0;
    virtual void refreshTextTexture() = 0;
    virtual void markNeedsUpdate() = 0;
};