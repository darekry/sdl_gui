#pragma once

#include "gui.hpp"
#include "menu_item.hpp"
#include "menu_popup.hpp"

// ---------------------------------------------------------------------------
// MenuBar: classic Win32 menu strip (File/Edit/... + dropdowns).
//
// - Owns the menu tree (roots in m_menus). addMenu() returns the root node so
//   menus are built with the chainable MenuItem API.
// - Pre-creates one MenuPopup per node-with-children BEFORE the first event:
//   opening a menu only toggles visibility/positions existing top-level
//   elements, so GUIManager's event propagation vector is never mutated from
//   inside an event callback.
// - Popups are top-level overlays, so they draw above all normal elements.
// - Win32 behavior: click/hover opens, hover-switch between open top-level
//   menus, Alt+mnemonic, F10 menu mode, Left/Right navigation, Esc closes one
//   level (top level closes all), Ctrl+... shortcuts fire while menus are
//   closed, mnemonics render with an underline.
//
// Content placement: the bar itself is a normal top-level element anchored to
// the top (see examples/68_menu_bar.cpp). Put app content in a Panel anchored
// below it — there is no global "content inset" in GUIManager yet.
//
// The first MenuBar added to a manager becomes its global key handler
// (GUIManager::addElement auto-registers); use GUIManager::setMenuBar() for
// explicit control or multiple bars.
// ---------------------------------------------------------------------------
class MenuBar : public GUIElement {
public:
    // height <= 0 -> auto (font line height + 8).
    MenuBar(GUIManager& manager, int x, int y, int width, int height = 0);
    ~MenuBar() override;

    // Adds a top-level menu and returns its root node (empty popup container).
    MenuItem& addMenu(std::string_view title);
    void clearMenus();

    [[nodiscard]] size_t getMenuCount() const { return m_menus.size(); }
    [[nodiscard]] MenuItem* getMenu(size_t index) const;
    [[nodiscard]] SDL_Rect getTitleRect(size_t index) const;

    [[nodiscard]] bool isMenuOpen() const { return !m_popupStack.empty(); }
    [[nodiscard]] bool isMenuMode() const { return m_menuMode; }
    [[nodiscard]] int getOpenMenuIndex() const { return m_activeMenu; }
    [[nodiscard]] size_t getOpenPopupDepth() const { return m_popupStack.size(); }
    [[nodiscard]] MenuPopup* getTopPopup() const;
    [[nodiscard]] MenuPopup* getPopupFor(const MenuItem* owner) const;

    void openMenu(size_t index);
    void closeMenus();

    // Command-id fallback for items without action() (layout files, C API).
    void setCommandHandler(std::function<void(std::string_view)> handler);

    // Called by GUIManager before focused-widget key routing (Alt/F10,
    // Alt+mnemonic, application shortcuts). Returns true when consumed.
    bool handleGlobalKey(const SDL_Event& e);

    bool handleSelf(const SDL_Event& e) override;
    bool wantsFocusOutline() const override { return false; }
    void onFocusLost() override;
    void layoutChildren() override;
    ComponentType getComponentTypeId() const override;

protected:
    void draw(SDL_Renderer* renderer) override;
    bool canShareRenderCache() const override { return false; }

private:
    struct TitleRect {
        int x = 0;
        int width = 0;
    };

    void rebuildPopups();
    void attachMenuHooks(MenuItem& node);
    MenuPopup* ensurePopup(MenuItem* owner);
    void measureTitles();
    void enterMenuMode();
    void leaveMenuMode();
    MenuPopup* createPopup(MenuItem* owner);

    void positionTopPopup(MenuPopup* popup, size_t menuIndex);
    void positionChildPopup(MenuPopup* child, MenuPopup* parent, int itemIndex);
    void openSubmenu(MenuPopup* parent, int itemIndex, bool selectFirst);
    void activateItem(MenuPopup* popup, int index);
    void handlePopupHover(MenuPopup* popup, int index);
    void handlePopupNavigate(MenuPopup* popup, int direction);
    void handlePopupDismiss(MenuPopup* popup);
    void switchMenu(int direction);

    [[nodiscard]] MenuPopup* popupAt(size_t stackIndex) const;
    [[nodiscard]] int indexOfPopup(const MenuPopup* popup) const;
    [[nodiscard]] int titleIndexAt(int globalX, int globalY) const;
    [[nodiscard]] int findMenuByMnemonic(char mnemonic) const;
    void closePopupStack();
    void fireItem(const MenuItem& item);
    bool dispatchShortcut(const SDL_Event& e);

    std::vector<std::unique_ptr<MenuItem>> m_menus;
    std::unordered_map<const MenuItem*, MenuPopup*> m_popups;
    std::vector<MenuPopup*> m_popupStack; // 0 = top level, back = deepest
    std::vector<TitleRect> m_titleRects;

    menu_render::Font m_font;
    int m_activeMenu = -1;   // highlighted/open top-level index
    int m_hoveredTitle = -1;
    int m_titlePadding = 10; // horizontal padding per side of a title
    bool m_menuMode = false;
    bool m_altHeld = false;
    bool m_swallowNextBarClick = false;

    std::function<void(std::string_view)> m_commandHandler;
    ElementHandle m_returnFocus;
};
