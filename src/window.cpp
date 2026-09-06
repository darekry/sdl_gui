#include "window.hpp"
#include "gui.hpp"

#include "std.hpp"

Window::Window(const std::string& title, int width, int height,
               const char* name, bool resizable)
    : m_title(title) {
    
    SDL_WindowFlags windowFlags = 0;
    if (resizable) {
        windowFlags |= SDL_WINDOW_RESIZABLE;
    }
    if (getenv("SDL_GUI_HIDDEN")) {
        windowFlags |= SDL_WINDOW_HIDDEN;
    }
    
    m_window = SDL_CreateWindow(title.c_str(), width, height, windowFlags);
    
    if (!m_window) {
        LOG_ERROR("Window", "SDL_CreateWindow failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed: " + std::string(SDL_GetError()));
    }
    
    m_renderer = SDL_CreateRenderer(m_window, name);
    if (!m_renderer) {
        LOG_ERROR("Window", "SDL_CreateRenderer failed: {}", SDL_GetError());
        SDL_DestroyWindow(m_window);
        throw std::runtime_error("SDL_CreateRenderer failed: " + std::string(SDL_GetError()));
    }

    // VSync paces SDL_RenderPresent() to the display refresh rate.
    // Without it the main loop spins at thousands of FPS, pinning one
    // CPU core at 100% even when the app is idle.
    if (!SDL_SetRenderVSync(m_renderer, 1)) {
        LOG_WARNING("Window", "SDL_SetRenderVSync failed, main loop will not be paced: {}", SDL_GetError());
    }
    
    m_windowID = SDL_GetWindowID(m_window);
    m_guiManager = std::make_unique<GUIManager>(m_renderer, Viewport{width, height});
    
    LOG_DEBUG("Window", "created window '{}' (ID={})", title, m_windowID);
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
    
    LOG_DEBUG("Window", "destroyed window '{}'", m_title);
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
    switch (e.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if (m_onCloseCallback) {
                m_onCloseCallback(this);
            } else {
                markForClose();
            }
            return true;

        case SDL_EVENT_WINDOW_RESIZED: {
            int w = e.window.data1;
            int h = e.window.data2;
            m_guiManager->handleResize(w, h);
            if (m_onResizeCallback) {
                m_onResizeCallback(this, w, h);
            }
            return true;
        }

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            int w = e.window.data1;
            int h = e.window.data2;
            m_guiManager->handleResize(w, h);
            if (m_onResizeCallback) {
                m_onResizeCallback(this, w, h);
            }
            return true;
        }

        case SDL_EVENT_WINDOW_SHOWN:
            m_visible = true;
            return true;

        case SDL_EVENT_WINDOW_HIDDEN:
            m_visible = false;
            return true;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            m_focused = true;
            return true;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            m_focused = false;
            return true;
    }
    
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
    SetDrawColor(m_renderer, bgColor);
    SDL_RenderClear(m_renderer);
    
    m_guiManager->render();
    
    SDL_RenderPresent(m_renderer);
}

void Window::cleanup() {
    m_guiManager->cleanup();
}