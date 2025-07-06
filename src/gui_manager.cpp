#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "gui.hpp" // Potrzebne do rzutowania na Button
import std.compat;

GUIManager::GUIManager(SDL_Renderer* renderer)
    : m_renderer(renderer), m_fontManager(), m_textureManager(renderer) {
    // Załaduj domyślną czcionkę
    m_fontManager.loadDefaultFont("assets/fonts/font.ttf", 24);

    // Utwórz domyślną teksturę zastępczą
    m_textureManager.createDefaultTexture(m_renderer, m_fontManager, "No Texture");
}

GUIManager::~GUIManager() {
    // Obiekty m_fontManager i m_textureManager są automatycznie niszczone,
    // a unique_ptrs w m_elements dbają o zwolnienie pamięci po elementach GUI.
}


void GUIManager::addElement(std::unique_ptr<GUIElement> element) {
    if (element) {
        m_elements.push_back(std::move(element)); // Przenieś własność do wektora
    }
}

bool GUIManager::processEvent(const SDL_Event& e) {
    // Przekaż zdarzenie do wszystkich elementów najwyższego poziomu.
    // Pętla zatrzyma się, gdy któryś element "skonsumuje" zdarzenie.
    for (const auto& element : m_elements) {
        if (element && element->handleEvent(const_cast<SDL_Event&>(e))) {
            // Jeśli element obsłużył zdarzenie, zwracamy true.
            return true;
        }
    }
    // Żaden element nie obsłużył zdarzenia.
    return false;
}

void GUIManager::render() {
    // Renderuj wszystkie zarządzane elementy
    for (const auto& element : m_elements) { // Iteracja po unique_ptr
        if (element) {
            element->render();
        }
    }
}

void GUIManager::cleanup() {
    // Najpierw rekurencyjnie wywołaj cleanup dla wszystkich elementów
    for (const auto& element : m_elements) {
        if (element) {
            element->cleanup();
        }
    }

    // Następnie usuń oznaczone elementy z głównego kontenera
    auto new_end = std::remove_if(m_elements.begin(), m_elements.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });
    
    m_elements.erase(new_end, m_elements.end());
}
