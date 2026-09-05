#pragma once


#include "gui.hpp"
#include "texture_manager.hpp"
#include "animation_manager.hpp"
#include <SDL3/SDL.h>

#include "std.hpp"
class AnimatedImage : public GUIElement {
public:
    enum class ScaleMode {
        Fit,    // Scale image to widget size (default)
        Center, // Center image, no scaling
        None    // Don't scale, draw at top-left corner
    };

    AnimatedImage(GUIManager& manager, int x, int y, int width, int height);
    ~AnimatedImage() override;

    // Sprite-sheet configuration
    // frameW / frameH: 0 means computed automatically from the texture size
    void setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0);

    // Animation parameters
    void setFPS(float fps);
    void setFrameDuration(float secondsPerFrame); // alternative
    void setLoop(bool loop);
    void setUseCache(bool useCache); // true = renderToCache (default true)
    void setScaleMode(ScaleMode mode);
    void setPreserveAspect(bool preserve);

    // Playback control
    void play();
    void pause();
    void stop(); // stop and reset to frame 0
    void setFrame(int frameIndex); // immediate frame change

    int getCurrentFrame() const;
    int getTotalFrames() const;
    bool isPlaying() const;

    // Optional callbacks
    void setOnAnimationEnd(std::function<void()> cb);
    void setOnFrameChanged(std::function<void(int)> cb);
    
    // Animation via AnimationManager: animate the frame value float -> int
    // duration_ms: duration in milliseconds
    // loop: whether to repeat after completion
    void animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false);

    // GUIElement overrides
    void draw(SDL_Renderer* renderer) override;
    // For rendering without cache we implement drawDirect (GUIElement::drawDirect)
    bool wantsDirectRender() const override;
    void drawDirect(SDL_Renderer* renderer) override;
    ComponentType getComponentTypeId() const override { return ComponentType::AnimatedImage; }

    bool canShareRenderCache() const override { return false; }
private:
    void ensureTextureLoaded();
    void recalcFrameGeometry(); // computes m_frameW/m_frameH/m_cols from the texture and settings
    void updateSrcRect(); // computes the SDL_Rect src for the current frame
    void advanceFrameIfNeeded();
    SDL_Rect computeScaledDstRect(int offsetX, int offsetY) const;
    void stopFrameAnimation();

    // Resources / configuration
    std::string m_texturePath;
    SharedTexture m_texture;
    int m_totalFrames = 0;
    int m_rows = 1;
    int m_cols = 0;
    int m_frameW = 0;
    int m_frameH = 0;

    // Playback state
    int m_currentFrame = 0;
    float m_frameDuration = 1.0f / 12.0f; // default 12 FPS
    bool m_isPlaying = false;
    bool m_loop = true;

    // Render / cache
    bool m_useCache = true;
    ScaleMode m_scaleMode = ScaleMode::Fit;
    bool m_preserveAspect = true;
    SDL_Rect m_srcRect{0,0,0,0};

    // AnimationManager integration
    // AnimationManager animates numeric properties by pointer; here we use m_animFrame (float)
    float m_animFrame = 0.0f; // internal animated property
    // Note: AnimationManager does not return an animation ID in this version.
    // We also use TimerManager for playback steps and render ticks,
    // whose IDs are stored below.
    
    // Callbacks
    std::function<void()> m_onAnimationEnd;
    std::function<void(int)> m_onFrameChanged;

    // Timer IDs (0 = none). Allow removing timers when pausing/stopping.
    uint32_t m_playTimerId = 0;
    uint32_t m_animTickTimerId = 0;

    // Access to managers (via GUIElement::m_manager)
    // We don't keep an extra pointer to AnimationManager/TextureManager — we fetch them from m_manager on the fly.
};
