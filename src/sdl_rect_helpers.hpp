#pragma once
#include <SDL3/SDL.h>

inline SDL_FRect SDLRectToFRect(const SDL_Rect& r) {
    return {static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)};
}

inline SDL_FRect SDLRectToFRect(int x, int y, int w, int h) {
    return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
}

inline void RenderFillRect(SDL_Renderer* renderer, const SDL_Rect& r) {
    SDL_FRect fr = SDLRectToFRect(r);
    SDL_RenderFillRect(renderer, &fr);
}

inline void RenderFillRect(SDL_Renderer* renderer, const SDL_FRect& fr) {
    SDL_RenderFillRect(renderer, &fr);
}

inline void RenderRect(SDL_Renderer* renderer, const SDL_Rect& r) {
    SDL_FRect fr = SDLRectToFRect(r);
    SDL_RenderRect(renderer, &fr);
}

inline void RenderRect(SDL_Renderer* renderer, const SDL_FRect& fr) {
    SDL_RenderRect(renderer, &fr);
}

inline void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect& dst) {
    SDL_FRect fDst = SDLRectToFRect(dst);
    SDL_RenderTexture(renderer, texture, nullptr, &fDst);
}

inline void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* src, const SDL_Rect* dst) {
    SDL_FRect fSrc, fDst;
    if (src) fSrc = SDLRectToFRect(*src);
    if (dst) fDst = SDLRectToFRect(*dst);
    SDL_RenderTexture(renderer, texture, src ? &fSrc : nullptr, dst ? &fDst : nullptr);
}

inline void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& dst) {
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
}

inline void RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* src, const SDL_FRect* dst) {
    SDL_RenderTexture(renderer, texture, src, dst);
}

inline void RenderLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
    SDL_RenderLine(renderer, static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2));
}

inline void RenderLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2) {
    SDL_RenderLine(renderer, x1, y1, x2, y2);
}

inline void RenderPoint(SDL_Renderer* renderer, int x, int y) {
    SDL_RenderPoint(renderer, static_cast<float>(x), static_cast<float>(y));
}

inline void RenderPoint(SDL_Renderer* renderer, float x, float y) {
    SDL_RenderPoint(renderer, x, y);
}

inline float I2F(int v) { return static_cast<float>(v); }

inline SDL_FColor ColorToFColor(SDL_Color c) {
    constexpr float s = 1.0f / 255.0f;
    return {c.r * s, c.g * s, c.b * s, c.a * s};
}

inline void SetDrawColor(SDL_Renderer* renderer, SDL_Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

inline int TextureWidth(SDL_Texture* texture) {
    float w = 0, h = 0;
    SDL_GetTextureSize(texture, &w, &h);
    return static_cast<int>(w);
}

inline int TextureHeight(SDL_Texture* texture) {
    float w = 0, h = 0;
    SDL_GetTextureSize(texture, &w, &h);
    return static_cast<int>(h);
}

// RAII: przełącza render target na teksturę i przywraca target/viewport/clip w destruktorze.
// Uwaga: SDL_GetRenderClipRect zwraca true nawet gdy clip jest wyłączony (flaga sukcesu) —
// stan clipa trzeba czytać przez SDL_RenderClipEnabled, inaczej przywrócenie clip=0,0,0x0
// przycina całą resztę renderowania do niczego.
class ScopedRenderTarget {
public:
    ScopedRenderTarget(SDL_Renderer* renderer, SDL_Texture* texture)
        : m_renderer(renderer), m_oldTarget(SDL_GetRenderTarget(renderer)) {
        SDL_GetRenderViewport(renderer, &m_oldViewport);
        m_hadClip = SDL_RenderClipEnabled(renderer);
        if (m_hadClip) {
            SDL_GetRenderClipRect(renderer, &m_oldClip);
        }
        SDL_SetRenderTarget(renderer, texture);
    }

    ~ScopedRenderTarget() {
        SDL_SetRenderTarget(m_renderer, m_oldTarget);
        SDL_SetRenderViewport(m_renderer, &m_oldViewport);
        SDL_SetRenderClipRect(m_renderer, m_hadClip ? &m_oldClip : nullptr);
    }

    ScopedRenderTarget(const ScopedRenderTarget&) = delete;
    ScopedRenderTarget& operator=(const ScopedRenderTarget&) = delete;

private:
    SDL_Renderer* m_renderer;
    SDL_Texture* m_oldTarget;
    SDL_Rect m_oldViewport;
    SDL_Rect m_oldClip;
    bool m_hadClip = false;
};

// Wyśrodkowanie prostokąta w kontenerze (wspólna logika dialogów i menu)
inline SDL_Point CenterRect(int containerW, int containerH, int w, int h) {
    return {(containerW - w) / 2, (containerH - h) / 2};
}
