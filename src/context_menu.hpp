#ifndef CONTEXT_MENU_HPP
#define CONTEXT_MENU_HPP

#include "gui.hpp"
#include "panel.hpp"
#include "button.hpp"
#include <functional>
#include <vector>
#include <string>
#include <memory>

struct ContextMenuItem {
    std::string text;
    std::function<void()> action;
    bool enabled = true;
    bool separator = false;

    ContextMenuItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true)
        : text(text), action(action), enabled(enabled), separator(false) {}

    ContextMenuItem(bool separator)
        : text(""), action(nullptr), enabled(false), separator(separator) {}
};

class ContextMenu : public GUIElement {
public:
    ContextMenu(GUIManager& manager);

    // Public methods
    void addItem(std::string_view text, std::function<void()> action = nullptr, bool enabled = true);
    void addSeparator();
    void clearItems();
    void showAt(int x, int y);
    void hide();
    bool isVisible() const { return m_visible; }

    // Override methods
    bool handleEvent(const SDL_Event& event) override;
    const char* getComponentType() const override;

protected:
    void draw([[maybe_unused]] SDL_Renderer* renderer) override;

private:
    // Private methods
    void createMenuButtons();
    void positionMenu(int x, int y);
    void closeMenu();
    bool shouldCloseOnClick(const SDL_Event& event) const;

    // Member variables
    std::vector<ContextMenuItem> m_items;
    Panel* m_panel;
    bool m_needsUpdate = true;
    const int m_itemHeight = 25;
    const int m_separatorHeight = 8;
};

#endif // CONTEXT_MENU_HPP