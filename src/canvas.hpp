#ifndef CANVAS_HPP
#define CANVAS_HPP

#include "gui.hpp"
#include <SDL2/SDL.h>

class Canvas : public GUIElement {
public:
    Canvas(GUIManager& manager, int x, int y, int width, int height);
    ~Canvas();

    void clear();

    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;

protected:
    void draw([[maybe_unused]] SDL_Renderer* renderer) override;
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;

private:
    SDL_Texture* m_canvasTex = nullptr;
    int m_texW = 0;
    int m_texH = 0;
    bool m_drawing = false;
    SDL_Point m_last{0,0};

    void ensureTexture(SDL_Renderer* renderer);
    void recreateTexture(SDL_Renderer* renderer, int w, int h);
    SDL_Point windowToLocal(int wx, int wy) const;
    void drawSegment(SDL_Renderer* renderer, SDL_Point a, SDL_Point b);
    void putBrush(SDL_Renderer* renderer, int x, int y);

    static constexpr int BRUSH_SIZE = 4;
};

#endif // CANVAS_HPP