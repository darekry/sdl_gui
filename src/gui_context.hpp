#pragma once

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"

class GUIContext {
public:
    GUIContext(const char* title, int width, int height, bool resizable = false)
        : m_app(std::make_unique<SDLApp>(title, width, height, resizable))
        , m_guiManager(m_app->getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(Theme::createDefaultTheme());
    }

    GUIContext(const char* title, int width, int height, Theme theme, bool resizable = false)
        : m_app(std::make_unique<SDLApp>(title, width, height, resizable))
        , m_guiManager(m_app->getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(std::move(theme));
    }

    /* GPU context (Vulkan + SDL_CreateGPURenderer) — for ShaderPanel etc. */
    GUIContext(const char* title, int width, int height, bool resizable, GPUBackend /*backend*/, bool gpuDebug = false)
        : m_app(std::make_unique<SDLApp>(title, width, height, resizable, GPU_VULKAN, gpuDebug))
        , m_guiManager(m_app->getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(Theme::createDefaultTheme());
    }

    /* Attach to an externally owned renderer (no SDL_CreateWindow/
       SDL_CreateRenderer/TTF_Init; caller owns SDL lifecycle).
       sdlgui_destroy() only drops the GUIManager, window/renderer stay alive. */
    GUIContext(SDL_Renderer* renderer, int width, int height)
        : m_app(nullptr)
        , m_guiManager(renderer, Viewport{width, height})
    {
        if (!renderer)
            throw std::invalid_argument("GUIContext: null renderer");
        if (width <= 0 || height <= 0)
            throw std::invalid_argument("GUIContext: viewport must be > 0");
        m_guiManager.setTheme(Theme::createDefaultTheme());
    }

    [[nodiscard]] SDL_Renderer* getRenderer() const { return m_guiManager.getRenderer(); }
    [[nodiscard]] SDL_Window* getWindow() const {
        if (m_app)
            return m_app->getWindow();
        return SDL_GetRenderWindow(m_guiManager.getRenderer());
    }
    [[nodiscard]] SDL_GPUDevice* getGPUDevice() const {
        if (m_app)
            return m_app->getGPUDevice();
        return m_guiManager.getGPUDevice();
    }
    [[nodiscard]] bool ownsWindow() const { return m_app != nullptr; }
    [[nodiscard]] SDLApp& getApp() {
        if (!m_app)
            throw std::logic_error("GUIContext: attached context owns no SDLApp");
        return *m_app;
    }
    [[nodiscard]] GUIManager& getGUIManager() { return m_guiManager; }

    void setTheme(Theme theme) { m_guiManager.setTheme(std::move(theme)); }
    Theme& getTheme() { return m_guiManager.getTheme(); }

    void handleResize(int width, int height) { m_guiManager.handleResize(width, height); }

    template<typename F = std::nullptr_t>
    void run(SDL_Color clearColor = {40, 42, 54, 255}, F onEvent = nullptr) {
        if (!m_app)
            throw std::logic_error("GUIContext::run needs an owned window (attach has none)");
        m_app->run(m_guiManager, clearColor, onEvent);
    }

private:
    std::unique_ptr<SDLApp> m_app;  // null when attached to an external renderer
    GUIManager m_guiManager;
};
