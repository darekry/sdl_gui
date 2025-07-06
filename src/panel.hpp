#ifndef PANEL_HPP
#define PANEL_HPP

#include "gui.hpp"

// Klasa Panel dziedzicząca po GUIElement
class Panel : public GUIElement {
public:
    // Konstruktor
    Panel(GUIManager& manager, int x, int y, int width, int height);

    // Przesłonięta metoda do renderowania
    void render(SDL_Renderer* renderer) override;
    // Przesłonięta metoda do obsługi zdarzeń
    bool handleEvent(SDL_Event& event) override;
    // Metody do ustawiania obramowania
    void setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void setBorderThickness(int thickness);
    // Metoda do ustawiania możliwości przeciągania
    void setDraggable(bool draggable);

private:
    SDL_Color m_borderColor = {0, 0, 0, 255}; // Domyślny kolor obramowania (czarny)
    int m_borderThickness = 1; // Domyślna grubość obramowania
    bool m_is_draggable = false;
    bool m_is_dragging = false;
    SDL_Point m_drag_offset;
};

#endif // PANEL_HPP