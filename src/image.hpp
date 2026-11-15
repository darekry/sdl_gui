#pragma once

#include "gui.hpp"
#include "texture_manager.hpp"
#include <SDL3/SDL.h>

#include "std.hpp"

// Image — minimal texture-backed element: a thin wrapper over a SharedTexture
// loaded and cached by TextureManager. No animation, no editing, no input.
// Rendering goes through the shared render cache (key includes the texture
// pointer, scale mode and sampling), so N widgets showing the same texture at
// the same size share one baked cache texture.
class Image : public GUIElement {
public:
    enum class ScaleMode {
        Fit,      // Contain, preserve aspect, centered (default)
        Stretch,  // Fill the element rect exactly, aspect ignored
        Center,   // Native pixel size, centered, clipped when larger
        None      // Native pixel size, top-left corner
    };

    Image(GUIManager& manager, int x, int y, int width, int height);

    // Loads through TextureManager's path cache (same texture shared with
    // every other widget using this path). Returns false on empty path or a
    // failed load; either case clears the current image.
    bool setImage(const std::string& path);

    // Decodes from memory (embedded assets) and caches under `key`.
    bool setImageFromMemory(const uint8_t* data, size_t size, std::string_view key);

    // Takes a shared texture (e.g. TextureManager::addTexture); no copy is
    // made and no ownership is transferred beyond the shared_ptr.
    void setImage(SharedTexture texture);

    void clearImage();

    [[nodiscard]] bool hasImage() const { return m_texture != nullptr; }
    [[nodiscard]] SharedTexture getTexture() const { return m_texture; }
    // Path passed to setImage(path); empty for memory/shared textures.
    [[nodiscard]] const std::string& getImagePath() const { return m_imagePath; }
    // Native texture dimensions; {0, 0} when no image is set.
    [[nodiscard]] std::pair<int, int> getImageSize() const;

    void setScaleMode(ScaleMode mode);
    [[nodiscard]] ScaleMode getScaleMode() const { return m_scaleMode; }

    // Nearest-neighbour sampling for pixel art (default: linear).
    // The scale mode is a property of the SDL texture, so this affects every
    // widget sharing that texture (TextureManager caches by path).
    void setNearestScaling(bool nearest);
    [[nodiscard]] bool isNearestScaling() const { return m_nearest; }

    // Resizes the element to the texture's native size. No-op without image.
    void sizeToImage();

    void draw(SDL_Renderer* renderer) override;
    ComponentType getComponentTypeId() const override { return ComponentType::Image; }
    bool supportsBevel() const override { return false; }

protected:
    uint64_t getRenderCacheKeySuffix() const override;

private:
    SDL_Rect computeDstRect(int offsetX, int offsetY) const;
    void applyScaleMode();

    SharedTexture m_texture;
    std::string m_imagePath;
    ScaleMode m_scaleMode = ScaleMode::Fit;
    bool m_nearest = false;
};
