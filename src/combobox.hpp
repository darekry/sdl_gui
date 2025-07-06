#ifndef COMBOBOX_HPP
#define COMBOBOX_HPP

#include "button.hpp"
#include "panel.hpp"
#include "gui.hpp"
import std.compat;

class ComboBox : public GUIElement {
public:
    ComboBox(GUIManager& manager, int x, int y, int w, int h);

    bool handleEvent(SDL_Event& event) override;
    void render() override;

    void addItem(const std::string& item);
    std::string getSelectedItem() const;
    int getSelectedIndex() const;
    void setSelectedIndex(int index);

bool isExpanded() const { return m_is_expanded; }
    std::function<void(int, const std::string&)> on_selection_changed;

private:
    void toggleDropdown();
    void selectItem(int index);
    void updateMainButtonText();
    void createDropdownButtons();

    bool m_is_expanded;
    std::vector<std::string> m_options;
    int m_selected_index;

    Button* m_main_button;
    Panel* m_dropdown_panel;
    bool m_needs_update;
};

#endif // COMBOBOX_HPP