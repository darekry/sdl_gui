#pragma once

#include "std.hpp"

/**
 * @brief Anchor defines how an element positions itself relative to its parent/container
 * 
 * Anchors use a flexible coordinate system:
 * - Negative value (-1 or less) = anchor is NOT set, use fixed position
 * - 0.0 = anchored to left/top edge (0 pixels from edge)
 * - 1.0 = anchored to right/bottom edge (element moves with edge)
 * - 0.0-1.0 = percentage of parent size (element scales proportionally)
 * - >1.0 = fixed pixel offset from that edge
 * 
 * **Special case: 0.5 = CENTER**
 * When left=0.5, the element is horizontally centered (center of element at center of parent)
 * When top=0.5, the element is vertically centered (center of element at center of parent)
 * 
 * When both left and right are set (both >= 0), the element stretches horizontally.
 * When both top and bottom are set (both >= 0), the element stretches vertically.
 * 
 * Example layouts:
 * - Top-left fixed: left=10, top=10, right=-1, bottom=-1 (10px from edges, fixed size)
 * - Centered: left=0.5, top=0.5, right=-1, bottom=-1 (element centered, not corner!)
 * - Bottom-right: left=-1, top=-1, right=10, bottom=10 (10px from right/bottom)
 * - Full stretch: left=0, top=0, right=0, bottom=0 (fills entire parent)
 * - Horizontal bar: left=0, top=-1, right=0, bottom=50 (full width, 50px from bottom)
 */
struct Anchor {
    float left = -1.0f;     // <0 = not set, 0-1 = %, >1 = pixels from left
    float top = -1.0f;      // <0 = not set, 0-1 = %, >1 = pixels from top
    float right = -1.0f;    // <0 = not set, 0-1 = %, >1 = pixels from right
    float bottom = -1.0f;   // <0 = not set, 0-1 = %, >1 = pixels from bottom
    
    // Check if anchor is set for this edge
    bool hasLeft() const { return left >= 0; }
    bool hasTop() const { return top >= 0; }
    bool hasRight() const { return right >= 0; }
    bool hasBottom() const { return bottom >= 0; }
    
    // Stretch modes
    bool stretchesHorizontal() const { return hasLeft() && hasRight(); }
    bool stretchesVertical() const { return hasTop() && hasBottom(); }
    bool isStretched() const { return stretchesHorizontal() || stretchesVertical(); }
    
    // Check if any anchor is set
    bool hasAnyAnchor() const { return hasLeft() || hasTop() || hasRight() || hasBottom(); }
    
    // === Preset anchors ===
    
    /** No anchor - use fixed position/size */
    static Anchor none() { return Anchor{}; }
    
    /** Anchor to top-left corner with optional margins */
    static Anchor topLeft(float margin = 0) {
        return Anchor{margin, margin, -1, -1};
    }
    
    /** Anchor to top-right corner with optional margins */
    static Anchor topRight(float margin = 0) {
        return Anchor{-1, margin, margin, -1};
    }
    
    /** Anchor to bottom-left corner with optional margins */
    static Anchor bottomLeft(float margin = 0) {
        return Anchor{margin, -1, -1, margin};
    }
    
    /** Anchor to bottom-right corner with optional margins */
    static Anchor bottomRight(float margin = 0) {
        return Anchor{-1, -1, margin, margin};
    }
    
    /** Center in parent (keeps original size) */
    static Anchor center() {
        return Anchor{0.5f, 0.5f, -1, -1};
    }
    
    /** Fill entire parent with uniform margin */
    static Anchor fill(float margin = 0) {
        return Anchor{margin, margin, margin, margin};
    }
    
    /** Fill horizontally with top/bottom margins (keeps original height if not stretched) */
    static Anchor horizontalStretch(float leftMargin = 0, float rightMargin = 0) {
        return Anchor{leftMargin, -1, rightMargin, -1};
    }
    
    /** Fill vertically with left/right margins (keeps original width if not stretched) */
    static Anchor verticalStretch(float topMargin = 0, float bottomMargin = 0) {
        return Anchor{-1, topMargin, -1, bottomMargin};
    }
    
    /** Bottom bar - full width, fixed height from bottom */
    static Anchor bottomBar(float height, float leftMargin = 0, float rightMargin = 0) {
        return Anchor{leftMargin, -1, rightMargin, height};
    }
    
    /** Top bar - full width, fixed height from top */
    static Anchor topBar(float height, float leftMargin = 0, float rightMargin = 0) {
        return Anchor{leftMargin, height, rightMargin, -1};
    }
    
    /** Left sidebar - full height, fixed width from left
     *  Width comes from the element's original size (kept by the anchor engine),
     *  the width parameter is accepted for API compatibility. */
    static Anchor leftSidebar(float width, float topMargin = 0, float bottomMargin = 0) {
        (void)width;
        return Anchor{0, topMargin, -1, bottomMargin};
    }
    
    /** Right sidebar - full height, fixed width from right
     *  Width comes from the element's original size (kept by the anchor engine),
     *  the width parameter is accepted for API compatibility. */
    static Anchor rightSidebar(float width, float topMargin = 0, float bottomMargin = 0) {
        (void)width;
        return Anchor{-1, topMargin, 0, bottomMargin};
    }
};

/**
 * @brief AnchorMode determines how anchor values are interpreted
 */
enum class AnchorMode {
    Pixels,     // All values are in pixels
    Percentage, // 0.0-1.0 values are percentages, >1.0 are pixels (hybrid)
    Hybrid      // Same as Percentage (default)
};