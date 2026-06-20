#pragma once

#include "screen.hpp"
#include "gui_manager.hpp"

#include "std.hpp"

/**
 * @brief Manages multiple screens and handles screen transitions
 * 
 * ScreenManager allows switching between different screens (e.g., menu -> game)
 * while using a single window and renderer. Each screen manages its own GUI elements.
 * 
 * Features:
 * - Add/remove screens by name
 * - Switch screens with onEnter/onExit lifecycle
 * - Optional screen transitions (future: fade, slide effects)
 * - Screen stack for pause/menu overlay on game screen
 * 
 * Usage:
 * @code
 * ScreenManager screenManager(guiManager);
 * screenManager.addScreen("menu", std::make_unique<MenuScreen>());
 * screenManager.addScreen("game", std::make_unique<GameScreen>());
 * screenManager.changeScreen("menu");
 * 
 * // In main loop:
 * screenManager.handleEvent(e);
 * screenManager.update();
 * screenManager.render(renderer);
 * @endcode
 */
class ScreenManager {
public:
    /**
     * @brief Construct ScreenManager with GUIManager reference
     * @param manager Reference to GUIManager (must remain valid during ScreenManager lifetime)
     */
    explicit ScreenManager(GUIManager& manager);
    
    ~ScreenManager() = default;
    
    // === Screen Management ===
    
    /**
     * @brief Add a screen with given name
     * @param name Unique screen identifier
     * @param screen Screen instance (ownership transferred)
     * @return true if added successfully, false if name already exists
     */
    bool addScreen(const std::string& name, std::unique_ptr<Screen> screen);
    
    /**
     * @brief Remove a screen by name
     * @param name Screen identifier
     * @return true if removed, false if not found or is current screen
     */
    bool removeScreen(const std::string& name);
    
    /**
     * @brief Check if screen exists
     * @param name Screen identifier
     */
    bool hasScreen(const std::string& name) const;
    
    /**
     * @brief Get screen by name (returns nullptr if not found)
     * @param name Screen identifier
     */
    Screen* getScreen(const std::string& name) const;
    
    // === Screen Switching ===
    
    /**
     * @brief Change to a different screen
     * Calls onExit() on current screen, then onEnter() on new screen
     * @param name Screen identifier to switch to
     * @return true if switched successfully, false if screen not found
     */
    bool changeScreen(const std::string& name);
    
    /**
     * @brief Get current screen name (empty string if no screen active)
     */
    std::string getCurrentScreenName() const;
    
    /**
     * @brief Get current screen (nullptr if no screen active)
     */
    Screen* getCurrentScreen() const;
    
    // === Screen Stack (for overlays) ===
    
    /**
     * @brief Push a screen onto the stack (overlay mode)
     * Current screen remains in background but doesn't receive events
     * Use for pause menus, settings overlays, etc.
     * @param name Screen identifier to push
     * @return true if pushed successfully
     */
    bool pushScreen(const std::string& name);
    
    /**
     * @brief Pop the top screen from stack
     * Returns to the previous screen (calls onExit/onEnter appropriately)
     * @return Name of popped screen, or empty string if stack empty
     */
    std::string popScreen();
    
    /**
     * @brief Get screen stack depth
     */
    size_t getStackDepth() const;
    
    /**
     * @brief Clear entire screen stack
     */
    void clearStack();
    
    // === Event Loop Integration ===
    
    /**
     * @brief Process SDL event through current screen
     * @param e SDL event
     * @return true if event was consumed
     */
    bool handleEvent(const SDL_Event& e);
    
    /**
     * @brief Update current screen
     */
    void update();
    
    /**
     * @brief Render current screen and all screens in stack (from bottom to top)
     * @param renderer SDL renderer
     */
    void render(SDL_Renderer* renderer);
    
    /**
     * @brief Cleanup marked elements in GUIManager
     */
    void cleanup();
    
private:
    GUIManager& m_guiManager;
    std::unordered_map<std::string, std::unique_ptr<Screen>> m_screens;
    std::vector<std::string> m_screenStack;  // Stack of screen names (bottom = base, top = overlay)
    
    void enterScreen(Screen* screen);
    void exitScreen(Screen* screen);
};