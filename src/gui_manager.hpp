#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "SDL2/SDL.h"
#include <vector>
#include "gui.hpp"

class GUIManager {
public:
    // Konstruktor
    GUIManager();

    // Destruktor
    ~GUIManager();

    // Metoda do dodawania elementów GUI do zarządzania
    void addElement(GUIElement* element);

    // Metoda do usuwania elementów GUI (opcjonalnie, na razie nie wymagane przez zadanie)
    // void removeElement(GUIElement* element);

    // Metoda do obsługi zdarzeń SDL i przekazywania ich do odpowiednich elementów GUI
    bool handleEvents();

    // Metoda do renderowania wszystkich elementów GUI
    void render(SDL_Renderer* renderer);

private:
    // Kontener na wskaźniki do elementów GUI
    std::vector<GUIElement*> m_elements;
};

#endif // GUI_MANAGER_HPP