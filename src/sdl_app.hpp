#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_properties.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <dirent.h>
#include "logger.hpp"

#include "std.hpp"
#include "gui_manager.hpp"

struct GPUBackend {};
inline constexpr GPUBackend GPU_VULKAN{};


class SDLApp {
public:
    /**
     * @brief Construct SDLApp with standard renderer (SDL_CreateRenderer)
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param resizable If true, window can be resized by user
     */
    SDLApp(const char* title, int width, int height, 
           bool resizable = false) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            LOG_ERROR("SDLApp", "SDL could not initialize! SDL_Error: {}", SDL_GetError());
            throw std::runtime_error("SDL_Init failed");
        }

        if (!TTF_Init()) {
            LOG_ERROR("SDLApp", "SDL_ttf could not initialize! SDL_ttf Error: {}", SDL_GetError());
            SDL_Quit();
            throw std::runtime_error("TTF_Init failed");
        }

        SDL_WindowFlags windowFlags = 0;
        if (resizable) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }
        if (getenv("SDL_GUI_HIDDEN")) {
            windowFlags |= SDL_WINDOW_HIDDEN;
        }

        m_window = SDL_CreateWindow(title, width, height, windowFlags);
        if (!m_window) {
            LOG_ERROR("SDLApp", "Window could not be created! SDL_Error: {}", SDL_GetError());
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        m_renderer = SDL_CreateRenderer(m_window, NULL);
        if (!m_renderer) {
            LOG_ERROR("SDLApp", "Renderer could not be created! SDL_Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer failed");
        }

        // VSync paces SDL_RenderPresent() to the display refresh rate.
        // Without it the main loop spins at thousands of FPS, pinning one
        // CPU core at 100% even when the app is idle.
        if (!SDL_SetRenderVSync(m_renderer, 1)) {
            LOG_WARNING("SDLApp", "SDL_SetRenderVSync failed, main loop will not be paced: {}", SDL_GetError());
        }
    }

    /**
     * @brief Construct SDLApp with GPU renderer (SDL_CreateGPURenderer, Vulkan/SPIRV)
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param resizable If true, window can be resized by user
     * @param backend Tag to select GPU backend (use GPU_VULKAN)
     * @param gpuDebug Enable GPU debug mode
     */
    SDLApp(const char* title, int width, int height, 
           bool resizable, GPUBackend /*backend*/, bool gpuDebug = false) {
        Uint64 t0 = SDL_GetTicks();
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            LOG_ERROR("SDLApp", "SDL could not initialize! SDL_Error: {}", SDL_GetError());
            throw std::runtime_error("SDL_Init failed");
        }
        LOG_INFO("SDLApp", "SDL_Init: {}ms", SDL_GetTicks() - t0);

        t0 = SDL_GetTicks();
        if (!TTF_Init()) {
            LOG_ERROR("SDLApp", "SDL_ttf could not initialize! SDL_ttf Error: {}", SDL_GetError());
            SDL_Quit();
            throw std::runtime_error("TTF_Init failed");
        }
        LOG_INFO("SDLApp", "TTF_Init: {}ms", SDL_GetTicks() - t0);

        SDL_WindowFlags windowFlags = 0;
        if (resizable) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }
        if (getenv("SDL_GUI_HIDDEN")) {
            windowFlags |= SDL_WINDOW_HIDDEN;
        }

        t0 = SDL_GetTicks();
        m_window = SDL_CreateWindow(title, width, height, windowFlags);
        LOG_INFO("SDLApp", "SDL_CreateWindow: {}ms", SDL_GetTicks() - t0);
        if (!m_window) {
            LOG_ERROR("SDLApp", "Window could not be created! SDL_Error: {}", SDL_GetError());
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        LOG_INFO("SDLApp", "Window created");

        if (!getenv("VK_ICD_FILENAMES") && !getenv("VK_DRIVER_FILES")) {
            std::string icdPaths;
            auto scanIcd = [&](const char* dir) {
                DIR* d = opendir(dir);
                if (!d) return;
                struct dirent* ent;
                while ((ent = readdir(d))) {
                    std::string_view name = ent->d_name;
                    if (!name.ends_with(".json")) continue;
                    // skip mobile, VM, emulation, and slow SDL3-init ICDs
                    if (name.contains("asahi") || name.contains("dzn") ||
                        name.contains("freedreno") || name.contains("gfxstream") ||
                        name.contains("virtio") || name.contains("hasvk") ||
                        name.contains("radeon") || name.contains("nouveau") ||
                        name.contains("nvidia")) continue;
                    if (!icdPaths.empty()) icdPaths += ":";
                    icdPaths += dir;
                    icdPaths += "/";
                    icdPaths += name;
                }
                closedir(d);
            };
            scanIcd("/usr/share/vulkan/icd.d");
            scanIcd("/etc/vulkan/icd.d");

            if (!icdPaths.empty()) {
                setenv("VK_ICD_FILENAMES", icdPaths.c_str(), 1);
                 LOG_INFO("SDLApp", "VK_ICD_FILENAMES={}", icdPaths);
            }
        }

        // SDL3 prepares the Vulkan driver twice (VULKAN_PrepareDriver + VULKAN_CreateDevice);
        // without this the loader dlcloses the ICD in between and libGLX_nvidia.so.0 re-inits
        // (wakes the dGPU on hybrid laptops: ~1.85s per wake, ~4.5s total instead of ~2.1s).
        setenv("VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING", "1", 0);

        t0 = SDL_GetTicks();
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");
        SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN, true);
        SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, gpuDebug);
        m_gpuDevice = SDL_CreateGPUDeviceWithProperties(props);
        SDL_DestroyProperties(props);
        LOG_INFO("SDLApp", "SDL_CreateGPUDevice (vulkan): {}ms", SDL_GetTicks() - t0);
        if (!m_gpuDevice) {
            LOG_ERROR("SDLApp", "GPU device could not be created! SDL_Error: {}", SDL_GetError());
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateGPUDevice failed");
        }

        t0 = SDL_GetTicks();
        m_renderer = SDL_CreateGPURenderer(m_gpuDevice, m_window);
        LOG_INFO("SDLApp", "SDL_CreateGPURenderer: {}ms", SDL_GetTicks() - t0);
        if (!m_renderer) {
            LOG_ERROR("SDLApp", "GPU renderer could not be created! SDL_Error: {}", SDL_GetError());
            SDL_DestroyGPUDevice(m_gpuDevice);
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateGPURenderer failed");
        }
    }

    ~SDLApp() {
        SDL_DestroyRenderer(m_renderer);
        if (m_gpuDevice) SDL_DestroyGPUDevice(m_gpuDevice);
        SDL_DestroyWindow(m_window);
        TTF_Quit();
        SDL_Quit();
    }

    [[nodiscard]] SDL_Renderer* getRenderer() const { return m_renderer; }
    [[nodiscard]] SDL_Window* getWindow() const { return m_window; }
    [[nodiscard]] SDL_GPUDevice* getGPUDevice() const { return m_gpuDevice; }

    /**
     * @brief Cap the frame rate by sleeping the remainder of the frame budget.
     *
     * Call once per frame, AFTER SDL_RenderPresent(), with the tick count
     * taken at the TOP of the frame:
     *
     *   while (!quit) {
     *       Uint64 frameStart = SDL_GetTicks();
     *       ...
     *       SDL_RenderPresent(renderer);
     *       app.endFrame(frameStart);
     *   }
     *
     * Why this is needed: SDL_SetRenderVSync() is only a hint and the driver
     * is free to ignore it (measured 271 FPS with vsync=1 on X11). Without
     * pacing, the loop spins at hundreds/thousands of FPS and pins one CPU
     * core at 100% even when the app is idle. If vsync already blocked for
     * the full budget, elapsed >= budget and no extra delay is added, so
     * this never double-throttles.
     *
     * @param frameStart SDL_GetTicks() value from the top of the frame
     * @param frameBudgetMs Target frame length in ms (default 16 ≈ 60 FPS)
     */
    void endFrame(Uint64 frameStart, Uint32 frameBudgetMs = 16) const {
        Uint64 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < frameBudgetMs) {
            SDL_Delay(static_cast<Uint32>(frameBudgetMs - elapsed));
        }
    }
    
    /**
     * @brief Get current window size
     */
    void getWindowSize(int& width, int& height) const {
        SDL_GetWindowSize(m_window, &width, &height);
    }

    /**
     * @brief Run the main event loop with GUIManager.
     *
     * Handles the full lifecycle: PollEvent → processEvent → update → cleanup →
     * clear (with given color) → render → present.
     *
     * @param guiManager The GUIManager to use
     * @param clearColor Background clear color (default: dark gray 40,42,54)
     * @param onEvent Optional callback for additional event handling (e.g. resize, custom keys).
     *                Called after guiManager.processEvent().
     */
    template<typename F = std::nullptr_t>
    void run(GUIManager& guiManager, SDL_Color clearColor = {40, 42, 54, 255}, F onEvent = nullptr) {
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    break;
                }
                guiManager.processEvent(e);
                if constexpr (!std::is_same_v<F, std::nullptr_t>) {
                    onEvent(e);
                }
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(m_renderer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
            SDL_RenderClear(m_renderer);
            guiManager.render();
            SDL_RenderPresent(m_renderer);
            endFrame(frameStart);
        }
    }

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_GPUDevice* m_gpuDevice = nullptr;
};
