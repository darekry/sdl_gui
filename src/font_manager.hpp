#pragma once

#include <SDL3_ttf/SDL_ttf.h>
#include "sdl_deleters.hpp"

import std.compat;

// Typ dla współdzielonego wskaźnika na czcionkę
using SharedFont = std::shared_ptr<TTF_Font>;
// Klucz dla mapy cache'u czcionek (ścieżka + rozmiar)
using FontKey = std::pair<std::string, int>;

// Hash function for text width cache key (font_ptr, text)
struct TextWidthKeyHash {
    size_t operator()(const std::pair<uintptr_t, std::string>& key) const noexcept {
        size_t h1 = std::hash<uintptr_t>{}(key.first);
        size_t h2 = std::hash<std::string>{}(key.second);
        return h1 ^ (h2 << 1);
    }
};

// Komparator dla klucza cache'u czcionek, umożliwiający transparentne wyszukiwanie
struct FontCacheKeyCompare {
    using is_transparent = void;

    // Porównanie dwóch pełnych kluczy
    bool operator()(const FontKey& lhs, const FontKey& rhs) const {
        return lhs < rhs;
    }

    // Porównanie pełnego klucza z parą (string_view, int)
    bool operator()(const FontKey& lhs, const std::pair<std::string_view, int>& rhs) const {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        return lhs.second < rhs.second;
    }

    // Porównanie pary (string_view, int) z pełnym kluczem
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
    // Konstruktor
    FontManager();

    // Destruktor
    ~FontManager();
    
    /**
     * @brief Check if SDL_ttf was initialized successfully.
     * @return true if initialization succeeded, false otherwise.
     */
    bool isInitialized() const { return m_initialized; }

    // Metoda do ładowania czcionki. Zwraca SharedFont.
    // Jeśli czcionka o danej ścieżce i rozmiarze została już załadowana, zwraca istniejący SharedFont.
    SharedFont loadFont(std::string_view path, int size);

    // Metoda do ładowania domyślnej czcionki
    void loadDefaultFont(std::string_view path, int size);
    
    // Metoda do pobierania domyślnej czcionki
    SharedFont getDefaultFont() const;

    // Metoda do pobierania surowego wskaźnika do czcionki (dla wydajności).
    // UWAGA: Ta metoda nie zarządza pamięcią, jedynie zwraca wskaźnik.
    TTF_Font* getFont(std::string_view path, int size);

    // Metoda do obliczania rozmiaru tekstu
    void getTextSize(std::string_view text, std::string_view fontPath, int fontSize, int* width, int* height);

    // Get cached text width, or compute and cache it
    int getTextWidth(TTF_Font* font, std::string_view text);
    void clearTextWidthCache();

private:
    std::map<FontKey, SharedFont, FontCacheKeyCompare> m_fontCache; // Mapa przechowująca załadowane czcionki
    SharedFont m_defaultFont; // Domyślna czcionka
    bool m_initialized = false; // SDL_ttf initialization status

    // Cache for text widths: (font_ptr, text) -> width
    std::unordered_map<std::pair<uintptr_t, std::string>, int, TextWidthKeyHash> m_textWidthCache;
    static constexpr size_t MAX_TEXT_CACHE_SIZE = 1000;
};
