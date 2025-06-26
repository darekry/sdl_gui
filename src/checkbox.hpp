#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"
#include <string>
#include <functional>
#include <memory>

class Checkbox : public GUIElement {
public:
    Checkbox(int x, int y, int w, int h);
    ~Checkbox() = default;

    bool isChecked() const { return m_isChecked; }
    void setChecked(bool checked);

    void setLabel(SDL_Renderer* renderer, const std::string& text, std::shared_ptr<TTF_Font> font, SDL_Color color, TextureManager& textureManager);

    using OnChangeCallback = std::function<void(Checkbox*, bool)>;
    void setOnChange(OnChangeCallback callback) { m_onChange = callback; }

    bool handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool m_isChecked;
    SharedTexture m_labelTexture;
    OnChangeCallback m_onChange;
};

#endif // CHECKBOX_HPP