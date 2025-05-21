#ifndef TEXT_INPUT_HPP
#define TEXT_INPUT_HPP

#include "gui.hpp"
#include <string>
#include <functional>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h" // Assuming SDL_ttf will be used for fonts
#include <memory>

#include "sdl_deleters.hpp"

class TextInput : public GUIElement {
public:
    TextInput(int x, int y, int w, int h);
    ~TextInput();

    void setText(const std::string& text);
    const std::string& getText() const;

    void setTextColor(SDL_Color color);
    void setBackgroundColor(SDL_Color color);
    void setBorderColor(SDL_Color color);
    void setFont(std::shared_ptr<TTF_Font> font); // Placeholder for font management

    void setOnTextChanged(std::function<void(TextInput*)> callback);
    void setOnEnterPressed(std::function<void(TextInput*)> callback);

    void setLocked(bool locked);
    bool isLocked() const;

    void render(SDL_Renderer* renderer) override;
    void handleEvent(SDL_Event& e) override;

private:
    std::string text;
    SDL_Color textColor;
    SDL_Color backgroundColor;
    SDL_Color borderColor;
    std::shared_ptr<TTF_Font> font;
    bool locked;
    bool active; // To indicate if the input field is currently active for typing
    std::shared_ptr<SDL_Texture> m_textTexture; // Tekstura zawierająca wyrenderowany tekst
    std::string m_renderedText; // Tekst, który został ostatnio wyrenderowany

    std::function<void(TextInput*)> onTextChanged;
    std::function<void(TextInput*)> onEnterPressed;
};

#endif // TEXT_INPUT_HPP