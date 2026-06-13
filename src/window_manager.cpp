#include "window_manager.hpp"
#include "window.hpp"

import std.compat;

WindowManager::WindowManager() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "WindowManager::WindowManager() - SDL_Init failed: " << SDL_GetError() << "\n";
        throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
    }
    
    if (!TTF_Init()) {
        std::cerr << "WindowManager::WindowManager() - TTF_Init failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        throw std::runtime_error("TTF_Init failed: " + std::string(SDL_GetError()));
    }
    
    m_sdlInitialized = true;
    LOG_DEBUG("WindowManager::WindowManager() - SDL initialized successfully");
}

WindowManager::~WindowManager() {
    LOG_DEBUG("WindowManager::~WindowManager() - destroying all windows (%zu)", m_windows.size());
    
    m_windows.clear();
    
    if (m_sdlInitialized) {
        TTF_Quit();
        SDL_Quit();
    }
}

Window* WindowManager::createWindow(const std::string& title, int width, int height, bool resizable) {
    return createWindow(title, width, height, NULL, resizable);
}

Window* WindowManager::createWindow(const std::string& title, int width, int height,
                                     const char* name, bool resizable) {
    try {
        auto window = std::make_unique<Window>(title, width, height, name, resizable);
        Window* rawPtr = window.get();
        m_windows.push_back(std::move(window));
        
        LOG_DEBUG("WindowManager::createWindow() - created '%s' (total windows: %zu)", 
                  title.c_str(), m_windows.size());
        
        return rawPtr;
    } catch (const std::runtime_error& e) {
        std::cerr << "WindowManager::createWindow() - failed: " << e.what() << "\n";
        return nullptr;
    }
}

Window* WindowManager::getWindowByID(Uint32 windowID) const {
    for (const auto& window : m_windows) {
        if (window && window->getWindowID() == windowID) {
            return window.get();
        }
    }
    return nullptr;
}

Window* WindowManager::getWindow(size_t index) const {
    if (index >= m_windows.size()) return nullptr;
    return m_windows[index].get();
}

size_t WindowManager::getWindowCount() const {
    return m_windows.size();
}

Window* WindowManager::getFocusedWindow() const {
    for (const auto& window : m_windows) {
        if (window && window->isFocused()) {
            return window.get();
        }
    }
    return nullptr;
}

bool WindowManager::hasOpenWindows() const {
    for (const auto& window : m_windows) {
        if (window && !window->isMarkedForClose()) {
            return true;
        }
    }
    return false;
}

bool WindowManager::closeWindow(Uint32 windowID) {
    Window* window = getWindowByID(windowID);
    if (window) {
        window->markForClose();
        return true;
    }
    return false;
}

void WindowManager::closeSecondaryWindows() {
    // Keep only the first window
    for (size_t i = 1; i < m_windows.size(); ++i) {
        if (m_windows[i]) {
            m_windows[i]->markForClose();
        }
    }
}

void WindowManager::closeAllWindows() {
    for (auto& window : m_windows) {
        if (window) {
            window->markForClose();
        }
    }
}

bool WindowManager::processEvents() {
    SDL_Event e;
    
    while (SDL_PollEvent(&e)) {
        // Global quit event
        if (e.type == SDL_EVENT_QUIT) {
            m_shouldQuit = true;
            closeAllWindows();
            return false;
        }
        
        // Route events to appropriate window based on windowID
        Uint32 windowID = 0;
        
        // Get windowID from various event types
        switch (e.type) {
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_SHOWN:
            case SDL_EVENT_WINDOW_HIDDEN:
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
            case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
            case SDL_EVENT_WINDOW_OCCLUDED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_DESTROYED:
            case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
                windowID = e.window.windowID;
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                windowID = e.key.windowID;
                break;
            case SDL_EVENT_TEXT_INPUT:
                windowID = e.text.windowID;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                windowID = e.motion.windowID;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                windowID = e.button.windowID;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                windowID = e.wheel.windowID;
                break;
            default:
                // Events without windowID field (e.g., SDL_EVENT_JOYSTICK_ADDED, 
                // SDL_EVENT_GAMEPAD_ADDED) are sent to focused window.
                // Most events have windowID and are handled above.
                Window* focused = getFocusedWindow();
                if (focused) {
                    focused->processEvent(e);
                }
                continue;
        }
        
        // Find window and process event
        Window* targetWindow = getWindowByID(windowID);
        if (targetWindow) {
            targetWindow->processEvent(e);
        }
    }
    
    return hasOpenWindows() && !m_shouldQuit;
}

void WindowManager::updateAll() {
    for (auto& window : m_windows) {
        if (window && !window->isMarkedForClose()) {
            window->update();
        }
    }
}

void WindowManager::renderAll() {
    for (auto& window : m_windows) {
        if (window && window->isVisible() && !window->isMarkedForClose()) {
            window->render();
        }
    }
}

void WindowManager::cleanupAll() {
    // Cleanup GUIManagers
    for (auto& window : m_windows) {
        if (window) {
            window->cleanup();
        }
    }
    
    // Remove windows marked for close
    removeMarkedWindows();
}

void WindowManager::requestQuit() {
    m_shouldQuit = true;
    closeAllWindows();
}

void WindowManager::removeMarkedWindows() {
    auto it = std::remove_if(m_windows.begin(), m_windows.end(),
        [](const std::unique_ptr<Window>& window) {
            return window && window->isMarkedForClose();
        });
    
    size_t removedCount = std::distance(it, m_windows.end());
    if (removedCount > 0) {
        LOG_DEBUG("WindowManager::removeMarkedWindows() - removed %zu windows", removedCount);
    }
    
    m_windows.erase(it, m_windows.end());
    
    // If all windows closed, set quit flag
    if (m_windows.empty()) {
        m_shouldQuit = true;
    }
}