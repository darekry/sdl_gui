#include "panel.hpp"
#include "gui.hpp" // Dla GUIElement::render

// Implementacja klasy Panel
Panel::Panel(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
    // Dodatkowa inicjalizacja dla Panelu, jeśli potrzebna
}

void Panel::setBorderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    m_borderColor = {r, g, b, a};
}

void Panel::setBorderThickness(int thickness) {
    m_borderThickness = thickness;
}
void Panel::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }
    
    SDL_Point absPos = getAbsolutePosition();
    SDL_Rect panelRect = { absPos.x, absPos.y, m_width, m_height };

    // Rysuj tło panelu
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); // Jasnoszary kolor tła
    SDL_RenderFillRect(renderer, &panelRect);

    // Rysuj obramowanie (jeśli jest grubsze niż 0)
    if (m_borderThickness > 0) {
        SDL_SetRenderDrawColor(renderer, m_borderColor.r, m_borderColor.g, m_borderColor.b, m_borderColor.a);
        for (int i = 0; i < m_borderThickness; ++i) {
            SDL_Rect borderRect = {absPos.x + i, absPos.y + i, m_width - 2 * i, m_height - 2 * i};
            SDL_RenderDrawRect(renderer, &borderRect);
        }
    }

    // Renderuj dzieci panelu
    GUIElement::render(renderer);
}