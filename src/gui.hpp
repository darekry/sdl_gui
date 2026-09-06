#pragma once

#include "std.hpp"

#include "texture_manager.hpp"
#include "font_manager.hpp"
#include "sdl_rect_helpers.hpp"
#include <SDL3/SDL.h>
#include "animation_manager.hpp"
#include "style.hpp"
#include "anchor.hpp"
#include "constants.hpp"
#include "component_type.hpp"

#include "logger.hpp"

class GUIManager;
class ILayoutManager;

class GUIElement {
private:
    std::string tooltip;
    uint32_t tooltipTimerId = 0;
public:
    int m_x, m_y; // Public fields as requested
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
    
    // === Layout pass (Measure/Arrange) ===

    /** Set anchor for responsive positioning (data for AnchorLayout) */
    void setAnchor(const Anchor& anchor);
    [[nodiscard]] const Anchor& getAnchor() const { return m_anchor; }
    [[nodiscard]] bool hasAnchor() const { return m_anchor.hasAnyAnchor(); }

    /** Custom layout manager for this container (null = default AnchorLayout) */
    void setLayoutManager(std::unique_ptr<ILayoutManager> manager);
    [[nodiscard]] ILayoutManager* getLayoutManager() const { return m_layoutManager.get(); }

    /**
     * @brief Arrange this element's children (one LayoutPass step).
     * Default: AnchorLayout — each child placed per its own Anchor.
     * Widgets with internal geometry (Button label, Slider parts,
     * ScrollArea sliders, TabControl tabs, dialog button strips) override it.
     * Replaces the old onSizeChanged() hook.
     */
    virtual void layoutChildren();

    /** Recompute own rect from parent size, then layoutChildren() (call on resize) */
    void updateLayout(int parentWidth, int parentHeight);

    SDL_Point getAbsolutePosition() const;
    SDL_Point getRelativePosition() const { return {m_x, m_y}; }
    void invalidateAbsPosCache();
    
    virtual bool contains(int x, int y) const;
    bool contains(float x, float y) const;
    
    SDL_Point toLocalCoords(int globalX, int globalY) const;
    
    void setRotation(double angleDegrees);
    double getRotation() const { return m_rotation; }
    void setRotationCenter(int cx, int cy);
    SDL_Point getRotationCenter() const { return m_rotationCenter; }
    virtual bool handleEvent(const SDL_Event& e);
    void processHoverTooltip(bool currentlyHovered);
    void processButtonEvent(const SDL_Event& e);

    // Right mouse button (context menu trigger).
    // Called with the element and the click position in window coordinates.
    // The element consumes the RMB press, so ancestors do not fire their own callbacks.
    using OnRightClickCallback = std::function<void(GUIElement*, float x, float y)>;
    void setOnRightClickCallback(OnRightClickCallback callback) { m_onRightClick = std::move(callback); }    void render(SDL_Renderer* renderer);
    
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
    void setBevel(ElementState state, BevelType type);
    void setThumbColor(ElementState state, SDL_Color color);
    void setFillColor(ElementState state, SDL_Color color);
    Style getComposedStyle(ElementState state) const;
    // Interned widget type ID. Each widget overrides it; the string name
    // exists only at the outer boundary (layout files, C-API) via
    // componentTypeFromString/componentTypeToString.
    virtual ComponentType getComponentTypeId() const = 0;
    // Whether the 3D bevel branch applies. Widgets without frames
    // (Label/Cursor/...) return false so bevel colors are ignored.
    virtual bool supportsBevel() const { return true; }
    void markForDeletion();
    void markDirty(bool cascadeToParents = true);
    void markDirtyRecursively();
    bool isMarkedForDeletion() const;
    void cleanup();

    GUIElement* addChild(std::unique_ptr<GUIElement> child);
    void clearChildren();
    [[nodiscard]] GUIElement* getParent() const { return m_parent; }
    [[nodiscard]] GUIManager& getManager() const { return m_manager; }
    [[nodiscard]] const std::vector<std::unique_ptr<GUIElement>>& getChildren() const { return m_children; }
    size_t countDescendants() const;
    GUIElement* findElementAt(int x, int y);

    // Focus and Capture API
    virtual void onFocusGained();
    virtual void onFocusLost();
    virtual void onMouseCaptureGained() {}
    virtual void onMouseCaptureLost() {}

    [[nodiscard]] bool canGetKeyboardFocus() const;
    void setCanGetKeyboardFocus(bool canFocus);
    [[nodiscard]] bool hasKeyboardFocus() const;

protected:
    void render(SDL_Renderer* renderer, const SDL_Rect& parent_clip_rect);

    // Fires m_onRightClick for an RMB press inside this element.
    // Returns true when the event was consumed (callback present and fired).
    bool processRightClick(const SDL_Event& e);
 
    uint32_t startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback);
    void stopTimer(uint32_t timerId);
    
    std::string m_id;
    bool m_canGetKeyboardFocus = false;
    bool m_clip_children = true;
    
    // === Layout data ===
    Anchor m_anchor;  // Anchor for responsive positioning (consumed by AnchorLayout)
    std::unique_ptr<ILayoutManager> m_layoutManager;  // null = default AnchorLayout

    virtual void draw(SDL_Renderer* renderer) { drawBackgroundAndBorder(renderer); }

    // Shared render cache (TextureManager::renderCache).
    // Default true: draw() depends only on the composed style + state + size.
    // Widgets whose draw() reads internal state (checkbox, slider, text...),
    // must return false or include that state via getRenderCacheKeySuffix().
    virtual bool canShareRenderCache() const { return true; }
    virtual uint64_t getRenderCacheKeySuffix() const { return 0; }
    uint64_t buildRenderCacheKey() const;
    void renderToCache();

    // Extension: direct drawing (without caching).
    // By default elements don't use drawDirect — they return false in wantsDirectRender().
    virtual bool wantsDirectRender() const { return false; }
    virtual void drawDirect([[maybe_unused]] SDL_Renderer* renderer) { /* no-op by default */ }
    void drawBackgroundAndBorder(SDL_Renderer* renderer);

    GUIManager& m_manager;
    bool m_isHovered = false;
    bool m_isMarkedForDeletion = false;
    OnRightClickCallback m_onRightClick;
    GUIElement* m_parent;
    mutable SDL_Point m_cachedAbsPos = {0, 0};
    mutable bool m_absPosValid = false;
    bool m_isDirty = true;
        SharedTexture m_cachedTexture;
        uint64_t m_cacheKey = 0;
        std::array<std::optional<Style>, 4> m_localStyles;
        // Resolved-style cache (StyleResolver phase 1): merged
        // local+theme+default per state, valid while theme epoch and
        // local epoch are unchanged. Makes repeated draw()/cache-key
        // builds O(1) instead of map+merge per call.
        mutable std::array<Style, 4> m_resolvedCache{};
        mutable std::array<bool, 4> m_resolvedValid{false, false, false, false};
        mutable uint64_t m_resolvedThemeEpoch = 0;
        uint64_t m_localStyleEpoch = 1;
        mutable uint64_t m_resolvedLocalEpoch = 0;
        void invalidateResolvedCache();
        static constexpr size_t stateIndex(ElementState state) {
            return static_cast<size_t>(state);
        }
        ElementState m_state = ElementState::Normal;
    std::vector<std::unique_ptr<GUIElement>> m_children;
    
    double m_rotation = 0.0;
    SDL_Point m_rotationCenter = {-1, -1};
};

void drawRoundedFilledRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color);
void drawRoundedRectBorder(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color, float thickness);
void drawRoundedTexturedRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_Texture* texture);

// One 1px bevel frame: topLeftColor on top and left, bottomRightColor on bottom and right.
// Bottom/right edges are drawn last — they overwrite the top-right and bottom-left corners (as in Win95).
void drawBevelFrame(SDL_Renderer* renderer, SDL_Rect rect, int thickness, SDL_Color topLeftColor, SDL_Color bottomRightColor);

// Draws the bevels stored in the style (outer and inner, 1px each).
// A plain border (borderColor/borderWidth) is then ignored.
void drawStyleBevel(SDL_Renderer* renderer, SDL_Rect rect, const Style& style);

// Single border renderer for all widgets: bevel (when the widget supports
// it and the style carries bevel colors) takes priority over the plain
// border, exactly as drawBackgroundAndBorder always did. Custom-draw widgets
// (e.g. RadioButton) must call this instead of duplicating the branch.
void drawResolvedBorder(SDL_Renderer* renderer, SDL_Rect rect, const Style& style, bool withBevel);

// Accent-color helpers with backwards-compat fallback to borderColor.
SDL_Color effectiveThumbColor(const Style& style, SDL_Color fallback = {100, 100, 100, 255});
SDL_Color effectiveFillColor(const Style& style, SDL_Color fallback = {100, 100, 100, 255});

// Fills the 4 edge colors in the style with the Windows 95/98 system palette.
// Used by GUIElement::setBevel and the layout parsers ("bevel" shorthand).
void applyBevelToStyle(Style& style, BevelType type);

inline void drawTitleBar(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SetDrawColor(renderer, constants::kTitleBarColor);
    RenderFillRect(renderer, SDL_Rect{x, y, w, h});
    SetDrawColor(renderer, constants::kTitleBarLineColor);
    RenderLine(renderer, x, y + h, x + w, y + h);
}
