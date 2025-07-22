#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"

import std.compat;

class Checkbox : public GUIElement {
public:
    Checkbox(GUIManager& manager, int x, int y, int w, int h);
    ~Checkbox() = default;

    bool isChecked() const { return m_isChecked; }
    void setChecked(bool checked);

void setLabel(std::string_view text, int fontSize, const SDL_Color& color);

using OnChangeCallback = std::function<void(Checkbox*, bool)>;
void setOnChange(OnChangeCallback callback) { m_onChange = callback; }
    bool handleEvent(const SDL_Event& e) override;

protected:
    void draw() override;

private:
    bool m_isChecked;
    SharedTexture m_labelTexture;
    OnChangeCallback m_onChange;
};

#endif // CHECKBOX_HPP