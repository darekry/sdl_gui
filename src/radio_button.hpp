#ifndef RADIOBUTTON_HPP
#define RADIOBUTTON_HPP

#include "gui.hpp"
#include "label.hpp"
import std.compat;

class RadioButton : public GUIElement {
public:
    RadioButton(GUIManager& manager, int x, int y, std::string_view text, int fontSize = 16);
    RadioButton(GUIManager& manager, int x, int y, SharedTexture texture);

    // Destruktor
    ~RadioButton() = default;

    bool isSelected() const;
    void setSelected(bool selected);
    Label* getLabel() const;

    using OnChangeCallback = std::function<void(RadioButton*, bool)>;
    void setOnChange(OnChangeCallback callback);

    void setNormalTexture(SharedTexture texture);
    void setHoverTexture(SharedTexture texture);
    void setSelectedTexture(SharedTexture texture);
    void setSelectedHoverTexture(SharedTexture texture);

    bool handleEvent(const SDL_Event& e) override;

protected:
    void draw() override;

private:
    void init();
    bool m_isSelected = false;
    OnChangeCallback m_onChange;

    SharedTexture m_tex_normal;
    SharedTexture m_tex_hover;
    SharedTexture m_tex_selected;
    SharedTexture m_tex_selected_hover;

    Label* m_label = nullptr;
};

#endif // RADIOBUTTON_HPP