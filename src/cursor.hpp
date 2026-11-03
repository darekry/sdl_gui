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
    // Builds the cursor texture from caller-owned surface pixels (the surface
    // is not adopted and may be freed immediately). Switches back to overlay
    // mode when the previous state was a system cursor.
    void setCursorSurface(CursorState state, SDL_Surface* surface,
                          int hotspotX = 0, int hotspotY = 0);
    // Shows a native SDL system cursor and disables overlay drawing. Any later
    // setCursorTexture/setCursorSurface returns to overlay mode.
    void setSystemCursor(SDL_SystemCursor cursor);
    [[nodiscard]] bool isSystemCursor() const { return m_systemMode; }
    
    void setAnimatedCursor(CursorState state, const std::string& path, int totalFrames, int rows = 1, 
                          float fps = 12.0f, int hotspotX = 0, int hotspotY = 0);
    
    void setState(CursorState state);
    CursorState getState() const { return m_currentState; }
    
    void setOffset(int offsetX, int offsetY);
    void getOffset(int& offsetX, int& offsetY) const;
    
    void setScale(float scale);
    float getScale() const { return m_scale; }
    
    void setOnStateChanged(std::function<void(CursorState)> callback);

    /**
     * @brief Mouse-position service feed (point 1c).
     *
     * The cursor is NOT an event consumer: GUIManager calls updatePosition()
     * once per mouse event (top of processEvent), covering propagation,
     * capture and unconsumed paths from a single site. Any mouse event
     * (motion or button) refreshes the position — one source of truth.
     */
    void updatePosition(const SDL_Event& event);
    void getPosition(int& mouseX, int& mouseY) const;
    void render(SDL_Renderer* renderer);

    ComponentType getComponentTypeId() const override;
    void setVisible(bool visible);
    bool isOverlay() const override { return true; }
    void renderOverlay(SDL_Renderer* renderer) override;

    protected:
    void draw(SDL_Renderer* renderer) override;

    bool canShareRenderCache() const override { return false; }
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
    void clearSystemCursor();

    static constexpr size_t kNumCursorStates = 9;
    std::array<std::optional<CursorData>, kNumCursorStates> m_cursors;
    CursorState m_currentState = CursorState::Normal;
    int m_offsetX = 0;
    int m_offsetY = 0;
    float m_scale = 1.0f;
    int m_mouseX = 0;
    int m_mouseY = 0;
    bool m_hasMousePos = false;
    bool m_systemMode = false;
    SDL_Cursor* m_systemCursor = nullptr;
    std::function<void(CursorState)> m_onStateChanged;
};
