#include "combobox.hpp"
#include "gui.hpp" 
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

ComboBox::ComboBox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h),
      m_is_expanded(false),
      m_selected_index(-1),
      m_needs_update(true) {

    auto dropdown_panel = std::make_unique<Panel>(manager, 0, h, w, 100);
    dropdown_panel->setVisible(false);
    m_dropdown_panel = dropdown_panel.get();

    addChild(std::move(dropdown_panel));

    setClipChildren(false);
    markDirty();
}

bool ComboBox::handleEvent(const SDL_Event& event) {
    if (!m_enabled || !m_visible) return false;

    if (m_is_expanded && m_dropdown_panel->handleEvent(event)) {
        return true;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (contains(event.button.x, event.button.y)) {
            toggleDropdown();
            return true;
        }
    }

    if (m_is_expanded && event.type == SDL_MOUSEBUTTONDOWN) {
        const auto p = SDL_Point{event.button.x, event.button.y};
        auto abs_pos = getAbsolutePosition();
        auto combined_area = SDL_Rect{abs_pos.x, abs_pos.y, m_width, m_height + m_dropdown_panel->getHeight()};
        if (!SDL_PointInRect(&p, &combined_area)) {
            toggleDropdown();
            return true; 
        }
    }
    
    return GUIElement::handleEvent(event);
}

void ComboBox::draw(SDL_Renderer* renderer) {
    auto style = getResolvedStyle();

    if (style.backgroundColor.has_value()) {
        SDL_SetRenderDrawColor(renderer, style.backgroundColor->r, style.backgroundColor->g, style.backgroundColor->b, style.backgroundColor->a);
        SDL_Rect bgRect = {0, 0, m_width, m_height};
        SDL_RenderFillRect(renderer, &bgRect);
    }
    if (style.borderColor.has_value()) {
        SDL_SetRenderDrawColor(renderer, style.borderColor->r, style.borderColor->g, style.borderColor->b, style.borderColor->a);
        SDL_Rect borderRect = {0, 0, m_width, m_height};
        SDL_RenderDrawRect(renderer, &borderRect);
    }

    // 2. Draw selected text
    if (m_selected_index != -1 && !m_options.empty()) {
        if (style.textColor) {
            int font_size = style.fontSize.value_or(16);
            auto& fontManager = m_manager.getFontManager();
            auto font = fontManager.loadFont(style.fontName.value_or("assets/fonts/font.ttf"), font_size);
            if (font) {
                auto& textureManager = m_manager.getTextureManager();
                SharedTexture textTexture = textureManager.createTextureFromText(m_options[m_selected_index], font, *style.textColor);
                if (textTexture) {
                    int text_w, text_h;
                    SDL_QueryTexture(textTexture.get(), nullptr, nullptr, &text_w, &text_h);
                    SDL_Rect dstRect = { 5, (m_height - text_h) / 2, text_w, text_h };
                    SDL_RenderCopy(renderer, textTexture.get(), nullptr, &dstRect);
                }
            }
        }
    }
    SDL_Color arrow_color = style.textColor.value_or(SDL_Color{0, 0, 0, 255});
    SDL_SetRenderDrawColor(renderer, arrow_color.r, arrow_color.g, arrow_color.b, arrow_color.a);
    
    const int arrow_size = 6;
    const int arrow_margin = 10;
    
    SDL_Point points[4];
    int center_y = m_height / 2;

    if (m_is_expanded) {
        points[0] = {m_width - arrow_margin - arrow_size, center_y + arrow_size / 2};
        points[1] = {m_width - arrow_margin, center_y - arrow_size / 2};
        points[2] = {m_width - arrow_margin - arrow_size, center_y - arrow_size / 2};
        points[3] = points[0]; 
    } else {
        points[0] = {m_width - arrow_margin - arrow_size, center_y - arrow_size / 2};
        points[1] = {m_width - arrow_margin, center_y + arrow_size / 2};
        points[2] = {m_width - arrow_margin - arrow_size / 2, center_y + arrow_size / 2};
        points[3] = points[0];
    }
    SDL_RenderDrawLines(renderer, points, 3);

    if (m_is_expanded && m_needs_update) {
        createDropdownButtons();
        m_needs_update = false;
    }
}

void ComboBox::addItem(std::string_view item) {
    m_options.emplace_back(item);
    if (m_selected_index == -1) {
        setSelectedIndex(0);
    }
    m_needs_update = true;
    markDirty();
}

void ComboBox::addItem(std::string&& item) {
    m_options.push_back(std::move(item));
    if (m_selected_index == -1) {
       setSelectedIndex(0);
    }
    m_needs_update = true;
    markDirty();
}

void ComboBox::addItem(const char* item) {
    addItem(std::string(item));
}

std::string ComboBox::getSelectedItem() const {
    if (m_selected_index >= 0 && static_cast<size_t>(m_selected_index) < m_options.size()) {
        return m_options[m_selected_index];
    }
    return "";
}

int ComboBox::getSelectedIndex() const {
    return m_selected_index;
}

void ComboBox::setSelectedIndex(int index) {
    if (index >= 0 && static_cast<size_t>(index) < m_options.size()) {
        if (m_selected_index != index) {
            m_selected_index = index;
             if (on_selection_changed) {
                on_selection_changed(m_selected_index, m_options[m_selected_index]);
            }
            markDirty();
        }
    }
}

void ComboBox::toggleDropdown() {
    m_is_expanded = !m_is_expanded;
    m_dropdown_panel->setVisible(m_is_expanded);
     if (m_is_expanded) {
        m_needs_update = true;
    }
    markDirty();
}

void ComboBox::selectItem(int index) {
    setSelectedIndex(index);
    toggleDropdown();
}

void ComboBox::updateMainButtonText() {
}

void ComboBox::createDropdownButtons() {
    m_dropdown_panel->clearChildren();

    const int item_h = 30;
    m_dropdown_panel->setSize(getWidth(), item_h * m_options.size());

    for (size_t i = 0; i < m_options.size(); ++i) {
        auto option_button = std::make_unique<Button>(m_manager, 0, i * item_h, getWidth(), item_h, m_options[i]);
       // tu trzeba dodajć nowe child, na razie zakomentowane i zostawione puste
       //  option_button->setLabel(m_options[i], 16);
        option_button->setVisible(true);
        option_button->setOnClickCallback([this, i](GUIElement*){
            selectItem(i);
        });
        m_dropdown_panel->addChild(std::move(option_button));
    }
}