#include "texture_manager.hpp"
#include "sdl_deleters.hpp"
#include <SDL2/SDL.h>

import std.compat;

TextureManager::TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {
    // Inicjalizacja SDL_image, jeśli nie została jeszcze zainicjowana
    // Sprawdzamy, czy wymagane formaty są już załadowane
    const auto imgFlags = IMG_INIT_PNG; // Można dodać więcej formatów, np. IMG_INIT_JPG
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
        // W przypadku błędu inicjalizacji, można podjąć odpowiednie działania, np. rzucić wyjątek
    }
}

TextureManager::~TextureManager() {
    // SDL_image Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    // IMG_Quit();
}

SharedTexture TextureManager::loadTexture(std::string_view path) {
    // Używamy find, aby uniknąć tworzenia std::string, jeśli to możliwe
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }

    const std::string path_str(path);
    auto* loadedSurface = IMG_Load(path_str.c_str());
    if (!loadedSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s", path_str.c_str(), IMG_GetError());
        return nullptr;
    }
    
    auto* newTexture = SDL_CreateTextureFromSurface(m_renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);

    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from %s! SDL Error: %s", path_str.c_str(), SDL_GetError());
        return nullptr;
    }

    auto sharedNewTexture = SharedTexture(newTexture, SDLTextureDeleter());
    auto [inserted_it, success] = m_textures.emplace(std::move(path_str), sharedNewTexture);
    
    return inserted_it->second;
}


SharedTexture TextureManager::createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Attempting to create texture for text: \"%s\"", std::string(text).c_str());
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Font is not loaded!");
        return nullptr;
    }

    auto* textSurface = TTF_RenderUTF8_Blended(font.get(), std::string(text).c_str(), color);
    if (!textSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to render text surface! SDL_ttf Error: %s", TTF_GetError());
        return nullptr;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Text surface created successfully (w: %d, h: %d).", textSurface->w, textSurface->h);

    auto* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to create texture from rendered text! SDL Error: %s", SDL_GetError());
        return nullptr;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Texture created successfully from text.");

    return {textTexture, SDLTextureDeleter()};
}

SharedTexture TextureManager::addTexture(std::string_view key, SDL_Texture* texture) {
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Attempted to add texture with existing key '%.*s'. Returning existing texture.", static_cast<int>(key.length()), key.data());
        return it->second;
    }

    if (!texture) {
        return nullptr;
    }
    auto shared = SharedTexture(texture, SDLTextureDeleter());
    auto [inserted_it, success] = m_textures.emplace(key, shared);
    return inserted_it->second;
}

SharedTexture TextureManager::addTexture(std::string_view key, SharedTexture texture) {
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Attempted to add texture with existing key '%.*s'. Returning existing texture.", static_cast<int>(key.length()), key.data());
        return it->second;
    }

    if (!texture) {
        return nullptr;
    }

    auto [inserted_it, success] = m_textures.emplace(key, texture);
    return inserted_it->second;
}

SharedTexture TextureManager::getTexture(std::string_view key) const {
    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        return it->second;
    }
    return nullptr;
}

bool TextureManager::hasTexture(std::string_view key) const {
    return m_textures.contains(key);
}


void TextureManager::createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text) {
    auto defaultFont = fontManager.getDefaultFont();
    if (!defaultFont) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Default font not loaded. Cannot create default texture.");
        return;
    }

    // Utwórz powierzchnię tła
    auto* bgSurface = SDL_CreateRGBSurface(0, 100, 30, 32, 0, 0, 0, 0);
    if (!bgSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Could not create background surface for default texture.");
        return;
    }
    SDL_FillRect(bgSurface, NULL, SDL_MapRGB(bgSurface->format, 200, 200, 200)); // Szare tło

    // Utwórz teksturę z tekstem
    auto textColor = SDL_Color{ 0, 0, 0, 255 }; // Czarny
    auto* textSurface = TTF_RenderUTF8_Blended(defaultFont.get(), std::string(text).c_str(), textColor);
    if (!textSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to render text for default texture. SDL_ttf Error: %s", TTF_GetError());
        SDL_FreeSurface(bgSurface);
        return;
    }

    // Blituj tekst na tło
    auto textRect = SDL_Rect{ (bgSurface->w - textSurface->w) / 2, (bgSurface->h - textSurface->h) / 2, textSurface->w, textSurface->h };
    SDL_BlitSurface(textSurface, NULL, bgSurface, &textRect);
    SDL_FreeSurface(textSurface);

    // Utwórz finalną teksturę
    auto* finalTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    if (!finalTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to create default texture. SDL Error: %s", SDL_GetError());
        return;
    }

    m_defaultTexture = SharedTexture(finalTexture, SDLTextureDeleter());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Default texture created successfully.");
}

SharedTexture TextureManager::getDefaultTexture() const {
    return m_defaultTexture;
}