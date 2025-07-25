#ifndef LABEL_HPP
#define LABEL_HPP

#include "gui.hpp"
import std.compat;

class Label : public GUIElement {
public:
    Label(GUIManager& manager, int x, int y, std::string_view text, int font_size, const SDL_Color& color);

    void setText(std::string_view text);

protected:
    void draw() override;

private:
    std::string m_text;
    int m_font_size;
    SDL_Color m_color;
};

#endif // LABEL_HPP