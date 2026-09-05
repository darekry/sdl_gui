#pragma once
#include "gui.hpp"
#include "gui_manager.hpp"


class TextArea : public GUIElement {
public:
    TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size);
    
    void setText(std::string_view text);
    void setText(std::string&& text);
    void setText(const char* text);
    const std::string& getText() const;

    void setWordWrap(bool enabled);
    bool getWordWrap() const;

    void setOnTextChanged(const std::function<void(TextArea*)>& callback);

    void setLocked(bool locked);
    bool isLocked() const;

    bool hasSelection() const;
    std::string getSelection() const;
    void clearSelection();
    void setSelection(size_t start, size_t end);

    // Clipboard API (byte-index based, matching TextArea cursor semantics)
    bool copyToClipboard();
    bool cutToClipboard();
    bool pasteFromClipboard();
    void selectAll();

    // Default right-click context menu (Cut/Copy/Paste/Select All, shared instance in GUIManager)
    void setContextMenuEnabled(bool enabled) { m_contextMenuEnabled = enabled; }
    [[nodiscard]] bool isContextMenuEnabled() const { return m_contextMenuEnabled; }

    bool handleEvent(const SDL_Event& e) override;
    void renderOverlay(SDL_Renderer* renderer) override;
    [[nodiscard]] const char* getComponentType() const override;
    bool canShareRenderCache() const override { return false; }
    void onFocusGained() override;
    void onFocusLost() override;
protected:
    void draw(SDL_Renderer* renderer) override;

private:
    void recalculateLines();
    void refreshTextures();
    void update_text_offset();
    size_t getLineFromPosition(size_t pos) const;
    size_t getPositionFromLineAndColumn(size_t line, size_t column) const;
    size_t getColumnFromPosition(size_t pos) const;

    // Builds Cut/Copy/Paste/Select All items and shows the shared context menu
    void showContextMenu(float x, float y);

    std::string m_text;
    std::vector<std::string> m_lines;
    bool m_wordWrap = true;
    std::string m_font_path;
    int m_font_size;
    
    std::vector<std::shared_ptr<SDL_Texture>> m_line_textures;
    bool m_needs_texture_update = true;
    int m_scroll_offset_y = 0;
    size_t m_cursorPos = 0;
    int m_text_offset_x = 0;
    Uint64 m_cursorBlinkTime = 0;
    bool m_showCursor = false;
    bool m_locked = false;
    size_t m_selectionStart = 0;
    size_t m_selectionEnd = 0;
    bool m_hasSelection = false;
    bool m_isDragging = false;
    size_t m_dragStartPos = 0;

    std::function<void(TextArea*)> m_onTextChanged;
    bool m_contextMenuEnabled = true;
};