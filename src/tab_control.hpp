#ifndef TAB_CONTROL_HPP
#define TAB_CONTROL_HPP

#include "button.hpp"
#include "gui.hpp"
#include "panel.hpp"
#include <vector>
#include <string>
#include <memory>

class TabControl : public GUIElement {
public:
    TabControl(int x, int y, int width, int height);

    // Dodaje nową zakładkę i zwraca wskaźnik do jej panelu zawartości
    Panel* addTab(const std::string& title);

    // Ustawia aktywną zakładkę na podstawie wskaźnika na jej przycisk
    void setActiveTab(Button* tabButton);

private:
    void reorderTabs(); // Prywatna metoda do aktualizacji pozycji przycisków

    std::vector<Button*> m_tabButtons;
    std::vector<Panel*> m_tabPanels;
    Button* m_activeTabButton = nullptr;
    
    int m_tabButtonHeight = 30;
};

#endif // TAB_CONTROL_HPP