#include "combobox.hpp"
#include "gui.hpp" 
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "std.hpp"
#include "constants.hpp"
ComboBox::ComboBox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h),
      m_is_expanded(false),
      m_selected_index(-1),
      m_dropdown_panel(nullptr),
      m_needs_update(true) {
    setClipChildren(false);
    markDirty();
}

ComboBox::~ComboBox() {
    if (m_is_expanded && m_dropdown_panel) {
        if (m_manager.isElementAlive(m_dropdown_panel)) {
            m_dropdown_panel->setVisible(false);
            m_dropdown_panel->markForDeletion();
        }
        m_dropdown_panel = nullptr;
    }
}

bool ComboBox::handleEvent(const SDL_Event& event) {
    if (!m_enabled || !m_visible) return false;

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (contains(event.button.x, event.button.y)) {
            toggleDropdown();
            return true;
        }
    }

    if (m_is_expanded && event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        const auto p = SDL_Point{static_cast<int>(event.button.x), static_cast<int>(event.button.y)};
        auto abs_pos = getAbsolutePosition();
        int dropdownHeight = m_dropdown_panel ? m_dropdown_panel->getHeight() : 0;
        auto combined_area = SDL_Rect{abs_pos.x, abs_pos.y, m_width, m_height + dropdownHeight};
        if (!SDL_PointInRect(&p, &combined_area)) {
            toggleDropdown();
            return true;
        }
    }
    
    return GUIElement::handleEvent(event);
}

void ComboBox::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    auto style = getComposedStyle(m_state);

    // 2. Draw selected text
    if (m_selected_index != -1 && !m_options.empty()) {
        if (style.textColor) {
            int font_size = style.fontSize.value_or(16);
            auto& fontManager = m_manager.getFontManager();
            auto font = fontManager.loadFont(style.fontName.value_or(constants::kDefaultFontPath), font_size);
            if (font) {
                auto& textureManager = m_manager.getTextureManager();
                SharedTexture textTexture = textureManager.createTextureFromText(m_options[static_cast<size_t>(m_selected_index)], font, *style.textColor);
                if (textTexture) {
                    int text_w = TextureWidth(textTexture.get()); int text_h = TextureHeight(textTexture.get());
                    SDL_Rect dstRect = { 5, (m_height - text_h) / 2, text_w, text_h };
                    RenderTexture(renderer, textTexture.get(), dstRect);
                }
            }
        }
    }
    SDL_Color arrow_color = style.textColor.value_or(SDL_Color{0, 0, 0, 255});
    SetDrawColor(renderer, arrow_color);
    
    const int arrow_size = 6;
    const int arrow_margin = 10;
    
    SDL_FPoint points[4];
    float center_y = static_cast<float>(m_height) / 2.0f;
    float aw = static_cast<float>(arrow_size);
    float am = static_cast<float>(arrow_margin);
    float fw = static_cast<float>(m_width);

    if (m_is_expanded) {
        points[0] = {fw - am - aw, center_y + aw / 2.0f};
        points[1] = {fw - am, center_y - aw / 2.0f};
        points[2] = {fw - am - aw, center_y - aw / 2.0f};
        points[3] = points[0]; 
    } else {
        points[0] = {fw - am - aw, center_y - aw / 2.0f};
        points[1] = {fw - am, center_y + aw / 2.0f};
        points[2] = {fw - am - aw / 2.0f, center_y + aw / 2.0f};
        points[3] = points[0];
    }
    SDL_RenderLines(renderer, points, 3);

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

void ComboBox::clearItems() {
    m_options.clear();
    m_selected_index = -1;
    if (m_is_expanded) {
        toggleDropdown();
    }
    m_needs_update = true;
    markDirty();
}

const char* ComboBox::getComponentType() const {
    return "ComboBox";
}

size_t ComboBox::getItemCount() const {
    return m_options.size();
}

std::string ComboBox::getItem(size_t index) const {
    if (index < m_options.size()) {
        return m_options[index];
    }
    return "";
}

std::string ComboBox::getSelectedItem() const {
    if (m_selected_index >= 0 && static_cast<size_t>(m_selected_index) < m_options.size()) {
        return m_options[static_cast<size_t>(m_selected_index)];
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
                on_selection_changed(m_selected_index, m_options[static_cast<size_t>(m_selected_index)]);
            }
            markDirty();
        }
    }
}

void ComboBox::toggleDropdown() {
    m_is_expanded = !m_is_expanded;
    if (m_is_expanded) {
        auto abs = getAbsolutePosition();
        auto dropdown = std::make_unique<Panel>(m_manager, abs.x, abs.y + m_height, m_width, 100);
        dropdown->setVisible(true);
        m_dropdown_panel = dropdown.get();
        m_manager.addElement(std::move(dropdown));
        m_needs_update = true;
    } else {
        if (m_dropdown_panel) {
            m_dropdown_panel->setVisible(false);
            m_dropdown_panel->markForDeletion();
            m_dropdown_panel = nullptr;
        }
    }
    markDirty();
}

void ComboBox::selectItem(int index) {
    setSelectedIndex(index);
    toggleDropdown();
}

void ComboBox::createDropdownButtons() {
    if (!m_dropdown_panel) return;

    m_dropdown_panel->clearChildren();

    const int item_h = 30;
    m_dropdown_panel->setSize(getWidth(), item_h * static_cast<int>(m_options.size()));

    for (size_t i = 0; i < m_options.size(); ++i) {
        auto option_button = std::make_unique<Button>(m_manager, 0, i * item_h, getWidth(), item_h, m_options[i]);
        option_button->setVisible(true);
        option_button->setOnClickCallback([this, i](GUIElement*){
            selectItem(static_cast<int>(i));
        });
        m_dropdown_panel->addChild(std::move(option_button));
    }
}