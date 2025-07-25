#ifndef PANEL_HPP
#define PANEL_HPP

#include "gui.hpp"

// Klasa Panel dziedzicząca po GUIElement
class Panel : public GUIElement {
public:
    // Konstruktor
    Panel(GUIManager& manager, int x, int y, int width, int height);

    // Przesłonięta metoda do obsługi zdarzeń
    bool handleEvent(const SDL_Event& event) override;
    // Metody do ustawiania obramowania
    void setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void setBorderThickness(int thickness);
    void setBackgroundColor(const SDL_Color& color);
    // Metoda do ustawiania możliwości przeciągania
    void setDraggable(bool draggable);


    protected:
        void draw() override;
        SDL_Color m_backgroundColor = {220, 220, 220, 255};
        SDL_Color m_foregroundColor = {100, 100, 100, 255}; // Kolor dla elementów na wierzchu, np. kciuk slidera
        SDL_Color m_borderColor = {0, 0, 0, 255};
        int m_borderWidth = 1; // Zmieniona nazwa dla spójności
        bool m_is_draggable = false;
        bool m_is_dragging = false;
        SDL_Point m_drag_offset;
    
};

#endif // PANEL_HPP