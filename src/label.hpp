#pragma once
#include "gui.hpp"

enum class TextHAlign { Left = 0, Center = 1, Right = 2 };
enum class TextVAlign { Top = 0, Middle = 1, Bottom = 2 };

class Label : public GUIElement {
public:
    Label(GUIManager& manager, int x, int y, std::string_view text, int font_size = -1);

    void setText(std::string_view text);
    const std::string& getText() const { return m_text; }

    // Alignment inside m_width/m_height. Auto-sized labels are unaffected
    // (their size equals the text block), manually sized ones are laid out
    // inside the box. Setters only markDirty() — never re-run recalculateSize.
    void setAlignment(TextHAlign h, TextVAlign v) {
        m_h_align = h;
        m_v_align = v;
        markDirty();
    }
    void setHAlign(TextHAlign h) { m_h_align = h; markDirty(); }
    void setVAlign(TextVAlign v) { m_v_align = v; markDirty(); }
    [[nodiscard]] TextHAlign getHAlign() const { return m_h_align; }
    [[nodiscard]] TextVAlign getVAlign() const { return m_v_align; }

    // -1 = inherit the theme font size. Re-runs the auto-size pass.
    void setFontSize(int font_size);
    [[nodiscard]] int getFontSize() const { return m_font_size; }

    ComponentType getComponentTypeId() const override { return ComponentType::Label; }

public:
    void draw(SDL_Renderer* renderer) override;

    uint64_t getRenderCacheKeySuffix() const override {
        uint64_t h = std::hash<std::string_view>{}(m_text);
        h ^= (static_cast<uint64_t>(m_font_size) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
        h ^= (static_cast<uint64_t>(m_h_align) << 1) ^ (static_cast<uint64_t>(m_v_align) << 3);
        return h;
    }

private:
    void recalculateSize();
    void updateLines();
    std::string m_text;
    std::vector<std::string> m_lines;
    int m_font_size;
    TextHAlign m_h_align = TextHAlign::Left;
    TextVAlign m_v_align = TextVAlign::Top;
};
