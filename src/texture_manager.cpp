#include "texture_manager.hpp"
#include "sdl_deleters.hpp"
#include "sdl_rect_helpers.hpp"
#include <SDL3/SDL.h>
#include "logger.hpp"



TextureManager::TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {
    m_initialized = true;
}

TextureManager::~TextureManager() {
    // SDL_image Quit is not needed here, since it should be called once at the end of the application's lifetime
    //
}

SharedTexture TextureManager::loadTexture(std::string_view path) {
    // Use find to avoid creating a std::string if possible
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

    const uint64_t key = textCacheKey(text, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(font.get())), color);
    auto it = m_textCache.find(key);
    if (it != m_textCache.end()) {
        return it->second;
    }

    // Create text string ONCE for the SDL call (cache key is already hashed).
    std::string textStr(text);

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
    m_textCache.emplace(key, sharedTexture);

    return sharedTexture;
}

SharedTexture TextureManager::createTextureFromText(std::string_view text, std::string_view fontPath, int fontSize, const SDL_Color& color) {
    // Font identity for the shared text store: hash of path + size
    // (no caller in lib right now, but the overload stays API-compatible).
    uint64_t fontId = 1469598103934665603ULL;
    for (char c : fontPath) {
        fontId ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        fontId *= 1099511628211ULL;
    }
    fontId ^= static_cast<uint64_t>(fontSize) + 0x9e3779b97f4a7c15ULL;

    const uint64_t key = textCacheKey(text, fontId, color);
    auto it = m_textCache.find(key);
    if (it != m_textCache.end()) {
        return it->second;
    }

    // Create strings ONCE - needed for SDL calls (key is already hashed).
    std::string textStr(text);
    std::string fontPathStr(fontPath);

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
    m_textCache.emplace(key, sharedTexture);
    
    return sharedTexture;
}

uint64_t TextureManager::textCacheKey(std::string_view text, uint64_t fontId, const SDL_Color& color) {
    uint64_t h = 1469598103934665603ULL;
    for (char c : text) {
        h ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        h *= 1099511628211ULL;
    }
    h ^= fontId + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    const uint64_t rgba = (static_cast<uint64_t>(color.r) << 24) | (static_cast<uint64_t>(color.g) << 16) |
                          (static_cast<uint64_t>(color.b) << 8) | static_cast<uint64_t>(color.a);
    h ^= rgba + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
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

SharedTexture TextureManager::renderCache(uint64_t key, int width, int height,
                                          const std::function<void(SDL_Renderer*)>& draw) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    auto it = m_renderCache.find(key);
    if (it != m_renderCache.end()) {
        return it->second;
    }

    SharedTexture tex(SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, width, height),
                      SDLTextureDeleter());
    if (!tex) {
        LOG_DEBUG("TextureManager::renderCache: SDL_CreateTexture failed for key %llu: %s",
                  static_cast<unsigned long long>(key), SDL_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(tex.get(), SDL_BLENDMODE_BLEND);

    {
        ScopedRenderTarget scope(m_renderer, tex.get());
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
        SDL_RenderClear(m_renderer);
        if (draw) {
            draw(m_renderer);
        }
    }

    m_renderCache.emplace(key, tex);
    return tex;
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

    // Create the background surface
    auto* bgSurface = SDL_CreateSurface(100, 30, SDL_PIXELFORMAT_RGBA8888);
    if (!bgSurface) {
        LOG_DEBUG("TextureManager ERROR: Could not create background surface for default texture.");
        return;
    }
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(bgSurface->format);
    if (fmt) {
        SDL_FillSurfaceRect(bgSurface, NULL, SDL_MapRGB(fmt, NULL, 200, 200, 200));
    } // Gray background

    // Create text string ONCE for SDL call
    std::string textStr(text);
    
    // Create the text surface
    auto textColor = SDL_Color{ 0, 0, 0, 255 }; // Black
    auto* textSurface = TTF_RenderText_Blended(defaultFont.get(), textStr.c_str(), textStr.length(), textColor);
    if (!textSurface) {
        LOG_DEBUG("TextureManager ERROR: Unable to render text for default texture. SDL_ttf Error: %s", SDL_GetError());
        SDL_DestroySurface(bgSurface);
        return;
    }

    // Blit the text onto the background
    auto textRect = SDL_Rect{ (bgSurface->w - textSurface->w) / 2, (bgSurface->h - textSurface->h) / 2, textSurface->w, textSurface->h };
    SDL_BlitSurface(textSurface, NULL, bgSurface, &textRect);
    SDL_DestroySurface(textSurface);

    // Create the final texture
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
    auto pruneMap = [](auto& map) -> size_t {
        size_t removed = 0;
        auto it = map.begin();
        while (it != map.end()) {
            if (it->second.use_count() == 1) {
                it = map.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
        return removed;
    };
    size_t removed = pruneMap(m_textureCache) + pruneMap(m_renderCache) + pruneMap(m_textCache);
    if (removed > 0) {
        LOG_DEBUG("TextureManager::pruneUnused(): Removed %zu unused textures.", removed);
    }
}

void TextureManager::clearCache() {
    size_t count = m_textureCache.size() + m_renderCache.size() + m_textCache.size();
    m_textureCache.clear();
    m_renderCache.clear();
    m_textCache.clear();
    if (count > 0) {
        LOG_DEBUG("TextureManager::clearCache(): Cleared %zu textures.", count);
    }
}

size_t TextureManager::getCacheSize() const {
    return m_textureCache.size();
}

size_t TextureManager::getRenderCacheSize() const {
    return m_renderCache.size();
}

size_t TextureManager::getTextCacheSize() const {
    return m_textCache.size();
}