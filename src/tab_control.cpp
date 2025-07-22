#include "tab_control.hpp"
#include "SDL2/SDL_ttf.h"
#include "gui_manager.hpp"
#include "font_manager.hpp"

import std.compat;

// --- Implementacja TabControl ---

TabControl::TabControl(GUIManager& manager, int x, int y, int width, int height, int tabButtonHeight)
    : GUIElement(manager, x, y, width, height), m_tabButtonHeight(tabButtonHeight) {
}

Panel* TabControl::addTab(std::string_view title) {
    auto button = std::make_unique<Button>(m_manager, 0, 0, 100, m_tabButtonHeight);
    SDL_Color textColor = {255, 255, 255, 255};
    button->setLabel(title, 16, textColor);
    
    Button* buttonPtr = button.get();
    m_tabButtons.push_back(buttonPtr);

    button->setOnClickCallback([this, buttonPtr](GUIElement*){
        setActiveTab(buttonPtr);
    });

    auto panel = std::make_unique<Panel>(m_manager, 0, m_tabButtonHeight, m_width, m_height - m_tabButtonHeight);
    panel->setVisible(false);
    Panel* panelPtr = panel.get();
    m_tabPanels.push_back(panelPtr);

    addChild(std::move(button));
    addChild(std::move(panel));

    if (m_tabButtons.size() == 1) {
        setActiveTab(buttonPtr);
    }

    reorderTabs();
    return panelPtr;
}

void TabControl::setActiveTab(Button* tabButton) {
    if (m_activeTabButton == tabButton) {
        return;
    }

    // Deaktywuj poprzednią zakładkę
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            if(m_tabPanels[i]) m_tabPanels[i]->setVisible(false);
            break;
        }
    }

    m_activeTabButton = tabButton;

    // Aktywuj nową zakładkę
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            if(m_tabPanels[i]) m_tabPanels[i]->setVisible(true);
            break;
        }
    }
}

void TabControl::reorderTabs() {
    int current_x = 0;
    for (Button* button : m_tabButtons) {
        button->setPosition(current_x, 0);
        current_x += button->getWidth() + 5;
    }
}

void TabControl::draw() {
    //
}