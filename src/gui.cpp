#include "label.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "sdl_deleters.hpp"
#include "constants.hpp"
#include <SDL3/SDL.h>
#include "std.hpp"

namespace {
    constexpr int kCornerSegments = 8;
    constexpr float kPi      = 3.1415927f;
    constexpr float kPiHalf  = 1.5707963f;
    constexpr float k3PiHalf = 4.712389f;
    constexpr float kBa[4]   = {kPi, k3PiHalf, 0, kPiHalf};
    constexpr SDL_FColor kWhite = {1, 1, 1, 1};

    void computeCornerCenters(float* cx, float* cy, float left, float top, float right, float bottom, float r) {
        cx[0] = left + r;  cx[1] = right - r; cx[2] = right - r; cx[3] = left + r;
        cy[0] = top + r;   cy[1] = top + r;   cy[2] = bottom - r; cy[3] = bottom - r;
    }

    template<typename TexCoordFn>
    void addRoundedCornerTriangles(std::vector<SDL_Vertex>& verts,
                                   float cx, float cy, float ba, float r,
                                   SDL_FColor color, TexCoordFn&& texCoord) {
        for (int i = 0; i < kCornerSegments; ++i) {
            float a0 = ba + static_cast<float>(i) * kPiHalf / static_cast<float>(kCornerSegments);
            float a1 = ba + static_cast<float>(i + 1) * kPiHalf / static_cast<float>(kCornerSegments);
            float px0 = cx + cosf(a0) * r;
            float py0 = cy + sinf(a0) * r;
            float px1 = cx + cosf(a1) * r;
            float py1 = cy + sinf(a1) * r;
            verts.push_back({SDL_FPoint{cx, cy}, color, texCoord(cx, cy)});
            verts.push_back({SDL_FPoint{px0, py0}, color, texCoord(px0, py0)});
            verts.push_back({SDL_FPoint{px1, py1}, color, texCoord(px1, py1)});
        }
    }

    SDL_FPoint zeroTexCoord(float, float) { return {0, 0}; }
}

void drawRoundedFilledRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color) {
    if (radius <= 0.0f || rect.w < 2.0f * radius || rect.h < 2.0f * radius) {
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        RenderFillRect(renderer, rect);
        return;
    }

    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    float left = rect.x, top = rect.y;
    float right = rect.x + rect.w, bottom = rect.y + rect.h;
    float r = radius;

    RenderFillRect(renderer, SDL_FRect{left + r, top, right - left - 2.0f*r, r});
    RenderFillRect(renderer, SDL_FRect{left + r, bottom - r, right - left - 2.0f*r, r});
    RenderFillRect(renderer, SDL_FRect{left, top + r, r, bottom - top - 2.0f*r});
    RenderFillRect(renderer, SDL_FRect{right - r, top + r, r, bottom - top - 2.0f*r});
    RenderFillRect(renderer, SDL_FRect{left + r, top + r, right - left - 2.0f*r, bottom - top - 2.0f*r});

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    float cx[4], cy[4];
    computeCornerCenters(cx, cy, left, top, right, bottom, r);

    std::vector<SDL_Vertex> verts;
    for (int corner = 0; corner < 4; ++corner) {
        addRoundedCornerTriangles(verts, cx[corner], cy[corner], kBa[corner], r, color, zeroTexCoord);
    }

    SDL_RenderGeometry(renderer, NULL, verts.data(), static_cast<int>(verts.size()), NULL, 0);
}

void drawRoundedTexturedRect(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_Texture* texture) {
    if (radius <= 0.0f || rect.w < 2.0f * radius || rect.h < 2.0f * radius) {
        RenderTexture(renderer, texture, rect);
        return;
    }

    float left = rect.x, top = rect.y;
    float right = rect.x + rect.w, bottom = rect.y + rect.h;
    float r = radius;

    auto texCoord = [&](float px, float py) -> SDL_FPoint {
        return SDL_FPoint{(px - left) / rect.w, (py - top) / rect.h};
    };

    auto addTri = [&](std::vector<SDL_Vertex>& verts, float x0, float y0, float x1, float y1, float x2, float y2) {
        verts.push_back({SDL_FPoint{x0, y0}, kWhite, texCoord(x0, y0)});
        verts.push_back({SDL_FPoint{x1, y1}, kWhite, texCoord(x1, y1)});
        verts.push_back({SDL_FPoint{x2, y2}, kWhite, texCoord(x2, y2)});
    };

    auto addQuad = [&](std::vector<SDL_Vertex>& verts, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
        addTri(verts, x0, y0, x1, y1, x2, y2);
        addTri(verts, x0, y0, x2, y2, x3, y3);
    };

    std::vector<SDL_Vertex> verts;

    addQuad(verts, left + r, top, right - r, top, right - r, top + r, left + r, top + r);
    addQuad(verts, left + r, bottom - r, right - r, bottom - r, right - r, bottom, left + r, bottom);
    addQuad(verts, left, top + r, left + r, top + r, left + r, bottom - r, left, bottom - r);
    addQuad(verts, right - r, top + r, right, top + r, right, bottom - r, right - r, bottom - r);
    addQuad(verts, left + r, top + r, right - r, top + r, right - r, bottom - r, left + r, bottom - r);

    float cx[4], cy[4];
    computeCornerCenters(cx, cy, left, top, right, bottom, r);

    for (int corner = 0; corner < 4; ++corner) {
        addRoundedCornerTriangles(verts, cx[corner], cy[corner], kBa[corner], r, kWhite, texCoord);
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_RenderGeometry(renderer, texture, verts.data(), static_cast<int>(verts.size()), nullptr, 0);
}

void drawRoundedRectBorder(SDL_Renderer* renderer, SDL_FRect rect, float radius, SDL_FColor color, float thickness) {
    if (radius <= 0.0f || rect.w < 2.0f * radius || rect.h < 2.0f * radius) {
        SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
        for (float i = 0; i < thickness; ++i) {
            RenderRect(renderer, SDL_FRect{rect.x + i, rect.y + i, rect.w - 2.0f * i, rect.h - 2.0f * i});
        }
        return;
    }

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
            RenderRect(renderer, SDL_FRect{x, y, w, h});
            continue;
        }

        RenderFillRect(renderer, SDL_FRect{x + ro, y, w - 2.0f*ro, 1.0f});
        RenderFillRect(renderer, SDL_FRect{x + ro, y + h - 1.0f, w - 2.0f*ro, 1.0f});
        RenderFillRect(renderer, SDL_FRect{x, y + ro, 1.0f, h - 2.0f*ro});
        RenderFillRect(renderer, SDL_FRect{x + w - 1.0f, y + ro, 1.0f, h - 2.0f*ro});

        float cx[4], cy[4];
        computeCornerCenters(cx, cy, x, y, x + w, y + h, ro);

        std::vector<SDL_Vertex> verts;
        verts.reserve(static_cast<size_t>(4 * kCornerSegments * 6));
        for (int corner = 0; corner < 4; ++corner) {
            for (int i = 0; i < kCornerSegments; ++i) {
                float a0 = kBa[corner] + static_cast<float>(i) * kPiHalf / static_cast<float>(kCornerSegments);
                float a1 = kBa[corner] + static_cast<float>(i + 1) * kPiHalf / static_cast<float>(kCornerSegments);
                float c0 = cosf(a0), s0 = sinf(a0);
                float c1 = cosf(a1), s1 = sinf(a1);
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
    invalidateAbsPosCache();
    markDirty();
}

void GUIElement::setSize(int width, int height) {
    int oldWidth = m_width;
    int oldHeight = m_height;
    m_width = width;
    m_height = height;
    if (oldWidth != width || oldHeight != height) {
        onSizeChanged(oldWidth, oldHeight);
    }
    markDirty();
}

void GUIElement::setParent(GUIElement* parent) {
    m_parent = parent;
    invalidateAbsPosCache();
}

void GUIElement::getSize(int& width, int& height) const {
    width = m_width;
    height = m_height;
}

void GUIElement::setClipChildren(bool clip) {
    m_clip_children = clip;
}

SDL_Point GUIElement::getAbsolutePosition() const {
    if (m_absPosValid) {
        return m_cachedAbsPos;
    }
    auto pos = SDL_Point{m_x, m_y};
    if (m_parent) {
        auto parentPos = m_parent->getAbsolutePosition();
        pos.x += parentPos.x;
        pos.y += parentPos.y;
    }
    m_cachedAbsPos = pos;
    m_absPosValid = true;
    return pos;
}

void GUIElement::invalidateAbsPosCache() {
    m_absPosValid = false;
    for (auto& child : m_children) {
        child->invalidateAbsPosCache();
    }
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

GUIElement* GUIElement::addChild(std::unique_ptr<GUIElement> child) {
    if (child && child->m_parent != this) {
        GUIElement* raw = child.get();
        m_manager.registerElement(raw);
        raw->m_parent = this;
        raw->invalidateAbsPosCache();
        m_children.push_back(std::move(child));
        raw->updateLayout(m_width, m_height);
        markDirty();
        return raw;
    }
    return nullptr;
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
        processHoverTooltip(contains(e.motion.x, e.motion.y));
    }

    processButtonEvent(e);

    // Right click: fire the callback and consume the event so that
    // ancestors along the DFS path do not fire their own callbacks too.
    if (processRightClick(e)) {
        return true;
    }

    return false;
}

bool GUIElement::processRightClick(const SDL_Event& e) {
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_RIGHT && contains(e.button.x, e.button.y) && m_onRightClick) {
        m_onRightClick(this, e.button.x, e.button.y);
        return true;
    }
    return false;
}

void GUIElement::processButtonEvent(const SDL_Event& e) {
    bool mouseInside = m_isHovered || (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && contains(e.button.x, e.button.y)) ||
                       (e.type == SDL_EVENT_MOUSE_BUTTON_UP && contains(e.button.x, e.button.y));
    
    if (mouseInside) {
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
            setState(ElementState::Pressed);
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
            if (m_state == ElementState::Pressed) {
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
        if (m_state != ElementState::Pressed) {
            setState(ElementState::Normal);
        } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
            setState(ElementState::Normal);
        }
    }
}

void GUIElement::processHoverTooltip(bool currentlyHovered) {
    if (currentlyHovered && !m_isHovered) {
        m_isHovered = true;
        if (!tooltip.empty()) {
            tooltipTimerId = startTimer(constants::kTooltipDelayMs, true, [](GUIElement* self) {
                self->m_manager.showTooltip(self, self->tooltip);
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
        return; // Element is completely outside the clip area
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
                SDL_FRect dst_rect = SDLRectToFRect(abs_pos.x, abs_pos.y, m_width, m_height);
                SDL_FPoint center = m_rotationCenter.x >= 0 
                    ? SDL_FPoint{static_cast<float>(m_rotationCenter.x), static_cast<float>(m_rotationCenter.y)} 
                    : SDL_FPoint{static_cast<float>(m_width) / 2.0f, static_cast<float>(m_height) / 2.0f};
                SDL_RenderTextureRotated(renderer, m_cachedTexture.get(), nullptr, &dst_rect,
                                 m_rotation, &center, SDL_FLIP_NONE);
            } else {
                SDL_Rect src_rect;
                src_rect.x = clipped_rect.x - abs_pos.x;
                src_rect.y = clipped_rect.y - abs_pos.y;
                src_rect.w = clipped_rect.w;
                src_rect.h = clipped_rect.h;
                RenderTexture(renderer, m_cachedTexture.get(), &src_rect, &clipped_rect);
            }
            if (m_rotation == 0.0 && hasKeyboardFocus()) {
                const Style& focusStyle = getComposedStyle(m_state);
                drawRoundedRectBorder(renderer, SDLRectToFRect(abs_pos.x, abs_pos.y, m_width, m_height),
                                      static_cast<float>(focusStyle.borderRadius.value_or(0)),
                                      ColorToFColor(constants::kFocusOutlineColor), 1.0f);
            }
        }
    }

    SDL_Rect child_clip_rect = m_clip_children ? clipped_rect : parent_clip_rect;
    if (m_rotation == 0.0) {
        for (auto& child : m_children) {
            if (child->isVisible()) {
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
        m_cachedTexture.reset();
        m_cacheKey = 0;
        return;
    }

    SDL_Renderer* renderer = m_manager.getRenderer();

    if (canShareRenderCache() && m_rotation == 0.0) {
        const uint64_t key = buildRenderCacheKey();
        if (key != m_cacheKey || !m_cachedTexture) {
            m_cachedTexture = m_manager.getTextureManager().renderCache(
                key, m_width, m_height, [this](SDL_Renderer* r) { draw(r); });
            m_cacheKey = key;
        }
        m_isDirty = false;
        return;
    }

    m_cacheKey = 0;
    SharedTexture tex(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, m_width, m_height),
                      SDLTextureDeleter());
    if (!tex) {
        m_isDirty = false;
        return;
    }
    SDL_SetTextureBlendMode(tex.get(), SDL_BLENDMODE_BLEND);

    {
        ScopedRenderTarget targetScope(renderer, tex.get());

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        draw(renderer);

        if (m_rotation != 0.0) {
            if (hasKeyboardFocus()) {
                const Style& focusStyle = getComposedStyle(m_state);
                drawRoundedRectBorder(renderer, SDLRectToFRect(0, 0, m_width, m_height),
                                      static_cast<float>(focusStyle.borderRadius.value_or(0)),
                                      ColorToFColor(constants::kFocusOutlineColor), 1.0f);
            }
            SDL_SetRenderTarget(renderer, nullptr);
            for (auto& child : m_children) {
                if (child->isVisible() && child->m_isDirty) {
                    child->renderToCache();
                }
            }

            SDL_SetRenderTarget(renderer, tex.get());
            for (auto& child : m_children) {
                if (child->isVisible() && child->m_cachedTexture) {
                    SDL_Rect childDst = {child->m_x, child->m_y, child->m_width, child->m_height};
                    RenderTexture(renderer, child->m_cachedTexture.get(), childDst);
                }
            }
        }
    }

    m_cachedTexture = std::move(tex);
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
        child->markDirtyRecursively();
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
        child->cleanup();
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
    return m_manager.getTimerManager()->addTimer(this, delay, singleShot, std::move(callback));
}

void GUIElement::stopTimer(uint32_t timerId) {
    m_manager.getTimerManager()->removeTimer(timerId);
}

// --- New styling API implementation ---

const char* GUIElement::getComponentType() const {
    return "GUIElement";
}

void GUIElement::setState(ElementState newState) {
    if (m_state == newState) {
        return;
    }

    const auto oldState = m_state;
    m_state = newState;

    if (getComposedStyle(oldState) != getComposedStyle(m_state)) {
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

void applyBevelToStyle(Style& style, BevelType type) {
    if (type == BevelType::Raised) {
        style.borderColorOuterTopLeft     = constants::kWin95Highlight;
        style.borderColorOuterBottomRight = constants::kWin95DarkShadow;
        style.borderColorInnerTopLeft     = constants::kWin95Light;
        style.borderColorInnerBottomRight = constants::kWin95Shadow;
    } else {
        style.borderColorOuterTopLeft     = constants::kWin95Shadow;
        style.borderColorOuterBottomRight = constants::kWin95Highlight;
        style.borderColorInnerTopLeft     = constants::kWin95DarkShadow;
        style.borderColorInnerBottomRight = constants::kWin95Light;
    }
}

void GUIElement::setBevel(ElementState state, BevelType type) {
    size_t idx = stateIndex(state);
    if (!m_localStyles[idx].has_value()) {
        m_localStyles[idx] = Style();
    }
    applyBevelToStyle(*m_localStyles[idx], type);
    markDirty();
}

void drawBevelFrame(SDL_Renderer* renderer, SDL_Rect rect, int thickness, SDL_Color topLeftColor, SDL_Color bottomRightColor) {
    SetDrawColor(renderer, topLeftColor);
    RenderFillRect(renderer, SDL_Rect{rect.x, rect.y, rect.w, thickness});
    RenderFillRect(renderer, SDL_Rect{rect.x, rect.y + thickness, thickness, rect.h - 2 * thickness});
    SetDrawColor(renderer, bottomRightColor);
    RenderFillRect(renderer, SDL_Rect{rect.x + rect.w - thickness, rect.y, thickness, rect.h});
    RenderFillRect(renderer, SDL_Rect{rect.x, rect.y + rect.h - thickness, rect.w, thickness});
}

void drawStyleBevel(SDL_Renderer* renderer, SDL_Rect rect, const Style& style) {
    constexpr int kBevelThickness = 1;
    if (style.borderColorOuterTopLeft && style.borderColorOuterBottomRight && rect.w >= 2 * kBevelThickness &&
        rect.h >= 2 * kBevelThickness) {
        drawBevelFrame(renderer, rect, kBevelThickness, *style.borderColorOuterTopLeft, *style.borderColorOuterBottomRight);
    }

    SDL_Rect inner{rect.x + kBevelThickness, rect.y + kBevelThickness, rect.w - 2 * kBevelThickness,
                   rect.h - 2 * kBevelThickness};
    if (style.borderColorInnerTopLeft && style.borderColorInnerBottomRight && inner.w >= 2 * kBevelThickness &&
        inner.h >= 2 * kBevelThickness) {
        drawBevelFrame(renderer, inner, kBevelThickness, *style.borderColorInnerTopLeft, *style.borderColorInnerBottomRight);
    }
}

void GUIElement::drawBackgroundAndBorder(SDL_Renderer* renderer) {
    const Style& style = getComposedStyle(m_state);
    int radius = style.borderRadius.value_or(0);
    SDL_FRect frect = SDLRectToFRect(0, 0, m_width, m_height);
    float fradius = static_cast<float>(radius);

    if (style.backgroundColor) {
        drawRoundedFilledRect(renderer, frect, fradius, ColorToFColor(*style.backgroundColor));
    }

    if (style.texture.has_value()) {
        SDL_FRect texRect = SDLRectToFRect(0, 0, m_width, m_height);
        drawRoundedTexturedRect(renderer, texRect, fradius, style.texture.value().get());
    }

    // The 3D bevel takes priority over a plain border; it is drawn sharp — Win95 has no rounded corners.
    if (style.borderColorOuterTopLeft || style.borderColorOuterBottomRight || style.borderColorInnerTopLeft ||
        style.borderColorInnerBottomRight) {
        drawStyleBevel(renderer, SDL_Rect{0, 0, m_width, m_height}, style);
    } else if (style.borderColor && style.borderWidth && *style.borderWidth > 0) {
        drawRoundedRectBorder(renderer, frect, fradius, ColorToFColor(*style.borderColor), static_cast<float>(*style.borderWidth));
    }
}

uint64_t GUIElement::buildRenderCacheKey() const {
    uint64_t h = 1469598103934665603ULL;
    auto mix = [&h](uint64_t v) {
        h ^= v;
        h *= 1099511628211ULL;
    };
    auto mixColor = [&mix](const std::optional<SDL_Color>& c) {
        mix(c.has_value() ? 1ULL : 0ULL);
        if (c) {
            const SDL_Color& v = *c;
            mix((static_cast<uint64_t>(v.r) << 24) | (static_cast<uint64_t>(v.g) << 16) |
                (static_cast<uint64_t>(v.b) << 8) | static_cast<uint64_t>(v.a));
        }
    };

    mix(std::hash<std::string_view>{}(getComponentType()));
    mix(static_cast<uint64_t>(m_width));
    mix(static_cast<uint64_t>(m_height));
    mix(static_cast<uint64_t>(m_state));

    const Style& st = getComposedStyle(m_state);
    mixColor(st.backgroundColor);
    mixColor(st.textColor);
    mix(st.texture.has_value() ? 1ULL : 0ULL);
    if (st.texture) {
        mix(reinterpret_cast<uint64_t>(st.texture->get()));
    }
    mixColor(st.borderColor);
    mix(st.borderWidth.has_value() ? 1ULL : 0ULL);
    if (st.borderWidth) {
        mix(static_cast<uint64_t>(*st.borderWidth));
    }
    mixColor(st.borderColorOuterTopLeft);
    mixColor(st.borderColorOuterBottomRight);
    mixColor(st.borderColorInnerTopLeft);
    mixColor(st.borderColorInnerBottomRight);
    mix(st.borderRadius.has_value() ? 1ULL : 0ULL);
    if (st.borderRadius) {
        mix(static_cast<uint64_t>(*st.borderRadius));
    }
    mix(st.fontSize.has_value() ? 1ULL : 0ULL);
    if (st.fontSize) {
        mix(static_cast<uint64_t>(*st.fontSize));
    }
    mix(st.fontName.has_value() ? 1ULL : 0ULL);
    if (st.fontName) {
        mix(std::hash<std::string_view>{}(*st.fontName));
    }

    mix(getRenderCacheKeySuffix());
    return h;
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

void GUIElement::onFocusGained() {
    markDirty();
}

void GUIElement::onFocusLost() {
    markDirty();
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
        // Fixed pixels (>1.0; exact 1.0 is already handled as 100% percentage)
        return static_cast<int>(value);
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
        child->updateLayout(m_width, m_height);
    }
}

void GUIElement::onParentResize(int parentWidth, int parentHeight) {
    updateLayout(parentWidth, parentHeight);
}
