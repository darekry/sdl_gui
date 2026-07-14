#pragma once
#include "button.hpp"
#include "panel.hpp"
#include "gui.hpp"
#include "label.hpp" // Dodano


class ComboBox : public GUIElement {
public:
    ComboBox(GUIManager& manager, int x, int y, int w, int h);
    ~ComboBox() override;

    bool handleEvent(const SDL_Event& event) override;

    void addItem(std::string_view item);
    void addItem(std::string&& item);
    void addItem(const char* item);
    void clearItems();
    [[nodiscard]] size_t getItemCount() const;
    [[nodiscard]] std::string getItem(size_t index) const;
    std::string getSelectedItem() const;
    int getSelectedIndex() const;
    void setSelectedIndex(int index);

    bool isExpanded() const { return m_is_expanded; }
    std::function<void(int, const std::string&)> on_selection_changed;

    const char* getComponentType() const override;

protected:
    void draw(SDL_Renderer* renderer) override;

private:
    void toggleDropdown();
    void selectItem(int index);
    void createDropdownButtons();

    bool m_is_expanded;
    std::vector<std::string> m_options;
    int m_selected_index;

    //Button* m_main_button;
    Panel* m_dropdown_panel;
    bool m_needs_update;
};
