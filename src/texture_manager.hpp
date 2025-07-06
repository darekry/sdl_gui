#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "font_manager.hpp" // Potrzebne do stworzenia domyślnej tekstury
import std.compat;

// Typ dla współdzielonego wskaźnika na teksturę
using SharedTexture = std::shared_ptr<SDL_Texture>;

class TextureManager {
public:
    // Konstruktor
    TextureManager(SDL_Renderer* renderer);

    // Destruktor
    ~TextureManager();

    // Metoda do ładowania tekstury. Zwraca SharedTexture.
    // Jeśli tekstura o danej ścieżce została już załadowana, zwraca istniejący SharedTexture.
    SharedTexture loadTexture(const std::string& path);

    // Metoda do tworzenia tekstury z tekstu.
    SharedTexture createTextureFromText(const std::string& text, std::shared_ptr<TTF_Font> font, SDL_Color color);

    // Metoda do dodawania istniejącej tekstury i przejmowania nad nią własności
    SharedTexture addTexture(const std::string& key, SDL_Texture* texture);

    // Metoda do tworzenia domyślnej tekstury
    void createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, const std::string& text);

    // Metoda do pobierania domyślnej tekstury
    SharedTexture getDefaultTexture();

private:
    SDL_Renderer* m_renderer;
    std::map<std::string, SharedTexture> m_textures; // Mapa przechowująca załadowane tekstury
    SharedTexture m_defaultTexture; // Domyślna tekstura
};

#endif // TEXTURE_MANAGER_HPP