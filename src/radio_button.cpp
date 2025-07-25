#include "radio_button.hpp"
#include "gui_manager.hpp"
#include "radio_group.hpp"
import std.compat;


namespace {
    // Funkcja pomocnicza do rysowania okręgu
    void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
                }
            }
        }
    }

    void drawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x * x + y * y <= radius * radius) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
    }
}


RadioButton::RadioButton(GUIManager& manager, int x, int y, std::string_view text, int fontSize)
    : GUIElement(manager, x, y, 0, 0) {
    init();

    auto label = std::make_unique<Label>(manager, 0, 0, text, fontSize, SDL_Color{0, 0, 0, 255});
    
    
    int labelW, labelH;
    label->getSize(labelW, labelH); // Poprawione wywołanie
    label->setPosition(m_width + 5, (m_height - labelH) / 2);

    addChild(std::move(label));
}

RadioButton::RadioButton(GUIManager& manager, int x, int y, SharedTexture texture)
    : GUIElement(manager, x, y, 0, 0) {
    init();

    auto label = std::make_unique<Label>(manager, 0, 0, "", 16, SDL_Color{0,0,0,255}); // Pusty label
    label->setTexture(texture);
    

    int labelW, labelH;
    label->getSize(labelW, labelH); // Poprawione wywołanie
    label->setPosition(m_width + 5, (m_height - labelH) / 2);
    
    addChild(std::move(label));
}


void RadioButton::init() {
    auto& tm = m_manager.getTextureManager();
    auto renderer = m_manager.getRenderer();

    static const std::string key_normal = "__radio_default_normal";
    static const std::string key_hover = "__radio_default_hover";
    static const std::string key_selected = "__radio_default_selected";
    static const std::string key_selected_hover = "__radio_default_selected_hover";
    
    if (!tm.hasTexture(key_normal)) {
        constexpr int size = 20;
        constexpr int radius = 9;

        // Normal
        SharedTexture tex_normal(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size), SDL_DestroyTexture);
        SDL_SetRenderTarget(renderer, tex_normal.get());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        drawCircle(renderer, radius, radius, radius - 1);
        tm.addTexture(key_normal, tex_normal);

        // Hover
        SharedTexture tex_hover(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size), SDL_DestroyTexture);
        SDL_SetRenderTarget(renderer, tex_hover.get());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        drawCircle(renderer, radius, radius, radius - 1);
        tm.addTexture(key_hover, tex_hover);

        // Selected
        SharedTexture tex_selected(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size), SDL_DestroyTexture);
        SDL_SetRenderTarget(renderer, tex_selected.get());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        drawCircle(renderer, radius, radius, radius - 1);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        drawFilledCircle(renderer, radius, radius, radius / 2);
        tm.addTexture(key_selected, tex_selected);
        
        // Selected Hover
        SharedTexture tex_selected_hover(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size, size), SDL_DestroyTexture);
        SDL_SetRenderTarget(renderer, tex_selected_hover.get());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        drawCircle(renderer, radius, radius, radius - 1);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        drawFilledCircle(renderer, radius, radius, radius / 2);
        tm.addTexture(key_selected_hover, tex_selected_hover);

        SDL_SetRenderTarget(renderer, nullptr);
    }
    
    m_tex_normal = tm.getTexture(key_normal);
    m_tex_hover = tm.getTexture(key_hover);
    m_tex_selected = tm.getTexture(key_selected);
    m_tex_selected_hover = tm.getTexture(key_selected_hover);

    if (m_tex_normal) {
        int w, h;
        SDL_QueryTexture(m_tex_normal.get(), nullptr, nullptr, &w, &h);
        setSize(w, h);
    }
}

bool RadioButton::isSelected() const {
    return m_isSelected;
}

void RadioButton::setSelected(bool selected) {
    if (m_isSelected != selected) {
        m_isSelected = selected;
        if (m_onChange) {
            m_onChange(this, m_isSelected);
        }
        if (m_isSelected && m_parent) {
             if (auto* group = dynamic_cast<RadioGroup*>(m_parent)) {
                 group->onButtonSelected(this);
            }
        }
    }
}

Label* RadioButton::getLabel() const {
    return m_label;
}

void RadioButton::setOnChange(OnChangeCallback callback) {
    m_onChange = std::move(callback);
}

void RadioButton::setNormalTexture(SharedTexture texture) {
    m_tex_normal = texture;
    if (m_tex_normal) {
        int w, h;
        SDL_QueryTexture(m_tex_normal.get(), nullptr, nullptr, &w, &h);
        setSize(w, h);
    }
}

void RadioButton::setHoverTexture(SharedTexture texture) {
    m_tex_hover = texture;
}

void RadioButton::setSelectedTexture(SharedTexture texture) {
    m_tex_selected = texture;
}

void RadioButton::setSelectedHoverTexture(SharedTexture texture) {
    m_tex_selected_hover = texture;
}

bool RadioButton::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) return false;

    if (GUIElement::handleEvent(e)) {
        return true;
    }
    
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (contains(e.button.x, e.button.y)) {
            if (!m_isSelected) {
                setSelected(true);
            }
            return true;
        }
    }

    return false;
}

void RadioButton::draw() {
    if (!m_visible) return;

    if (m_isSelected) {
        m_texture = m_isHovered ? m_tex_selected_hover : m_tex_selected;
    } else {
        m_texture = m_isHovered ? m_tex_hover : m_tex_normal;
    }

    GUIElement::draw();
}