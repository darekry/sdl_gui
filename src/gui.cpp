#include <iostream>
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
        if (m_currentState != ElementState::Disabled) {
            m_currentState = ElementState::Disabled;
            markDirty();
        }
        return false;
    }

    ElementState previousState = m_currentState;

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
            m_currentState = ElementState::Pressed;
        } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            if (m_currentState == ElementState::Pressed) {
                m_currentState = ElementState::Hover;
            }
        } else if (m_currentState != ElementState::Pressed) {
            m_currentState = ElementState::Hover;
        }
    } else {
        m_currentState = ElementState::Normal;
    }

    if (previousState != m_currentState) {
        markDirty();
    }

    return false;
}

void GUIElement::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }

    // Jeśli element wspiera rysowanie bezpośrednie (no-cache), zawsze rysujemy bezpośrednio
    // (nie korzystamy z m_cachedTexture), niezależnie od stanu m_isDirty.
    if (wantsDirectRender()) {
        // Przygotuj clipping tak jak dla standardowej ścieżki
        SDL_Rect old_clip_rect;
        SDL_bool clip_was_enabled = SDL_RenderIsClipEnabled(renderer);
        if (clip_was_enabled) {
            SDL_RenderGetClipRect(renderer, &old_clip_rect);
        }

        if (m_clip_children) {
            auto abs_pos = getAbsolutePosition();
            SDL_Rect element_rect = {abs_pos.x, abs_pos.y, m_width, m_height};
            if (clip_was_enabled) {
                SDL_IntersectRect(&old_clip_rect, &element_rect, &element_rect);
            }
            SDL_RenderSetClipRect(renderer, &element_rect);
        }

        // Rysuj element bezpośrednio na rendererze (zawsze, nawet gdy nie jest "brudny")
        drawDirect(renderer);
        // Po direct render oznaczamy element jako czysty (cache nie jest używany)
        m_isDirty = false;

        // Renderuj dzieci
        for (auto& child : m_children) {
            if (child && child->isVisible()) {
                child->render(renderer);
            }
        }

        // Przywróć clip
        if (clip_was_enabled) {
            SDL_RenderSetClipRect(renderer, &old_clip_rect);
        } else if (m_clip_children) {
            SDL_RenderSetClipRect(renderer, nullptr);
        }
        return;
    }

    // Standardowa ścieżka: z buforowaniem do m_cachedTexture
    if (m_isDirty) {
        renderToCache();
    }

    if (m_cachedTexture) {
        auto absPos = getAbsolutePosition();
        SDL_Rect destRect = { absPos.x, absPos.y, m_width, m_height };
        SDL_RenderCopy(renderer, m_cachedTexture.get(), nullptr, &destRect);
    }

    // Clipping i renderowanie dzieci (jak wcześniej)
    SDL_Rect old_clip_rect;
    SDL_bool clip_was_enabled = SDL_RenderIsClipEnabled(renderer);
    if(clip_was_enabled) {
        SDL_RenderGetClipRect(renderer, &old_clip_rect);
    }
    
    if (m_clip_children) {
        auto abs_pos = getAbsolutePosition();
        SDL_Rect element_rect = {abs_pos.x, abs_pos.y, m_width, m_height};
        if (clip_was_enabled) {
            SDL_IntersectRect(&old_clip_rect, &element_rect, &element_rect);
        }
        SDL_RenderSetClipRect(renderer, &element_rect);
    }

    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render(renderer);
        }
    }

    if (clip_was_enabled) {
        SDL_RenderSetClipRect(renderer, &old_clip_rect);
    } else if (m_clip_children) {
        SDL_RenderSetClipRect(renderer, nullptr);
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
        std::cout << "[DEBUG] GUIElement::cleanup(): Removed " << removed_count << " child elements." << std::endl;
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

void GUIElement::setStyle(ElementState state, Style style) {
    m_styles[state] = std::move(style);
    markDirty();
}

std::optional<Style> GUIElement::getStyle(ElementState state) const {
    auto it = m_styles.find(state);
    if (it != m_styles.end()) {
        return it->second;
    }
    return std::nullopt;
}

Style GUIElement::getResolvedStyle() const {
    const ElementState currentState = m_enabled ? m_currentState : ElementState::Disabled;
    const auto& themeStyle = m_manager.getTheme().getStyle(getComponentType(), currentState);

    if (auto it = m_styles.find(currentState); it != m_styles.end()) {
        return resolveStyle(themeStyle, it->second);
    }
    if (currentState != ElementState::Normal) {
        if (auto it = m_styles.find(ElementState::Normal); it != m_styles.end()) {
            return resolveStyle(themeStyle, it->second);
        }
    }
    
    return themeStyle;
}

Style GUIElement::resolveStyle(const Style& base, const std::optional<Style>& override) const {
    if (!override) {
        return base;
    }
    
    Style resolved = base;
    if (override->backgroundColor) resolved.backgroundColor = override->backgroundColor;
    if (override->textColor) resolved.textColor = override->textColor;
    if (override->texture) resolved.texture = override->texture;
    if (override->borderColor) resolved.borderColor = override->borderColor;
    if (override->borderWidth) resolved.borderWidth = override->borderWidth;
    
    return resolved;
}

void GUIElement::setBackgroundColor(ElementState state, SDL_Color color) {
    m_styles[state].backgroundColor = color;
    markDirty();
}

void GUIElement::setTextColor(ElementState state, SDL_Color color) {
    m_styles[state].textColor = color;
    markDirty();
}

void GUIElement::setTexture(ElementState state, SharedTexture texture) {
    m_styles[state].texture = std::move(texture);
    markDirty();
}
void GUIElement::setBorder(ElementState state, SDL_Color color, int width) {
    m_styles[state].borderColor = color;
    m_styles[state].borderWidth = width;
    markDirty();
}

size_t GUIElement::countDescendants() const {
    size_t count = m_children.size();
    for (const auto& child : m_children) {
        count += child->countDescendants();
    }
    return count;
}
