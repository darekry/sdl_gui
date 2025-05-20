#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "gui.hpp" // Potrzebne do rzutowania na Button

GUIManager::GUIManager() {
    // Inicjalizacja, jeśli potrzebna
}

GUIManager::~GUIManager() {
    // Zwolnienie zasobów, jeśli potrzebne (np. usunięcie elementów, jeśli GUIManager jest ich właścicielem)
    // W tym zadaniu GUIManager zarządza wskaźnikami, a cykl życia obiektów GUIElement jest zarządzany zewnętrznie.
}

void GUIManager::addElement(GUIElement* element) {
    if (element) {
        m_elements.push_back(element);
    }
}

bool GUIManager::handleEvents() {
    SDL_Event e;
    bool quit = false;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else {
            // Przekaż zdarzenie do wszystkich zarządzanych elementów
            for (GUIElement* element : m_elements) {
                if (element) {
                    // Sprawdź typ zdarzenia i pozycję myszy dla zdarzeń myszy
                    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONUP) {
                        int mouseX, mouseY;
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            mouseX = e.button.x;
                            mouseY = e.button.y;
                        } else if (e.type == SDL_MOUSEBUTTONUP) {
                             mouseX = e.button.x;
                             mouseY = e.button.y;
                        }
                        else { // SDL_MOUSEMOTION
                            mouseX = e.motion.x;
                            mouseY = e.motion.y;
                        }

                        // Sprawdź, czy zdarzenie myszy miało miejsce w obrębie elementu
                        if (element->contains(mouseX, mouseY)) {
                            // Przekaż zdarzenie do metody handleEvent elementu
                            element->handleEvent(e);

                            // Specyficzna obsługa dla przycisków
                            Button* button = dynamic_cast<Button*>(element);
                            if (button) {
                                if (e.type == SDL_MOUSEBUTTONDOWN) {
                                    //button->triggerOnClick(); // Zmieniamy logikę kliknięcia na puszczenie przycisku
                                } else if (e.type == SDL_MOUSEMOTION) {
                                    button->triggerOnMouseOver();
                                }
                            }
                        }
                    } else {
                        // Dla innych typów zdarzeń, po prostu przekaż je do elementu
                        element->handleEvent(e);
                    }
                }
            }
        }
    }
    return quit;
}

void GUIManager::render(SDL_Renderer* renderer) {
    // Renderuj wszystkie zarządzane elementy
    for (GUIElement* element : m_elements) {
        if (element) {
            element->render(renderer);
        }
    }
}