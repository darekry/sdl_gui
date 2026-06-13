#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

import std.compat;



class SDLApp {
public:
    /**
     * @brief Construct a new SDLApp
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param resizable If true, window can be resized by user
     */
    SDLApp(const char* title, int width, int height, 
           bool resizable = false) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            throw std::runtime_error("SDL_Init failed");
        }

        if (!TTF_Init()) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            throw std::runtime_error("TTF_Init failed");
        }

        SDL_WindowFlags windowFlags = 0;
        if (resizable) {
            windowFlags |= SDL_WINDOW_RESIZABLE;
        }

        m_window = SDL_CreateWindow(title, width, height, windowFlags);
        if (!m_window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        m_renderer = SDL_CreateRenderer(m_window, NULL);
        if (!m_renderer) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateRenderer failed");
        }
    }

    ~SDLApp() {
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        TTF_Quit();
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
