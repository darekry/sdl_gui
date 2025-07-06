#ifndef SDL_APP_HPP
#define SDL_APP_HPP

#include "SDL_render.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

import std.compat;

class SDLApp {
public:
    SDLApp(const char* title, int width, int height) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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

        m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        if (!m_window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);
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

    SDL_Renderer* getRenderer() const { return m_renderer; }

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};

#endif // SDL_APP_HPP