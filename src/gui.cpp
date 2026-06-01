#include "label.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"


// Implementacja klasy GUIElement
GUIElement::GUIElement(GUIManager& manager, int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_manager(manager), m_parent(nullptr) {
}

GUIElement::~GUIElement() {
    // Stop tooltip timer if running
    if (tooltipTimerId != 0) {
        stopTimer(tooltipTimerId);
        tooltipTimerId = 0;
    }
}

void GUIElement::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
    markDirty();
}

void GUIElement::setSize(int width, int height) {
    m_width = width;
    m_height = height;
    markDirty();
}

void GUIElement::setParent(GUIElement* parent) {
    m_parent = parent;
}

void GUIElement::getSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void GUIElement::setClipChildren(bool clip) {
    m_clip_children = clip;
}

SDL_Point GUIElement::getAbsolutePosition() const {
    auto pos = SDL_Point{m_x, m_y};
    if (m_parent) {
        auto parentPos = m_parent->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    return pos;
}

bool GUIElement::contains(int x, int y) const {
    auto absPos = getAbsolutePosition();
    return (x >= absPos.x && x < absPos.x + m_width &&
            y >= absPos.y && y < absPos.y + m_height);
}

void GUIElement::addChild(std::unique_ptr<GUIElement> child) {
    if (child && child->m_parent != this) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
        markDirty();
    }
}

void GUIElement::clearChildren() {
    m_children.clear();
    markDirty();
}

void GUIElement::setTooltip(const std::string& text) {
    this->tooltip = text;
}

void GUIElement::setID(std::string_view id) {
    m_id = id;
}

std::string_view GUIElement::getID() const {
    return m_id;
}

bool GUIElement::handleEvent(const SDL_Event& e) {
    if (!m_visible) {
        return false;
    }

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handleEvent(e)) {
            return true;
        }
    }

    if (!m_enabled) {
        setState(ElementState::Disabled);
        return false;
    }

    if (e.type == SDL_MOUSEMOTION) {
        bool currentlyHovered = contains(e.motion.x, e.motion.y);

        if (currentlyHovered && !m_isHovered) {
            m_isHovered = true;
            if (!tooltip.empty()) {
                tooltipTimerId = startTimer(500, true, [](GUIElement* self) {
                    if (self) { self->m_manager.showTooltip(self, self->tooltip); }
                });
            }
        } else if (!currentlyHovered && m_isHovered) {
            m_isHovered = false;
            if (tooltipTimerId != 0) {
                stopTimer(tooltipTimerId);
                tooltipTimerId = 0;
            }
            m_manager.hideTooltip();
        }
    }

    // For mouse button events, check actual position at event time
    bool mouseInside = m_isHovered || (e.type == SDL_MOUSEBUTTONDOWN && contains(e.button.x, e.button.y)) ||
                       (e.type == SDL_MOUSEBUTTONUP && contains(e.button.x, e.button.y));
    
    if (mouseInside) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            setState(ElementState::Pressed);
        } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            if (m_state == ElementState::Pressed) {
                // Only set Hover if mouse is actually inside at release time
                if (contains(e.button.x, e.button.y)) {
                    setState(ElementState::Hover);
                } else {
                    setState(ElementState::Normal);
                }
            }
        } else if (m_state != ElementState::Pressed) {
            setState(ElementState::Hover);
        }
    } else {
        // Mouse outside - set Normal (unless still pressed for drag scenarios)
        if (m_state != ElementState::Pressed) {
            setState(ElementState::Normal);
        } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            // Released outside while pressed - set Normal
            setState(ElementState::Normal);
        }
    }

 
    return false;
}

void GUIElement::render(SDL_Renderer* renderer) {
    SDL_Rect viewport;
    SDL_RenderGetViewport(renderer, &viewport);
    render(renderer, viewport);
}

void GUIElement::render(SDL_Renderer* renderer, const SDL_Rect& parent_clip_rect) {
    if (!m_visible) {
        return;
    }

    auto abs_pos = getAbsolutePosition();
    SDL_Rect element_rect = {abs_pos.x, abs_pos.y, m_width, m_height};
    SDL_Rect clipped_rect;

    if (!SDL_IntersectRect(&element_rect, &parent_clip_rect, &clipped_rect)) {
        return; // Element jest całkowicie poza obszarem przycinania
    }

    if (wantsDirectRender()) {
        SDL_RenderSetClipRect(renderer, &clipped_rect);
        drawDirect(renderer);
        SDL_RenderSetClipRect(renderer, &parent_clip_rect);
        m_isDirty = false;
    } else {
        if (m_isDirty) {
            renderToCache();
        }

        if (m_cachedTexture) {
            SDL_Rect src_rect;
            src_rect.x = clipped_rect.x - abs_pos.x;
            src_rect.y = clipped_rect.y - abs_pos.y;
            src_rect.w = clipped_rect.w;
            src_rect.h = clipped_rect.h;
            SDL_RenderCopy(renderer, m_cachedTexture.get(), &src_rect, &clipped_rect);
        }
    }

    SDL_Rect child_clip_rect = m_clip_children ? clipped_rect : parent_clip_rect;
    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render(renderer, child_clip_rect);
        }
    }
}

void GUIElement::renderOverlay(SDL_Renderer* renderer) {
    render(renderer);
}

void GUIElement::renderToCache() {
    if (!m_visible || m_width <= 0 || m_height <= 0) {
        if (m_cachedTexture) m_cachedTexture.reset();
        return;
    }

    int tex_w = 0, tex_h = 0;
    if (m_cachedTexture) {
        if (SDL_QueryTexture(m_cachedTexture.get(), nullptr, nullptr, &tex_w, &tex_h) != 0) {
            LOG_DEBUG("GUIElement: SDL_QueryTexture failed: %s", SDL_GetError());
            tex_w = tex_h = 0;
        }
    }
    if (!m_cachedTexture || tex_w != m_width || tex_h != m_height) {
        m_cachedTexture.reset(SDL_CreateTexture(m_manager.getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_width, m_height));
        if (m_cachedTexture) {
            SDL_SetTextureBlendMode(m_cachedTexture.get(), SDL_BLENDMODE_BLEND);
        } else {
            m_isDirty = false;
            return;
        }
    }

    SDL_Renderer* renderer = m_manager.getRenderer();
    SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, m_cachedTexture.get());

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    draw(renderer);

    SDL_SetRenderTarget(renderer, oldTarget);
    m_isDirty = false;
}

void GUIElement::markDirty(bool cascadeToParents) {
    m_isDirty = true;
    if (cascadeToParents && m_parent) {
        m_parent->markDirty(true);
    }
}

void GUIElement::markDirtyRecursively() {
    m_isDirty = true;
    for (auto& child : m_children) {
        if (child) {
            child->markDirtyRecursively();
        }
    }
}


void GUIElement::markForDeletion() {
    m_isMarkedForDeletion = true;
}

bool GUIElement::isMarkedForDeletion() const {
    return m_isMarkedForDeletion;
}
void GUIElement::cleanup() {
    for (const auto& child : m_children) {
        if (child) {
            child->cleanup();
        }
    }

    const auto initial_size = m_children.size();

    auto new_end = std::remove_if(m_children.begin(), m_children.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });

    m_children.erase(new_end, m_children.end());

    const auto removed_count = initial_size - m_children.size();
    if (removed_count > 0) {
        LOG_DEBUG("GUIElement::cleanup(): Removed %zu child elements.", removed_count);
        markDirty();
    }
}

uint32_t GUIElement::startTimer(uint32_t delay, bool singleShot, std::function<void(GUIElement*)> callback) {
    if (m_manager.getTimerManager()) {
        return m_manager.getTimerManager()->addTimer(this, delay, singleShot, [callback](GUIElement* target) {
            callback(target);
        });
    }
    return 0;
}

void GUIElement::stopTimer(uint32_t timerId) {
    if (m_manager.getTimerManager()) {
        m_manager.getTimerManager()->removeTimer(timerId);
    }
}

// --- Implementacja nowego API do stylizacji ---

const char* GUIElement::getComponentType() const {
    return "GUIElement";
}

void GUIElement::setState(ElementState newState) {
    if (m_state == newState) {
        return;
    }

    LOG_DEBUG("setState for %s from %d to %d", getComponentType(), static_cast<int>(m_state), static_cast<int>(newState));

        const auto oldStyle = getComposedStyle(m_state);
        const auto newStyle = getComposedStyle(newState);

    logStyle(oldStyle, "oldStyle");
    logStyle(newStyle, "newStyle");

    m_state = newState;

    if (oldStyle != newStyle) {
        LOG_DEBUG("Styles are different, marking dirty.");
        markDirty();
    } else {
        LOG_DEBUG("Styles are the same, not marking dirty.");
    }
}
void GUIElement::setStyle(ElementState state, Style style) {
    m_localStyles[stateIndex(state)] = std::move(style);
    markDirty();
}
Style GUIElement::getComposedStyle(ElementState state) const {
    // Start with theme style as base result
    Style result = m_manager.getTheme().getStyle(getComponentType(), state);
    
    size_t idx = stateIndex(state);
    if (m_localStyles[idx].has_value()) {
        // Merge local style directly into result (avoid intermediate copy)
        result.mergeWith(*m_localStyles[idx]);
        return result;
    }
    
    // Fallback: try Normal state local style
    size_t normalIdx = stateIndex(ElementState::Normal);
    if (m_localStyles[normalIdx].has_value()) {
        result.mergeWith(*m_localStyles[normalIdx]);
        return result;
    }
    
    return result;
}

void GUIElement::setBackgroundColor(ElementState state, SDL_Color color) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    m_localStyles[idx]->backgroundColor = color;
    markDirty();
}

void GUIElement::setTextColor(ElementState state, SDL_Color color) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    m_localStyles[idx]->textColor = color;
    markDirty();
}

void GUIElement::setTexture(ElementState state, SharedTexture texture) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    m_localStyles[idx]->texture = std::move(texture);
    markDirty();
}
void GUIElement::setBorder(ElementState state, SDL_Color color, int width) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    m_localStyles[idx]->borderColor = color;
    m_localStyles[idx]->borderWidth = width;
    markDirty();
}
void GUIElement::setBorderRadius(ElementState state, int radius) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    m_localStyles[idx]->borderRadius = radius;
    markDirty();
}

void GUIElement::drawBackgroundAndBorder(SDL_Renderer* renderer) {
    const Style& style = getComposedStyle(m_state);
    int radius = style.borderRadius.value_or(0);
    // Clamp radius to half of the smaller dimension to avoid overflow
    radius = std::min(radius, std::min(m_width, m_height) / 2);

    // Rysowanie tła
    if (style.backgroundColor) {
        const SDL_Color& bg = *style.backgroundColor;
        if (radius > 0) {
            // Zaokrąglone rogi - używamy SDL2_gfx
            roundedBoxRGBA(renderer, 0, 0, m_width - 1, m_height - 1, radius,
                           bg.r, bg.g, bg.b, bg.a);
        } else {
            // Ostre rogi - standardowy SDL
            SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
            SDL_Rect bgRect = {0, 0, m_width, m_height};
            SDL_RenderFillRect(renderer, &bgRect);
        }
    }

    // Rysowanie obramowania
    if (style.borderColor && style.borderWidth && *style.borderWidth > 0) {
        const SDL_Color& bc = *style.borderColor;
        int width = *style.borderWidth;
        if (radius > 0) {
            // Zaokrąglone obramowanie - używamy SDL2_gfx
            // Rysujemy kilka obramowań z malejącym radius dla grubości > 1
            for (int i = 0; i < width; ++i) {
                int innerRadius = radius - i;
                if (innerRadius < 0) innerRadius = 0;
                roundedRectangleRGBA(renderer, i, i, m_width - 1 - i, m_height - 1 - i,
                                      innerRadius, bc.r, bc.g, bc.b, bc.a);
            }
        } else {
            // Ostre obramowanie - standardowy SDL
            SDL_SetRenderDrawColor(renderer, bc.r, bc.g, bc.b, bc.a);
            SDL_Rect borderRect = {0, 0, m_width, m_height};
            for (int i = 0; i < width; ++i) {
                SDL_RenderDrawRect(renderer, &borderRect);
                borderRect.x++;
                borderRect.y++;
                borderRect.w -= 2;
                borderRect.h -= 2;
            }
        }
    }
}

size_t GUIElement::countDescendants() const {
    size_t count = m_children.size();
    for (const auto& child : m_children) {
        count += child->countDescendants();
    }
    return count;
}

bool GUIElement::canGetKeyboardFocus() const {
    return m_canGetKeyboardFocus;
}

void GUIElement::setCanGetKeyboardFocus(bool canFocus) {
    m_canGetKeyboardFocus = canFocus;
}

bool GUIElement::hasKeyboardFocus() const {
    return m_manager.getKeyboardFocus() == this;
}

GUIElement* GUIElement::findElementAt(int x, int y) {
    if (!m_visible || !contains(x, y)) {
        return nullptr;
    }

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (auto* found = (*it)->findElementAt(x, y)) {
            return found;
        }
    }

    return this;
}

// === Anchor system implementation ===

void GUIElement::setAnchor(const Anchor& anchor) {
    m_anchor = anchor;
    // Store original size before anchor modifications
    if (m_originalWidth == 0 && m_originalHeight == 0) {
        storeOriginalSize();
    }
}

void GUIElement::storeOriginalSize() {
    m_originalWidth = m_width;
    m_originalHeight = m_height;
}

void GUIElement::applyAnchor(int parentWidth, int parentHeight) {
    if (!m_anchor.hasAnyAnchor()) {
        return; // No anchor set, use fixed position
    }
    
    // Calculate new position and size
    int newX = m_x;
    int newY = m_y;
    int newWidth = m_width;
    int newHeight = m_height;
    
    // Helper function to convert anchor value to pixels
    auto toPixels = [parentWidth, parentHeight](float value, bool isHorizontal) -> int {
        if (value < 0) return -1; // Not set
        if (value <= 1.0f) {
            // Percentage
            return static_cast<int>((isHorizontal ? parentWidth : parentHeight) * value);
        }
        // Fixed pixels (subtract 1 to distinguish from percentages)
        return static_cast<int>(value - 1.0f);
    };
    
    // Determine element size to use for positioning
    int effectiveWidth = m_originalWidth > 0 ? m_originalWidth : m_width;
    int effectiveHeight = m_originalHeight > 0 ? m_originalHeight : m_height;
    
    // Handle horizontal positioning
    if (m_anchor.stretchesHorizontal()) {
        // Stretch mode: both left and right are set
        int leftPx = toPixels(m_anchor.left, true);
        int rightPx = toPixels(m_anchor.right, true);
        newX = leftPx;
        newWidth = parentWidth - leftPx - rightPx;
        if (newWidth < 0) newWidth = 0;
    } else if (m_anchor.hasLeft()) {
        int leftPx = toPixels(m_anchor.left, true);
        // Special case: 0.5 means center horizontally
        if (m_anchor.left == 0.5f) {
            newX = leftPx - effectiveWidth / 2;  // Center the element, not its corner
        } else {
            newX = leftPx;
        }
        newWidth = effectiveWidth;
    } else if (m_anchor.hasRight()) {
        // Only right anchor: position from right edge, keep original width
        int rightPx = toPixels(m_anchor.right, true);
        newWidth = effectiveWidth;
        newX = parentWidth - rightPx - newWidth;
    }
    
    // Handle vertical positioning
    if (m_anchor.stretchesVertical()) {
        // Stretch mode: both top and bottom are set
        int topPx = toPixels(m_anchor.top, false);
        int bottomPx = toPixels(m_anchor.bottom, false);
        newY = topPx;
        newHeight = parentHeight - topPx - bottomPx;
        if (newHeight < 0) newHeight = 0;
    } else if (m_anchor.hasTop()) {
        int topPx = toPixels(m_anchor.top, false);
        // Special case: 0.5 means center vertically
        if (m_anchor.top == 0.5f) {
            newY = topPx - effectiveHeight / 2;  // Center the element, not its corner
        } else {
            newY = topPx;
        }
        newHeight = effectiveHeight;
    } else if (m_anchor.hasBottom()) {
        // Only bottom anchor: position from bottom edge, keep original height
        int bottomPx = toPixels(m_anchor.bottom, false);
        newHeight = effectiveHeight;
        newY = parentHeight - bottomPx - newHeight;
    }
    
    // Apply calculated values
    if (newX != m_x || newY != m_y) {
        setPosition(newX, newY);
    }
    if (newWidth != m_width || newHeight != m_height) {
        setSize(newWidth, newHeight);
    }
}

void GUIElement::updateLayout(int parentWidth, int parentHeight) {
    // Apply anchor to this element
    applyAnchor(parentWidth, parentHeight);
    
    // Propagate to children
    for (auto& child : m_children) {
        if (child) {
            child->updateLayout(m_width, m_height);
        }
    }
}

void GUIElement::onParentResize(int parentWidth, int parentHeight) {
    updateLayout(parentWidth, parentHeight);
}
