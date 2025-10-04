#include "context_menu.hpp"
#include "gui_manager.hpp"
#include <algorithm>

ContextMenu::ContextMenu(GUIManager& manager)
    : GUIElement(manager, 0, 0, 200, 100) {

    // Create panel as container
    auto panel = std::make_unique<Panel>(manager, 0, 0, 200, 100);
    panel->setVisible(false);
    m_panel = panel.get();

    addChild(std::move(panel));
    setClipChildren(false);
    setVisible(false);
}

void ContextMenu::addItem(std::string_view text, std::function<void()> action, bool enabled) {
    m_items.emplace_back(text, action, enabled);
    m_needsUpdate = true;
    markDirty();
}

void ContextMenu::addSeparator() {
    m_items.emplace_back(true); // separator = true
    m_needsUpdate = true;
    markDirty();
}

void ContextMenu::clearItems() {
    m_items.clear();
    m_panel->clearChildren();
    m_needsUpdate = true;
    markDirty();
}

void ContextMenu::showAt(int x, int y) {
    positionMenu(x, y);
    setVisible(true);
    m_panel->setVisible(true);

    if (m_needsUpdate) {
        createMenuButtons();
        m_needsUpdate = false;
    }

    markDirty();
}

void ContextMenu::hide() {
    setVisible(false);
    m_panel->setVisible(false);
    markDirty();
}

bool ContextMenu::handleEvent(const SDL_Event& event) {
    if (!m_visible || !m_enabled) return false;

    // First, let the panel handle the event
    if (m_panel->handleEvent(event)) {
        return true;
    }

    // Check for clicks outside the menu area
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (shouldCloseOnClick(event)) {
            closeMenu();
            return true;
        }
    }

    return GUIElement::handleEvent(event);
}

void ContextMenu::draw([[maybe_unused]] SDL_Renderer* renderer) {
    // ContextMenu itself doesn't draw anything directly
    // All rendering is handled by the Panel and its children
    if (m_needsUpdate && m_visible) {
        createMenuButtons();
        m_needsUpdate = false;
    }
}

const char* ContextMenu::getComponentType() const {
    return "ContextMenu";
}

void ContextMenu::createMenuButtons() {
    m_panel->clearChildren();

    int currentY = 0;
    int menuWidth = 200; // Default width

    for (size_t i = static_cast<size_t>(0); i < m_items.size(); ++i) {
        const auto& item = m_items[i];

        if (item.separator) {
            // Create a simple separator (just a visual element)
            auto separator = std::make_unique<Panel>(m_manager, 0, currentY, menuWidth, m_separatorHeight);
            separator->setBackgroundColor(ElementState::Normal, SDL_Color{200, 200, 200, 255});
            m_panel->addChild(std::move(separator));
            currentY += m_separatorHeight;
        } else {
            // Create button for menu item
            auto button = std::make_unique<Button>(m_manager, 0, currentY, menuWidth, m_itemHeight, item.text);
            button->setEnabled(item.enabled);

            if (item.action) {
                button->setOnClickCallback([this, action = item.action](GUIElement*) {
                    action();
                    closeMenu();
                });
            }

            m_panel->addChild(std::move(button));
            currentY += m_itemHeight;
        }
    }

    // Update panel size
    m_panel->setSize(menuWidth, currentY);
    setSize(menuWidth, currentY);
}

void ContextMenu::positionMenu(int x, int y) {
    // Get window dimensions for boundary checking
    int windowWidth = 800;  // Default, could be made configurable
    int windowHeight = 600; // Default, could be made configurable

    int menuWidth = 200;
    int menuHeight = static_cast<int>(m_items.size()) * m_itemHeight;

    // Adjust position to stay within window bounds
    if (x + menuWidth > windowWidth) {
        x = windowWidth - menuWidth;
    }
    if (y + menuHeight > windowHeight) {
        y = windowHeight - menuHeight;
    }

    // Ensure menu stays on screen
    x = std::max(0, x);
    y = std::max(0, y);

    setPosition(x, y);
    m_panel->setPosition(0, 0);
}

void ContextMenu::closeMenu() {
    hide();
}

bool ContextMenu::shouldCloseOnClick(const SDL_Event& event) const {
    if (event.type != SDL_MOUSEBUTTONDOWN) return false;

    const auto p = SDL_Point{event.button.x, event.button.y};
    auto abs_pos = getAbsolutePosition();
    auto menu_area = SDL_Rect{abs_pos.x, abs_pos.y, m_width, m_height};

    return !SDL_PointInRect(&p, &menu_area);
}