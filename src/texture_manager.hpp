#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "font_manager.hpp" // Needed to create the default texture

#include "std.hpp"

struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
    size_t operator()(const std::string& s) const noexcept { return std::hash<std::string>{}(s); }
};

using SharedTexture = std::shared_ptr<SDL_Texture>;

/**
 * @brief Manages texture loading and caching for the GUI.
 * 
 * @warning This class is NOT thread-safe. All methods must be called from the same thread
 *          that owns the GUIManager and SDL renderer. Concurrent calls from different threads
 *          may cause data races in the texture cache and SDL texture operations.
 *          If multi-threaded texture loading is needed, use external synchronization (e.g., std::mutex).
 */
class TextureManager {
public:
    // Punkt 6 plaster 4 (byte-budget LRU): domyślny limit pamięci cache'y.
    // Wystarcza na setki pełnoekranowych tekstur 800x600x4; testy i aplikacje
    // VRAM-świadome mogą go ścisnąć setByteBudget().
    static constexpr size_t kDefaultByteBudget = 128u * 1024u * 1024u;

    explicit TextureManager(SDL_Renderer* renderer);

    ~TextureManager();
    
    /**
     * @brief Check if SDL_image was initialized successfully.
     * @return true if initialization succeeded, false otherwise.
     */
    bool isInitialized() const { return m_initialized; }

    // Loads a texture. Returns SharedTexture.
    // If a texture at the given path is already loaded, returns the existing SharedTexture.
    SharedTexture loadTexture(std::string_view path);

    SharedTexture createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color);

    // Creates a text texture with a stable cache key (font_path + font_size).
    SharedTexture createTextureFromText(std::string_view text, std::string_view fontPath, int fontSize, const SDL_Color& color);
    
    // Loads a texture from memory (for embedded assets)
    SharedTexture loadTextureFromMemory(const uint8_t* data, size_t size, std::string_view key);

    // Renders an element into a shared, immutable cache texture.
    // If a texture with the given key already exists, returns it WITHOUT re-rendering.
    // The entry is immutable - never overwritten, removed by pruneUnused()
    // when it stops being used. The key must uniquely describe all draw()
    // inputs (type, size, state, style, widget internal state).
    SharedTexture renderCache(uint64_t key, int width, int height,
                              const std::function<void(SDL_Renderer*)>& draw);

    // Adds an existing texture and takes ownership of it
    SharedTexture addTexture(std::string_view key, SDL_Texture* texture);
    SharedTexture addTexture(std::string_view key, SharedTexture texture);

    SharedTexture getTexture(std::string_view key) const;
    
    bool hasTexture(std::string_view key) const;

    // Queries a texture's size (loads the texture if not already loaded)
    // Returns true and sets width/height if the texture is available; otherwise false.
    bool queryTexture(std::string_view key, int& width, int& height);
    
    void createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text);
    
    SharedTexture getDefaultTexture() const;

    // Cleanup methods
    void pruneUnused();
    void clearCache();
    [[nodiscard]] size_t getCacheSize() const;
    [[nodiscard]] size_t getRenderCacheSize() const;
    [[nodiscard]] size_t getTextCacheSize() const;

    // Punkt 6 plaster 4: limit bajtów na wszystkie trzy cache'e łącznie.
    // setByteBudget() egzekwuje nowy limit natychmiast (wyrzuca martwe wpisy
    // od najstarszego użycia). Wpisy trzymane przez widgety (use_count > 1)
    // nigdy nie są wyrzucane — przy samych żywych wpisach budżet może być
    // chwilowo przekroczony.
    void setByteBudget(size_t bytes);
    [[nodiscard]] size_t getByteBudget() const { return m_byteBudget; }
    [[nodiscard]] size_t getBytesUsed() const { return m_bytesUsed; }

private:
    // Jeden wpis cache'a: współdzielona tekstura + szacunek GPU-bajtów
    // (w*h*4) + recency LRU. lastUse jest mutable, żeby const getTexture()
    // mógł odświeżać kolejność bez zmiany sygnatury API.
    struct CacheEntry {
        SharedTexture texture;
        size_t bytes = 0;
        mutable uint64_t lastUse = 0;
    };
    // TextShaper key (punkt 6): FNV-1a over text bytes mixed with font
    // identity + color. No std::string allocation on the hot path (the old
    // key built text+"|"+ptr+"|"+rgba per call, i.e. per frame for
    // non-shared widgets like ComboBox/ProgressBar/StringGrid).
    static uint64_t textCacheKey(std::string_view text, uint64_t fontId, const SDL_Color& color);

    // Szacunek GPU-bajtów tekstury (w*h*4); 0 gdy rozmiaru nie da się odczytać.
    static size_t textureBytes(SDL_Texture* tex);
    // Odświeża recency wpisu (const, żeby działało też z getTexture()).
    void touch(const CacheEntry& entry) const;
    // Wstawia wpis, księguje bajty i egzekwuje budżet. Wołane PO sprawdzeniu
    // hita przez callerów (wszystkie ścieżki wstawiania w .cpp).
    template <typename Map, typename Key>
    SharedTexture insertEntry(Map& map, Key&& key, SharedTexture tex, size_t bytes);
    // Wyrzuca martwe wpisy (use_count == 1) od najstarszego użycia,
    // aż m_bytesUsed <= m_byteBudget albo zabraknie martwych wpisów.
    void enforceByteBudget();

    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, CacheEntry, StringHash, std::equal_to<>> m_textureCache;
    std::unordered_map<uint64_t, CacheEntry> m_renderCache;
    std::unordered_map<uint64_t, CacheEntry> m_textCache;
    SharedTexture m_defaultTexture;
    bool m_initialized = false; // SDL_image initialization status
    size_t m_bytesUsed = 0;
    size_t m_byteBudget = kDefaultByteBudget;
    mutable uint64_t m_lruClock = 0;
};
