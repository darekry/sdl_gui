#pragma once


#include "texture_manager.hpp"
#include "font_manager.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include "animation_manager.hpp"
#include "style.hpp"
#include "anchor.hpp"


#if DEBUG
#include <SDL3/SDL_log.h>
#define LOG_DEBUG(...) SDL_Log(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif
import std.compat;

class GUIManager;

class GUIElement {
private:
    std::string tooltip;
    uint32_t tooltipTimerId = 0;
public:
    int m_x, m_y; // Pola publiczne zgodnie z życzeniem
    int m_width, m_height;
    bool m_enabled = true;
    bool m_visible = true;

    GUIElement(GUIManager& manager, int x, int y, int width, int height);
    virtual ~GUIElement();
    void setTooltip(const std::string& text);
    void setID(std::string_view id);
    [[nodiscard]] std::string_view getID() const;
    [[nodiscard]] int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void getSize(int& width, int& height) const;

    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setParent(GUIElement* parent);
    
    // === Anchor system (responsive layout) ===
    
    /** Set anchor for responsive positioning */
    void setAnchor(const Anchor& anchor);
    [[nodiscard]] const Anchor& getAnchor() const { return m_anchor; }
    [[nodiscard]] bool hasAnchor() const { return m_anchor.hasAnyAnchor(); }
    
    /** Apply anchor to recalculate position/size based on parent dimensions */
    void applyAnchor(int parentWidth, int parentHeight);
    
    /** Update layout for this element and all children (call on resize) */
    void updateLayout(int parentWidth, int parentHeight);
    
    /** Internal: called when parent size changes */
    virtual void onParentResize(int parentWidth, int parentHeight);
    
    // === Original size storage (for anchor calculations) ===
    
    /** Store original size (used when anchor doesn't specify size) */
    void storeOriginalSize();
    [[nodiscard]] int getOriginalWidth() const { return m_originalWidth; }
    [[nodiscard]] int getOriginalHeight() const { return m_originalHeight; }
    SDL_Point getAbsolutePosition() const;
    SDL_Point getRelativePosition() const { return {m_x, m_y}; }
    
    virtual bool contains(int x, int y) const;
    
    SDL_Point toLocalCoords(int globalX, int globalY) const;
    
    void setRotation(double angleDegrees);
    double getRotation() const { return m_rotation; }
    void setRotationCenter(int cx, int cy);
    SDL_Point getRotationCenter() const { return m_rotationCenter; }
    virtual bool handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer);
    void renderToCache();
    [[nodiscard]] SDL_Texture* getCachedTexture() const { return m_cachedTexture.get(); }
    
    void setGPUState(SDL_GPURenderState* state) { m_gpuState = state; }
    [[nodiscard]] SDL_GPURenderState* getGPUState() const { return m_gpuState; }
    
    virtual bool isOverlay() const { return false; }
    virtual void renderOverlay(SDL_Renderer* renderer);
    void setClipChildren(bool clip);

    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    [[nodiscard]] bool isEnabled() const { return m_enabled; }
    void setVisible(bool visible) { m_visible = visible; }
    [[nodiscard]] bool isVisible() const { return m_visible; }
    [[nodiscard]] bool isHovered() const { return m_isHovered; }
    void setState(ElementState newState);
    [[nodiscard]] ElementState getState() const { return m_state; }
    void setStyle(ElementState state, Style style);
    void setBackgroundColor(ElementState state, SDL_Color color);
    void setTextColor(ElementState state, SDL_Color color);
    void setTexture(ElementState state, SharedTexture texture);
    void setBorder(ElementState state, SDL_Color color, int width);
    void setBorderRadius(ElementState state, int radius);
    virtual const char* getComponentType() const;
    void markForDeletion();
    void markDirty(bool cascadeToParents = true);
    void markDirtyRecursively();
    bool isMarkedForDeletion() const;
    void cleanup();

    void addChild(std::unique_ptr<GUIElement> child);
    void clearChildren();
    [[nodiscard]] GUIElement* getParent() const { return m_parent; }
    [[nodiscard]] const std::vector<std::unique_ptr<GUIElement>>& getChildren() const { return m_children; }
    size_t countDescendants() const;
    GUIElement* findElementAt(int x, int y);

    // Focus and Capture API
    virtual void onFocusGained() {}
    virtual void onFocusLost() {}
    virtual void onMouseCaptureGained() {}
    virtual void onMouseCaptureLost() {}

    [[nodiscard]] bool canGetKeyboardFocus() const;
    void setCanGetKeyboardFocus(bool canFocus);
    [[nodiscard]] bool hasKeyboardFocus() const;

protected:
    void render(SDL_Renderer* renderer, const SDL_Rect& parent_clip_rect);
 
protected:
    uint32_t startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void stopTimer(uint32_t timerId);
    
    std::string m_id;
    bool m_canGetKeyboardFocus = false;
    bool m_clip_children = true;
    
    // === Anchor system ===
    Anchor m_anchor;                    // Anchor for responsive positioning
    int m_originalWidth = 0;            // Original width before anchor modifications
    int m_originalHeight = 0;           // Original height before anchor modifications
    
    virtual void draw(SDL_Renderer* renderer) = 0;

    // Rozszerzenie: możliwość rysowania bezpośrednio (bez buforowania).
    // Domyślnie elementy nie korzystają z drawDirect — zwracają false w wantsDirectRender().
    virtual bool wantsDirectRender() const { return false; }
    virtual void drawDirect([[maybe_unused]] SDL_Renderer* renderer) { /* domyślnie brak */ }
    void drawBackgroundAndBorder(SDL_Renderer* renderer);

        Style getComposedStyle(ElementState state) const;

    GUIManager& m_manager;
    bool m_isHovered = false;
    bool m_isMarkedForDeletion = false;
    GUIElement* m_parent;
    bool m_isDirty = true;
        std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_cachedTexture{nullptr, SDL_DestroyTexture};
        std::array<std::optional<Style>, 4> m_localStyles;
        static constexpr size_t stateIndex(ElementState state) {
            return static_cast<size_t>(state);
        }
        ElementState m_state = ElementState::Normal;
    bool m_style_dirty = true;
    std::vector<std::unique_ptr<GUIElement>> m_children;
    SDL_GPURenderState* m_gpuState = nullptr;
    
    double m_rotation = 0.0;
    SDL_Point m_rotationCenter = {-1, -1};
};

void drawRoundedFilledRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color);
void drawRoundedRectBorder(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color, float thickness);
