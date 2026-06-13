#pragma once

#include "gui_manager.hpp"

import std.compat;

/**
 * @brief Represents a single window with its own SDL_Window, SDL_Renderer, and GUIManager
 * 
 * Window wraps SDL_Window and SDL_Renderer together with a GUIManager instance.
 * Each Window has its own event handling and rendering pipeline.
 * 
 * Usage:
 * @code
 * WindowManager windowManager;
 * Window* mainWindow = windowManager.createWindow("Main App", 800, 600);
 * Window* settingsWindow = windowManager.createWindow("Settings", 400, 300);
 * 
 * // Add elements to specific window's GUIManager
 * mainWindow->getGUIManager().addElement(std::make_unique<Button>(...));
 * 
 * // Main loop:
 * windowManager.processEvents();
 * windowManager.updateAll();
 * windowManager.renderAll();
 * @endcode
 */
class Window {
public:
    /**
     * @brief Construct a Window with given parameters
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param rendererFlags SDL renderer flags (default: VSync)
     * @param resizable If true, window can be resized by user
     * @throw std::runtime_error if SDL window/renderer creation fails
     */
    Window(const std::string& title, int width, int height,
           const char* name = NULL,
           bool resizable = false);
    
    /**
     * @brief Destructor - destroys SDL_Window and SDL_Renderer
     */
    ~Window();
    
    // === Accessors ===
    
    /**
     * @brief Get SDL_Window handle
     */
    SDL_Window* getSDLWindow() const { return m_window; }
    
    /**
     * @brief Get SDL_Renderer handle
     */
    SDL_Renderer* getRenderer() const { return m_renderer; }
    
    /**
     * @brief Get GUIManager for this window
     */
    GUIManager& getGUIManager() { return *m_guiManager; }
    
    /**
     * @brief Get GUIManager for this window (const)
     */
    const GUIManager& getGUIManager() const { return *m_guiManager; }
    
    /**
     * @brief Get window ID (used to identify window in SDL events)
     */
    Uint32 getWindowID() const { return m_windowID; }
    
    /**
     * @brief Get window title
     */
    std::string getTitle() const { return m_title; }
    
    /**
     * @brief Get current window size
     */
    void getSize(int& width, int& height) const;
    
    // === State ===
    
    /**
     * @brief Check if window is visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Show window
     */
    void show();
    
    /**
     * @brief Hide window
     */
    void hide();
    
    /**
     * @brief Check if window is marked for close
     */
    bool isMarkedForClose() const { return m_markedForClose; }
    
    /**
     * @brief Mark window for close (will be removed by WindowManager)
     */
    void markForClose() { m_markedForClose = true; }
    
    /**
     * @brief Check if window is focused
     */
    bool isFocused() const { return m_focused; }
    
    // === Event Handling ===
    
    /**
     * @brief Process SDL event for this window
     * @param e SDL event (should be for this window's ID)
     * @return true if event was consumed
     */
    bool processEvent(const SDL_Event& e);
    
    /**
     * @brief Update window's GUIManager
     */
    void update();
    
    /**
     * @brief Render window contents
     */
    void render();
    
    /**
     * @brief Cleanup marked elements in GUIManager
     */
    void cleanup();
    
    // === Callbacks ===
    
    /**
     * @brief Callback type for window close event
     */
    using CloseCallback = std::function<void(Window*)>;
    
    /**
     * @brief Set callback for window close request
     * Callback is called when user clicks close button or presses Alt+F4.
     * 
     * Note: ESC key handling is NOT built-in. To close window on ESC:
     * @code
     * window->setOnCloseCallback([](Window* w) {
     *     w->markForClose();
     * });
     * // Then in your Screen or GUI element, handle ESC and call:
     * // window->getOnCloseCallback()(window);
     * @endcode
     */
    void setOnCloseCallback(CloseCallback callback) { m_onCloseCallback = callback; }
    
    /**
     * @brief Callback type for window resize event
     */
    using ResizeCallback = std::function<void(Window*, int, int)>;
    
    /**
     * @brief Set callback for window resize
     */
    void setOnResizeCallback(ResizeCallback callback) { m_onResizeCallback = callback; }
    
private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    std::unique_ptr<GUIManager> m_guiManager;
    Uint32 m_windowID = 0;
    std::string m_title;
    
    bool m_visible = true;
    bool m_markedForClose = false;
    bool m_focused = false;
    
    CloseCallback m_onCloseCallback;
    ResizeCallback m_onResizeCallback;
};