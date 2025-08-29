#include "canvas.hpp"
#include "gui_manager.hpp"
#include <algorithm>
#include <cmath>

namespace {
    inline int iclamp(int v, int lo, int hi) {
        return std::max(lo, std::min(v, hi));
    }
}

Canvas::Canvas(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
    // Utwórz początkową teksturę i wyczyść na biało
    ensureTexture(m_manager.getRenderer());
    clear();
}

Canvas::~Canvas() {
    if (m_canvasTex) {
        SDL_DestroyTexture(m_canvasTex);
        m_canvasTex = nullptr;
    }
}

void Canvas::ensureTexture(SDL_Renderer* renderer) {
    if (!renderer) return;
    if (m_width <= 0 || m_height <= 0) return;

    if (m_canvasTex == nullptr || m_texW != m_width || m_texH != m_height) {
        recreateTexture(renderer, m_width, m_height);
    }
}

void Canvas::recreateTexture(SDL_Renderer* renderer, int w, int h) {
    if (m_canvasTex) {
        SDL_DestroyTexture(m_canvasTex);
        m_canvasTex = nullptr;
    }
    if (w <= 0 || h <= 0) {
        m_texW = m_texH = 0;
        return;
    }

    m_canvasTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!m_canvasTex) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Canvas: SDL_CreateTexture failed: %s", SDL_GetError());
        m_texW = m_texH = 0;
        return;
    }
    SDL_SetTextureBlendMode(m_canvasTex, SDL_BLENDMODE_BLEND);
    m_texW = w;
    m_texH = h;

    // Wyczyść na biało
    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, m_canvasTex);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, oldTarget);
}

void Canvas::clear() {
    SDL_Renderer* renderer = m_manager.getRenderer();
    ensureTexture(renderer);
    if (!renderer || !m_canvasTex) return;

    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, m_canvasTex);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, oldTarget);
}

SDL_Point Canvas::windowToLocal(int wx, int wy) const {
    SDL_Point abs = getAbsolutePosition();
    return SDL_Point{ wx - abs.x, wy - abs.y };
}

void Canvas::putBrush(SDL_Renderer* renderer, int x, int y) {
    if (!renderer || !m_canvasTex) return;

    // Prostokąt pędzla wycentrowany w (x, y)
    const int half = BRUSH_SIZE / 2;
    int rx = x - half;
    int ry = y - half;
    int rw = BRUSH_SIZE;
    int rh = BRUSH_SIZE;

    // Przytnij do granic tekstury
    int maxX = m_texW - 1;
    int maxY = m_texH - 1;

    // Oblicz wymiary po przycięciu
    if (rx < 0) { rw += rx; rx = 0; }
    if (ry < 0) { rh += ry; ry = 0; }
    if (rx + rw - 1 > maxX) { rw = maxX - rx + 1; }
    if (ry + rh - 1 > maxY) { rh = maxY - ry + 1; }

    if (rw <= 0 || rh <= 0) return;

    SDL_Rect r{ rx, ry, rw, rh };
    SDL_RenderFillRect(renderer, &r);
}

void Canvas::drawSegment(SDL_Renderer* renderer, SDL_Point a, SDL_Point b) {
    if (!renderer || !m_canvasTex) return;

    // Ustaw rysowanie na teksturę płótna
    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, m_canvasTex);

    // Kolor pędzla: czarny, pełna alfa
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Proste próbkowanie punktów na odcinku i stemplowanie pędzla
    int dx = b.x - a.x;
    int dy = b.y - a.y;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) {
        int cx = iclamp(a.x, 0, m_texW - 1);
        int cy = iclamp(a.y, 0, m_texH - 1);
        putBrush(renderer, cx, cy);
        SDL_SetRenderTarget(renderer, oldTarget);
        return;
    }

    float fx = static_cast<float>(a.x);
    float fy = static_cast<float>(a.y);
    float sx = static_cast<float>(dx) / static_cast<float>(steps);
    float sy = static_cast<float>(dy) / static_cast<float>(steps);

    for (int i = 0; i <= steps; ++i) {
        int cx = iclamp(static_cast<int>(std::round(fx)), 0, m_texW - 1);
        int cy = iclamp(static_cast<int>(std::round(fy)), 0, m_texH - 1);
        putBrush(renderer, cx, cy);
        fx += sx;
        fy += sy;
    }

    SDL_SetRenderTarget(renderer, oldTarget);
}

bool Canvas::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    SDL_Renderer* renderer = m_manager.getRenderer();
    ensureTexture(renderer);

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (contains(e.button.x, e.button.y)) {
            SDL_Point local = windowToLocal(e.button.x, e.button.y);
            local.x = iclamp(local.x, 0, std::max(0, m_texW - 1));
            local.y = iclamp(local.y, 0, std::max(0, m_texH - 1));
            m_drawing = true;
            m_last = local;
            // Narysuj punkt startowy
            SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, m_canvasTex);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            putBrush(renderer, local.x, local.y);
            SDL_SetRenderTarget(renderer, oldTarget);

            // Przechwyć mysz, by kontynuować rysowanie podczas przeciągania
            m_manager.captureMouse(this);
            return true;
        }
    } else if (e.type == SDL_MOUSEMOTION) {
        if (m_drawing) {
            SDL_Point cur = windowToLocal(e.motion.x, e.motion.y);
            cur.x = iclamp(cur.x, 0, std::max(0, m_texW - 1));
            cur.y = iclamp(cur.y, 0, std::max(0, m_texH - 1));

            drawSegment(renderer, m_last, cur);
            m_last = cur;
            return true;
        }
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        if (m_drawing) {
            m_drawing = false;
            m_manager.releaseMouse();
            return true;
        }
    }

    // Dla pozostałych zdarzeń deleguj do bazowej logiki (hover/tooltip itp.)
    return GUIElement::handleEvent(e);
}

const char* Canvas::getComponentType() const {
    return "Canvas";
}

void Canvas::draw([[maybe_unused]] SDL_Renderer* renderer) {
    // Nieużywane, bo wantsDirectRender() == true, rysujemy w drawDirect
}

void Canvas::drawDirect(SDL_Renderer* renderer) {
    ensureTexture(renderer);
    if (!renderer || !m_canvasTex) return;

    SDL_Point abs = getAbsolutePosition();
    SDL_Rect dest{ abs.x, abs.y, m_width, m_height };
    SDL_RenderCopy(renderer, m_canvasTex, nullptr, &dest);
}