#include "panel.hpp"
#include "gui.hpp" // Dla GUIElement::render
#include "gui_manager.hpp"
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
void Panel::setDraggable(bool draggable) {
    m_is_draggable = draggable;
}

bool Panel::handleEvent(SDL_Event& event) {
    if (!m_visible) {
        return false;
    }

    // 1. Najpierw przekaż zdarzenie do dzieci.
    if (GUIElement::handleEvent(event)) {
        return true; // Jeśli dziecko obsłużyło zdarzenie, zakończ.
    }

    // 2. Jeśli żadne dziecko nie obsłużyło zdarzenia, obsłuż przeciąganie.
    if (m_is_draggable) {
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            SDL_Point mousePos = {event.button.x, event.button.y};
            // Sprawdź, czy kliknięcie jest w obrębie samego panelu (bez dzieci)
            // Ta uproszczona wersja zakłada, że kliknięcie w dowolnym miejscu panelu rozpoczyna przeciąganie.
            SDL_Point absPos = getAbsolutePosition();
            SDL_Rect panelRect = {absPos.x, absPos.y, m_width, m_height};

            if (SDL_PointInRect(&mousePos, &panelRect)) {
                m_is_dragging = true;
                m_drag_offset.x = mousePos.x - absPos.x;
                m_drag_offset.y = mousePos.y - absPos.y;
                return true; // Zdarzenie obsłużone przez rozpoczęcie przeciągania.
            }
        } else if (event.type == SDL_MOUSEMOTION && m_is_dragging) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            setPosition(mouseX - m_drag_offset.x - (m_parent ? m_parent->getAbsolutePosition().x : 0),
                        mouseY - m_drag_offset.y - (m_parent ? m_parent->getAbsolutePosition().y : 0));
            return true; // Zdarzenie obsłużone przez przeciąganie.
        } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
            if (m_is_dragging) {
                m_is_dragging = false;
                return true; // Zdarzenie obsłużone przez zakończenie przeciągania.
            }
        }
    }

    // Żadne zdarzenie nie zostało obsłużone.
    return false;
}

void Panel::render() {
    if (!m_visible) {
        return;
    }
    SDL_Renderer* renderer = m_manager.getRenderer();
    
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
    GUIElement::render();
}