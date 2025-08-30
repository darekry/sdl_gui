# AnimatedImage — animated image widget

This page is also available in Polish: [`docs/pl/animated_image.md`](../pl/animated_image.md)

## Short introduction

`AnimatedImage` is a widget that displays animations from a sprite sheet (multiple frames in a single texture). It supports:
- frame playback at a fixed rate (play/pause/stop),
- animating transitions between frames via `AnimationManager`,
- scaling configuration and aspect ratio preservation,
- rendering via the element render cache or direct rendering to the renderer.

## Requirements and dependencies

- Access to managers through `GUIManager`:
  - `TextureManager` (texture loading) — used in the implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:26).
  - `AnimationManager` (smooth property animations) — checked in [`src/animated_image.cpp`](../../src/animated_image.cpp:334).
  - `TimerManager` (timers for playback mode) — methods `startTimer`/`stopTimer` available in [`src/gui.hpp`](../../src/gui.hpp:76).
- The widget derives from `GUIElement` — constructor: [`src/animated_image.hpp`](../../src/animated_image.hpp:22).

## Construction and basic usage

Minimal usage example (compilable):

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"

int main() {
    SDLApp app("Animated image", 640, 480);
    GUIManager gui(app.getRenderer());

    // Create the widget: see constructor
    auto anim = std::make_unique<AnimatedImage>(gui, 50, 50, 256, 128); // constructor: [`src/animated_image.hpp`](../../src/animated_image.hpp:22)

    // Load a sprite sheet (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:19))
    anim->setSpriteSheet("assets/my_sprite.png", 12, 3); // totalFrames = 12, rows = 3

    // Configure and start
    anim->setFPS(12.0f);
    anim->setLoop(true);
    gui.addElement(std::move(anim));

    // -> rest of the SDL loop: processEvent / render (as in example)
    return 0;
}
```

The `setSpriteSheet` call in the implementation ([`src/animated_image.cpp`](../../src/animated_image.cpp:19)) automatically:
- loads the texture via `TextureManager` (`m_manager.getTextureManager().loadTexture`),
- computes frame geometry by calling `recalcFrameGeometry()` ([`src/animated_image.cpp`](../../src/animated_image.cpp:40)),
- sets the current frame to 0.

## Playback (play/pause/stop)

Playback control:
- `play()` — starts a timer that advances the frame at fixed intervals (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:268)).
- `pause()` — stops the timer (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:303)).
- `stop()` — stops the timer and resets frame to 0 (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:312)).

Usage:

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->play();   // start (timer-driven)
anim_ptr->pause();  // pause (keep current frame)
anim_ptr->stop();   // stop and set frame 0
```

## Animating to a specific frame (animateToFrame)

`animateToFrame(target, duration_ms, loop)` uses `AnimationManager` to animate the internal float property `m_animFrame`. Implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:333).

Example:

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->animateToFrame(8, 500 /*ms*/, false); // smooth transition to frame 8 over 500ms
```

If `AnimationManager` is not available, the method sets the frame immediately and invokes the completion callback (see implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:335-340)).

## Scaling modes and aspect ratio handling

Scaling mode is defined by the `ScaleMode` enum in [`src/animated_image.hpp`](../../src/animated_image.hpp:15):
- Fit — fit to widget size (default),
- Center — center without scaling,
- None — no scaling, draw in the top-left corner.

Configuration:

```cpp
anim_ptr->setScaleMode(AnimatedImage::ScaleMode::Fit);      // enum: [`src/animated_image.hpp`](../../src/animated_image.hpp:15)
anim_ptr->setPreserveAspect(true);                          // preserve aspect ratio in Fit mode
```

## Public method list

Below is the list of public methods with signatures and brief descriptions. Signatures are from the header: [`src/animated_image.hpp`](../../src/animated_image.hpp:13).

- setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0) — load sprite sheet and set frames/rows. ([`src/animated_image.hpp`](../../src/animated_image.hpp:27), implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:19))
- setFPS(float fps) — set frames per second; wrapper for setFrameDuration. ([`src/animated_image.hpp`](../../src/animated_image.hpp:30))
- setFrameDuration(float secondsPerFrame) — set per-frame duration in seconds; restarts timer if playing. ([`src/animated_image.hpp`](../../src/animated_image.hpp:31), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:219))
- setLoop(bool loop) — enable/disable looping. ([`src/animated_image.hpp`](../../src/animated_image.hpp:32))
- setUseCache(bool useCache) — enable/disable rendering to cache (`m_useCache`, default true); when disabled, widget may render directly via `drawDirect`. ([`src/animated_image.hpp`](../../src/animated_image.hpp:33), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:234))
- setScaleMode(ScaleMode mode) — set scaling mode. ([`src/animated_image.hpp`](../../src/animated_image.hpp:34))
- setPreserveAspect(bool preserve) — preserve aspect ratio when Fit. ([`src/animated_image.hpp`](../../src/animated_image.hpp:35))

Playback control:
- play() — start timer-driven playback. ([`src/animated_image.hpp`](../../src/animated_image.hpp:38), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:268))
- pause() — pause playback and stop the timer. ([`src/animated_image.hpp`](../../src/animated_image.hpp:39), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:303))
- stop() — stop and reset to frame 0. ([`src/animated_image.hpp`](../../src/animated_image.hpp:40), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:312))
- setFrame(int frameIndex) — immediately set a specific frame. ([`src/animated_image.hpp`](../../src/animated_image.hpp:41), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:200))

Getters:
- getCurrentFrame() const — current frame index. ([`src/animated_image.hpp`](../../src/animated_image.hpp:44), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:256))
- getTotalFrames() const — total frame count. ([`src/animated_image.hpp`](../../src/animated_image.hpp:45), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:260))
- isPlaying() const — whether the timer is running. ([`src/animated_image.hpp`](../../src/animated_image.hpp:46), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:264))

Callbacks:
- setOnAnimationEnd(std::function<void()> cb) — called when animation completes (e.g., reaches end with loop=false). ([`src/animated_image.hpp`](../../src/animated_image.hpp:49), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:325))
- setOnFrameChanged(std::function<void(int)> cb) — called on each frame change. ([`src/animated_image.hpp`](../../src/animated_image.hpp:50), impl: [`src/animated_image.cpp`](../../src/animated_image.cpp:329))

Property animation:
- animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false) — smooth frame animation via `AnimationManager` (float property `m_animFrame`), implementation in [`src/animated_image.cpp`](../../src/animated_image.cpp:333).

## Internal mechanisms important to users

- Computing columns/rows and frame size:
  - `recalcFrameGeometry()` computes column count (`m_cols`) and frame width/height (`m_frameW`, `m_frameH`) based on the texture size and settings (`totalFrames`, `rows`, optional `frameW/frameH`). Implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:40).
  - `updateSrcRect()` builds `SDL_Rect m_srcRect` for the current frame (column/row): [`src/animated_image.cpp`](../../src/animated_image.cpp:70).
  - Columns are computed as ceil(totalFrames / rows) (see [`src/animated_image.cpp`](../../src/animated_image.cpp:52)).

- Two animation modes:
  1) Timer‑driven: calling `play()` creates a timer via `GUIElement::startTimer` (declared in [`src/gui.hpp`](../../src/gui.hpp:76)) that advances frames at fixed intervals. Implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:268).
  2) Property animation via `AnimationManager`: `animateToFrame(...)` animates the internal float property `m_animFrame`. `draw()`/`drawDirect()` round `m_animFrame` to an int and update `m_currentFrame`. Implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:90) and [`src/animated_image.cpp`](../../src/animated_image.cpp:151).

- Role of `m_useCache`:
  - Default `m_useCache = true` (field declared in [`src/animated_image.hpp`](../../src/animated_image.hpp:88)).
  - If `m_useCache` is false, the widget aims to render directly — `wantsDirectRender()` dictates the path and `drawDirect()` performs immediate drawing (see [`src/animated_image.cpp`](../../src/animated_image.cpp:140) and [`src/animated_image.cpp`](../../src/animated_image.cpp:144)).
  - Disabling cache removes the cached image if present (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:238-242)).

- Callbacks:
  - `m_onAnimationEnd` — invoked when animation finishes (e.g., non-looping reaches end) — set via `setOnAnimationEnd` ([`src/animated_image.hpp`](../../src/animated_image.hpp:49)).
  - `m_onFrameChanged` — invoked on each frame change (both timer and property animation) — set via `setOnFrameChanged` ([`src/animated_image.hpp`](../../src/animated_image.hpp:50)).

- Timers and cleanup:
  - `play()` stores the timer id in `m_playTimerId` (field in header: [`src/animated_image.hpp`](../../src/animated_image.hpp:105)) so it can be stopped in `pause()`/`stop()` (implementations: [`src/animated_image.cpp`](../../src/animated_image.cpp:279), [`src/animated_image.cpp`](../../src/animated_image.cpp:303), [`src/animated_image.cpp`](../../src/animated_image.cpp:312)).
  - `animateToFrame()` starts an additional "tick timer" (`m_animTickTimerId`) to regularly call `markDirty()` during the animation (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:374-380)). Timers are stopped appropriately (e.g., `stopFrameAnimation` in [`src/animated_image.cpp`](../../src/animated_image.cpp:387)).

## Tips & Gotchas (most common)

- Texture not loading:
  - Check `m_texturePath` and `m_texture` — if `m_texture` is null, `ensureTextureLoaded()` tries to reload from `m_texturePath` (implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:34-38)). Ensure the path is correct and the file exists.
- Manually setting frame size:
  - You can provide `frameW` / `frameH` in `setSpriteSheet(...)`. If left 0, the widget auto‑computes frame size by dividing texture dimensions by columns/rows (see [`src/animated_image.cpp`](../../src/animated_image.cpp:55-61)).
- `animateToFrame` vs no `AnimationManager`:
  - If `AnimationManager` is available it will be used; if not, the method applies the frame immediately and triggers `m_onAnimationEnd` (see [`src/animated_image.cpp`](../../src/animated_image.cpp:334-340)).
- Direct vs cached rendering:
  - For frequently changing animations, consider `setUseCache(false)` and ensure that `GUIManager`/renderer support `drawDirect()` (see [`src/gui.hpp`](../../src/gui.hpp:82) and `drawDirect` in [`src/animated_image.cpp`](../../src/animated_image.cpp:144)).

## Additional references / examples

- Full example in the examples directory: [`examples/example_animated_image.cpp`](../../examples/example_animated_image.cpp:1)
- Widget constructor: [`src/animated_image.hpp`](../../src/animated_image.hpp:22)
- `setSpriteSheet` implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:19)
- `animateToFrame` implementation: [`src/animated_image.cpp`](../../src/animated_image.cpp:333)
- `ScaleMode` enum: [`src/animated_image.hpp`](../../src/animated_image.hpp:15)
- Direct render/wantsDirectRender mechanism: [`src/gui.hpp`](../../src/gui.hpp:82)

## Frequently reviewed code lines (important implementation spots)

- Constructor / destructor: [`src/animated_image.cpp`](../../src/animated_image.cpp:9)
- setSpriteSheet: [`src/animated_image.cpp`](../../src/animated_image.cpp:19)
- recalcFrameGeometry: [`src/animated_image.cpp`](../../src/animated_image.cpp:40)
- updateSrcRect: [`src/animated_image.cpp`](../../src/animated_image.cpp:70)
- draw(): [`src/animated_image.cpp`](../../src/animated_image.cpp:84)
- drawDirect(): [`src/animated_image.cpp`](../../src/animated_image.cpp:144)
- play(): [`src/animated_image.cpp`](../../src/animated_image.cpp:268)
- pause(): [`src/animated_image.cpp`](../../src/animated_image.cpp:303)
- stop(): [`src/animated_image.cpp`](../../src/animated_image.cpp:312)
- animateToFrame(): [`src/animated_image.cpp`](../../src/animated_image.cpp:333)

## Conclusion

This document helps you quickly understand how to use `AnimatedImage`, how it works internally (at a high level), and what common pitfalls to watch for. If you need extended examples (e.g., synchronization with other widgets), consider integrating with `AnimationManager` and `TimerManager` as shown above.