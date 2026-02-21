#pragma once

#include "SDL2/SDL_ttf.h"
#include "sdl_deleters.hpp"

import std.compat;

// Typ dla współdzielonego wskaźnika na czcionkę
using SharedFont = std::shared_ptr<TTF_Font>;
// Klucz dla mapy cache'u czcionek (ścieżka + rozmiar)
using FontKey = std::pair<std::string, int>;

// Komparator dla klucza cache'u czcionek, umożliwiający transparentne wyszukiwanie
struct FontCacheKeyCompare {
    using is_transparent = void;

    // Porównanie dwóch pełnych kluczy
    bool operator()(const FontKey& lhs, const FontKey& rhs) const {
        return lhs < rhs;
    }

    // Porównanie pełnego klucza z parą (string_view, int)
    bool operator()(const FontKey& lhs, const std::pair<std::string_view, int>& rhs) const {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        return lhs.second < rhs.second;
    }

    // Porównanie pary (string_view, int) z pełnym kluczem
    bool operator()(const std::pair<std::string_view, int>& lhs, const FontKey& rhs) const {
        if (lhs.first < rhs.first) return true;
        if (rhs.first < lhs.first) return false;
        return lhs.second < rhs.second;
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
    std::map<FontKey, SharedFont, FontCacheKeyCompare> m_fontCache; // Mapa przechowująca załadowane czcionki
    SharedFont m_defaultFont; // Domyślna czcionka
};
