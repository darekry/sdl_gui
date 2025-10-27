#ifndef MOUSE_CURSOR_HPP
#define MOUSE_CURSOR_HPP

#include "SDL2/SDL.h"
#include "texture_manager.hpp"
#include "animation_manager.hpp"
#include <string>
#include <map>
#include <memory>
#include <functional>

class GUIManager;

enum class CursorState {
    Normal,
    Hover,
    Pressed,
    Disabled,
    Busy,
    Text,
    Custom1,
    Custom2,
    Custom3
};

class MouseCursor {
public:
    explicit MouseCursor(GUIManager& manager);
    ~MouseCursor();

    void setCursorTexture(CursorState state, const std::string& path, int hotspotX = 0, int hotspotY = 0);
    
    void setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows = 1, 
                          float fps = 12.0f, int hotspotX = 0, int hotspotY = 0);
    
    void setState(CursorState state);
    CursorState getState() const { return m_currentState; }
    
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }
    
    void setOffset(int offsetX, int offsetY);
    void getOffset(int& offsetX, int& offsetY) const;
    
    void setScale(float scale);
    float getScale() const { return m_scale; }
    
    void update();
    void render(SDL_Renderer* renderer);
    
    void setOnStateChanged(std::function<void(CursorState)> callback);

private:
    struct CursorData {
        SharedTexture texture;
        int hotspotX = 0;
        int hotspotY = 0;
        
        bool isAnimated = false;
        int totalFrames = 0;
        int rows = 1;
        int cols = 0;
        int frameW = 0;
        int frameH = 0;
        float frameDuration = 1.0f / 12.0f;
        int currentFrame = 0;
        uint64_t lastFrameTime = 0;
        bool loop = true;
    };

    void updateAnimation(CursorData& data);
    void renderCursor(SDL_Renderer* renderer, const CursorData& data, int mouseX, int mouseY);
    SDL_Rect getSrcRect(const CursorData& data) const;

    GUIManager& m_manager;
    std::map<CursorState, CursorData> m_cursors;
    CursorState m_currentState = CursorState::Normal;
    bool m_visible = true;
    int m_offsetX = 0;
    int m_offsetY = 0;
    float m_scale = 1.0f;
    std::function<void(CursorState)> m_onStateChanged;
};

#endif // MOUSE_CURSOR_HPP
