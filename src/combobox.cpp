#include "combobox.hpp"
#include "gui.hpp" // Dla Button i Panel
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"

#include <iostream>

ComboBox::ComboBox(int x, int y, int w, int h)
    : GUIElement(x, y, w, h),
      m_is_expanded(false),
      m_selected_index(-1) {

    auto main_button = std::make_unique<Button>(0, 0, w, h);
    main_button->setOnClickCallback([this](GUIElement*){
        toggleDropdown();
    });
    m_main_button = main_button.get();
    addChild(std::move(main_button));

    auto dropdown_panel = std::make_unique<Panel>(0, h, w, 0);
    dropdown_panel->setVisible(false);
    m_dropdown_panel = dropdown_panel.get();
    addChild(std::move(dropdown_panel));
}

bool ComboBox::handleEvent(SDL_Event& event) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    // Zdarzenia są przekazywane do dzieci przez GUIElement::handleEvent
    // Najpierw sprawdzamy panel, jeśli jest rozwinięty, bo jest "wyżej"
    if (m_is_expanded && m_dropdown_panel->handleEvent(event)) {
        return true;
    }

    // Potem sprawdzamy główny przycisk
    if (m_main_button->handleEvent(event)) {
        return true;
    }

    // Obsługa kliknięcia poza komponentem w celu zamknięcia
    if (m_is_expanded && event.type == SDL_MOUSEBUTTONDOWN) {
        if (!contains(event.button.x, event.button.y)) {
            toggleDropdown();
            return true;
        }
    }

    return false;
}

void ComboBox::render(SDL_Renderer* renderer) {
    if (!m_visible) {
        return;
    }
    
    m_main_button->render(renderer);
    if (m_is_expanded) {
        m_dropdown_panel->render(renderer);
    }
}

void ComboBox::addItem(const std::string& item) {
    m_options.push_back(item);
    if (m_selected_index == -1) {
        setSelectedIndex(0);
    }
    createDropdownButtons();
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
        m_selected_index = index;
        updateMainButtonText();
        if (on_selection_changed) {
            on_selection_changed(m_selected_index, m_options[m_selected_index]);
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
    if (m_selected_index != -1 && getGUIManager()) {
        FontManager* fontManager = getGUIManager()->getFontManager();
        SDL_Renderer* renderer = getGUIManager()->getRenderer();

        if (!fontManager || !renderer) return;

        SharedFont font = fontManager->loadFont("assets/fonts/font.ttf", 16);
        if (!font) {
            std::cerr << "Failed to load font in ComboBox" << std::endl;
            return;
        }
        
        SDL_Color textColor = { 0, 0, 0, 255 };
        SharedTexture textTexture = createTextTexture(renderer, font, m_options[m_selected_index], textColor);
        m_main_button->setTexture(textTexture);
    }
}

void ComboBox::createDropdownButtons() {
    if (!getGUIManager()) return;
    FontManager* fontManager = getGUIManager()->getFontManager();
    SDL_Renderer* renderer = getGUIManager()->getRenderer();

    if (!fontManager || !renderer) return;

    m_dropdown_panel->clearChildren();

    int item_h = getHeight();
    m_dropdown_panel->setSize(getWidth(), item_h * m_options.size());

    for (size_t i = 0; i < m_options.size(); ++i) {
        auto option_button = std::make_unique<Button>(0, i * item_h, getWidth(), item_h);
        
        SharedFont font = fontManager->loadFont("assets/fonts/font.ttf", 16);
        if (font) {
            SDL_Color textColor = { 0, 0, 0, 255 };
            SharedTexture textTexture = createTextTexture(renderer, font, m_options[i], textColor);
            option_button->setTexture(textTexture);
        } else {
             std::cerr << "Failed to load font for dropdown button" << std::endl;
        }

        option_button->setOnClickCallback([this, i](GUIElement*){
            selectItem(i);
        });
        m_dropdown_panel->addChild(std::move(option_button));
    }
}