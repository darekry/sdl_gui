#ifndef LABEL_HPP
#define LABEL_HPP

#include "gui.hpp"
#include "texture_manager.hpp"
import std.compat;

class Label : public GUIElement {
public:
    Label(GUIManager& manager, int x, int y, std::string_view text, int font_size = -1);

    void setText(std::string_view text);

protected:
    void draw() override;
    void updateTexture();

private:
    std::string m_text;
    int m_font_size;
    SharedTexture m_texture;
};

#endif // LABEL_HPP