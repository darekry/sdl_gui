#pragma once
#include "button.hpp"
#include "gui.hpp"
#include "panel.hpp"


class TabControl : public Panel {
public:
    TabControl(GUIManager & manager, int x, int y, int width, int height, int tabButtonHeight = 30);

    // Dodaje nową zakładkę i zwraca wskaźnik do jej panelu zawartości
    Panel* addTab(std::string_view title,  int width = 100, int height =-1);

    // Ustawia aktywną zakładkę na podstawie wskaźnika na jej przycisk
    void setActiveTab(Button* tabButton);

    // Ustawia aktywną zakładkę po indeksie (0-based)
    void setActiveTab(int index);

    const char* getComponentType() const override;

protected:

private:
    void reorderTabs();  // Prywatna metoda do aktualizacji pozycji przycisków

    std::vector<Button*> m_tabButtons;
    std::vector<Panel*> m_tabPanels;
    Button* m_activeTabButton = nullptr;
    
    int m_tabButtonHeight = 30;
};
