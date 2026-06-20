#include "screen_manager.hpp"
#include "screen.hpp"

#include "std.hpp"

ScreenManager::ScreenManager(GUIManager& manager)
    : m_guiManager(manager) {
}

bool ScreenManager::addScreen(const std::string& name, std::unique_ptr<Screen> screen) {
    if (m_screens.contains(name)) {
        LOG_ERROR("ScreenManager", "addScreen() - screen '{}' already exists", name);
        return false;
    }
    m_screens[name] = std::move(screen);
    return true;
}

bool ScreenManager::removeScreen(const std::string& name) {
    // Cannot remove current screen
    if (!m_screenStack.empty() && m_screenStack.back() == name) {
        LOG_ERROR("ScreenManager", "removeScreen() - cannot remove current screen '{}'", name);
        return false;
    }
    
    // Check if screen is in stack (cannot remove screens in stack)
    for (const auto& stackName : m_screenStack) {
        if (stackName == name) {
            LOG_ERROR("ScreenManager", "removeScreen() - cannot remove screen '{}' from stack", name);
            return false;
        }
    }
    
    auto it = m_screens.find(name);
    if (it == m_screens.end()) {
        LOG_ERROR("ScreenManager", "removeScreen() - screen '{}' not found", name);
        return false;
    }
    
    m_screens.erase(it);
    return true;
}

bool ScreenManager::hasScreen(const std::string& name) const {
    return m_screens.contains(name);
}

Screen* ScreenManager::getScreen(const std::string& name) const {
    auto it = m_screens.find(name);
    return it != m_screens.end() ? it->second.get() : nullptr;
}

bool ScreenManager::changeScreen(const std::string& name) {
    auto it = m_screens.find(name);
    if (it == m_screens.end()) {
        LOG_ERROR("ScreenManager", "changeScreen() - screen '{}' not found", name);
        return false;
    }
    
    // Exit all screens in stack (from top to bottom)
    while (!m_screenStack.empty()) {
        std::string topName = m_screenStack.back();
        Screen* topScreen = getScreen(topName);
        if (topScreen) {
            exitScreen(topScreen);
        }
        m_screenStack.pop_back();
    }
    
    // Enter new screen
    m_screenStack.push_back(name);
    enterScreen(it->second.get());
    
    return true;
}

std::string ScreenManager::getCurrentScreenName() const {
    return m_screenStack.empty() ? "" : m_screenStack.back();
}

Screen* ScreenManager::getCurrentScreen() const {
    if (m_screenStack.empty()) return nullptr;
    return getScreen(m_screenStack.back());
}

bool ScreenManager::pushScreen(const std::string& name) {
    auto it = m_screens.find(name);
    if (it == m_screens.end()) {
        LOG_ERROR("ScreenManager", "pushScreen() - screen '{}' not found", name);
        return false;
    }
    
    // Don't push if already at top
    if (!m_screenStack.empty() && m_screenStack.back() == name) {
        LOG_ERROR("ScreenManager", "pushScreen() - screen '{}' already at top of stack", name);
        return false;
    }
    
    m_screenStack.push_back(name);
    enterScreen(it->second.get());
    
    return true;
}

std::string ScreenManager::popScreen() {
    if (m_screenStack.empty()) {
        LOG_ERROR("ScreenManager", "popScreen() - stack is empty");
        return "";
    }
    
    std::string poppedName = m_screenStack.back();
    Screen* poppedScreen = getScreen(poppedName);
    if (poppedScreen) {
        exitScreen(poppedScreen);
    }
    
    m_screenStack.pop_back();
    
    // Enter previous screen if stack not empty
    if (!m_screenStack.empty()) {
        Screen* prevScreen = getScreen(m_screenStack.back());
        if (prevScreen) {
            enterScreen(prevScreen);
        }
    }
    
    return poppedName;
}

size_t ScreenManager::getStackDepth() const {
    return m_screenStack.size();
}

void ScreenManager::clearStack() {
    // Exit all screens from top to bottom
    while (!m_screenStack.empty()) {
        Screen* topScreen = getScreen(m_screenStack.back());
        if (topScreen) {
            exitScreen(topScreen);
        }
        m_screenStack.pop_back();
    }
}

bool ScreenManager::handleEvent(const SDL_Event& e) {
    if (m_screenStack.empty()) return false;
    
    Screen* currentScreen = getCurrentScreen();
    if (!currentScreen) return false;
    
    // Let screen pre-process if it wants
    if (currentScreen->wantsPreProcessEvent()) {
        if (currentScreen->handleEvent(m_guiManager, e)) {
            return true;
        }
    }
    
    // Pass to GUIManager
    return m_guiManager.processEvent(e);
}

void ScreenManager::update() {
    m_guiManager.update();
    
    if (!m_screenStack.empty()) {
        Screen* currentScreen = getCurrentScreen();
        if (currentScreen) {
            currentScreen->update(m_guiManager);
        }
    }
}

void ScreenManager::render(SDL_Renderer* renderer) {
    // Render all screens in stack (from bottom to top)
    // This allows overlays to render on top of base screens
    for (const auto& screenName : m_screenStack) {
        Screen* screen = getScreen(screenName);
        if (screen) {
            screen->render(m_guiManager, renderer);
        }
    }
    
    // Render GUIManager elements (only for top screen)
    m_guiManager.render();
}

void ScreenManager::cleanup() {
    m_guiManager.cleanup();
}

void ScreenManager::enterScreen(Screen* screen) {
    if (screen) {
        LOG_DEBUG("ScreenManager", "entering '{}'", screen->getName());
        screen->onEnter(m_guiManager);
    }
}

void ScreenManager::exitScreen(Screen* screen) {
    if (screen) {
        LOG_DEBUG("ScreenManager", "exiting '{}'", screen->getName());
        screen->onExit(m_guiManager);
        // Cleanup after exiting screen
        m_guiManager.cleanup();
    }
}