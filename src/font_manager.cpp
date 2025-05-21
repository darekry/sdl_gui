#include "font_manager.hpp"
#include "SDL2/SDL.h" // Potrzebne do SDL_GetError()

FontManager::FontManager() {
    // Inicjalizacja SDL_ttf, jeśli nie została jeszcze zainicjowana
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        // W przypadku błędu inicjalizacji, można podjąć odpowiednie działania, np. rzucić wyjątek
    }
}

FontManager::~FontManager() {
    // TTF_Quit nie jest konieczne tutaj, ponieważ powinno być wywołane raz na koniec działania aplikacji
    // TTF_Quit();
}

SharedFont FontManager::loadFont(const std::string& path, int size) {
    // Utwórz klucz dla cache'u
    FontKey key = {path, size};

    // Sprawdź, czy czcionka o danej ścieżce i rozmiarze została już załadowana
    if (m_fonts.count(key)) {
        return m_fonts[key];
    }

    // Załaduj czcionkę z pliku
    TTF_Font* loadedFont = TTF_OpenFont(path.c_str(), size);
    if (!loadedFont) {
        std::cerr << "Unable to load font " << path << " with size " << size << "! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr; // Zwróć nullptr w przypadku błędu ładowania
    }

    // Utwórz shared_ptr z custom deleterem i zapisz w mapie
    SharedFont sharedNewFont(loadedFont, TTFFontDeleter());
    m_fonts[key] = sharedNewFont;

    return sharedNewFont;
}