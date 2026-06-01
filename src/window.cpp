#include "window.hpp"
#include "gui.hpp"

import std.compat;

Window::Window(const std::string& title, int width, int height,
               Uint32 rendererFlags, bool resizable)
    : m_title(title) {
    
    Uint32 windowFlags = SDL_WINDOW_SHOWN;
    if (resizable) {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }
    
    m_window = SDL_CreateWindow(title.c_str(), 
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                width, height, windowFlags);
    
    if (!m_window) {
        std::cerr << "Window::Window() - SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    
    m_renderer = SDL_CreateRenderer(m_window, -1, rendererFlags);
    if (!m_renderer) {
        std::cerr << "Window::Window() - SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(m_window);
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }
    
    m_windowID = SDL_GetWindowID(m_window);
    m_guiManager = std::make_unique<GUIManager>(m_renderer);
    m_guiManager->setWindowSize(width, height);
    
    LOG_DEBUG("Window::Window() - created window '%s' (ID=%u)", title.c_str(), m_windowID);
}

Window::~Window() {
    m_guiManager.reset();
    
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    LOG_DEBUG("Window::~Window() - destroyed window '%s'", m_title.c_str());
}

void Window::getSize(int& width, int& height) const {
    SDL_GetWindowSize(m_window, &width, &height);
}

void Window::show() {
    SDL_ShowWindow(m_window);
    m_visible = true;
}

void Window::hide() {
    SDL_HideWindow(m_window);
    m_visible = false;
}

bool Window::processEvent(const SDL_Event& e) {
    // Handle window-specific events
    switch (e.type) {
        case SDL_WINDOWEVENT:
            switch (e.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    // User clicked close button
                    if (m_onCloseCallback) {
                        m_onCloseCallback(this);
                    } else {
                        markForClose();
                    }
                    return true;
                    
                case SDL_WINDOWEVENT_RESIZED: {
                    int w = e.window.data1;
                    int h = e.window.data2;
                    m_guiManager->handleResize(w, h);
                    if (m_onResizeCallback) {
                        m_onResizeCallback(this, w, h);
                    }
                    return true;
                }
                    
                case SDL_WINDOWEVENT_SHOWN:
                    m_visible = true;
                    return true;
                    
                case SDL_WINDOWEVENT_HIDDEN:
                    m_visible = false;
                    return true;
                    
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    m_focused = true;
                    return true;
                    
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    m_focused = false;
                    return true;
            }
            break;
            
        // Note: Keyboard events (SDL_KEYDOWN, SDL_KEYUP, SDL_TEXTINPUT) are
        // passed directly to GUIManager. ESC key handling can be implemented
        // by user via onCloseCallback or in the GUIManager's keyboard focus element.
    }
    
    // Pass to GUIManager
    return m_guiManager->processEvent(e);
}

void Window::update() {
    m_guiManager->update();
}

void Window::render() {
    if (!m_visible) return;
    
    // Clear with theme background color or default white
    auto& theme = m_guiManager->getTheme();
    SDL_Color bgColor = theme.getDefaultStyle().backgroundColor.value_or(SDL_Color{255, 255, 255, 255});
    SDL_SetRenderDrawColor(m_renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(m_renderer);
    
    m_guiManager->render();
    
    SDL_RenderPresent(m_renderer);
}

void Window::cleanup() {
    m_guiManager->cleanup();
}