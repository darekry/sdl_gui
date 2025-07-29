#include "label.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "SDL2/SDL.h"


// Implementacja klasy GUIElement
GUIElement::GUIElement(GUIManager& manager, int x, int y, int width, int height)
    : m_manager(manager), m_x(x), m_y(y), m_width(width), m_height(height), m_parent(nullptr) {
}

void GUIElement::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

void GUIElement::setSize(int width, int height) {
    m_width = width;
    m_height = height;
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
    }
}

void GUIElement::clearChildren() {
    m_children.clear();
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
        m_currentState = ElementState::Disabled;
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

    return false;
}

void GUIElement::render() {
    if (!m_visible) {
        return;
    }
    auto* renderer = m_manager.getRenderer();
    auto old_clip_rect = SDL_Rect{};
    bool clipping_was_active = false;

    if (m_clip_children) {
        clipping_was_active = true;
        SDL_RenderGetClipRect(renderer, &old_clip_rect);
        auto abs_pos = getAbsolutePosition();
        auto new_clip_rect = SDL_Rect{abs_pos.x, abs_pos.y, m_width, m_height};

        if (old_clip_rect.w != 0 || old_clip_rect.h != 0) {
            SDL_IntersectRect(&old_clip_rect, &new_clip_rect, &new_clip_rect);
        }
        SDL_RenderSetClipRect(renderer, &new_clip_rect);
    }
    const auto& style = getResolvedStyle();
    const auto absPos = getAbsolutePosition();
    const auto renderQuad = SDL_Rect{absPos.x, absPos.y, m_width, m_height};

    // 1. Renderuj tło (kolor)
    if (style.backgroundColor) {
        const auto& c = style.backgroundColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &renderQuad);
    }

    // 2. Renderuj teksturę (jeśli istnieje)
    if (style.texture && style.texture->get()) {
        SDL_RenderCopy(renderer, style.texture->get(), nullptr, &renderQuad);
    }

    // 3. Renderuj ramkę
    if (style.borderWidth.value_or(0) > 0 && style.borderColor) {
        const auto& c = style.borderColor.value();
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        for (int i = 0; i < style.borderWidth.value_or(0); ++i) {
            SDL_Rect borderRect = {
                absPos.x + i,
                absPos.y + i,
                m_width - 2 * i,
                m_height - 2 * i
            };
            SDL_RenderDrawRect(renderer, &borderRect);
        }
    }
    
    // Narysuj zawartość specyficzną dla elementu (np. tekst w Label)
    draw();

    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render();
        }
    }

    if (clipping_was_active) {
        SDL_RenderSetClipRect(renderer, &old_clip_rect);
    }
}
void GUIElement::draw() {
    // Ta metoda jest teraz pusta dla bazowego GUIElement.
    // Klasy pochodne (np. Label) mogą ją nadpisać, aby renderować
    // swoją specyficzną zawartość po narysowaniu tła/ramki w render().
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
    auto new_end = std::remove_if(m_children.begin(), m_children.end(),
                                  [](const std::unique_ptr<GUIElement>& element) {
        return element->isMarkedForDeletion();
    });
    m_children.erase(new_end, m_children.end());
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
}

void GUIElement::setTextColor(ElementState state, SDL_Color color) {
    m_styles[state].textColor = color;
}

void GUIElement::setTexture(ElementState state, SharedTexture texture) {
    m_styles[state].texture = std::move(texture);
}
void GUIElement::setBorder(ElementState state, SDL_Color color, int width) {
    m_styles[state].borderColor = color;
    m_styles[state].borderWidth = width;
}

void GUIElement::setLabel(std::string_view text, int font_size) {
    if (!m_label) {
        m_label = std::make_unique<Label>(m_manager, 0, 0, text, font_size);
        m_label->setParent(this);
    } else {
        static_cast<Label*>(m_label.get())->setText(text);
    }

    if (m_label) {
        int label_width, label_height;
        m_label->getSize(label_width, label_height);
        int label_x = (m_width - label_width) / 2;
        int label_y = (m_height - label_height) / 2;
        m_label->setPosition(label_x, label_y);
    }
}
