#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include "sdl_deleters.hpp"

#include "std.hpp"

// Type for the shared font pointer
using SharedFont = std::shared_ptr<TTF_Font>;
// Key for the font cache map (path + size)
using FontKey = std::pair<std::string, int>;

// Hash function for text width cache key (font_ptr, text)
struct TextWidthKeyHash {
    size_t operator()(const std::pair<uintptr_t, std::string>& key) const noexcept {
        size_t h1 = std::hash<uintptr_t>{}(key.first);
        size_t h2 = std::hash<std::string>{}(key.second);
        return h1 ^ (h2 << 1);
    }
};

// Comparator for the font cache key, enabling transparent lookup
struct FontCacheKeyCompare {
    using is_transparent = void;

    // Compare two full keys
    bool operator()(const FontKey& lhs, const FontKey& rhs) const {
        return lhs < rhs;
    }

    // Compare a full key with a (string_view, int) pair
    bool operator()(const FontKey& lhs, const std::pair<std::string_view, int>& rhs) const {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        return lhs.second < rhs.second;
    }

    // Compare a (string_view, int) pair with a full key
    bool operator()(const std::pair<std::string_view, int>& lhs, const FontKey& rhs) const {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        return lhs.second < rhs.second;
    }
};

/**
 * @brief Manages font loading and caching for the GUI.
 * 
 * @warning This class is NOT thread-safe. All methods must be called from the same thread
 *          that owns the GUIManager. Concurrent calls from different threads may cause data races
 *          in the font cache. If multi-threaded font loading is needed, use external synchronization.
 */
class FontManager {
public:
    FontManager();

    ~FontManager();
    
    /**
     * @brief Check if SDL_ttf was initialized successfully.
     * @return true if initialization succeeded, false otherwise.
     */
    bool isInitialized() const { return m_initialized; }

    // Loads a font. Returns SharedFont.
    // If a font with the same path and size is already loaded, returns the existing SharedFont.
    SharedFont loadFont(std::string_view path, int size);

    // Loads a font from memory (for embedded assets)
    SharedFont loadFontFromMemory(const uint8_t* data, size_t size, int fontSize, std::string_view key);

    void loadDefaultFont(std::string_view path, int size);
    
    SharedFont getDefaultFont() const;

    // Returns a raw font pointer (for performance).
    // NOTE: This method does not manage memory; it only returns the pointer.
    TTF_Font* getFont(std::string_view path, int size);

    void getTextSize(std::string_view text, std::string_view fontPath, int fontSize, int* width, int* height);

    // Get cached text width, or compute and cache it
    int getTextWidth(TTF_Font* font, std::string_view text);
    void clearTextWidthCache();

private:
    std::map<FontKey, SharedFont, FontCacheKeyCompare> m_fontCache; // Map storing loaded fonts
    SharedFont m_defaultFont; // Default font
    bool m_initialized = false; // SDL_ttf initialization status

    // Cache for text widths: (font_ptr, text) -> width
    std::unordered_map<std::pair<uintptr_t, std::string>, int, TextWidthKeyHash> m_textWidthCache;
    static constexpr size_t MAX_TEXT_CACHE_SIZE = 1000;
};
