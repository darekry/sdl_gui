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
 *
 * Ownership model (2026-11-22):
 *  - assets path cache (`m_textureCache`) is STRONG: PNG/TTF assets are static,
 *    few and cheap to keep for the process lifetime;
 *  - render/text caches are WEAK: the widget is the holder. An entry lives
 *    only while some widget still references the texture; `releaseRenderCache`
 *    /`releaseTextCache` erase the (now expired) metadata when the last holder
 *    drops its ref. No periodic prune, no byte budget, no global GC.
 */
class TextureManager {
public:
    /**
     * @brief Move-only strong lease of a cached text texture.
     *
     * A widget that keeps a text texture between frames (TextInput, TextArea,
     * StringGrid, menus) owns one lease per texture. The lease keeps both the
     * `SharedTexture` and the cache key: on destruction it drops the texture
     * first and only then erases the cache entry if the weak ref expired, so
     * sharing between widgets is preserved.
     */
    class TextLease {
    public:
        TextLease() = default;
        TextLease(TextLease&& other) noexcept
            : m_owner(other.m_owner), m_key(other.m_key), m_texture(std::move(other.m_texture)) {
            other.m_owner = nullptr;
            other.m_key = 0;
        }
        TextLease& operator=(TextLease&& other) noexcept {
            if (this != &other) {
                reset();
                m_owner = other.m_owner;
                m_key = other.m_key;
                m_texture = std::move(other.m_texture);
                other.m_owner = nullptr;
                other.m_key = 0;
            }
            return *this;
        }
        TextLease(const TextLease&) = delete;
        TextLease& operator=(const TextLease&) = delete;
        ~TextLease() { reset(); }

        // Drops the strong ref first, then releases the cache entry (no-op
        // when another holder still keeps the texture alive).
        void reset() {
            if (!m_owner) {
                m_texture.reset();
                return;
            }
            m_texture.reset();
            TextureManager* owner = m_owner;
            const uint64_t key = m_key;
            m_owner = nullptr;
            m_key = 0;
            owner->releaseTextCache(key);
        }

        [[nodiscard]] SDL_Texture* get() const { return m_texture.get(); }
        [[nodiscard]] const SharedTexture& shared() const { return m_texture; }
        [[nodiscard]] uint64_t key() const { return m_key; }
        explicit operator bool() const { return static_cast<bool>(m_texture); }

    private:
        friend class TextureManager;
        TextLease(TextureManager* owner, uint64_t key, SharedTexture texture)
            : m_owner(owner), m_key(key), m_texture(std::move(texture)) {}

        TextureManager* m_owner = nullptr;
        uint64_t m_key = 0;
        SharedTexture m_texture;
    };

    explicit TextureManager(SDL_Renderer* renderer);

    ~TextureManager();
    
    /**
     * @brief Check if SDL_image was initialized successfully.
     * @return true if initialization succeeded, false otherwise.
     */
    bool isInitialized() const { return m_initialized; }

    // Loads a texture. Returns SharedTexture.
    // If a texture at the given path is already loaded, returns the existing SharedTexture.
    // Path cache is strong (assets are static and few); no prune/budget.
    SharedTexture loadTexture(std::string_view path);

    // Uncached: always rasterizes a fresh texture and stores no cache entry.
    // Use acquireTextTexture() when the texture must survive between frames.
    SharedTexture createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color);

    // Uncached variant with an explicit font file + size (opens the font per
    // call). Kept for callers that store the result in a Style/Image; does not
    // populate the text cache.
    SharedTexture createTextureFromText(std::string_view text, std::string_view fontPath, int fontSize, const SDL_Color& color);

    // Cached acquire: weak lock or rasterize + weak entry. The returned lease
    // is the strong ref; identical text/font/color shares one texture.
    TextLease acquireTextTexture(std::string_view text, const SharedFont& font, const SDL_Color& color);
    
    // Loads a texture from memory (for embedded assets)
    SharedTexture loadTextureFromMemory(const uint8_t* data, size_t size, std::string_view key);

    // Renders an element into a shared, immutable cache texture.
    // A live weak entry (another widget holds the texture) is returned WITHOUT
    // re-rendering; an expired entry is overwritten by the freshly rendered
    // texture. The returned ref belongs to the caller (the widget).
    SharedTexture renderCache(uint64_t key, int width, int height,
                              const std::function<void(SDL_Renderer*)>& draw);

    // Erases the render-cache entry only when its weak ref expired (no other
    // holder). Safe to call with a key that was already released.
    void releaseRenderCache(uint64_t key);

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
    //
    // clearCache() drops the maps (including the strong path cache). Textures
    // still held by widgets stay alive through their SharedTexture/TextLease;
    // subsequent lookups may rasterize duplicates — call it only on teardown
    // or explicit "forget everything" transitions.
    void clearCache();
    [[nodiscard]] size_t getCacheSize() const;
    [[nodiscard]] size_t getRenderCacheSize() const;
    [[nodiscard]] size_t getTextCacheSize() const;

private:
    // TextShaper key (punkt 6): FNV-1a over text bytes mixed with font
    // identity + color. No std::string allocation on the hot path.
    static uint64_t textCacheKey(std::string_view text, uint64_t fontId, const SDL_Color& color);

    // Rasterizes without touching the caches (shared by uncached create and
    // acquireTextTexture).
    SharedTexture rasterizeTextTexture(std::string_view text, const SharedFont& font, const SDL_Color& color);

    // Text-cache counterpart of releaseRenderCache (used by TextLease).
    void releaseTextCache(uint64_t key);

    SDL_Renderer* m_renderer;
    std::unordered_map<std::string, SharedTexture, StringHash, std::equal_to<>> m_textureCache;
    std::unordered_map<uint64_t, std::weak_ptr<SDL_Texture>> m_renderCache;
    std::unordered_map<uint64_t, std::weak_ptr<SDL_Texture>> m_textCache;
    SharedTexture m_defaultTexture;
    bool m_initialized = false; // SDL_image initialization status
};
