#pragma once
#include "button.hpp"
#include "gui.hpp"
#include "panel.hpp"


class TabControl : public Panel {
public:
    TabControl(GUIManager & manager, int x, int y, int width, int height, int tabButtonHeight = 30);

    // Adds a new tab and returns a pointer to its content panel
    Panel* addTab(std::string_view title,  int width = 100, int height =-1);

    // Sets the active tab based on a pointer to its button
    void setActiveTab(Button* tabButton);

    // Sets the active tab by index (0-based)
    void setActiveTab(int index);

    ComponentType getComponentTypeId() const override;

protected:
    // Własna geometria: pasek zakładek + panele treści na pełny rozmiar.
    void layoutChildren() override;

private:
    void reorderTabs();  // Updates button positions

    std::vector<Button*> m_tabButtons;
    std::vector<Panel*> m_tabPanels;
    Button* m_activeTabButton = nullptr;
    
    int m_tabButtonHeight = 30;
};
