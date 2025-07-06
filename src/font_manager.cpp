#include "font_manager.hpp"
#include "SDL2/SDL.h" // Potrzebne do SDL_GetError()
#include "SDL2/SDL_log.h"

import std.compat;

FontManager::FontManager() {
    // Inicjalizacja SDL_ttf, jeśli nie została jeszcze zainicjowana
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_ttf could not initialize! SDL_ttf Error: %s", TTF_GetError());
        // W przypadku błędu inicjalizacji, można podjąć odpowiednie działania, np. rzucić wyjątek
    }
}

FontManager::~FontManager() {
    // TTF_Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    // TTF_Quit();
}
SharedFont FontManager::loadFont(const std::string& path, int size) {
    FontKey key = {path, size};
    if (m_fonts.count(key)) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Loading font from cache: %s (size %d)", path.c_str(), size);
        return m_fonts[key];
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Loading font from file: %s (size %d)", path.c_str(), size);
    TTF_Font* loadedFont = TTF_OpenFont(path.c_str(), size);
    if (!loadedFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FontManager ERROR: Unable to load font %s with size %d! SDL_ttf Error: %s", path.c_str(), size, TTF_GetError());
        return nullptr;
    }

    SharedFont sharedNewFont(loadedFont, TTFFontDeleter());
    m_fonts[key] = sharedNewFont;
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Font loaded and cached successfully.");

    return sharedNewFont;
}

void FontManager::loadDefaultFont(const std::string& path, int size) {
    m_defaultFont = loadFont(path, size);
    if (!m_defaultFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FontManager ERROR: Failed to load default font from %s", path.c_str());
    }
}

SharedFont FontManager::getDefaultFont() {
    return m_defaultFont;
}