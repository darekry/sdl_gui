#include "animated_image.hpp"
#include "gui_manager.hpp"
#include "sdl_deleters.hpp"
#include <SDL3/SDL.h>

#include "std.hpp"

AnimatedImage::AnimatedImage(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
    markDirty();
}

AnimatedImage::~AnimatedImage() {
    // ensure any timers are stopped by parent cleanup; nothing special to do here
    stopFrameAnimation();
}

void AnimatedImage::setSpriteSheet(const std::string& path, int totalFrames, int rows, int frameW, int frameH) {
    m_texturePath = path;
    m_totalFrames = std::max(0, totalFrames);
    m_rows = std::max(1, rows);
    if (frameW > 0) m_frameW = frameW;
    if (frameH > 0) m_frameH = frameH;

    // load texture (cached by TextureManager)
    m_texture = m_manager.getTextureManager().loadTexture(path);

    recalcFrameGeometry();
    setFrame(0);
    markDirty();
}

void AnimatedImage::ensureTextureLoaded() {
    if (!m_texture && !m_texturePath.empty()) {
        m_texture = m_manager.getTextureManager().loadTexture(m_texturePath);
    }
}

void AnimatedImage::recalcFrameGeometry() {
    ensureTextureLoaded();
    if (!m_texture) {
        m_cols = 0;
        m_frameW = m_frameH = 0;
        return;
    }

    int texW = 0, texH = 0;
    texW = TextureWidth(m_texture.get()); texH = TextureHeight(m_texture.get());

    // columns = ceil(totalFrames / rows)
    m_cols = (m_totalFrames + m_rows - 1) / m_rows;
    if (m_cols <= 0) m_cols = 1;

    if (m_frameW <= 0) {
        if (m_cols > 0) m_frameW = texW / m_cols;
        else m_frameW = texW;
    }
    if (m_frameH <= 0) {
        m_frameH = texH / m_rows;
    }

    // clamp to texture size
    m_frameW = std::max(0, std::min(m_frameW, texW));
    m_frameH = std::max(0, std::min(m_frameH, texH));

    updateSrcRect();
}

void AnimatedImage::updateSrcRect() {
    if (m_totalFrames <= 0 || m_cols <= 0) {
        m_srcRect = {0,0,0,0};
        return;
    }
    int idx = std::clamp(m_currentFrame, 0, m_totalFrames - 1);
    int col = idx % m_cols;
    int row = idx / m_cols;
    m_srcRect.x = col * m_frameW;
    m_srcRect.y = row * m_frameH;
    m_srcRect.w = m_frameW;
    m_srcRect.h = m_frameH;
}

void AnimatedImage::advanceFrameIfNeeded() {
    int computedFrame = static_cast<int>(std::round(m_animFrame));
    computedFrame = std::clamp(computedFrame, 0, std::max(0, m_totalFrames - 1));
    if (computedFrame != m_currentFrame) {
        m_currentFrame = computedFrame;
        updateSrcRect();
        if (m_onFrameChanged) m_onFrameChanged(m_currentFrame);
    }
}

void AnimatedImage::draw(SDL_Renderer* renderer) {
    ensureTextureLoaded();
    if (!m_texture || m_totalFrames <= 0 || m_frameW <= 0 || m_frameH <= 0) {
        return;
    }

    advanceFrameIfNeeded();

    // Prepare destination rect according to scale mode
    SDL_Rect dst{0,0, m_width, m_height};

    if (m_scaleMode == ScaleMode::None) {
        dst.w = m_frameW;
        dst.h = m_frameH;
    } else if (m_scaleMode == ScaleMode::Center) {
        dst.w = m_frameW;
        dst.h = m_frameH;
        if (dst.w > m_width || dst.h > m_height) {
            // if greater than widget, clamp
            dst.w = std::min(dst.w, m_width);
            dst.h = std::min(dst.h, m_height);
        }
        dst.x = (m_width - dst.w) / 2;
        dst.y = (m_height - dst.h) / 2;
    } else { // Fit
        // preserve aspect optionally
        if (m_preserveAspect) {
            float srcRatio = (m_frameW > 0) ? (static_cast<float>(m_frameW) / static_cast<float>(m_frameH)) : 1.0f;
            float dstRatio = (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : srcRatio;
            if (srcRatio > dstRatio) {
                // limited by width
                dst.w = m_width;
                dst.h = static_cast<int>(std::round(static_cast<float>(m_width) / srcRatio));
                dst.y = (m_height - dst.h) / 2;
            } else {
                dst.h = m_height;
                dst.w = static_cast<int>(std::round(static_cast<float>(m_height) * srcRatio));
                dst.x = (m_width - dst.w) / 2;
            }
        } else {
            dst.w = m_width;
            dst.h = m_height;
        }
    }

    // Render current frame from sprite sheet
    RenderTexture(renderer, m_texture.get(), &m_srcRect, &dst);
}

bool AnimatedImage::wantsDirectRender() const {
    return !m_useCache;
}

void AnimatedImage::drawDirect(SDL_Renderer* renderer) {
    // Direct drawing into the main renderer (coordinates are absolute)
    ensureTextureLoaded();
    if (!m_texture || m_totalFrames <= 0 || m_frameW <= 0 || m_frameH <= 0) {
        return;
    }

    advanceFrameIfNeeded();

    auto absPos = getAbsolutePosition();
    SDL_Rect dst{absPos.x, absPos.y, m_width, m_height};

    if (m_scaleMode == ScaleMode::None) {
        dst.w = m_frameW;
        dst.h = m_frameH;
    } else if (m_scaleMode == ScaleMode::Center) {
        dst.w = m_frameW;
        dst.h = m_frameH;
        if (dst.w > m_width || dst.h > m_height) {
            dst.w = std::min(dst.w, m_width);
            dst.h = std::min(dst.h, m_height);
        }
        dst.x = absPos.x + (m_width - dst.w) / 2;
        dst.y = absPos.y + (m_height - dst.h) / 2;
    } else { // Fit
        if (m_preserveAspect) {
            float srcRatio = (m_frameW > 0) ? (static_cast<float>(m_frameW) / static_cast<float>(m_frameH)) : 1.0f;
            float dstRatio = (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : srcRatio;
            if (srcRatio > dstRatio) {
                dst.w = m_width;
                dst.h = static_cast<int>(std::round(static_cast<float>(m_width) / srcRatio));
                dst.x = absPos.x;
                dst.y = absPos.y + (m_height - dst.h) / 2;
            } else {
                dst.h = m_height;
                dst.w = static_cast<int>(std::round(static_cast<float>(m_height) * srcRatio));
                dst.x = absPos.x + (m_width - dst.w) / 2;
                dst.y = absPos.y;
            }
        } else {
            dst.w = m_width;
            dst.h = m_height;
            dst.x = absPos.x;
            dst.y = absPos.y;
        }
    }

    RenderTexture(renderer, m_texture.get(), &m_srcRect, &dst);
}

void AnimatedImage::setFrame(int frameIndex) {
    int clamped = 0;
    if (m_totalFrames > 0) {
        clamped = std::clamp(frameIndex, 0, m_totalFrames - 1);
    }
    if (clamped != m_currentFrame) {
        m_currentFrame = clamped;
        m_animFrame = static_cast<float>(m_currentFrame);
        updateSrcRect();
        markDirty();
        if (m_onFrameChanged) m_onFrameChanged(m_currentFrame);
    }
}

void AnimatedImage::setFPS(float fps) {
    if (fps <= 0.0f) return;
    setFrameDuration(1.0f / fps);
}

void AnimatedImage::setFrameDuration(float secondsPerFrame) {
    if (secondsPerFrame <= 0.0f) return;
    m_frameDuration = secondsPerFrame;
    // If playing via timer, restart timer with new interval
    if (m_isPlaying) {
        // restart timer by stopping and starting play()
        pause();
        play();
    }
}

void AnimatedImage::setLoop(bool loop) {
    m_loop = loop;
}

void AnimatedImage::setUseCache(bool useCache) {
    if (m_useCache == useCache) return;
    m_useCache = useCache;
    // If disabling cache, free existing cached texture to avoid stale content.
    if (!m_useCache) {
        if (m_cachedTexture) {
            m_cachedTexture.reset();
        }
    }
    markDirty();
}

void AnimatedImage::setScaleMode(ScaleMode mode) {
    m_scaleMode = mode;
    markDirty();
}

void AnimatedImage::setPreserveAspect(bool preserve) {
    m_preserveAspect = preserve;
    markDirty();
}

int AnimatedImage::getCurrentFrame() const {
    return m_currentFrame;
}

int AnimatedImage::getTotalFrames() const {
    return m_totalFrames;
}

bool AnimatedImage::isPlaying() const {
    return m_isPlaying;
}

void AnimatedImage::play() {
    if (m_totalFrames <= 0) return;
    if (m_isPlaying) return;
    m_isPlaying = true;

    // Use TimerManager to advance frames at fixed interval
    const uint32_t interval = static_cast<uint32_t>(std::max(1.0f, m_frameDuration * 1000.0f));
    // store timer id in a local variable via capture by value is not possible, so use this pointer and stopTimer by tracking id in closure
    // We'll keep a repeating timer that increments frame
    // Use GUIElement::startTimer to add timer
    // Timer callback will be called with GUIElement* target == this
    m_playTimerId = startTimer(interval, false, [](GUIElement* self) {
        if (!self) return;
        auto* widget = static_cast<AnimatedImage*>(self);
        if (!widget->m_isPlaying) return;
        int next = widget->m_currentFrame + 1;
        if (next >= widget->m_totalFrames) {
            if (widget->m_loop) {
                next = 0;
            } else {
                // stop at last frame
                widget->pause();
                if (widget->m_onAnimationEnd) widget->m_onAnimationEnd();
                return;
            }
        }
        widget->setFrame(next);
    });

    // markDirty to ensure first frame rendered immediately
    markDirty();

    // timer id stored in m_playTimerId so we can stop it on pause/stop
}

void AnimatedImage::pause() {
    m_isPlaying = false;
    if (m_playTimerId) {
        stopTimer(m_playTimerId);
        m_playTimerId = 0;
    }
    markDirty();
}

void AnimatedImage::stop() {
    m_isPlaying = false;
    if (m_playTimerId) {
        stopTimer(m_playTimerId);
        m_playTimerId = 0;
    }
    setFrame(0);
    if (m_animTickTimerId) {
        stopTimer(m_animTickTimerId);
        m_animTickTimerId = 0;
    }
}

void AnimatedImage::setOnAnimationEnd(std::function<void()> cb) {
    m_onAnimationEnd = std::move(cb);
}

void AnimatedImage::setOnFrameChanged(std::function<void(int)> cb) {
    m_onFrameChanged = std::move(cb);
}

void AnimatedImage::animateToFrame(int targetFrame, uint32_t duration_ms, bool loop) {
    ensureTextureLoaded();
    if (!m_manager.getAnimationManager()) {
        // Fallback: immediate set
        setFrame(targetFrame);
        if (m_onAnimationEnd) m_onAnimationEnd();
        return;
    }
    // clamp target
    int clamped = (m_totalFrames > 0) ? std::clamp(targetFrame, 0, m_totalFrames - 1) : targetFrame;
    m_loop = loop;

    // Create animation on m_animFrame from current to target
    float start_val = m_animFrame;
    float end_val = static_cast<float>(clamped);

    // createAnimation will write start value to target_property immediately.
    m_manager.getAnimationManager()->createAnimation<float>(
        &m_animFrame,
        start_val,
        end_val,
        duration_ms,
        Easing::linear,
        [this, loop, clamped, duration_ms]() {
            // animation complete
            // ensure final frame set
            int finalFrame = std::clamp(static_cast<int>(std::round(m_animFrame)), 0, std::max(0, m_totalFrames - 1));
            setFrame(finalFrame);
            if (m_onAnimationEnd) m_onAnimationEnd();
            // stop tick timer
            if (m_animTickTimerId) {
                stopTimer(m_animTickTimerId);
                m_animTickTimerId = 0;
            }
            if (loop) {
                // restart same animation
                this->animateToFrame(clamped, duration_ms, loop);
            }
        }
    );

    // Start a frequent timer to markDirty while animation progresses so render will update.
    // Use 16ms tick for smoothness
    m_animTickTimerId = startTimer(16, false, [](GUIElement* self) {
        if (!self) return;
        // markDirty each tick to force re-render while animation progresses
        self->markDirty();
    });
}

void AnimatedImage::stopFrameAnimation() {
    m_isPlaying = false;
    if (m_playTimerId) {
        stopTimer(m_playTimerId);
        m_playTimerId = 0;
    }
    if (m_animTickTimerId) {
        stopTimer(m_animTickTimerId);
        m_animTickTimerId = 0;
    }
}