#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "SDL2/SDL.h"
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
import std.compat;


class GUIManager {
public:
    // Konstruktor
    GUIManager(SDL_Renderer* renderer);

    // Destruktor
    ~GUIManager();

    // Metoda do dodawania elementów GUI do zarządzania

    // Metoda do dodawania elementów GUI do zarządzania, przejmując własność
    void addElement(std::unique_ptr<GUIElement> element);

    // Metoda do usuwania elementów GUI (opcjonalnie, na razie nie wymagane przez zadanie)
    // void removeElement(GUIElement* element); // Usunięcie elementu z unique_ptr oznacza jego zniszczenie

    // Metoda do obsługi zdarzeń SDL i przekazywania ich do odpowiednich elementów GUI
    bool processEvent(const SDL_Event& e);

    // Metoda do renderowania wszystkich elementów GUI
    void render();

    // Metoda do czyszczenia elementów oznaczonych do usunięcia
    void cleanup();

    // Metody dostępu do kontekstu
    SDL_Renderer* getRenderer() const { return m_renderer; }
    FontManager& getFontManager() { return m_fontManager; }
    TextureManager& getTextureManager() { return m_textureManager; }

    // Metoda do uzyskiwania dostępu do elementów (potrzebna do ręcznej obsługi zdarzeń)
    std::vector<std::unique_ptr<GUIElement>>& getElements() { return m_elements; }

private:
    // Kontener na unikalne wskaźniki do elementów GUI
    std::vector<std::unique_ptr<GUIElement>> m_elements;

    // Kontekst aplikacji
    SDL_Renderer* m_renderer;
    FontManager m_fontManager;
    TextureManager m_textureManager;
};

#endif // GUI_MANAGER_HPP