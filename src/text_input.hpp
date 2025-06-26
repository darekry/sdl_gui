#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include "gui.hpp"
#include <string>
#include <functional>
#include <string>
#include <functional>
#include <memory>

#include "sdl_deleters.hpp"
class TextInput : public GUIElement {
public:
    TextInput(int x, int y, int w, int h);
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

    void render(SDL_Renderer* renderer) override;
    bool handleEvent(SDL_Event& e) override;

private:
    void updateTextTexture(SDL_Renderer* renderer, TextureManager& textureManager);

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