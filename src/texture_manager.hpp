#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <string>
#include <map>
#include <memory>

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

private:
    SDL_Renderer* m_renderer;
    std::map<std::string, SharedTexture> m_textures; // Mapa przechowująca załadowane tekstury
};

#endif // TEXTURE_MANAGER_HPP