#pragma once

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"

class GUIContext {
public:
    GUIContext(const char* title, int width, int height, bool resizable = false)
        : m_app(title, width, height, resizable)
        , m_guiManager(m_app.getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(Theme::createDefaultTheme());
    }

    GUIContext(const char* title, int width, int height, Theme theme, bool resizable = false)
        : m_app(title, width, height, resizable)
        , m_guiManager(m_app.getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(std::move(theme));
    }

    /* GPU context (Vulkan + SDL_CreateGPURenderer) — for ShaderPanel etc. */
    GUIContext(const char* title, int width, int height, bool resizable, GPUBackend /*backend*/, bool gpuDebug = false)
        : m_app(title, width, height, resizable, GPU_VULKAN, gpuDebug)
        , m_guiManager(m_app.getRenderer(), Viewport{width, height})
    {
        m_guiManager.setTheme(Theme::createDefaultTheme());
    }

    [[nodiscard]] SDL_Renderer* getRenderer() const { return m_app.getRenderer(); }
    [[nodiscard]] SDL_Window* getWindow() const { return m_app.getWindow(); }
    [[nodiscard]] SDL_GPUDevice* getGPUDevice() const { return m_app.getGPUDevice(); }
    [[nodiscard]] SDLApp& getApp() { return m_app; }
    [[nodiscard]] GUIManager& getGUIManager() { return m_guiManager; }

    void setTheme(Theme theme) { m_guiManager.setTheme(std::move(theme)); }
    Theme& getTheme() { return m_guiManager.getTheme(); }

    void handleResize(int width, int height) { m_guiManager.handleResize(width, height); }

    template<typename F = std::nullptr_t>
    void run(SDL_Color clearColor = {40, 42, 54, 255}, F onEvent = nullptr) {
        m_app.run(m_guiManager, clearColor, onEvent);
    }

private:
    SDLApp m_app;
    GUIManager m_guiManager;
};
