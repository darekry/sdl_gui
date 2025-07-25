#ifndef FONT_MANAGER_HPP
#define FONT_MANAGER_HPP

#include "SDL2/SDL_ttf.h"
#include "sdl_deleters.hpp"
import std.compat;

// Typ dla współdzielonego wskaźnika na czcionkę
using SharedFont = std::shared_ptr<TTF_Font>;
// Klucz dla mapy cache'u czcionek (ścieżka + rozmiar)
struct FontKey {
    std::string path;
    int size;

    FontKey(std::string_view p, int s) : path(p), size(s) {}
    FontKey(std::string&& p, int s) : path(std::move(p)), size(s) {}

    // Operator porównania dla użycia w std::map
    bool operator<(const FontKey& other) const {
        if (path != other.path) {
            return path < other.path;
        }
        return size < other.size;
    }
};

class FontManager {
public:
    // Konstruktor
    FontManager();

    // Destruktor
    ~FontManager();

    // Metoda do ładowania czcionki. Zwraca SharedFont.
    // Jeśli czcionka o danej ścieżce i rozmiarze została już załadowana, zwraca istniejący SharedFont.
    SharedFont loadFont(std::string_view path, int size);

    // Metoda do ładowania domyślnej czcionki
    void loadDefaultFont(std::string_view path, int size);
    
    // Metoda do pobierania domyślnej czcionki
    SharedFont getDefaultFont() const;

    // Metoda do pobierania surowego wskaźnika do czcionki (dla wydajności).
    // UWAGA: Ta metoda nie zarządza pamięcią, jedynie zwraca wskaźnik.
    TTF_Font* getFont(std::string_view path, int size);

    // Metoda do obliczania rozmiaru tekstu
    void getTextSize(std::string_view text, std::string_view fontPath, int fontSize, int* width, int* height);

private:
    std::map<FontKey, SharedFont> m_fonts; // Mapa przechowująca załadowane czcionki
    SharedFont m_defaultFont; // Domyślna czcionka
};

#endif // FONT_MANAGER_HPP