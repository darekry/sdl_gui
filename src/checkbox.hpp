#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "gui.hpp"

class Checkbox : public GUIElement {
public:
    Checkbox(GUIManager& manager, int x, int y, int w, int h);
    ~Checkbox() = default;

    bool isChecked() const;
    void setChecked(bool checked);

    using OnChangeCallback = std::function<void(Checkbox*, bool)>;
    void setOnChange(OnChangeCallback callback);

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw() override;

private:
    bool m_isChecked;
    OnChangeCallback m_onChange;
};

#endif // CHECKBOX_HPP