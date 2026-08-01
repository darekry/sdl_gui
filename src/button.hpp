#pragma once
#include "gui.hpp"

// Klasa przycisku dziedzicząca po GUIElement
class Button : public GUIElement {
public:
    // Konstruktor
    // Button(GUIManager& manager, int x, int y, int width, int height);
     Button(GUIManager& manager, int x, int y, int width, int height, std::string_view label = "");
    // Destruktor
    ~Button() = default;

    // Typy callbacków dla zdarzeń
    using OnClickCallback = std::function<void(GUIElement*)>;
    using OnMouseOverCallback = std::function<void(GUIElement*)>;

    // Metody do przypisywania callbacków
    void setOnClickCallback(OnClickCallback callback);
    void setOnMouseOverCallback(OnMouseOverCallback callback);
    
    // Przesłonięte metody
    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:

private:
    OnClickCallback m_onClick;
    OnMouseOverCallback m_onMouseOver;
};
