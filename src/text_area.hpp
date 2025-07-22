#ifndef TEXT_AREA_HPP
#define TEXT_AREA_HPP

#include "gui.hpp"
#include "gui_manager.hpp"
import std.compat;

class TextArea : public GUIElement {
public:
    TextArea(GUIManager& manager, int x, int y, int w, int h, const std::string& font_path, int font_size);
    
    void setText(const std::string& text);
    const std::string& getText() const;

    void setTextColor(SDL_Color color);
    void setWordWrap(bool enabled);
    bool getWordWrap() const;

    bool handleEvent(SDL_Event& e) override;

protected:
    void draw() override;

private:
    void recalculateLines();
    void renderCursor();
    void refreshTextures();
    void update_text_offset();

    std::string m_text;
    std::vector<std::string> m_lines;
    bool m_wordWrap = true;
    SDL_Color m_textColor = {0, 0, 0, 255};
    std::string m_font_path;
    int m_font_size;
    
    std::vector<std::shared_ptr<SDL_Texture>> m_line_textures;
    bool m_needs_texture_update = true;
// Do obsługi kursora i wprowadzania tekstu
size_t m_cursorPos = 0; // Pozycja kursora w m_text
int m_text_offset_x = 0;
Uint32 m_cursorBlinkTime = 0;
bool m_showCursor = false;
};

#endif // TEXT_AREA_HPP