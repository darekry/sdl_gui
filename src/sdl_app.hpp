#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

import std.compat;



class SDLApp {
public:
    /**
     * @brief Construct a new SDLApp
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param rendererFlags SDL renderer flags (default: VSync)
     * @param resizable If true, window can be resized by user
     */
    SDLApp(const char* title, int width, int height, 
           SDL_RendererFlags rendererFlags = SDL_RENDERER_PRESENTVSYNC,
           bool resizable = false) {
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init failed");
        }

        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            SDL_Quit();
            throw std::runtime_error("IMG_Init failed");
        }

        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("TTF_Init failed");
        }

        Uint32 windowFlags = SDL_WINDOW_SHOWN;
        if (resizable) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }

        m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                      width, height, windowFlags);
        if (!m_window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, rendererFlags);
        if (!m_renderer) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer failed");
        }
    }

    ~SDLApp() {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    [[nodiscard]] SDL_Renderer* getRenderer() const { return m_renderer; }
    [[nodiscard]] SDL_Window* getWindow() const { return m_window; }
    
    /**
     * @brief Get current window size
     */
    void getWindowSize(int& width, int& height) const {
        SDL_GetWindowSize(m_window, &width, &height);
    }

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
