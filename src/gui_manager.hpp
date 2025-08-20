#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP


#include "SDL2/SDL.h"
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "timer_manager.hpp"
#include "theme.hpp"
#include "animation_manager.hpp"


class TimerManager;
class AnimationManager;
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
    AnimationManager* getAnimationManager();
    
    // Tooltip
    void showTooltip(GUIElement* target, const std::string& text);
    void hideTooltip();

// --- Zarządzanie motywem ---
void setTheme(Theme theme);
Theme& getTheme();

// --- Zarządzanie fokusem i przechwytywaniem ---
void captureMouse(GUIElement* element);
void releaseMouse();
void setKeyboardFocus(GUIElement* element);
[[nodiscard]] GUIElement* getKeyboardFocus() const;

private:
// Kontener na unikalne wskaźniki do elementów GUI
std::vector<std::unique_ptr<GUIElement>> m_elements;
std::unique_ptr<GUIElement> tooltipElement;

// Stan fokusu i przechwytywania
GUIElement* m_mouseCaptureElement = nullptr;
GUIElement* m_keyboardFocusElement = nullptr;

// Kontekst aplikacji
SDL_Renderer* m_renderer;
FontManager m_fontManager;
TextureManager m_textureManager;
std::unique_ptr<TimerManager> timerManager;
std::unique_ptr<AnimationManager> animation_manager;
Theme m_theme;
};

#endif // GUI_MANAGER_HPP