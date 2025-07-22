#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "gui.hpp"
import std.compat;

// Klasa przycisku dziedzicząca po GUIElement
class Button : public GUIElement {
public:
    // Konstruktor
    Button(GUIManager& manager, int x, int y, int width, int height, SharedTexture texture = nullptr);

    // Destruktor
    ~Button() = default;

    // Typy callbacków dla zdarzeń
    using OnClickCallback = std::function<void(GUIElement*)>;
    using OnMouseOverCallback = std::function<void(GUIElement*)>;

    // Metody do przypisywania callbacków
    void setOnClickCallback(OnClickCallback callback) { m_onClick = callback; }
    void setOnMouseOverCallback(OnMouseOverCallback callback) { m_onMouseOver = callback; }

    // Metody wywołujące callbacki (publiczne na potrzeby testów/integracji)
    void triggerOnClick() { if (m_onClick) m_onClick(static_cast<GUIElement*>(this)); }
    void triggerOnMouseOver() { if (m_onMouseOver) m_onMouseOver(static_cast<GUIElement*>(this)); }
    void triggerOnRelease() {
        if (m_onClick) m_onClick(static_cast<GUIElement*>(this));
    } // Używamy m_onClick dla zdarzenia puszczenia przycisku
    // Przesłonięte metody do obsługi zdarzeń i renderowania (na razie puste)
    bool handleEvent(SDL_Event& e) override;
    void setLabel(const std::string& text, int fontSize, SDL_Color color);

protected:
    void draw() override;

private:
    OnClickCallback m_onClick;
    OnMouseOverCallback m_onMouseOver;
};

#endif // BUTTON_HPP