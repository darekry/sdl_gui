#pragma once
#include "gui.hpp"

class Button : public GUIElement {
public:
     Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label = "");
    ~Button() = default;

    // Callback types for events
    using OnClickCallback = std::function<void(GUIElement*)>;
    using OnMouseOverCallback = std::function<void(GUIElement*)>;

    // Methods for assigning callbacks
    void setOnClickCallback(OnClickCallback callback);
    void setOnMouseOverCallback(OnMouseOverCallback callback);
    
    // Overridden methods
    bool handleEvent(const SDL_Event& e) override;
    ComponentType getComponentTypeId() const override;

protected:
    void layoutChildren() override;

private:
    OnClickCallback m_onClick;
    OnMouseOverCallback m_onMouseOver;
    class Label* m_label = nullptr;
};
