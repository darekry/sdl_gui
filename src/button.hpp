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

    // Label text: retargets (or lazily creates) the centered child label.
    // This replaces the manual "Button + Label child" pattern from examples.
    void setText(std::string_view text);
    [[nodiscard]] std::string getText() const;
    
    // Overridden methods
    bool handleSelf(const SDL_Event& e) override;
    ComponentType getComponentTypeId() const override;

protected:
    void layoutChildren() override;

private:
    OnClickCallback m_onClick;
    OnMouseOverCallback m_onMouseOver;
    class Label* m_label = nullptr;
};
