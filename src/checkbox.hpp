#pragma once
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
    ComponentType getComponentTypeId() const override;

public:
    void draw(SDL_Renderer* renderer) override;

    bool canShareRenderCache() const override { return false; }
private:
    bool m_isChecked;
    OnChangeCallback m_onChange;
};
