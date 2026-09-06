# AnimatedImage — animated image widget

This page is also available in Polish: [`docs/pl/animated_image.md`](../pl/animated_image.md)

`AnimatedImage` is a widget that displays frame-based animations from a sprite sheet file.

**Key Features:**
- Playback at a fixed rate (`play`/`pause`/`stop`).
- Smooth transitions between frames.
- Configuration of image scaling and aspect ratio.

## Construction and Basic Usage

To use `AnimatedImage`, create it, load a sprite sheet, and add it to the `GUIManager`.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"

int main() {
    SDLApp app("Animated Image", 640, 480);
    GUIManager gui(app.getRenderer());

    // Create the widget
    auto anim = std::make_unique<AnimatedImage>(gui, 50, 50, 256, 128);

    // Load a sprite sheet with path, total frames, and rows
    anim->setSpriteSheet("assets/my_sprite.png", 12, 3);

    // Configure and start
    anim->setFPS(12.0f);
    anim->setLoop(true);
    anim->play();

    gui.addElement(std::move(anim));

    // Main application loop
    while (app.isRunning()) {
        app.handleEvents();
        gui.processEvent(app.getEvent());
        
        app.clearScreen();
        gui.render();
        app.present();
    }

    return 0;
}
```

## Playback Control

Use these methods to control the animation:
- `play()`: Starts playback, looping if `setLoop(true)` was called.
- `pause()`: Pauses the animation on the current frame.
- `stop()`: Stops the animation and resets it to the first frame.

```cpp
// anim_ptr: AnimatedImage*
anim_ptr->play();
anim_ptr->pause();
anim_ptr->stop();
```

## Animating to a Specific Frame

The `animateToFrame` method allows for a smooth transition to a target frame over a specified duration.

```cpp
// Smoothly transition to frame 8 over 500 ms
anim_ptr->animateToFrame(8, 500, false);
```

## Scaling Modes

The `AnimatedImage` widget provides several scaling modes, defined in the `ScaleMode` enum:

- `Fit`: Fits the image to the widget's size (default).
- `Center`: Displays the image at its original size, centered within the widget.
- `None`: Displays the image at its original size in the top-left corner of the widget.

You can also preserve the original aspect ratio of the image during scaling.

```cpp
anim_ptr->setScaleMode(AnimatedImage::ScaleMode::Fit);
anim_ptr->setPreserveAspect(true); // Preserve aspect ratio
```

## API Reference

### Configuration
- `AnimatedImage(GUIManager& manager, int x, int y, int w, int h)`: Constructor.
- `setSpriteSheet(const std::string& path, int totalFrames, int rows = 1, int frameW = 0, int frameH = 0)`: Loads a sprite sheet.
- `setFPS(float fps)`: Sets the playback speed in frames per second.
- `setFrameDuration(float secondsPerFrame)`: Sets the duration of a single frame.
- `setLoop(bool loop)`: Enables or disables animation looping.
- `setScaleMode(ScaleMode mode)`: Sets the scaling mode.
- `setPreserveAspect(bool preserve)`: Enables or disables aspect ratio preservation.

### Control
- `play()`: Starts playback.
- `pause()`: Pauses playback.
- `stop()`: Stops and resets the animation.
- `setFrame(int frameIndex)`: Sets the animation to a specific frame.
- `animateToFrame(int targetFrame, uint32_t duration_ms, bool loop = false)`: Animates a transition to a frame.

### Getters
- `getCurrentFrame() const`: Returns the index of the current frame.
- `getTotalFrames() const`: Returns the total number of frames.
- `isPlaying() const`: Checks if the animation is currently playing.

### Callbacks
- `setOnAnimationEnd(std::function<void()> cb)`: Sets a callback that is invoked when the animation ends (when `loop` is `false`).
- `setOnFrameChanged(std::function<void(int)> cb)`: Sets a callback that is invoked whenever the frame changes.

## Example

A complete, working example of how to use `AnimatedImage` can be found in:
- [`examples/example_animated_image.cpp`](../../examples/example_animated_image.cpp)