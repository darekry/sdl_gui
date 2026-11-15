#include "image.hpp"
#include "gui_manager.hpp"
#include <SDL3/SDL.h>

#include "std.hpp"

Image::Image(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height) {
}

bool Image::setImage(const std::string& path) {
    if (path.empty()) {
        clearImage();
        return false;
    }
    m_texture = m_manager.getTextureManager().loadTexture(path);
    m_imagePath = m_texture ? path : std::string{};
    applyScaleMode();
    markDirty();
    return m_texture != nullptr;
}

bool Image::setImageFromMemory(const uint8_t* data, size_t size, std::string_view key) {
    if (!data || size == 0 || key.empty()) {
        clearImage();
        return false;
    }
    m_texture = m_manager.getTextureManager().loadTextureFromMemory(data, size, key);
    m_imagePath.clear();
    applyScaleMode();
    markDirty();
    return m_texture != nullptr;
}

void Image::setImage(SharedTexture texture) {
    m_texture = std::move(texture);
    m_imagePath.clear();
    applyScaleMode();
    markDirty();
}

void Image::clearImage() {
    if (!m_texture && m_imagePath.empty()) {
        return;
    }
    m_texture.reset();
    m_imagePath.clear();
    markDirty();
}

std::pair<int, int> Image::getImageSize() const {
    if (!m_texture) {
        return {0, 0};
    }
    return {TextureWidth(m_texture.get()), TextureHeight(m_texture.get())};
}

void Image::setScaleMode(ScaleMode mode) {
    if (m_scaleMode == mode) {
        return;
    }
    m_scaleMode = mode;
    markDirty();
}

void Image::setNearestScaling(bool nearest) {
    if (m_nearest == nearest) {
        return;
    }
    m_nearest = nearest;
    applyScaleMode();
    markDirty();
}

void Image::sizeToImage() {
    const auto [w, h] = getImageSize();
    if (w > 0 && h > 0) {
        setSize(w, h);
    }
}

void Image::applyScaleMode() {
    if (m_texture) {
        SDL_SetTextureScaleMode(m_texture.get(), m_nearest ? SDL_SCALEMODE_NEAREST : SDL_SCALEMODE_LINEAR);
    }
}

SDL_Rect Image::computeDstRect(int offsetX, int offsetY) const {
    if (!m_texture || m_width <= 0 || m_height <= 0) {
        return {offsetX, offsetY, 0, 0};
    }
    const int tw = TextureWidth(m_texture.get());
    const int th = TextureHeight(m_texture.get());
    if (tw <= 0 || th <= 0) {
        return {offsetX, offsetY, 0, 0};
    }

    switch (m_scaleMode) {
        case ScaleMode::Stretch:
            return {offsetX, offsetY, m_width, m_height};
        case ScaleMode::Center:
            return {offsetX + (m_width - tw) / 2, offsetY + (m_height - th) / 2, tw, th};
        case ScaleMode::None:
            return {offsetX, offsetY, tw, th};
        case ScaleMode::Fit:
        default: {
            const float scale = std::min(static_cast<float>(m_width) / static_cast<float>(tw),
                                         static_cast<float>(m_height) / static_cast<float>(th));
            const int w = std::max(1, static_cast<int>(std::lround(static_cast<float>(tw) * scale)));
            const int h = std::max(1, static_cast<int>(std::lround(static_cast<float>(th) * scale)));
            return {offsetX + (m_width - w) / 2, offsetY + (m_height - h) / 2, w, h};
        }
    }
}

void Image::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    if (!m_texture) {
        return;
    }
    const SDL_Rect dst = computeDstRect(0, 0);
    if (dst.w <= 0 || dst.h <= 0) {
        return;
    }
    RenderTexture(renderer, m_texture.get(), dst);
}

uint64_t Image::getRenderCacheKeySuffix() const {
    uint64_t h = reinterpret_cast<uint64_t>(m_texture.get());
    auto mix = [&h](uint64_t v) {
        h ^= v + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
    };
    mix(static_cast<uint64_t>(m_scaleMode));
    mix(m_nearest ? 1ULL : 0ULL);
    return h;
}
