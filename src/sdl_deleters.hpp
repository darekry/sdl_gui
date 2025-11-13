#pragma once
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL.h"

// Niestandardowy deleter dla TTF_Font
struct TTFFontDeleter {
    void operator()(TTF_Font* font) const {
        if (font) {
            TTF_CloseFont(font);
        }
    }
};

// Niestandardowy deleter dla SDL_Texture
struct SDLTextureDeleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};
