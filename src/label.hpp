#ifndef LABEL_HPP
#define LABEL_HPP

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
};

#endif // LABEL_HPP