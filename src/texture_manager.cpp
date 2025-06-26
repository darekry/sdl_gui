#include "texture_manager.hpp"
#include "sdl_deleters.hpp"
#include <iostream>

TextureManager::TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {
    // Inicjalizacja SDL_image, jeśli nie została jeszcze zainicjowana
    // Sprawdzamy, czy wymagane formaty są już załadowane
    int imgFlags = IMG_INIT_PNG; // Można dodać więcej formatów, np. IMG_INIT_JPG
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        // W przypadku błędu inicjalizacji, można podjąć odpowiednie działania, np. rzucić wyjątek
    }
}

TextureManager::~TextureManager() {
    // SDL_image Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    // IMG_Quit();
}

SharedTexture TextureManager::loadTexture(const std::string& path) {
    // Sprawdź, czy tekstura o danej ścieżce została już załadowana
    if (m_textures.count(path)) {
        return m_textures[path];
    }

    // Załaduj teksturę z pliku
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr; // Zwróć nullptr w przypadku błędu ładowania
    }

    // Utwórz teksturę z powierzchni
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(m_renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface); // Zwolnij powierzchnię po utworzeniu tekstury

    if (!newTexture) {
        std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr; // Zwróć nullptr w przypadku błędu tworzenia tekstury
    }

    // Utwórz shared_ptr z custom deleterem i zapisz w mapie
    SharedTexture sharedNewTexture(newTexture, SDLTextureDeleter());
    m_textures[path] = sharedNewTexture;

    return sharedNewTexture;
}

SharedTexture TextureManager::createTextureFromText(const std::string& text, std::shared_ptr<TTF_Font> font, SDL_Color color) {
    if (!font) {
        std::cerr << "Font is not loaded!" << std::endl;
        return nullptr;
    }

    // Renderuj tekst do powierzchni
    SDL_Surface* textSurface = TTF_RenderText_Blended(font.get(), text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // Utwórz teksturę z powierzchni
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_FreeSurface(textSurface); // Zwolnij powierzchnię

    if (!textTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Zwróć teksturę jako shared_ptr z custom deleterem
    return {textTexture, SDLTextureDeleter()};
}