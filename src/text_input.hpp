#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP


#include "gui.hpp"
#include "style.hpp"
#include "sdl_deleters.hpp"

class TextInput : public GUIElement {
public:
    TextInput(GUIManager& manager, int x, int y, int w, int h);
    ~TextInput() = default;

    void setText(std::string_view text);
    void setText(std::string&& text);
    const std::string& getText() const;

    void setTextColor(const SDL_Color& color);
    void setBackgroundColor(const SDL_Color& color);
using GUIElement::setBorder;
    void setBorderColor(const SDL_Color& color);
    void setOnTextChanged(const std::function<void(TextInput*)>& callback);
    void setOnEnterPressed(const std::function<void(TextInput*)>& callback);

    void setLocked(bool locked);
    bool isLocked() const;

    bool handleEvent(const SDL_Event& e) override;
    void onFocusGained() override;
    void onFocusLost() override;
    const char* getComponentType() const;
protected:
    void draw(SDL_Renderer* renderer) override;

private:
    void update_text_offset();
    
    std::string m_text;
    bool m_locked;

    size_t m_cursor_pos = 0;
    int m_text_offset_x = 0;
    bool m_show_cursor = false;
    Uint32 m_cursor_blink_time = 0;

    std::function<void(TextInput*)> m_onTextChanged;
    std::function<void(TextInput*)> m_onEnterPressed;
};

#endif // TEXT_INPUT_HPP