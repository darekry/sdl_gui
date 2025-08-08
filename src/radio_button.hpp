#ifndef RADIOBUTTON_HPP
#define RADIOBUTTON_HPP

#include "gui.hpp"

class RadioGroup;

class RadioButton : public GUIElement {
public:
    RadioButton(GUIManager& manager, int x, int y, int w, int h); // Uproszczony konstruktor
    ~RadioButton() = default;

    bool isSelected() const;
    void setSelected(bool selected);

    using OnChangeCallback = std::function<void(RadioButton*, bool)>;
    void setOnChange(OnChangeCallback callback);

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

private:
    bool m_isSelected = false;
    OnChangeCallback m_onChange;
};

#endif // RADIOBUTTON_HPP