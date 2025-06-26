#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "gui.hpp" // Potrzebne do rzutowania na Button
#include "font_manager.hpp"
#include "texture_manager.hpp"

GUIManager::GUIManager(SDL_Renderer* renderer, FontManager* fontManager, TextureManager* textureManager)
    : m_renderer(renderer), m_fontManager(fontManager), m_textureManager(textureManager) {
    // Konstruktor teraz przechowuje kontekst
}

GUIManager::~GUIManager() {
    // Zwolnienie zasobów, jeśli potrzebne (np. usunięcie elementów, jeśli GUIManager jest ich właścicielem)
    // W tym zadaniu GUIManager zarządza wskaźnikami, a cykl życia obiektów GUIElement jest zarządzany zewnętrznie.
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
