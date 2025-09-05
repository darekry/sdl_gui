#include "label.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "SDL2/SDL.h"


// Implementacja klasy GUIElement
GUIElement::GUIElement(GUIManager& manager, int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_manager(manager), m_parent(nullptr) {
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
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        bool currentlyHovered = contains(mouseX, mouseY);

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

    if (m_isHovered) {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            setState(ElementState::Pressed);
        } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            if (m_state == ElementState::Pressed) {
                setState(ElementState::Hover);
            }
        } else if (m_state != ElementState::Pressed) {
            setState(ElementState::Hover);
        }
    } else {
        // Only change to Normal if not currently pressed (to handle drag scenarios)
        if (m_state != ElementState::Pressed) {
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

void GUIElement::renderToCache() {
    if (!m_visible || m_width <= 0 || m_height <= 0) {
        if (m_cachedTexture) m_cachedTexture.reset();
        return;
    }

    int tex_w = 0, tex_h = 0;
    if (m_cachedTexture) {
        SDL_QueryTexture(m_cachedTexture.get(), nullptr, nullptr, &tex_w, &tex_h);
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

    LOG_DEBUG("setState for %s from %d to %d", getComponentType(), (int)m_state, (int)newState);

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
    m_localStyles[state] = std::move(style);
    markDirty();
}
Style GUIElement::getComposedStyle(ElementState state) const {
    // 1. Pobierz styl bazowy dla tego typu widgetu z motywu.
    //    Ten styl jest już połączony ze stylem domyślnym motywu.
    Style finalStyle = m_manager.getTheme().getStyle(getComponentType());

    // 2. Znajdź styl lokalny dla konkretnego stanu i połącz go.
    auto it = m_localStyles.find(state);
    if (it != m_localStyles.end()) {
        // Styl lokalny ma pierwszeństwo, więc najpierw go kopiujemy,
        // a potem uzupełniamy brakujące pola ze stylu z motywu.
        Style localCopy = it->second;
        localCopy.mergeWith(finalStyle);
        return localCopy;
    }
    
    // 3. Jeśli nie ma stylu lokalnego dla tego stanu, spróbuj stanu Normal.
    it = m_localStyles.find(ElementState::Normal);
    if (it != m_localStyles.end()) {
        Style localCopy = it->second;
        localCopy.mergeWith(finalStyle);
        return localCopy;
    }

    // 4. Jeśli brak jakiegokolwiek stylu lokalnego, zwróć styl z motywu.
    return finalStyle;
}

void GUIElement::setBackgroundColor(ElementState state, SDL_Color color) {
    m_localStyles[state].backgroundColor = color;
    markDirty();
}

void GUIElement::setTextColor(ElementState state, SDL_Color color) {
    m_localStyles[state].textColor = color;
    markDirty();
}

void GUIElement::setTexture(ElementState state, SharedTexture texture) {
    m_localStyles[state].texture = std::move(texture);
    markDirty();
}
void GUIElement::setBorder(ElementState state, SDL_Color color, int width) {
    m_localStyles[state].borderColor = color;
    m_localStyles[state].borderWidth = width;
    markDirty();
}

void GUIElement::drawBackgroundAndBorder(SDL_Renderer* renderer) {
    const Style& style = getComposedStyle(m_state);

    // Rysowanie tła
    if (style.backgroundColor) {
        SDL_SetRenderDrawColor(renderer, style.backgroundColor->r, style.backgroundColor->g, style.backgroundColor->b, style.backgroundColor->a);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        SDL_RenderFillRect(renderer, &bgRect);
    }

    // Rysowanie obramowania
    if (style.borderColor && style.borderWidth && *style.borderWidth > 0) {
        SDL_SetRenderDrawColor(renderer, style.borderColor->r, style.borderColor->g, style.borderColor->b, style.borderColor->a);
        SDL_Rect borderRect = {0, 0, m_width, m_height};
        for (int i = 0; i < *style.borderWidth; ++i) {
            SDL_RenderDrawRect(renderer, &borderRect);
            borderRect.x++;
            borderRect.y++;
            borderRect.w -= 2;
            borderRect.h -= 2;
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
