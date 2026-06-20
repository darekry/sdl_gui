#pragma once

#include "gui_manager.hpp"

#include "std.hpp"

/**
 * @brief Abstract base class for screens in ScreenManager
 * 
 * A Screen represents a distinct UI state (e.g., menu screen, game screen, settings screen).
 * Each screen manages its own GUI elements and handles events independently.
 * 
 * Usage:
 * 1. Create a class inheriting from Screen
 * 2. Implement onEnter() to add elements to GUIManager
 * 3. Implement onExit() to cleanup/remove elements
 * 4. Implement handleEvent(), update(), render() for screen-specific logic
 * 
 * Example:
 * @code
 * class MenuScreen : public Screen {
 *     void onEnter(GUIManager& manager) override {
 *         // Add menu buttons, labels, etc.
 *     }
 *     void onExit(GUIManager& manager) override {
 *         // Cleanup menu elements
 *     }
 * };
 * @endcode
 */
class Screen {
public:
    virtual ~Screen() = default;
    
    /**
     * @brief Called when screen becomes active
     * Use this to add GUI elements to the manager
     * @param manager The GUIManager to add elements to
     */
    virtual void onEnter(GUIManager& manager) = 0;
    
    /**
     * @brief Called when screen becomes inactive (before switching to another screen)
     * Use this to cleanup elements or save state
     * @param manager The GUIManager to cleanup elements from
     */
    virtual void onExit(GUIManager& manager) = 0;
    
    /**
     * @brief Handle SDL event for this screen
     * @param manager The GUIManager
     * @param e The SDL event to process
     * @return true if event was consumed, false otherwise
     */
    virtual bool handleEvent(GUIManager& manager, const SDL_Event& e) = 0;
    
    /**
     * @brief Update screen state (called each frame)
     * @param manager The GUIManager
     */
    virtual void update(GUIManager& manager) = 0;
    
    /**
     * @brief Render screen content (called each frame)
     * @param manager The GUIManager
     * @param renderer The SDL renderer
     */
    virtual void render(GUIManager& manager, SDL_Renderer* renderer) = 0;
    
    /**
     * @brief Get screen name (for debugging/logging)
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Check if screen wants to handle event before GUIManager
     * Override to return true if screen needs to intercept events before GUI elements
     */
    virtual bool wantsPreProcessEvent() const { return false; }
};