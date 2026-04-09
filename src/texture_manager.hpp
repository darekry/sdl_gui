#pragma once

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "font_manager.hpp" // Potrzebne do stworzenia domyślnej tekstury

import std.compat;


// Typ dla współdzielonego wskaźnika na teksturę
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
    // Konstruktor
    explicit TextureManager(SDL_Renderer* renderer);

    // Destruktor
    ~TextureManager();
    
    /**
     * @brief Check if SDL_image was initialized successfully.
     * @return true if initialization succeeded, false otherwise.
     */
    bool isInitialized() const { return m_initialized; }

    // Metoda do ładowania tekstury. Zwraca SharedTexture.
    // Jeśli tekstura o danej ścieżce została już załadowana, zwraca istniejący SharedTexture.
    SharedTexture loadTexture(std::string_view path);

    // Metoda do tworzenia tekstury z tekstu.
    SharedTexture createTextureFromText(std::string_view text, const SharedFont& font, const SDL_Color& color);

    // Metoda do dodawania istniejącej tekstury i przejmowania nad nią własności
    SharedTexture addTexture(std::string_view key, SDL_Texture* texture);
    SharedTexture addTexture(std::string_view key, SharedTexture texture);

    // Metoda do pobierania tekstury po kluczu
    SharedTexture getTexture(std::string_view key) const;
    
    // Metoda do sprawdzania, czy tekstura o danym kluczu istnieje
    bool hasTexture(std::string_view key) const;

    // Metoda do zapytania rozmiaru tekstury (ładuje teksturę jeśli nie jest załadowana)
    // Zwraca true oraz ustawia width/height jeśli tekstura jest dostępna; w przeciwnym razie false.
    bool queryTexture(std::string_view key, int& width, int& height);
    
    // Metoda do tworzenia domyślnej tekstury
    void createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text);
    
    // Metoda do pobierania domyślnej tekstury
    SharedTexture getDefaultTexture() const;

private:
    SDL_Renderer* m_renderer;
    std::map<std::string, SharedTexture, std::less<>> m_textureCache; // Mapa przechowująca załadowane tekstury
    SharedTexture m_defaultTexture; // Domyślna tekstura
    bool m_initialized = false; // SDL_image initialization status
};
