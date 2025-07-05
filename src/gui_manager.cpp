#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "gui.hpp" // Potrzebne do rzutowania na Button

GUIManager::GUIManager(SDL_Renderer* renderer)
    : m_renderer(renderer), m_fontManager(), m_textureManager(renderer) {
    // Konstruktor teraz tworzy i zarządza menedżerami zasobów
}

GUIManager::~GUIManager() {
    // Obiekty m_fontManager i m_textureManager są automatycznie niszczone,
    // a unique_ptrs w m_elements dbają o zwolnienie pamięci po elementach GUI.
}


void GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        // Ustaw wskaźnik na ten GUIManager w dodawanym elemencie
        element->setGUIManager(this);
        m_elements.push_back(std::move(element)); // Przenieś własność do wektora
    }
}

bool GUIManager::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            return true; // Zasygnalizuj wyjście
        }

        // Przekaż zdarzenie do wszystkich elementów najwyższego poziomu.
        // Pętla zatrzyma się, gdy któryś element "skonsumuje" zdarzenie.
        for (const auto& element : m_elements) {
            if (element && element->handleEvent(e)) {
                // Jeśli element obsłużył zdarzenie, możemy przerwać pętlę,
                // aby uniknąć obsługi tego samego zdarzenia przez wiele elementów.
                break;
            }
        }
    }
    return false; // Kontynuuj pętlę główną
}
void GUIManager::render(SDL_Renderer* renderer) {
    // Renderuj wszystkie zarządzane elementy
    for (const auto& element : m_elements) { // Iteracja po unique_ptr
        if (element) {
            element->render(renderer); // Użyj operatora -> na unique_ptr
        }
    }
}
