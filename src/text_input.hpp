#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP


#include "gui.hpp"
#include "sdl_deleters.hpp"
import std.compat;
class TextInput : public GUIElement {
public:
    TextInput(GUIManager& manager, int x, int y, int w, int h);
    ~TextInput() = default;

    void setText(const std::string& text);
    const std::string& getText() const;

    void setTextColor(SDL_Color color);
    void setBackgroundColor(SDL_Color color);
    void setBorderColor(SDL_Color color);
    void setOnTextChanged(std::function<void(TextInput*)> callback);
    void setOnEnterPressed(std::function<void(TextInput*)> callback);

    void setLocked(bool locked);
    bool isLocked() const;

    void render() override;
    bool handleEvent(SDL_Event& e) override;

private:
    void updateTextTexture(TextureManager& textureManager);

    std::string m_text;
    SDL_Color m_textColor;
    SDL_Color m_backgroundColor;
    SDL_Color m_borderColor;
    bool m_locked;
    bool m_active; // To indicate if the input field is currently active for typing

    std::function<void(TextInput*)> m_onTextChanged;
    std::function<void(TextInput*)> m_onEnterPressed;
};

#endif // TEXT_INPUT_HPP