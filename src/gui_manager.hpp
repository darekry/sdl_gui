#pragma once


#include "SDL2/SDL.h"
#include "gui.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "timer_manager.hpp"
#include "theme.hpp"
#include "animation_manager.hpp"
#include "cursor.hpp"

import std.compat;

class TimerManager;
class AnimationManager;
class GUIElement;
class Theme;
class Panel;
class Label;

/**
 * @brief Callback type for window resize events
 * Parameters: new window width, new window height
 */
using ResizeCallback = std::function<void(int, int)>;

class GUIManager {
public:
    GUIManager(SDL_Renderer* renderer);
    ~GUIManager();

    GUIElement* addElement(std::unique_ptr<GUIElement> element);
    bool processEvent(const SDL_Event& e);
    void update();
    void render();
    void cleanup();
    
    // === Resize handling ===
    
    /**
     * @brief Handle window resize - updates all anchored elements
     * Call this when window size changes (SDL_WINDOWEVENT_RESIZED)
     * @param width New window width
     * @param height New window height
     */
    void handleResize(int width, int height);
    
    /**
     * @brief Set custom resize callback for additional handling
     * Callback is called after anchored elements are updated
     */
    void setResizeCallback(ResizeCallback callback);
    
    /**
     * @brief Get current stored window size
     */
    void getWindowSize(int& width, int& height) const;
    
    /**
     * @brief Set window size (call once at initialization)
     */
    void setWindowSize(int width, int height);

    SDL_Renderer* getRenderer() const { return m_renderer; }
    FontManager& getFontManager() { return m_fontManager; }
    const FontManager& getFontManager() const { return m_fontManager; }
    TextureManager& getTextureManager() { return m_textureManager; }
    const TextureManager& getTextureManager() const { return m_textureManager; }
    TimerManager* getTimerManager();
    AnimationManager* getAnimationManager();
    
    void showTooltip(GUIElement* target, const std::string& text);
    void hideTooltip();

    void setTheme(Theme theme);
    Theme& getTheme();

    GUIElement* findElementAt(int x, int y);

    void captureMouse(GUIElement* element);
    void releaseMouse();
    void setKeyboardFocus(GUIElement* element);
    [[nodiscard]] GUIElement* getKeyboardFocus() const;

    void setCursor(std::unique_ptr<Cursor> new_cursor);
    
    bool isElementAlive(GUIElement* element) const;
    void registerElement(GUIElement* element);
    void unregisterElement(GUIElement* element);

private:
    std::vector<std::unique_ptr<GUIElement>> m_elements;
    std::unique_ptr<GUIElement> tooltipElement;
    std::unique_ptr<Cursor> cursor;
    
    // Cached tooltip components (reuse to avoid allocations)
    std::unique_ptr<Panel> m_tooltipPanel;
    Label* m_tooltipLabel = nullptr;

    GUIElement* m_mouseCaptureElement = nullptr;
    GUIElement* m_keyboardFocusElement = nullptr;

    SDL_Renderer* m_renderer;
    FontManager m_fontManager;
    TextureManager m_textureManager;
    std::unique_ptr<TimerManager> timerManager;
    std::unique_ptr<AnimationManager> animation_manager;

    Theme m_theme;
    
    std::unordered_set<GUIElement*> m_liveElements;
    
    // === Resize handling ===
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    ResizeCallback m_resizeCallback;
};
