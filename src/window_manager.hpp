#pragma once

#include "window.hpp"

#include "std.hpp"

/**
 * @brief Manages multiple SDL windows with their own renderers and GUIManagers
 * 
 * WindowManager creates and manages multiple Window instances, each with its own
 * SDL_Window, SDL_Renderer, and GUIManager. Handles event distribution across windows.
 * 
 * Features:
 * - Create multiple independent windows
 * - Each window has its own GUIManager and elements
 * - Automatic event routing to correct window (via SDL_WINDOWID)
 * - Window lifecycle management (create, close, cleanup)
 * - Main loop helper: processEvents(), updateAll(), renderAll()
 * 
 * Usage:
 * @code
 * WindowManager windowManager;
 * 
 * // Create main window
 * Window* mainWindow = windowManager.createWindow("Main App", 800, 600);
 * mainWindow->getGUIManager().addElement(std::make_unique<Panel>(...));
 * 
 * // Create secondary window
 * Window* formWindow = windowManager.createWindow("Form", 400, 300);
 * formWindow->getGUIManager().addElement(std::make_unique<TextInput>(...));
 * 
 * // Set close callback
 * formWindow->setOnCloseCallback([](Window* w) {
 *     std::cout << "Form window closed\n";
 * });
 * 
 * // Main loop
 * while (windowManager.hasOpenWindows()) {
 *     windowManager.processEvents();
 *     windowManager.updateAll();
 *     windowManager.renderAll();
 *     windowManager.cleanupAll();
 * }
 * @endcode
 */
class WindowManager {
public:
    /**
     * @brief Construct WindowManager
     * Initializes SDL, SDL_image, SDL_ttf
     * @throw std::runtime_error if SDL initialization fails
     */
    WindowManager();
    
    /**
     * @brief Destructor - destroys all windows and quits SDL
     */
    ~WindowManager();
    
    // === Window Creation/Management ===
    
    /**
     * @brief Create a new window
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param resizable If true, window can be resized by user
     * @return Pointer to created Window, nullptr if creation failed
     */
    Window* createWindow(const std::string& title, int width, int height, bool resizable = false);
    
    /**
     * @brief Create a new window with custom renderer name
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param name SDL renderer name (NULL for default)
     * @param resizable If true, window can be resized by user
     * @return Pointer to created Window, nullptr if creation failed
     */
    Window* createWindow(const std::string& title, int width, int height, 
                         const char* name, bool resizable);
    
    /**
     * @brief Get window by SDL window ID
     * @param windowID SDL window ID (from event.window.windowID)
     * @return Pointer to Window, nullptr if not found
     */
    Window* getWindowByID(Uint32 windowID) const;
    
    /**
     * @brief Get window by index
     * @param index Index in window list (0-based)
     * @return Pointer to Window, nullptr if index invalid
     */
    Window* getWindow(size_t index) const;
    
    /**
     * @brief Get number of windows (including hidden ones)
     */
    size_t getWindowCount() const;
    
    /**
     * @brief Get focused window (nullptr if none focused)
     */
    Window* getFocusedWindow() const;
    
    /**
     * @brief Check if there are any open (not marked for close) windows
     */
    bool hasOpenWindows() const;
    
    // === Window Removal ===
    
    /**
     * @brief Close a specific window
     * Window will be marked for close and removed during cleanup
     * @param windowID SDL window ID
     * @return true if window was found and marked for close
     */
    bool closeWindow(Uint32 windowID);
    
    /**
     * @brief Close all windows except the main window
     */
    void closeSecondaryWindows();
    
    /**
     * @brief Close all windows (will exit application)
     */
    void closeAllWindows();
    
    // === Main Loop Helpers ===
    
    /**
     * @brief Process all pending SDL events
     * Events are routed to the appropriate window based on windowID
     * @return false if all windows were closed (quit signal)
     */
    bool processEvents();
    
    /**
     * @brief Update all windows' GUIManagers
     */
    void updateAll();
    
    /**
     * @brief Render all visible windows
     */
    void renderAll();
    
    /**
     * @brief Cleanup marked elements in all windows
     * Also removes windows marked for close
     */
    void cleanupAll();
    
    // === Application Control ===
    
    /**
     * @brief Check if application should quit
     * True when all windows are closed or SDL_EVENT_QUIT received
     */
    bool shouldQuit() const { return m_shouldQuit; }
    
    /**
     * @brief Request application quit
     * Closes all windows
     */
    void requestQuit();
    
private:
    std::vector<std::unique_ptr<Window>> m_windows;
    bool m_shouldQuit = false;
    bool m_sdlInitialized = false;
    
    void removeMarkedWindows();
};