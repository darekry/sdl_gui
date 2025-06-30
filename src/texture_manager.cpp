#include "texture_manager.hpp"
#include "sdl_deleters.hpp"
#include <SDL2/SDL.h>

TextureManager::TextureManager(SDL_Renderer* renderer) : m_renderer(renderer) {
    // Inicjalizacja SDL_image, jeśli nie została jeszcze zainicjowana
    // Sprawdzamy, czy wymagane formaty są już załadowane
    int imgFlags = IMG_INIT_PNG; // Można dodać więcej formatów, np. IMG_INIT_JPG
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
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
    // Załaduj teksturę z pliku
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image %s! SDL_image Error: %s", path.c_str(), IMG_GetError());
        return nullptr; // Zwróć nullptr w przypadku błędu ładowania
    }
    // Utwórz teksturę z powierzchni
    // Utwórz teksturę z powierzchni
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(m_renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface); // Zwolnij powierzchnię po utworzeniu tekstury

    if (!newTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture from %s! SDL Error: %s", path.c_str(), SDL_GetError());
        return nullptr; // Zwróć nullptr w przypadku błędu tworzenia tekstury
    }
    // Utwórz shared_ptr z custom deleterem i zapisz w mapie
    SharedTexture sharedNewTexture(newTexture, SDLTextureDeleter());
    m_textures[path] = sharedNewTexture;

    return sharedNewTexture;
}

SharedTexture TextureManager::createTextureFromText(const std::string& text, std::shared_ptr<TTF_Font> font, SDL_Color color) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Attempting to create texture for text: \"%s\"", text.c_str());
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Font is not loaded!");
        return nullptr;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(font.get(), text.c_str(), color);
    if (!textSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to render text surface! SDL_ttf Error: %s", TTF_GetError());
        return nullptr;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Text surface created successfully (w: %d, h: %d).", textSurface->w, textSurface->h);

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TextureManager ERROR: Unable to create texture from rendered text! SDL Error: %s", SDL_GetError());
        return nullptr;
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "TextureManager: Texture created successfully from text.");

    return {textTexture, SDLTextureDeleter()};
}