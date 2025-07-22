#include "combobox.hpp"
#include "gui.hpp" // Dla Button i Panel
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include <SDL2/SDL.h>

import std.compat;

ComboBox::ComboBox(GUIManager& manager, int x, int y, int w, int h)
    : GUIElement(manager, x, y, w, h),
      m_is_expanded(false),
      m_selected_index(-1),
      m_needs_update(true) {

    auto main_button = std::make_unique<Button>(manager, 0, 0, w, h);
    main_button->setOnClickCallback([this](GUIElement*){
        toggleDropdown();
    });
    m_main_button = main_button.get();
    
    // Poprawka: Inicjalizuj panel z poprawną pozycją Y, ale wysokość zostanie ustawiona później
    auto dropdown_panel = std::make_unique<Panel>(manager, 0, h, w, 100);
    dropdown_panel->setVisible(false);
    m_dropdown_panel = dropdown_panel.get();

    // Kolejność dodawania ma znaczenie dla renderowania i obsługi zdarzeń
    addChild(std::move(main_button));
    addChild(std::move(dropdown_panel));
}

bool ComboBox::handleEvent(SDL_Event& event) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    // Pozwól klasie bazowej najpierw obsłużyć zdarzenie dla dzieci.
    if (GUIElement::handleEvent(event)) {
        return true;
    }

    // Jeśli kliknięto poza obszarem całego comboboxa (wraz z panelem)
    // a lista jest rozwinięta, zamknij ją.
    if (m_is_expanded && event.type == SDL_MOUSEBUTTONDOWN) {
        SDL_Point p = { event.button.x, event.button.y };
        SDL_Point abs_pos = getAbsolutePosition();
        SDL_Rect combobox_area = { abs_pos.x, abs_pos.y, m_width, m_height + m_dropdown_panel->getHeight() };
        
        if (!SDL_PointInRect(&p, &combobox_area)) {
            toggleDropdown();
            return true; // "Zużyj" kliknięcie
        }
    }

    return false;
return false;
}
void ComboBox::draw() {
    if (m_needs_update) {
        createDropdownButtons();
        updateMainButtonText();
        m_needs_update = false;
    }
    // Rysowanie jest obsługiwane przez dzieci (Button, Panel), które są
    // renderowane przez pętlę w GUIElement::render(). Ta metoda jest pusta.
}
void ComboBox::addItem(const std::string& item) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::addItem: Adding item \"%s\"", item.c_str());
    m_options.push_back(item);
    if (m_selected_index == -1) {
        m_selected_index = 0;
    }
    m_needs_update = true;
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
            updateMainButtonText();
            if (on_selection_changed) {
                on_selection_changed(m_selected_index, m_options[m_selected_index]);
            }
        }
    }
}

void ComboBox::toggleDropdown() {
    m_is_expanded = !m_is_expanded;
    m_dropdown_panel->setVisible(m_is_expanded);
}

void ComboBox::selectItem(int index) {
    setSelectedIndex(index);
    toggleDropdown(); // Zamknij po wybraniu
}
void ComboBox::updateMainButtonText() {
    if (m_selected_index != -1) {
        m_main_button->setLabel(m_options[m_selected_index], 16, {0,0,0,255});
    } else {
        m_main_button->setTexture(nullptr);
    }
}
void ComboBox::createDropdownButtons() {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::createDropdownButtons: Creating dropdown buttons.");

    m_dropdown_panel->clearChildren();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::createDropdownButtons: Dropdown panel cleared.");

    const int item_h = 30;
    m_dropdown_panel->setSize(getWidth(), item_h * m_options.size());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::createDropdownButtons: Dropdown panel resized to %dx%zu.", getWidth(), item_h * m_options.size());

    for (size_t i = 0; i < m_options.size(); ++i) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::createDropdownButtons: Creating button for item \"%s\"", m_options[i].c_str());
        auto option_button = std::make_unique<Button>(m_manager, 0, i * item_h, getWidth(), item_h);
        option_button->setLabel(m_options[i], 16, {0, 0, 0, 255});
        option_button->setVisible(true);
        option_button->setOnClickCallback([this, i](GUIElement*){
            selectItem(i);
        });
        m_dropdown_panel->addChild(std::move(option_button));
    }
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "ComboBox::createDropdownButtons: Finished creating dropdown buttons.");
}