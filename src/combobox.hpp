#ifndef COMBOBOX_HPP
#define COMBOBOX_HPP

#include "gui.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

class ComboBox : public GUIElement {
public:
    ComboBox(int x, int y, int w, int h);

    bool handleEvent(SDL_Event& event) override;
    void render(SDL_Renderer* renderer) override;

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
};

#endif // COMBOBOX_HPP