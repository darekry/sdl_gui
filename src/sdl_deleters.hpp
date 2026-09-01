#pragma once
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL.h>

struct TTFFontDeleter {
    void operator()(TTF_Font* font) const {
        if (font) {
            TTF_CloseFont(font);
        }
    }
};

struct SDLTextureDeleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
};
