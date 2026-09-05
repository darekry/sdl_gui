#include "tab_control.hpp"
#include "gui_manager.hpp"




TabControl::TabControl(GUIManager& manager, int x, int y, int width, int height, int tabButtonHeight)
    : Panel(manager, x, y, width, height), m_tabButtonHeight(tabButtonHeight) {
}

#include "label.hpp"

Panel* TabControl::addTab(std::string_view title, int width , int height ) {
    if (height == -1) height = m_tabButtonHeight;
    auto button = std::make_unique<Button>(m_manager, 0, 0, width, height);
    auto label = std::make_unique<Label>(m_manager, 0, 0, title, 16);
    label->setPosition((button->getWidth() - label->getWidth())/2, (button->getHeight() - label->getHeight())/2);
    button->addChild(std::move(label));
    
    auto* buttonPtr = button.get();
    m_tabButtons.push_back(buttonPtr);

    button->setOnClickCallback([this, buttonPtr](GUIElement*){
        setActiveTab(buttonPtr);
    });

    auto panel = std::make_unique<Panel>(m_manager, 0, m_tabButtonHeight, m_width, m_height - m_tabButtonHeight);
    panel->setVisible(false);
    auto* panelPtr = panel.get();
    m_tabPanels.push_back(panelPtr);

    addChild(std::move(button));
    addChild(std::move(panel));

    if (m_tabButtons.size() == 1) {
        setActiveTab(buttonPtr);
    }

    reorderTabs();
    markDirty();
    return panelPtr;
}

void TabControl::setActiveTab(Button* tabButton) {
    if (m_activeTabButton == tabButton) {
        return;
    }

    // Deactivate the previous tab
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            m_tabPanels[i]->setVisible(false);
            break;
        }
    }

    m_activeTabButton = tabButton;

    // Activate the new tab
    for (size_t i = 0; i < m_tabButtons.size(); ++i) {
        if (m_tabButtons[i] == m_activeTabButton) {
            m_tabPanels[i]->setVisible(true);
            break;
        }
    }
    markDirty();
}

void TabControl::setActiveTab(int index) {
    if (index < 0 || static_cast<size_t>(index) >= m_tabButtons.size()) return;
    setActiveTab(m_tabButtons[static_cast<size_t>(index)]);
}

void TabControl::reorderTabs() {
    auto current_x = 0;
    for (auto* button : m_tabButtons) {
        button->setPosition(current_x, 0);
        current_x += button->getWidth() + 5;
    }
}

ComponentType TabControl::getComponentTypeId() const {
    return ComponentType::TabControl;
}