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
SharedFont FontManager::loadFont(std::string_view path, int size) {
    auto key = FontKey{std::string(path), size};
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Loading font from cache: %s (size %d)", key.path.c_str(), size);
        return it->second;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Loading font from file: %s (size %d)", key.path.c_str(), size);
    auto* loadedFont = TTF_OpenFont(key.path.c_str(), size);
    if (!loadedFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FontManager ERROR: Unable to load font %s with size %d! SDL_ttf Error: %s", key.path.c_str(), size, TTF_GetError());
        return nullptr;
    }

    auto sharedNewFont = SharedFont(loadedFont, TTFFontDeleter());
    auto [inserted_it, success] = m_fonts.emplace(std::move(key), sharedNewFont);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FontManager: Font loaded and cached successfully.");

    return inserted_it->second;
}

void FontManager::loadDefaultFont(std::string_view path, int size) {
    m_defaultFont = loadFont(path, size);
    if (!m_defaultFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FontManager ERROR: Failed to load default font from %s", std::string(path).c_str());
    }
}

SharedFont FontManager::getDefaultFont() const {
    return m_defaultFont;
}

TTF_Font* FontManager::getFont(std::string_view path, int size) {
    auto font = loadFont(path, size);
    if (font) {
        return font.get();
    }
    return nullptr;
}

void FontManager::getTextSize(std::string_view text, std::string_view fontPath, int fontSize, int* width, int* height) {
    if (!width || !height) {
        return;
    }
    *width = 0;
    *height = 0;

    auto font = loadFont(fontPath, fontSize);
    if (font) {
        if (TTF_SizeUTF8(font.get(), text.data(), width, height) != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_SizeUTF8 failed: %s", TTF_GetError());
            *width = 0;
            *height = 0;
        }
    }
}