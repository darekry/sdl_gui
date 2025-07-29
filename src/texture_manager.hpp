#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "font_manager.hpp" // Potrzebne do stworzenia domyślnej tekstury
#include <memory>
#include <string_view>


// Typ dla współdzielonego wskaźnika na teksturę
using SharedTexture = std::shared_ptr<SDL_Texture>;

class TextureManager {
public:
    // Konstruktor
    explicit TextureManager(SDL_Renderer* renderer);

    // Destruktor
    ~TextureManager();

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

    // Metoda do tworzenia domyślnej tekstury
    void createDefaultTexture(SDL_Renderer* renderer, FontManager& fontManager, std::string_view text);

    // Metoda do pobierania domyślnej tekstury
    SharedTexture getDefaultTexture() const;

private:
    SDL_Renderer* m_renderer;
    std::map<std::string, SharedTexture, std::less<>> m_textures; // Mapa przechowująca załadowane tekstury
    SharedTexture m_defaultTexture; // Domyślna tekstura
};
#endif // TEXTURE_MANAGER_HPP