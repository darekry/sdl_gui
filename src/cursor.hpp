#pragma once


#include "gui.hpp"
#include "std.hpp"

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

class Cursor : public GUIElement {
public:
    explicit Cursor(GUIManager& manager);
    ~Cursor() override;

    void setCursorTexture(CursorState state, const std::string& path, int hotspotX = 0, int hotspotY = 0);
    
    void setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows = 1, 
                          float fps = 12.0f, int hotspotX = 0, int hotspotY = 0);
    
    void setState(CursorState state);
    CursorState getState() const { return m_currentState; }
    
    void setOffset(int offsetX, int offsetY);
    void getOffset(int& offsetX, int& offsetY) const;
    
    void setScale(float scale);
    float getScale() const { return m_scale; }
    
    void setOnStateChanged(std::function<void(CursorState)> callback);

    bool handleEvent(const SDL_Event& event) override;
    const char* getComponentType() const override;
    void setVisible(bool visible);
    bool isOverlay() const override { return true; }
    void renderOverlay(SDL_Renderer* renderer) override;

    protected:
    void draw(SDL_Renderer* renderer) override;

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
        bool loop = true;
        uint32_t animation_id = 0;
    };

    void renderCursor(SDL_Renderer* renderer, const CursorData& data, int mouseX, int mouseY);
    SDL_Rect getSrcRect(const CursorData& data) const;

    std::map<CursorState, CursorData> m_cursors;
    CursorState m_currentState = CursorState::Normal;
    int m_offsetX = 0;
    int m_offsetY = 0;
    float m_scale = 1.0f;
    std::function<void(CursorState)> m_onStateChanged;
};
