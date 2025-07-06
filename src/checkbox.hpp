#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"
#include <string>
#include <functional>
#include <memory>

class Checkbox : public GUIElement {
public:
    Checkbox(GUIManager& manager, int x, int y, int w, int h);
    ~Checkbox() = default;

    bool isChecked() const { return m_isChecked; }
    void setChecked(bool checked);

void setLabel(const std::string& text, int fontSize, SDL_Color color);

using OnChangeCallback = std::function<void(Checkbox*, bool)>;
void setOnChange(OnChangeCallback callback) { m_onChange = callback; }
    bool handleEvent(SDL_Event& e) override;
    void render() override;

private:
    bool m_isChecked;
    SharedTexture m_labelTexture;
    OnChangeCallback m_onChange;
};

#endif // CHECKBOX_HPP