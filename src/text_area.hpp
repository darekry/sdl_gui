#pragma once
#include "text_editable.hpp"

/**
 * Multi-line text editing area. Inherits the shared char-index (UTF-8)
 * text model from TextEditable (text, cursor, selection, clipboard,
 * focus, context menu, locked state) and adds line layout (wrapping,
 * scrolling, multi-line cursor navigation).
 *
 * All positions (cursor, selection) are UTF-8 character indices,
 * consistent with TextInput.
 */
class TextArea : public TextEditable {
public:
    TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size);

    // Text access (override to keep line layout in sync; char-index semantics)
    void setText(std::string_view text) override;
    void setText(std::string&& text);
    void setText(const char* text);

    void setWordWrap(bool enabled);
    bool getWordWrap() const;

    // TextArea-specific callback (argument is TextArea*; forwards base callback)
    void setOnTextChanged(const std::function<void(TextArea*)>& callback);

    // Locked state (override shared implementation: also hides cursor/hover)
    void setLocked(bool locked) override;
    bool isLocked() const override;

    bool handleEvent(const SDL_Event& e) override;
    void renderOverlay(SDL_Renderer* renderer) override;
    [[nodiscard]] ComponentType getComponentTypeId() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

    // TextEditable virtuals (multiline versions)
    void updateTextOffset() override;
    void refreshTextTexture() override;
    void markNeedsUpdate() override;

private:
    void recalculateLines();
    void refreshTextures();
    // All helpers below operate on char indices (UTF-8 characters, not bytes)
    size_t getLineFromPosition(size_t charPos) const;
    size_t getPositionFromLineAndColumn(size_t line, size_t columnChars) const;
    size_t getColumnFromPosition(size_t charPos) const;
    // Char index within the whole document from (row, char-in-line)
    size_t charPosFromRowColumn(size_t row, size_t columnChars) const;
    // Byte offset of a char column inside a single line (for measurement)
    static size_t lineByteOffset(std::string_view line, size_t columnChars);

    std::vector<std::string> m_lines;
    bool m_wordWrap = true;
    std::string m_font_path;
    int m_font_size;

    std::vector<std::shared_ptr<SDL_Texture>> m_line_textures;
    bool m_needs_texture_update = true;
    int m_scroll_offset_y = 0;
    int m_text_offset_x = 0;

    std::function<void(TextArea*)> m_onTextChanged;
};
