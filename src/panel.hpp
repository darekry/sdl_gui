#ifndef PANEL_HPP
#define PANEL_HPP

#include "gui.hpp"

// Klasa Panel dziedzicząca po GUIElement
class Panel : public GUIElement {
public:
    // Konstruktor
    Panel(int x, int y, int width, int height);

    // Przesłonięta metoda do renderowania
    void render(SDL_Renderer* renderer) override;

    // Metody do ustawiania obramowania
    void setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void setBorderThickness(int thickness);

private:
    SDL_Color m_borderColor = {0, 0, 0, 255}; // Domyślny kolor obramowania (czarny)
    int m_borderThickness = 1; // Domyślna grubość obramowania
};

#endif // PANEL_HPP