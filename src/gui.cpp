#include "label.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include <SDL3/SDL.h>
#include "std.hpp"

void drawRoundedFilledRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color) {
    if (radius <= 0.0f || rect.w < 2.0f * radius || rect.h < 2.0f * radius) {
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        { SDL_FRect _fr = {static_cast<float>(rect.x), static_cast<float>(rect.y), static_cast<float>(rect.w), static_cast<float>(rect.h)}; SDL_RenderFillRect(renderer, &_fr); }
        return;
    }

    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    float left = rect.x, top = rect.y;
    float right = rect.x + rect.w, bottom = rect.y + rect.h;
    float r = radius;

    { SDL_FRect _fr = {left + r, top, right - left - 2.0f*r, r}; SDL_RenderFillRect(renderer, &_fr); }
    { SDL_FRect _fr = {left + r, bottom - r, right - left - 2.0f*r, r}; SDL_RenderFillRect(renderer, &_fr); }
    { SDL_FRect _fr = {left, top + r, r, bottom - top - 2.0f*r}; SDL_RenderFillRect(renderer, &_fr); }
    { SDL_FRect _fr = {right - r, top + r, r, bottom - top - 2.0f*r}; SDL_RenderFillRect(renderer, &_fr); }
    { SDL_FRect _fr = {left + r, top + r, right - left - 2.0f*r, bottom - top - 2.0f*r}; SDL_RenderFillRect(renderer, &_fr); }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const int segments = 8;
    std::vector<SDL_Vertex> verts;
    float cx[4] = {left + r, right - r, right - r, left + r};
    float cy[4] = {top + r, top + r, bottom - r, bottom - r};
    float ba[4] = {3.1415927f, 4.712389f, 0, 1.5707963f};

    for (int corner = 0; corner < 4; ++corner) {
        for (int i = 0; i < segments; ++i) {
            float a0 = ba[corner] + static_cast<float>(i) * 1.5707963f / static_cast<float>(segments);
            float a1 = ba[corner] + static_cast<float>(i + 1) * 1.5707963f / static_cast<float>(segments);
            verts.push_back({SDL_FPoint{cx[corner], cy[corner]}, color, SDL_FPoint{0, 0}});
            verts.push_back({SDL_FPoint{cx[corner] + cosf(a0) * r, cy[corner] + sinf(a0) * r}, color, SDL_FPoint{0, 0}});
            verts.push_back({SDL_FPoint{cx[corner] + cosf(a1) * r, cy[corner] + sinf(a1) * r}, color, SDL_FPoint{0, 0}});
        }
    }

    SDL_RenderGeometry(renderer, NULL, verts.data(), static_cast<int>(verts.size()), NULL, 0);
}

void drawRoundedRectBorder(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color, float thickness) {
    if (radius <= 0.0f || rect.w < 2.0f * radius || rect.h < 2.0f * radius) {
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        for (float i = 0; i < thickness; ++i) {
            SDL_FRect r = {rect.x + i, rect.y + i, rect.w - 2.0f * i, rect.h - 2.0f * i};
            { SDL_FRect _fr = {static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)}; SDL_RenderRect(renderer, &_fr); }
        }
        return;
    }

    const int segments = 8;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (float t = 0; t < thickness; ++t) {
        float rInner = radius - t - 1.0f;
        float rOuter = radius - t;
        if (rOuter <= 0.0f) rOuter = 0.0f;
        if (rInner < 0.0f) rInner = 0.0f;
        float x = rect.x + t, y = rect.y + t;
        float w = rect.w - 2.0f * t, h = rect.h - 2.0f * t;
        float ro = rOuter, ri = rInner;

        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        if (ro < 1.0f) {
            { SDL_FRect _fr = {x, y, w, h}; SDL_RenderRect(renderer, &_fr); }
            continue;
        }

        { SDL_FRect _fr = {x + ro, y, w - 2.0f*ro, 1.0f}; SDL_RenderFillRect(renderer, &_fr); }
        { SDL_FRect _fr = {x + ro, y + h - 1.0f, w - 2.0f*ro, 1.0f}; SDL_RenderFillRect(renderer, &_fr); }
        { SDL_FRect _fr = {x, y + ro, 1.0f, h - 2.0f*ro}; SDL_RenderFillRect(renderer, &_fr); }
        { SDL_FRect _fr = {x + w - 1.0f, y + ro, 1.0f, h - 2.0f*ro}; SDL_RenderFillRect(renderer, &_fr); }

        std::vector<SDL_Vertex> verts;
        float cx[4] = {x + ro, x + w - ro, x + w - ro, x + ro};
        float cy[4] = {y + ro, y + ro, y + h - ro, y + h - ro};
        float ba[4] = {3.1415927f, 4.712389f, 0, 1.5707963f};

        for (int corner = 0; corner < 4; ++corner) {
            for (int i = 0; i < segments; ++i) {
                float a0 = ba[corner] + static_cast<float>(i) * 1.5707963f / static_cast<float>(segments);
                float a1 = ba[corner] + static_cast<float>(i + 1) * 1.5707963f / static_cast<float>(segments);
                float c0 = cosf(a0), s0 = sinf(a0);
                float c1 = cosf(a1), s1 = sinf(a1);
                // quad strip: outer_a0 → outer_a1 → inner_a1, outer_a0 → inner_a1 → inner_a0
                verts.push_back({SDL_FPoint{cx[corner] + c0*ro, cy[corner] + s0*ro}, color, SDL_FPoint{0,0}});
                verts.push_back({SDL_FPoint{cx[corner] + c1*ro, cy[corner] + s1*ro}, color, SDL_FPoint{0,0}});
                verts.push_back({SDL_FPoint{cx[corner] + c1*ri, cy[corner] + s1*ri}, color, SDL_FPoint{0,0}});
                verts.push_back({SDL_FPoint{cx[corner] + c0*ro, cy[corner] + s0*ro}, color, SDL_FPoint{0,0}});
                verts.push_back({SDL_FPoint{cx[corner] + c1*ri, cy[corner] + s1*ri}, color, SDL_FPoint{0,0}});
                verts.push_back({SDL_FPoint{cx[corner] + c0*ri, cy[corner] + s0*ri}, color, SDL_FPoint{0,0}});
            }
        }
        SDL_RenderGeometry(renderer, NULL, verts.data(), static_cast<int>(verts.size()), NULL, 0);
    }
}


// Implementacja klasy GUIElement
GUIElement::GUIElement(GUIManager& manager, int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_manager(manager), m_parent(nullptr) {
    m_manager.registerElement(this);
}

GUIElement::~GUIElement() {
    m_manager.unregisterElement(this);
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

bool GUIElement::contains(float x, float y) const {
    return contains(static_cast<int>(x), static_cast<int>(y));
}

SDL_Point GUIElement::toLocalCoords(int globalX, int globalY) const {
    auto abs = getAbsolutePosition();
    return {globalX - abs.x, globalY - abs.y};
}

void GUIElement::setRotation(double angleDegrees) {
    m_rotation = angleDegrees;
    markDirty();
}

void GUIElement::setRotationCenter(int cx, int cy) {
    m_rotationCenter = {cx, cy};
    markDirty();
}

void GUIElement::addChild(std::unique_ptr<GUIElement> child) {
    if (child && child->m_parent != this) {
        m_manager.registerElement(child.get());
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

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
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
    bool mouseInside = m_isHovered || (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && contains(e.button.x, e.button.y)) ||
                       (e.type == SDL_EVENT_MOUSE_BUTTON_UP && contains(e.button.x, e.button.y));
    
    if (mouseInside) {
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
            setState(ElementState::Pressed);
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
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
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
            // Released outside while pressed - set Normal
            setState(ElementState::Normal);
        }
    }

 
    return false;
}

void GUIElement::render(SDL_Renderer* renderer) {
    SDL_Rect viewport;
    SDL_GetRenderViewport(renderer, &viewport);
    render(renderer, viewport);
}

void GUIElement::render(SDL_Renderer* renderer, const SDL_Rect& parent_clip_rect) {
    if (!m_visible) {
        return;
    }

    auto abs_pos = getAbsolutePosition();
    SDL_Rect element_rect = {abs_pos.x, abs_pos.y, m_width, m_height};
    SDL_Rect clipped_rect;

    if (!SDL_GetRectIntersection(&element_rect, &parent_clip_rect, &clipped_rect)) {
        return; // Element jest całkowicie poza obszarem przycinania
    }

    if (wantsDirectRender()) {
        SDL_SetRenderClipRect(renderer, &clipped_rect);
        drawDirect(renderer);
        SDL_SetRenderClipRect(renderer, &parent_clip_rect);
        m_isDirty = false;
    } else {
        if (m_isDirty) {
            renderToCache();
        }

        if (m_cachedTexture) {
            if (m_rotation != 0.0) {
                SDL_FRect dst_rect = {static_cast<float>(abs_pos.x), static_cast<float>(abs_pos.y), static_cast<float>(m_width), static_cast<float>(m_height)};
                SDL_FPoint center = m_rotationCenter.x >= 0 
                    ? SDL_FPoint{static_cast<float>(m_rotationCenter.x), static_cast<float>(m_rotationCenter.y)} 
                    : SDL_FPoint{static_cast<float>(m_width) / 2.0f, static_cast<float>(m_height) / 2.0f};
                if (m_gpuState) SDL_SetGPURenderState(renderer, m_gpuState);
                SDL_RenderTextureRotated(renderer, m_cachedTexture.get(), nullptr, &dst_rect,
                                 m_rotation, &center, SDL_FLIP_NONE);
                if (m_gpuState) SDL_SetGPURenderState(renderer, nullptr);
            } else {
                SDL_Rect src_rect;
                src_rect.x = clipped_rect.x - abs_pos.x;
                src_rect.y = clipped_rect.y - abs_pos.y;
                src_rect.w = clipped_rect.w;
                src_rect.h = clipped_rect.h;
                if (m_gpuState) SDL_SetGPURenderState(renderer, m_gpuState);
                { SDL_FRect _sr = {static_cast<float>(src_rect.x), static_cast<float>(src_rect.y), static_cast<float>(src_rect.w), static_cast<float>(src_rect.h)}; SDL_FRect _dr = {static_cast<float>(clipped_rect.x), static_cast<float>(clipped_rect.y), static_cast<float>(clipped_rect.w), static_cast<float>(clipped_rect.h)}; SDL_RenderTexture(renderer, m_cachedTexture.get(), &_sr, &_dr); }
                if (m_gpuState) SDL_SetGPURenderState(renderer, nullptr);
            }
        }
    }

    SDL_Rect child_clip_rect = m_clip_children ? clipped_rect : parent_clip_rect;
    if (m_rotation == 0.0) {
        for (auto& child : m_children) {
            if (child && child->isVisible()) {
                child->render(renderer, child_clip_rect);
            }
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
        float tw = 0.0f, th = 0.0f;
        if (!SDL_GetTextureSize(m_cachedTexture.get(), &tw, &th)) {
            LOG_DEBUG("GUIElement: SDL_GetTextureSize failed: %s", SDL_GetError());
            tex_w = tex_h = 0;
        } else {
            tex_w = static_cast<int>(tw);
            tex_h = static_cast<int>(th);
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

    if (m_rotation != 0.0) {
        SDL_SetRenderTarget(renderer, nullptr);
        for (auto& child : m_children) {
            if (child && child->isVisible() && child->m_isDirty) {
                child->renderToCache();
            }
        }
        
        SDL_SetRenderTarget(renderer, m_cachedTexture.get());
        for (auto& child : m_children) {
            if (child && child->isVisible() && child->m_cachedTexture) {
                SDL_Rect childDst = {child->m_x, child->m_y, child->m_width, child->m_height};
                { SDL_FRect _dr = {static_cast<float>(childDst.x), static_cast<float>(childDst.y), static_cast<float>(childDst.w), static_cast<float>(childDst.h)}; SDL_RenderTexture(renderer, child->m_cachedTexture.get(), nullptr, &_dr); }
            }
        }
    }

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

  //  LOG_DEBUG("setState for %s from %d to %d", getComponentType(), static_cast<int>(m_state), static_cast<int>(newState));

        const auto oldStyle = getComposedStyle(m_state);
        const auto newStyle = getComposedStyle(newState);

    logStyle(oldStyle, "oldStyle");
    logStyle(newStyle, "newStyle");

    m_state = newState;

    if (oldStyle != newStyle) {
        LOG_DEBUG("Styles are different, marking dirty.");
        markDirty();
    }
}
void GUIElement::setStyle(ElementState state, Style style) {
    m_localStyles[stateIndex(state)] = std::move(style);
    markDirty();
}
Style GUIElement::getComposedStyle(ElementState state) const {
    size_t idx = stateIndex(state);
    
    // Start from local style (most specific), fill gaps from theme
    if (m_localStyles[idx].has_value()) {
        Style result = *m_localStyles[idx];
        result.mergeWith(m_manager.getTheme().getStyle(getComponentType(), state));
        return result;
    }
    
    // Fallback: try Normal state local style
    size_t normalIdx = stateIndex(ElementState::Normal);
    if (m_localStyles[normalIdx].has_value()) {
        Style result = *m_localStyles[normalIdx];
        result.mergeWith(m_manager.getTheme().getStyle(getComponentType(), state));
        return result;
    }
    
    // No local styles: use theme directly
    return m_manager.getTheme().getStyle(getComponentType(), state);
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
    SDL_FRect frect = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height)};
    SDL_FColor fcolor;
    float fradius = static_cast<float>(radius);

    if (style.backgroundColor) {
        fcolor.r = style.backgroundColor->r / 255.0f;
        fcolor.g = style.backgroundColor->g / 255.0f;
        fcolor.b = style.backgroundColor->b / 255.0f;
        fcolor.a = style.backgroundColor->a / 255.0f;
        drawRoundedFilledRect(renderer, frect, fradius, fcolor);
    }

    if (style.texture.has_value()) {
        SDL_FRect texRect = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height)};
        { SDL_FRect _dr = {static_cast<float>(texRect.x), static_cast<float>(texRect.y), static_cast<float>(texRect.w), static_cast<float>(texRect.h)}; SDL_RenderTexture(renderer, style.texture.value().get(), nullptr, &_dr); }
    }

    if (style.borderColor && style.borderWidth && *style.borderWidth > 0) {
        fcolor.r = style.borderColor->r / 255.0f;
        fcolor.g = style.borderColor->g / 255.0f;
        fcolor.b = style.borderColor->b / 255.0f;
        fcolor.a = style.borderColor->a / 255.0f;
        drawRoundedRectBorder(renderer, frect, fradius, fcolor, static_cast<float>(*style.borderWidth));
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
            return static_cast<int>(static_cast<float>(isHorizontal ? parentWidth : parentHeight) * value);
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
