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

    // Metoda do dodawania elementów GUI do zarządzania, przejmując własność
    void addElement(std::unique_ptr<GUIElement> element);

    // Metoda do usuwania elementów GUI (opcjonalnie, na razie nie wymagane przez zadanie)
    // void removeElement(GUIElement* element); // Usunięcie elementu z unique_ptr oznacza jego zniszczenie

    // Metoda do obsługi zdarzeń SDL i przekazywania ich do odpowiednich elementów GUI
    bool handleEvents();

    // Metoda do renderowania wszystkich elementów GUI
    void render(SDL_Renderer* renderer);

private:
    // Kontener na unikalne wskaźniki do elementów GUI
    std::vector<std::unique_ptr<GUIElement>> m_elements;
};

#endif // GUI_MANAGER_HPP