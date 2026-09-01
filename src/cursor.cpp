#include "cursor.hpp"
#include "gui_manager.hpp"

#include "std.hpp"

namespace {
constexpr float kDefaultFPS = 12.0f;
}

Cursor::Cursor(GUIManager& manager)
    : GUIElement(manager, 0, 0, 1, 1) {
    SDL_HideCursor();
}

Cursor::~Cursor() {
    for (auto& opt : m_cursors) {
        if (opt && opt->animation_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(opt->animation_id);
            opt->animation_id = 0;
        }
    }
    SDL_ShowCursor();
}

void Cursor::setCursorTexture(CursorState state, const std::string& path, int hotspotX, int hotspotY) {
    auto& opt = m_cursors[static_cast<size_t>(state)];
    if (!opt) opt.emplace();
    CursorData& data = *opt;
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

    auto& opt = m_cursors[static_cast<size_t>(state)];
    if (!opt) opt.emplace();
    CursorData& data = *opt;
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
        auto& opt = m_cursors[static_cast<size_t>(state)];
        if (opt) {
            CursorData& d = *opt;
            if (!d.loop && d.currentFrame >= d.totalFrames - 1) {
                return;
            }
            d.currentFrame = (d.currentFrame + 1) % d.totalFrames;
        }
    });

    if (data.texture) {
        int texW = TextureWidth(data.texture.get());
        int texH = TextureHeight(data.texture.get());
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

bool Cursor::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        m_mouseX = static_cast<int>(event.motion.x);
        m_mouseY = static_cast<int>(event.motion.y);
        m_hasMousePos = true;
    }
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

    int mouseX = m_mouseX;
    int mouseY = m_mouseY;
    if (!m_hasMousePos) {  /* no events yet -> fall back to system state */
        float _mx, _my;
        SDL_GetMouseState(&_mx, &_my);
        mouseX = static_cast<int>(_mx);
        mouseY = static_cast<int>(_my);
    }

    auto& opt = m_cursors[static_cast<size_t>(m_currentState)];
    if (!opt) {
        return;
    }

    renderCursor(renderer, *opt, mouseX, mouseY);
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

    RenderTexture(renderer, data.texture.get(), &src, &dst);
}

SDL_Rect Cursor::getSrcRect(const CursorData& data) const {
    if (!data.isAnimated || data.totalFrames <= 1 || data.cols <= 0 || data.rows <= 0) {
        int texW = TextureWidth(data.texture.get());
        int texH = TextureHeight(data.texture.get());
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
