#include "window_manager.hpp"
#include "window.hpp"

import std.compat;

WindowManager::WindowManager() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "WindowManager::WindowManager() - SDL_Init failed: " << SDL_GetError() << "\n";
        throw std::runtime_error("SDL_Init failed: " + std::string(SDL_GetError()));
    }
    
    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "WindowManager::WindowManager() - IMG_Init failed: " << IMG_GetError() << "\n";
        SDL_Quit();
        throw std::runtime_error("IMG_Init failed: " + std::string(IMG_GetError()));
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "WindowManager::WindowManager() - TTF_Init failed: " << TTF_GetError() << "\n";
        IMG_Quit();
        SDL_Quit();
        throw std::runtime_error("TTF_Init failed: " + std::string(TTF_GetError()));
    }
    
    m_sdlInitialized = true;
    LOG_DEBUG("WindowManager::WindowManager() - SDL initialized successfully");
}

WindowManager::~WindowManager() {
    LOG_DEBUG("WindowManager::~WindowManager() - destroying all windows (%zu)", m_windows.size());
    
    m_windows.clear();
    
    if (m_sdlInitialized) {
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
}

Window* WindowManager::createWindow(const std::string& title, int width, int height, bool resizable) {
    return createWindow(title, width, height, SDL_RENDERER_PRESENTVSYNC, resizable);
}

Window* WindowManager::createWindow(const std::string& title, int width, int height,
                                     Uint32 rendererFlags, bool resizable) {
    try {
        auto window = std::make_unique<Window>(title, width, height, rendererFlags, resizable);
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
        if (e.type == SDL_QUIT) {
            m_shouldQuit = true;
            closeAllWindows();
            return false;
        }
        
        // Route events to appropriate window based on windowID
        Uint32 windowID = 0;
        
        // Get windowID from various event types
        switch (e.type) {
            case SDL_WINDOWEVENT:
                windowID = e.window.windowID;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                windowID = e.key.windowID;
                break;
            case SDL_TEXTINPUT:
                windowID = e.text.windowID;
                break;
            case SDL_MOUSEMOTION:
                windowID = e.motion.windowID;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                windowID = e.button.windowID;
                break;
            case SDL_MOUSEWHEEL:
                windowID = e.wheel.windowID;
                break;
            default:
                // Events without windowID field (e.g., SDL_JOYDEVICEADDED, 
                // SDL_CONTROLLERDEVICEADDED) are sent to focused window.
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