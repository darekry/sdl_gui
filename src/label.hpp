#pragma once
#include "gui.hpp"
#include "texture_manager.hpp"


class Label : public GUIElement {
public:
    Label(GUIManager& manager, int x, int y, std::string_view text, int font_size = -1);

    void setText(std::string_view text);

public:
    void draw(SDL_Renderer* renderer) override;

private:
    void recalculateSize();
    std::string m_text;
    int m_font_size;
    SharedTexture m_cachedTextTexture;
    std::string m_cachedTextContent;
    SDL_Color m_cachedTextColor{};
    int m_cachedFontSize = -1;
    bool m_textTextureDirty = true;
};
