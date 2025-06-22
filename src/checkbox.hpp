#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"
#include <string>
#include <functional>
#include <memory>

class Checkbox : public GUIElement {
public:
    Checkbox(int x, int y, int w, int h, const std::string& label = "");
    ~Checkbox();

    bool isChecked() const { return m_isChecked; }
    void setChecked(bool checked);

    void setLabel(const std::string& label);
    const std::string& getLabel() const { return m_labelText; }

    void setFont(SharedFont font);
    SharedFont getFont() const { return m_font; }

    void setTextColor(SDL_Color color);
    SDL_Color getTextColor() const { return m_textColor; }

    using OnChangeCallback = std::function<void(Checkbox*, bool)>;
    void setOnChange(OnChangeCallback callback) { m_onChange = callback; }

    bool handleEvent(SDL_Event& e) override;
    void render(SDL_Renderer* renderer) override;

private:
    bool m_isChecked;
    std::string m_labelText;
    SDL_Color m_textColor;
    SharedFont m_font;
    OnChangeCallback m_onChange;
    std::string m_renderedText;

    std::shared_ptr<SDL_Texture> m_labelTexture;
    int m_labelWidth;
    int m_labelHeight;
};

#endif // CHECKBOX_HPP