#include "font_manager.hpp"
#include "SDL2/SDL.h" // Potrzebne do SDL_GetError()
#include "SDL2/SDL_log.h"
#include "gui.hpp"



FontManager::FontManager() {
    // Inicjalizacja SDL_ttf, jeśli nie została jeszcze zainicjowana
    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "FontManager CRITICAL: SDL_ttf could not initialize! SDL_ttf Error: %s", 
            TTF_GetError());
        m_initialized = false;
        // Obiekt będzie działać w ograniczonym trybie - nie będzie mógł ładować czcionek
    } else {
        m_initialized = true;
    }
}

FontManager::~FontManager() {
    // TTF_Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    // TTF_Quit();
}
SharedFont FontManager::loadFont(std::string_view path, int size) {
    // Tworzymy klucz jako std::pair do wyszukiwania
    auto key_sv = std::make_pair(path, size);
    
    // Używamy find z kluczem std::pair<std::string_view, int>, co jest możliwe dzięki transparentnemu komparatorowi.
    // To pozwala uniknąć tworzenia std::string.
    auto it = m_fontCache.find(key_sv);

    if (it != m_fontCache.end()) {
     //   LOG_DEBUG("FontManager: Loading font from cache: %s (size %d)", it->first.first.c_str(), size);
        return it->second;
    }

    std::string path_str(path);
    LOG_DEBUG("FontManager: Loading font from file: %s (size %d)", path_str.c_str(), size);
    auto* loadedFont = TTF_OpenFont(path_str.c_str(), size);
    if (!loadedFont) {
        LOG_DEBUG("FontManager ERROR: Unable to load font %s with size %d! SDL_ttf Error: %s", path_str.c_str(), size, TTF_GetError());
        return nullptr;
    }

    auto sharedNewFont = SharedFont(loadedFont, TTFFontDeleter());
    // Klucz do wstawienia musi być już typu std::string
    auto [inserted_it, success] = m_fontCache.emplace(FontKey(std::move(path_str), size), sharedNewFont);
    LOG_DEBUG("FontManager: Font loaded and cached successfully.");

    return inserted_it->second;
}

void FontManager::loadDefaultFont(std::string_view path, int size) {
    m_defaultFont = loadFont(path, size);
    if (!m_defaultFont) {
        std::string pathStr(path);  // Create once for logging
        LOG_DEBUG("FontManager ERROR: Failed to load default font from %s", pathStr.c_str());
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
        *width = getTextWidth(font.get(), text);
        int h = 0;
        if (TTF_SizeUTF8(font.get(), text.data(), nullptr, &h) != 0) {
            LOG_DEBUG("TTF_SizeUTF8 failed: %s", TTF_GetError());
            *height = 0;
        } else {
            *height = h;
        }
    }
}

int FontManager::getTextWidth(TTF_Font* font, std::string_view text) {
    if (!font || text.empty()) return 0;

    uintptr_t fontPtr = reinterpret_cast<uintptr_t>(font);
    auto key = std::make_pair(fontPtr, std::string(text));

    auto it = m_textWidthCache.find(key);
    if (it != m_textWidthCache.end()) {
        return it->second;
    }

    int width = 0;
    TTF_SizeUTF8(font, text.data(), &width, nullptr);

    if (m_textWidthCache.size() >= MAX_TEXT_CACHE_SIZE) {
        m_textWidthCache.clear();
    }

    m_textWidthCache.emplace(std::move(key), width);
    return width;
}

void FontManager::clearTextWidthCache() {
    m_textWidthCache.clear();
}