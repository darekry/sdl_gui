#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP


#include "SDL2/SDL.h"
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "timer_manager.hpp"
#include "theme.hpp"
import std.compat;

class TimerManager;
class GUIElement;
class Theme;

class GUIManager {
public:
    // Konstruktor
    explicit GUIManager(SDL_Renderer* renderer);

    // Destruktor
    ~GUIManager();

    // Metoda do dodawania elementów GUI do zarządzania

    // Metoda do dodawania elementów GUI do zarządzania, przejmując własność
    GUIElement* addElement(std::unique_ptr<GUIElement> element);

    // Metoda do usuwania elementów GUI (opcjonalnie, na razie nie wymagane przez zadanie)
    // void removeElement(GUIElement* element); // Usunięcie elementu z unique_ptr oznacza jego zniszczenie

    // Metoda do obsługi zdarzeń SDL i przekazywania ich do odpowiednich elementów GUI
    bool processEvent(const SDL_Event& e);

    // Metoda do renderowania wszystkich elementów GUI
    void render();

    // Metoda do czyszczenia elementów oznaczonych do usunięcia
    void cleanup();

    // Metody dostępu do kontekstu
    // Metody dostępu do kontekstu
    SDL_Renderer* getRenderer() const { return m_renderer; }
    FontManager& getFontManager() { return m_fontManager; }
    const FontManager& getFontManager() const { return m_fontManager; }
    TextureManager& getTextureManager() { return m_textureManager; }
    const TextureManager& getTextureManager() const { return m_textureManager; }
    TimerManager* getTimerManager();
    
    // Tooltip
    void showTooltip(GUIElement* target, const std::string& text);
    void hideTooltip();
// Metoda do uzyskiwania dostępu do elementów (potrzebna do ręcznej obsługi zdarzeń)
const std::vector<std::unique_ptr<GUIElement>>& getElements() const { return m_elements; }

// --- Zarządzanie motywem ---
void setTheme(Theme theme);
Theme& getTheme();

private:
// Kontener na unikalne wskaźniki do elementów GUI
std::vector<std::unique_ptr<GUIElement>> m_elements;
std::unique_ptr<GUIElement> tooltipElement;


// Kontekst aplikacji
SDL_Renderer* m_renderer;
FontManager m_fontManager;
TextureManager m_textureManager;
std::unique_ptr<TimerManager> timerManager;
Theme m_theme;
};

#endif // GUI_MANAGER_HPP