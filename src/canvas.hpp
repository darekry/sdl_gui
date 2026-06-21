#pragma once
#include "gui.hpp"
#include <SDL3/SDL.h>

class Canvas : public GUIElement {
public:
    Canvas(GUIManager& manager, int x, int y, int width, int height);
    ~Canvas();

    void clear();
    void setPenColor(SDL_Color color);

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw([[maybe_unused]] SDL_Renderer* renderer) override;
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;

private:
    // Use unique_ptr with custom deleter for automatic texture cleanup
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_canvasTex{nullptr, SDL_DestroyTexture};
    int m_texW = 0;
    int m_texH = 0;
    bool m_drawing = false;
    SDL_Point m_last{0,0};

    void ensureTexture(SDL_Renderer* renderer);
    void recreateTexture(SDL_Renderer* renderer, int w, int h);
    SDL_Point windowToLocal(int wx, int wy) const;
    void drawSegment(SDL_Renderer* renderer, SDL_Point a, SDL_Point b);
    void putBrush(SDL_Renderer* renderer, int x, int y);

    SDL_Color m_penColor = {0, 0, 0, 255};

    static constexpr int BRUSH_SIZE = 4;
};
