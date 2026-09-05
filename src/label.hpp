#pragma once
#include "gui.hpp"


class Label : public GUIElement {
public:
    Label(GUIManager& manager, int x, int y, std::string_view text, int font_size = -1);

    void setText(std::string_view text);
    const std::string& getText() const { return m_text; }

    ComponentType getComponentTypeId() const override { return ComponentType::Label; }

public:
    void draw(SDL_Renderer* renderer) override;

    uint64_t getRenderCacheKeySuffix() const override {
        uint64_t h = std::hash<std::string_view>{}(m_text);
        return h ^ (static_cast<uint64_t>(m_font_size) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2));
    }

private:
    void recalculateSize();
    void updateLines();
    std::string m_text;
    std::vector<std::string> m_lines;
    int m_font_size;
};
