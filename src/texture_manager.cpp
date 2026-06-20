#include "texture_manager.hpp"
#include "sdl_deleters.hpp"
#include <SDL3/SDL.h>
#include "gui.hpp"



TextureManager::TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {
    m_initialized = true;
}

TextureManager::~TextureManager() {
    // SDL_image Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    //
}

SharedTexture TextureManager::loadTexture(std::string_view path) {
    // Używamy find, aby uniknąć tworzenia std::string, jeśli to możliwe
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    const std::string path_str(path);
    auto* loadedSurface = IMG_Load(path_str.c_str());
    if (!loadedSurface) {
        LOG_DEBUG("Unable to load image %s! SDL_image Error: %s", path_str.c_str(), SDL_GetError());
        return nullptr;
    }
    
    auto* newTexture = SDL_CreateTextureFromSurface(m_renderer, loadedSurface);
    SDL_DestroySurface(loadedSurface);

    if (!newTexture) {
        LOG_DEBUG("Unable to create texture from %s! SDL Error: %s", path_str.c_str(), SDL_GetError());
        return nullptr;
    }

    auto sharedNewTexture = SharedTexture(newTexture, SDLTextureDeleter());
    auto [inserted_it, success] = m_textureCache.emplace(std::move(path_str), sharedNewTexture);
    
    return inserted_it->second;
}


SharedTexture TextureManager::createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color) {
    if (!font) {
        LOG_DEBUG("TextureManager ERROR: Font is not loaded!");
        return nullptr;
    }

    // Create text string ONCE - needed for both cache key and SDL call
    std::string textStr(text);
    
    std::string cacheKey = textStr + "|" + std::to_string(reinterpret_cast<uintptr_t>(font.get())) 
                          + "|" + std::to_string(color.r) + "," + std::to_string(color.g) 
                          + "," + std::to_string(color.b) + "," + std::to_string(color.a);
    
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    auto* textSurface = TTF_RenderText_Blended(font.get(), textStr.c_str(), textStr.length(), color);
    if (!textSurface) {
        LOG_DEBUG("TextureManager ERROR: Unable to render text surface! SDL_ttf Error: %s", SDL_GetError());
        return nullptr;
    }

    auto* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_DestroySurface(textSurface);

    if (!textTexture) {
        LOG_DEBUG("TextureManager ERROR: Unable to create texture from rendered text! SDL Error: %s", SDL_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(textTexture, SDL_BLENDMODE_BLEND);

    auto sharedTexture = SharedTexture(textTexture, SDLTextureDeleter());
    m_textureCache.emplace(std::move(cacheKey), sharedTexture);
    
    return sharedTexture;
}

SharedTexture TextureManager::createTextureFromText(std::string_view text, std::string_view fontPath, int fontSize, const SDL_Color& color) {
    // Create strings ONCE - needed for both cache key and SDL calls
    std::string textStr(text);
    std::string fontPathStr(fontPath);
    
    std::string cacheKey = textStr + "|" + fontPathStr + "|" + std::to_string(fontSize)
                          + "|" + std::to_string(color.r) + "," + std::to_string(color.g) 
                          + "," + std::to_string(color.b) + "," + std::to_string(color.a);
    
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    auto* font = TTF_OpenFont(fontPathStr.c_str(), static_cast<float>(fontSize));
    if (!font) {
        LOG_DEBUG("TextureManager ERROR: Unable to load font %s! SDL_ttf Error: %s", fontPathStr.c_str(), SDL_GetError());
        return nullptr;
    }

    auto* textSurface = TTF_RenderText_Blended(font, textStr.c_str(), textStr.length(), color);
    TTF_CloseFont(font);
    
    if (!textSurface) {
        LOG_DEBUG("TextureManager ERROR: Unable to render text surface! SDL_ttf Error: %s", SDL_GetError());
        return nullptr;
    }

    auto* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_DestroySurface(textSurface);

    if (!textTexture) {
        LOG_DEBUG("TextureManager ERROR: Unable to create texture from rendered text! SDL Error: %s", SDL_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(textTexture, SDL_BLENDMODE_BLEND);

    auto sharedTexture = SharedTexture(textTexture, SDLTextureDeleter());
    m_textureCache.emplace(std::move(cacheKey), sharedTexture);
    
    return sharedTexture;
}

SharedTexture TextureManager::loadTextureFromMemory(const uint8_t* data, size_t size, std::string_view key) {
    auto it = m_textureCache.find(key);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    SDL_IOStream* io = SDL_IOFromConstMem(data, size);
    if (!io) {
        LOG_DEBUG("TextureManager: SDL_IOFromConstMem failed for key '%.*s': %s", static_cast<int>(key.length()), key.data(), SDL_GetError());
        return nullptr;
    }

    SDL_Surface* loadedSurface = IMG_Load_IO(io, true);
    if (!loadedSurface) {
        LOG_DEBUG("TextureManager: IMG_Load_IO failed for key '%.*s': %s", static_cast<int>(key.length()), key.data(), SDL_GetError());
        return nullptr;
    }

    auto* newTexture = SDL_CreateTextureFromSurface(m_renderer, loadedSurface);
    SDL_DestroySurface(loadedSurface);

    if (!newTexture) {
        LOG_DEBUG("TextureManager: SDL_CreateTextureFromSurface failed for key '%.*s': %s", static_cast<int>(key.length()), key.data(), SDL_GetError());
        return nullptr;
    }

    auto sharedNewTexture = SharedTexture(newTexture, SDLTextureDeleter());
    auto [inserted_it, success] = m_textureCache.emplace(std::string(key), sharedNewTexture);

    return inserted_it->second;
}

SharedTexture TextureManager::addTexture(std::string_view key, SDL_Texture* texture) {
    auto it = m_textureCache.find(key);
    if (it != m_textureCache.end()) {
        LOG_DEBUG("TextureManager: Attempted to add texture with existing key '%.*s'. Returning existing texture.", static_cast<int>(key.length()), key.data());
        return it->second;
    }

    if (!texture) {
        return nullptr;
    }
    auto shared = SharedTexture(texture, SDLTextureDeleter());
    auto [inserted_it, success] = m_textureCache.emplace(key, shared);
    return inserted_it->second;
}

SharedTexture TextureManager::addTexture(std::string_view key, SharedTexture texture) {
    auto it = m_textureCache.find(key);
    if (it != m_textureCache.end()) {
        LOG_DEBUG("TextureManager: Attempted to add texture with existing key '%.*s'. Returning existing texture.", static_cast<int>(key.length()), key.data());
        return it->second;
    }

    if (!texture) {
        return nullptr;
    }

    auto [inserted_it, success] = m_textureCache.emplace(key, texture);
    return inserted_it->second;
}

SharedTexture TextureManager::getTexture(std::string_view key) const {
    auto it = m_textureCache.find(key);
    if (it != m_textureCache.end()) {
        return it->second;
    }
    return nullptr;
}

bool TextureManager::hasTexture(std::string_view key) const {
    return m_textureCache.contains(key);
}


void TextureManager::createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text) {
    auto defaultFont = fontManager.getDefaultFont();
    if (!defaultFont) {
        LOG_DEBUG("TextureManager ERROR: Default font not loaded. Cannot create default texture.");
        return;
    }

    // Utwórz powierzchnię tła
    auto* bgSurface = SDL_CreateSurface(100, 30, SDL_PIXELFORMAT_RGBA8888);
    if (!bgSurface) {
        LOG_DEBUG("TextureManager ERROR: Could not create background surface for default texture.");
        return;
    }
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(bgSurface->format);
    if (fmt) {
        SDL_FillSurfaceRect(bgSurface, NULL, SDL_MapRGB(fmt, NULL, 200, 200, 200));
    } // Szare tło

    // Create text string ONCE for SDL call
    std::string textStr(text);
    
    // Utwórz teksturę z tekstem
    auto textColor = SDL_Color{ 0, 0, 0, 255 }; // Czarny
    auto* textSurface = TTF_RenderText_Blended(defaultFont.get(), textStr.c_str(), textStr.length(), textColor);
    if (!textSurface) {
        LOG_DEBUG("TextureManager ERROR: Unable to render text for default texture. SDL_ttf Error: %s", SDL_GetError());
        SDL_DestroySurface(bgSurface);
        return;
    }

    // Blituj tekst na tło
    auto textRect = SDL_Rect{ (bgSurface->w - textSurface->w) / 2, (bgSurface->h - textSurface->h) / 2, textSurface->w, textSurface->h };
    SDL_BlitSurface(textSurface, NULL, bgSurface, &textRect);
    SDL_DestroySurface(textSurface);

    // Utwórz finalną teksturę
    auto* finalTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_DestroySurface(bgSurface);

    if (!finalTexture) {
        LOG_DEBUG("TextureManager ERROR: Unable to create default texture. SDL Error: %s", SDL_GetError());
        return;
    }

    m_defaultTexture = SharedTexture(finalTexture, SDLTextureDeleter());
    LOG_DEBUG("TextureManager: Default texture created successfully.");
}

SharedTexture TextureManager::getDefaultTexture() const {
    return m_defaultTexture;
}

bool TextureManager::queryTexture(std::string_view path, int& width, int& height) {
    SharedTexture tex = getTexture(path);
    if (!tex) {
        tex = loadTexture(path);
        if (!tex) {
            return false;
        }
    }
    float fw = 0.0f, fh = 0.0f;
    if (!SDL_GetTextureSize(tex.get(), &fw, &fh)) {
        LOG_DEBUG("TextureManager: SDL_GetTextureSize failed for %.*s: %s", static_cast<int>(path.size()), path.data(), SDL_GetError());
        return false;
    }
    width = static_cast<int>(fw);
    height = static_cast<int>(fh);
    return true;
}

void TextureManager::pruneUnused() {
    auto it = m_textureCache.begin();
    size_t removed = 0;
    while (it != m_textureCache.end()) {
        if (it->second.use_count() == 1) {
            it = m_textureCache.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }
    if (removed > 0) {
        LOG_DEBUG("TextureManager::pruneUnused(): Removed %zu unused textures.", removed);
    }
}

void TextureManager::clearCache() {
    size_t count = m_textureCache.size();
    m_textureCache.clear();
    if (count > 0) {
        LOG_DEBUG("TextureManager::clearCache(): Cleared %zu textures.", count);
    }
}

size_t TextureManager::getCacheSize() const {
    return m_textureCache.size();
}