#include "cursor.hpp"
#include "gui_manager.hpp"

import std.compat;

namespace {
constexpr float kDefaultFPS = 12.0f;
}

Cursor::Cursor(GUIManager& manager)
    : GUIElement(manager, 0, 0, 1, 1) {
    SDL_HideCursor();
}

Cursor::~Cursor() {
    // Cancel all active animations to prevent dangling pointer callbacks
    for (auto& [state, data] : m_cursors) {
        if (data.animation_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(data.animation_id);
            data.animation_id = 0;
        }
    }
    SDL_ShowCursor();
}

void Cursor::setCursorTexture(CursorState state, const std::string& path, int hotspotX, int hotspotY) {
    CursorData& data = m_cursors[state];
    data.texture = m_manager.getTextureManager().loadTexture(path);
    data.hotspotX = hotspotX;
    data.hotspotY = hotspotY;
    data.isAnimated = false;
    data.totalFrames = 0;
}

void Cursor::setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows,
                               float fps, int hotspotX, int hotspotY) {
    if (totalFrames <= 0) {
        setCursorTexture(state, path, hotspotX, hotspotY);
        return;
    }

    CursorData& data = m_cursors[state];
    data.texture = m_manager.getTextureManager().loadTexture(path);
    data.hotspotX = hotspotX;
    data.hotspotY = hotspotY;
    data.isAnimated = true;
    data.totalFrames = totalFrames;
    data.rows = std::max(1, rows);
    data.currentFrame = 0;
    data.frameDuration = (fps > 0.0f) ? (1.0f / fps) : (1.0f / kDefaultFPS);

    if (data.animation_id != 0) {
        m_manager.getAnimationManager()->removeAnimation(data.animation_id);
    }

    data.animation_id = m_manager.getAnimationManager()->addAnimation(static_cast<uint32_t>(data.frameDuration * 1000), [this, state]() {
        auto it = m_cursors.find(state);
        if (it != m_cursors.end()) {
            CursorData& data = it->second;
            if (!data.loop && data.currentFrame >= data.totalFrames - 1) {
                return;
            }
            data.currentFrame = (data.currentFrame + 1) % data.totalFrames;
        }
    });

    if (data.texture) {
        int texW = 0;
        int texH = 0;
        {  float _fw=0,_fh=0; SDL_GetTextureSize(data.texture.get(), &_fw, &_fh); texW=static_cast<int>(_fw); texH=static_cast<int>(_fh); }
        data.cols = (data.totalFrames + data.rows - 1) / data.rows;
        if (data.cols <= 0) data.cols = 1;
        data.frameW = (data.cols > 0) ? texW / data.cols : texW;
        data.frameH = (data.rows > 0) ? texH / data.rows : texH;
    }
}

void Cursor::setState(CursorState state) {
    if (m_currentState == state) {
        return;
    }

    m_currentState = state;
    if (m_onStateChanged) {
        m_onStateChanged(state);
    }
}

void Cursor::setOffset(int offsetX, int offsetY) {
    m_offsetX = offsetX;
    m_offsetY = offsetY;
}

void Cursor::getOffset(int& offsetX, int& offsetY) const {
    offsetX = m_offsetX;
    offsetY = m_offsetY;
}

void Cursor::setScale(float scale) {
    m_scale = std::max(0.1f, scale);
}

void Cursor::setOnStateChanged(std::function<void(CursorState)> callback) {
    m_onStateChanged = std::move(callback);
}

bool Cursor::handleEvent(const SDL_Event& /*event*/) {
    return false;
}

const char* Cursor::getComponentType() const {
    return "Cursor";
}

void Cursor::setVisible(bool visible) {
    if (m_visible == visible) {
        return;
    }
    GUIElement::setVisible(visible);
    if (visible) { SDL_HideCursor(); } else { SDL_ShowCursor(); };
}

void Cursor::draw(SDL_Renderer* /*renderer*/) {
    // Cursor does not use cached drawing.
}

void Cursor::renderOverlay(SDL_Renderer* renderer) {
    if (!isVisible()) {
        return;
    }

    int mouseX = 0;
    int mouseY = 0;
    {  float _mx,_my; SDL_GetMouseState(&_mx, &_my); mouseX = static_cast<int>(_mx); mouseY = static_cast<int>(_my); }

    auto it = m_cursors.find(m_currentState);
    if (it == m_cursors.end()) {
        return;
    }

    CursorData& data = it->second;
    renderCursor(renderer, data, mouseX, mouseY);
}

void Cursor::renderCursor(SDL_Renderer* renderer, const CursorData& data, int mouseX, int mouseY) {
    if (!data.texture) {
        return;
    }

    SDL_Rect src = getSrcRect(data);
    SDL_Rect dst{};

    dst.w = static_cast<int>(std::round(static_cast<float>(src.w) * m_scale));
    dst.h = static_cast<int>(std::round(static_cast<float>(src.h) * m_scale));
    dst.x = mouseX - static_cast<int>(std::round(static_cast<float>(data.hotspotX) * m_scale)) + m_offsetX;
    dst.y = mouseY - static_cast<int>(std::round(static_cast<float>(data.hotspotY) * m_scale)) + m_offsetY;

    { SDL_FRect _sr = {static_cast<float>(src.x), static_cast<float>(src.y), static_cast<float>(src.w), static_cast<float>(src.h)}; SDL_FRect _dr = {static_cast<float>(dst.x), static_cast<float>(dst.y), static_cast<float>(dst.w), static_cast<float>(dst.h)}; SDL_RenderTexture(renderer, data.texture.get(), &_sr, &_dr); }
}

SDL_Rect Cursor::getSrcRect(const CursorData& data) const {
    if (!data.isAnimated || data.totalFrames <= 1 || data.cols <= 0 || data.rows <= 0) {
        int texW = 0;
        int texH = 0;
        {  float _fw=0,_fh=0; SDL_GetTextureSize(data.texture.get(), &_fw, &_fh); texW=static_cast<int>(_fw); texH=static_cast<int>(_fh); }
        return SDL_Rect{0, 0, texW, texH};
    }

    int frameIndex = std::clamp(data.currentFrame, 0, std::max(0, data.totalFrames - 1));
    int col = frameIndex % data.cols;
    int row = frameIndex / data.cols;

    SDL_Rect src{};
    src.x = col * data.frameW;
    src.y = row * data.frameH;
    src.w = data.frameW;
    src.h = data.frameH;
    return src;
}
