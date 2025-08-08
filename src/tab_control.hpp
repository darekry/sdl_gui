#ifndef TAB_CONTROL_HPP
#define TAB_CONTROL_HPP

#include "button.hpp"
#include "gui.hpp"
#include "panel.hpp"


class TabControl : public Panel {
public:
    TabControl(GUIManager& manager, int x, int y, int w, int h, int tabButtonHeight = 30);

    // Dodaje nową zakładkę i zwraca wskaźnik do jej panelu zawartości
    Panel* addTab(std::string_view title);

    // Ustawia aktywną zakładkę na podstawie wskaźnika na jej przycisk
    void setActiveTab(Button* tabButton);

    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

private:
    void reorderTabs(); // Prywatna metoda do aktualizacji pozycji przycisków

    std::vector<Button*> m_tabButtons;
    std::vector<Panel*> m_tabPanels;
    Button* m_activeTabButton = nullptr;
    
    int m_tabButtonHeight = 30;
};

#endif // TAB_CONTROL_HPP