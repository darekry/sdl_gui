#include "mouse_cursor.hpp"
#include "gui_manager.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace {
constexpr float kDefaultFPS = 12.0f;
}

MouseCursor::MouseCursor(GUIManager& manager)
    : m_manager(manager) {}

MouseCursor::~MouseCursor() = default;

void MouseCursor::setCursorTexture(CursorState state, const std::string& path, int hotspotX, int hotspotY) {
    CursorData& data = m_cursors[state];
    data.texture = m_manager.getTextureManager().loadTexture(path);
    data.hotspotX = hotspotX;
    data.hotspotY = hotspotY;
    data.isAnimated = false;
    data.totalFrames = 0;
}

void MouseCursor::setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows,
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
    data.lastFrameTime = SDL_GetTicks64();

    if (data.texture) {
        int texW = 0;
        int texH = 0;
        SDL_QueryTexture(data.texture.get(), nullptr, nullptr, &texW, &texH);
        data.cols = (data.totalFrames + data.rows - 1) / data.rows;
        if (data.cols <= 0) data.cols = 1;
        data.frameW = (data.cols > 0) ? texW / data.cols : texW;
        data.frameH = (data.rows > 0) ? texH / data.rows : texH;
    }
}

void MouseCursor::setState(CursorState state) {
    if (m_currentState == state) {
        return;
    }

    if (!m_cursors.contains(state)) {
        return;
    }

    m_currentState = state;
    if (m_onStateChanged) {
        m_onStateChanged(state);
    }
}

void MouseCursor::setVisible(bool visible) {
    m_visible = visible;
}

void MouseCursor::setOffset(int offsetX, int offsetY) {
    m_offsetX = offsetX;
    m_offsetY = offsetY;
}

void MouseCursor::getOffset(int& offsetX, int& offsetY) const {
    offsetX = m_offsetX;
    offsetY = m_offsetY;
}

void MouseCursor::setScale(float scale) {
    m_scale = std::max(0.1f, scale);
}

void MouseCursor::update() {
    if (!m_visible) {
        return;
    }

    auto it = m_cursors.find(m_currentState);
    if (it == m_cursors.end()) {
        return;
    }

    if (it->second.isAnimated) {
        updateAnimation(it->second);
    }
}

void MouseCursor::updateAnimation(CursorData& data) {
    if (!data.texture) {
        return;
    }

    uint64_t currentTime = SDL_GetTicks64();
    float elapsedSeconds = static_cast<float>(currentTime - data.lastFrameTime) / 1000.0f;
    if (elapsedSeconds >= data.frameDuration) {
        int frameAdvance = static_cast<int>(std::floor(elapsedSeconds / data.frameDuration));
        data.currentFrame = (data.currentFrame + frameAdvance) % std::max(1, data.totalFrames);
        data.lastFrameTime = currentTime;
    }
}

void MouseCursor::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }

    int mouseX = 0;
    int mouseY = 0;
    SDL_GetMouseState(&mouseX, &mouseY);

    auto it = m_cursors.find(m_currentState);
    if (it == m_cursors.end()) {
        return;
    }

    renderCursor(renderer, it->second, mouseX, mouseY);
}

void MouseCursor::renderCursor(SDL_Renderer* renderer, const CursorData& data, int mouseX, int mouseY) {
    if (!data.texture) {
        return;
    }

    SDL_Rect src = getSrcRect(data);
    SDL_Rect dst{};

    dst.w = static_cast<int>(std::round(static_cast<float>(src.w) * m_scale));
    dst.h = static_cast<int>(std::round(static_cast<float>(src.h) * m_scale));
    dst.x = mouseX - static_cast<int>(std::round(static_cast<float>(data.hotspotX) * m_scale)) + m_offsetX;
    dst.y = mouseY - static_cast<int>(std::round(static_cast<float>(data.hotspotY) * m_scale)) + m_offsetY;

    SDL_RenderCopy(renderer, data.texture.get(), &src, &dst);
}

SDL_Rect MouseCursor::getSrcRect(const CursorData& data) const {
    if (!data.isAnimated || data.totalFrames <= 1 || data.cols <= 0 || data.rows <= 0) {
        int texW = 0;
        int texH = 0;
        SDL_QueryTexture(data.texture.get(), nullptr, nullptr, &texW, &texH);
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

void MouseCursor::setOnStateChanged(std::function<void(CursorState)> callback) {
    m_onStateChanged = std::move(callback);
}
