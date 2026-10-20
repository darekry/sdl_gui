#include "menu_bar.hpp"

#include "constants.hpp"
#include "gui_manager.hpp"
#include "sdl_rect_helpers.hpp"

#include "std.hpp"

namespace {

constexpr SDL_Keymod kModMask =
    static_cast<SDL_Keymod>(SDL_KMOD_CTRL | SDL_KMOD_SHIFT | SDL_KMOD_ALT);

char keyToMnemonic(SDL_Keycode key) {
    if (key >= SDLK_A && key <= SDLK_Z) return static_cast<char>(key);
    if (key >= SDLK_0 && key <= SDLK_9) return static_cast<char>(key);
    return '\0';
}

} // namespace

MenuBar::MenuBar(GUIManager& manager, int x, int y, int width, int height)
    : GUIElement(manager, x, y, width, height > 0 ? height : 1) {
    setCanGetKeyboardFocus(true);
    m_font = menu_render::loadFont(m_manager, getComposedStyle(ElementState::Normal));
    if (height <= 0) {
        m_height = m_font.lineHeight + 8;
    }
    measureTitles();
}

MenuBar::~MenuBar() {
    for (auto& entry : m_popups) {
        MenuPopup* popup = entry.second;
        if (!popup || !m_manager.isElementAlive(popup)) continue;
        popup->clearCallbacks();
        popup->detachItems();
        popup->markForDeletion();
    }
    m_popups.clear();
    m_popupStack.clear();
}

MenuItem& MenuBar::addMenu(std::string_view title) {
    m_menus.push_back(std::make_unique<MenuItem>(title));
    attachMenuHooks(*m_menus.back());
    rebuildPopups();
    measureTitles();
    markDirty();
    return *m_menus.back();
}

void MenuBar::clearMenus() {
    closeMenus();
    for (auto& entry : m_popups) {
        MenuPopup* popup = entry.second;
        if (!popup || !m_manager.isElementAlive(popup)) continue;
        popup->clearCallbacks();
        popup->detachItems();
        popup->markForDeletion();
    }
    m_popups.clear();
    m_menus.clear();
    m_titleRects.clear();
    markDirty();
}

MenuItem* MenuBar::getMenu(size_t index) const {
    return index < m_menus.size() ? m_menus[index].get() : nullptr;
}

SDL_Rect MenuBar::getTitleRect(size_t index) const {
    if (index >= m_titleRects.size()) return SDL_Rect{0, 0, 0, 0};
    return SDL_Rect{m_titleRects[index].x, 0, m_titleRects[index].width, m_height};
}

MenuPopup* MenuBar::getTopPopup() const {
    return m_popupStack.empty() ? nullptr : m_popupStack.back();
}

MenuPopup* MenuBar::getPopupFor(const MenuItem* owner) const {
    const auto it = m_popups.find(owner);
    return it == m_popups.end() ? nullptr : it->second;
}

void MenuBar::setCommandHandler(std::function<void(std::string_view)> handler) {
    m_commandHandler = std::move(handler);
}

MenuPopup* MenuBar::popupAt(size_t stackIndex) const {
    return stackIndex < m_popupStack.size() ? m_popupStack[stackIndex] : nullptr;
}

int MenuBar::indexOfPopup(const MenuPopup* popup) const {
    for (size_t i = 0; i < m_popupStack.size(); ++i) {
        if (m_popupStack[i] == popup) return static_cast<int>(i);
    }
    return -1;
}

MenuPopup* MenuBar::ensurePopup(MenuItem* owner) {
    if (!owner) return nullptr;
    const auto it = m_popups.find(owner);
    if (it != m_popups.end()) return it->second;
    return createPopup(owner);
}

void MenuBar::attachMenuHooks(MenuItem& node) {
    // Every node reports children as they are appended, so a popup is created
    // exactly when a node gains its first child — including nested submenus
    // built after addMenu("&File"). Setup-time only: creates top-level
    // elements, so don't build menus from inside a running event callback.
    node.onItemAdded = [this](MenuItem& parent, MenuItem& child) {
        attachMenuHooks(child);
        if (parent.submenu.size() == 1) {
            ensurePopup(&parent);
        }
    };
    for (auto& child : node.submenu) {
        attachMenuHooks(*child);
    }
}

void MenuBar::rebuildPopups() {
    std::function<void(MenuItem*)> walk = [this, &walk](MenuItem* node) {
        if (!node) return;
        attachMenuHooks(*node);
        if (node->hasSubmenu()) {
            ensurePopup(node);
        }
        for (auto& child : node->submenu) {
            walk(child.get());
        }
    };
    for (auto& menu : m_menus) {
        walk(menu.get());
    }
}

MenuPopup* MenuBar::createPopup(MenuItem* owner) {
    if (!owner) return nullptr;

    auto popup = std::make_unique<MenuPopup>(m_manager, &owner->submenu);
    MenuPopup* raw = popup.get();
    raw->setOnActivate([this, raw](int index) { activateItem(raw, index); });
    raw->setOnHover([this, raw](int index) { handlePopupHover(raw, index); });
    raw->setOnNavigate([this, raw](int direction) { handlePopupNavigate(raw, direction); });
    raw->setOnDismiss([this, raw]() { handlePopupDismiss(raw); });
    raw->setOnOutsideClick([this](float x, float y) {
        const int titleIndex = titleIndexAt(static_cast<int>(x), static_cast<int>(y));
        const bool sameTitle = isMenuOpen() && titleIndex == m_activeMenu;
        const bool insideBar = contains(x, y);
        closeMenus();
        m_swallowNextBarClick = sameTitle;
        return !insideBar;
    });

    if (m_manager.addElement(std::move(popup)) == nullptr) {
        return nullptr;
    }
    m_popups[owner] = raw;
    return raw;
}

void MenuBar::measureTitles() {
    m_font = menu_render::loadFont(m_manager, getComposedStyle(ElementState::Normal));
    m_titleRects.clear();
    m_titleRects.reserve(m_menus.size());

    int x = 2;
    for (const auto& menu : m_menus) {
        const int textWidth = menu_render::textWidth(m_font, menu_text::displayText(menu->text));
        const int width = textWidth + 2 * m_titlePadding;
        m_titleRects.push_back(TitleRect{x, width});
        x += width;
    }
}

int MenuBar::titleIndexAt(int globalX, int globalY) const {
    const SDL_Point absolute = getAbsolutePosition();
    const int localX = globalX - absolute.x;
    const int localY = globalY - absolute.y;
    if (localX < 0 || localY < 0 || localX >= m_width || localY >= m_height) return -1;

    for (size_t i = 0; i < m_titleRects.size(); ++i) {
        const TitleRect& rect = m_titleRects[i];
        if (localX >= rect.x && localX < rect.x + rect.width) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int MenuBar::findMenuByMnemonic(char mnemonic) const {
    if (mnemonic == '\0') return -1;
    for (size_t i = 0; i < m_menus.size(); ++i) {
        const char candidate = menu_text::mnemonic(m_menus[i]->text);
        if (candidate == '\0') continue;
        if (std::tolower(static_cast<unsigned char>(candidate)) ==
            std::tolower(static_cast<unsigned char>(mnemonic))) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void MenuBar::enterMenuMode() {
    if (m_menus.empty()) return;
    if (!m_menuMode && !isMenuOpen() && !m_manager.isFocusInside(this)) {
        m_returnFocus = m_manager.getHandle(m_manager.getKeyboardFocus());
    }
    m_menuMode = true;
    if (m_activeMenu < 0 || static_cast<size_t>(m_activeMenu) >= m_menus.size()) {
        m_activeMenu = 0;
    }
    m_manager.requestFocus(this);
    markDirty();
}

void MenuBar::leaveMenuMode() {
    if (!m_menuMode && m_popupStack.empty()) return;
    closePopupStack();
    m_menuMode = false;
    m_activeMenu = -1;
    m_hoveredTitle = -1;
    m_altHeld = false;
    m_manager.setKeyboardFocus(m_manager.resolve(m_returnFocus));
    m_returnFocus.reset();
    markDirty();
}

void MenuBar::closePopupStack() {
    for (MenuPopup* popup : m_popupStack) {
        if (popup) popup->hide();
    }
    m_popupStack.clear();
}

void MenuBar::closeMenus() {
    const bool hadMenu = m_menuMode || !m_popupStack.empty();
    closePopupStack();
    m_menuMode = false;
    m_activeMenu = -1;
    m_hoveredTitle = -1;
    m_altHeld = false;
    if (hadMenu) {
        m_manager.setKeyboardFocus(m_manager.resolve(m_returnFocus));
    }
    m_returnFocus.reset();
    markDirty();
}

void MenuBar::positionTopPopup(MenuPopup* popup, size_t menuIndex) {
    if (!popup || menuIndex >= m_titleRects.size()) return;
    popup->refresh();

    const SDL_Point absolute = getAbsolutePosition();
    int x = absolute.x + m_titleRects[menuIndex].x;
    int y = absolute.y + m_height;

    int windowWidth = 0;
    int windowHeight = 0;
    m_manager.getWindowSize(windowWidth, windowHeight);

    if (x + popup->getWidth() > windowWidth) {
        x = std::max(0, windowWidth - popup->getWidth());
    }
    if (y + popup->getHeight() > windowHeight) {
        y = std::max(0, windowHeight - popup->getHeight());
    }
    popup->openAt(x, y);
}

void MenuBar::positionChildPopup(MenuPopup* child, MenuPopup* parent, int itemIndex) {
    if (!child || !parent || itemIndex < 0) return;
    child->refresh();

    const int parentX = parent->getX();
    const int parentY = parent->getY();
    const SDL_Rect itemRect = parent->getItemRect(static_cast<size_t>(itemIndex));

    int x = parentX + parent->getWidth() - 4;
    int y = parentY + itemRect.y - 4;

    int windowWidth = 0;
    int windowHeight = 0;
    m_manager.getWindowSize(windowWidth, windowHeight);

    if (x + child->getWidth() > windowWidth) {
        x = parentX - child->getWidth() + 4;
    }
    x = std::clamp(x, 0, std::max(0, windowWidth - child->getWidth()));
    y = std::clamp(y, 0, std::max(0, windowHeight - child->getHeight()));
    child->openAt(x, y);
}

void MenuBar::openMenu(size_t index) {
    if (index >= m_menus.size()) return;
    MenuPopup* popup = getPopupFor(m_menus[index].get());
    if (!popup) return;

    if (!m_menuMode && m_popupStack.empty() && !m_manager.isFocusInside(this)) {
        m_returnFocus = m_manager.getHandle(m_manager.getKeyboardFocus());
    }

    closePopupStack();
    m_activeMenu = static_cast<int>(index);
    m_menuMode = true;
    m_swallowNextBarClick = false;

    positionTopPopup(popup, index);
    m_popupStack.push_back(popup);
    m_manager.requestFocus(popup);
    popup->setHoveredIndex(-1);
    markDirty();
}

void MenuBar::openSubmenu(MenuPopup* parent, int itemIndex, bool selectFirst) {
    if (!parent || itemIndex < 0) return;
    const int parentIndex = indexOfPopup(parent);
    if (parentIndex < 0) return;

    MenuItem* item = parent->itemAt(static_cast<size_t>(itemIndex));
    if (!item || !item->hasSubmenu()) return;
    MenuPopup* child = getPopupFor(item);
    if (!child) return;

    while (static_cast<int>(m_popupStack.size()) > parentIndex + 1) {
        MenuPopup* deeper = m_popupStack.back();
        if (deeper) deeper->hide();
        m_popupStack.pop_back();
    }

    positionChildPopup(child, parent, itemIndex);
    m_popupStack.push_back(child);
    m_manager.requestFocus(child);
    child->setHoveredIndex(selectFirst ? child->firstSelectableIndex() : -1);
}

void MenuBar::switchMenu(int direction) {
    if (m_menus.empty()) return;
    const int count = static_cast<int>(m_menus.size());
    const int current = m_activeMenu < 0 ? 0 : m_activeMenu;
    const int next = ((current + direction) % count + count) % count;
    openMenu(static_cast<size_t>(next));
}

void MenuBar::activateItem(MenuPopup* popup, int index) {
    if (!popup || index < 0) return;
    MenuItem* item = popup->itemAt(static_cast<size_t>(index));
    if (!item || !item->isSelectable()) return;

    if (item->hasSubmenu()) {
        openSubmenu(popup, index, false);
        return;
    }

    if (item->check == MenuCheck::Radio) {
        for (size_t i = 0; i < popup->itemCount(); ++i) {
            MenuItem* sibling = popup->itemAt(i);
            if (sibling && sibling->check == MenuCheck::Radio) {
                sibling->checked = false;
            }
        }
        item->checked = true;
    } else if (item->check == MenuCheck::Check) {
        item->checked = !item->checked;
    }
    popup->markDirty();

    auto action = item->action;
    auto command = item->command;
    closeMenus();

    if (action) {
        action();
    } else if (!command.empty() && m_commandHandler) {
        m_commandHandler(command);
    }
}

void MenuBar::handlePopupHover(MenuPopup* popup, int index) {
    const int stackIndex = indexOfPopup(popup);
    if (stackIndex < 0) return;

    if (index >= 0) {
        MenuItem* item = popup->itemAt(static_cast<size_t>(index));
        if (item && item->hasSubmenu()) {
            openSubmenu(popup, index, false);
            return;
        }
    }

    while (static_cast<int>(m_popupStack.size()) > stackIndex + 1) {
        MenuPopup* deeper = m_popupStack.back();
        if (deeper) deeper->hide();
        m_popupStack.pop_back();
    }
}

void MenuBar::handlePopupNavigate(MenuPopup* popup, int direction) {
    if (indexOfPopup(popup) != static_cast<int>(m_popupStack.size()) - 1) return;

    if (direction > 0) {
        const int hovered = popup->getHoveredIndex();
        MenuItem* item = hovered >= 0 ? popup->itemAt(static_cast<size_t>(hovered)) : nullptr;
        if (item && item->hasSubmenu()) {
            openSubmenu(popup, hovered, true);
            return;
        }
        switchMenu(+1);
        return;
    }

    if (m_popupStack.size() > 1) {
        MenuPopup* top = m_popupStack.back();
        if (top) top->hide();
        m_popupStack.pop_back();
        m_manager.requestFocus(m_popupStack.back());
    } else {
        switchMenu(-1);
    }
}

void MenuBar::handlePopupDismiss(MenuPopup* popup) {
    if (indexOfPopup(popup) != static_cast<int>(m_popupStack.size()) - 1) return;

    if (m_popupStack.size() > 1) {
        MenuPopup* top = m_popupStack.back();
        if (top) top->hide();
        m_popupStack.pop_back();
        m_manager.requestFocus(m_popupStack.back());
    } else {
        closeMenus();
    }
}

void MenuBar::onFocusLost() {
    // F10/menu mode with no open popup: leaving the bar (click elsewhere, Tab)
    // exits menu mode. Popup focus is handled by the open/close paths instead.
    if (!m_menuMode || !m_popupStack.empty()) return;
    m_menuMode = false;
    m_activeMenu = -1;
    m_hoveredTitle = -1;
    m_altHeld = false;
    m_returnFocus.reset();
    markDirty();
}

bool MenuBar::handleGlobalKey(const SDL_Event& e) {
    if (!m_visible || !m_enabled) return false;

    // A modal overlay above the bar (dialog/editor) owns the keyboard.
    if (GUIElement* overlay = m_manager.getActiveOverlay()) {
        if (dynamic_cast<MenuPopup*>(overlay) == nullptr) return false;
    }

    if (e.type == SDL_EVENT_KEY_UP) {
        if ((e.key.key == SDLK_LALT || e.key.key == SDLK_RALT) && m_altHeld) {
            m_altHeld = false;
            if (m_menuMode && m_popupStack.empty()) leaveMenuMode();
            return true;
        }
        return false;
    }
    if (e.type != SDL_EVENT_KEY_DOWN) return false;

    if (e.key.key == SDLK_F10 && !e.key.repeat) {
        if (m_menuMode || !m_popupStack.empty()) closeMenus();
        else enterMenuMode();
        return true;
    }

    const bool alt = (e.key.mod & SDL_KMOD_ALT) != 0;
    const bool ctrl = (e.key.mod & SDL_KMOD_CTRL) != 0;

    if (!m_menus.empty() && alt && !ctrl) {
        if (e.key.key == SDLK_LALT || e.key.key == SDLK_RALT) {
            if (!e.key.repeat) {
                m_altHeld = true;
                if (!m_menuMode) enterMenuMode();
            }
            return true;
        }
        const char mnemonicKey = keyToMnemonic(e.key.key);
        if (mnemonicKey != '\0') {
            const int index = findMenuByMnemonic(mnemonicKey);
            if (index >= 0) {
                openMenu(static_cast<size_t>(index));
                return true;
            }
        }
    }

    if (!m_menuMode && m_popupStack.empty() && !alt && dispatchShortcut(e)) {
        return true;
    }
    return false;
}

bool MenuBar::dispatchShortcut(const SDL_Event& e) {
    if (e.type != SDL_EVENT_KEY_DOWN || e.key.repeat) return false;
    const SDL_Keymod mod = static_cast<SDL_Keymod>(e.key.mod & kModMask);
    const MenuItem* found = nullptr;

    std::function<void(const MenuPopup::ItemList&)> walk = [&](const MenuPopup::ItemList& items) {
        for (const auto& item : items) {
            if (found || !item) return;
            if (item->isSelectable() && item->shortcutKey != SDLK_UNKNOWN &&
                item->shortcutKey == e.key.key &&
                static_cast<SDL_Keymod>(item->shortcutMod & kModMask) == mod &&
                (item->action || !item->command.empty())) {
                found = item.get();
                return;
            }
            if (!item->submenu.empty()) walk(item->submenu);
        }
    };
    for (const auto& menu : m_menus) {
        if (found) break;
        walk(menu->submenu);
    }

    if (!found) return false;
    fireItem(*found);
    return true;
}

void MenuBar::fireItem(const MenuItem& item) {
    auto action = item.action;
    auto command = item.command;
    auto handler = m_commandHandler;
    if (action) {
        action();
    } else if (!command.empty() && handler) {
        handler(command);
    }
}

bool MenuBar::handleSelf(const SDL_Event& e) {
    if (!m_visible || !m_enabled) return false;

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        const int hovered = titleIndexAt(static_cast<int>(e.motion.x), static_cast<int>(e.motion.y));
        if (hovered != m_hoveredTitle) {
            m_hoveredTitle = hovered;
            if (isMenuOpen() && hovered >= 0 && hovered != m_activeMenu) {
                openMenu(static_cast<size_t>(hovered));
            } else if (m_menuMode && !isMenuOpen() && hovered >= 0) {
                // F10 mode without an open popup: moving the mouse moves the
                // single highlighted title (no second highlight).
                m_activeMenu = hovered;
            }
            markDirty();
        }
        return hovered >= 0;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (!contains(e.button.x, e.button.y)) return false;
        if (m_swallowNextBarClick) {
            m_swallowNextBarClick = false;
            return true;
        }
        const int index = titleIndexAt(static_cast<int>(e.button.x), static_cast<int>(e.button.y));
        if (index >= 0) {
            if (index == m_activeMenu && isMenuOpen()) {
                closeMenus();
            } else {
                openMenu(static_cast<size_t>(index));
            }
        }
        return true;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        return contains(e.button.x, e.button.y);
    }

    if (e.type == SDL_EVENT_KEY_DOWN && m_menuMode && !e.key.repeat && hasKeyboardFocus()) {
        switch (e.key.key) {
            case SDLK_LEFT: {
                if (m_menus.empty() || isMenuOpen()) return true;
                const int count = static_cast<int>(m_menus.size());
                m_activeMenu = ((m_activeMenu - 1) % count + count) % count;
                markDirty();
                return true;
            }
            case SDLK_RIGHT: {
                if (m_menus.empty() || isMenuOpen()) return true;
                const int count = static_cast<int>(m_menus.size());
                m_activeMenu = (m_activeMenu + 1) % count;
                markDirty();
                return true;
            }
            case SDLK_DOWN:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                if (!m_menus.empty()) {
                    openMenu(static_cast<size_t>(m_activeMenu < 0 ? 0 : m_activeMenu));
                }
                return true;
            case SDLK_ESCAPE:
                leaveMenuMode();
                return true;
            default:
                return false;
        }
    }

    return false;
}

void MenuBar::layoutChildren() {
    measureTitles();
}

void MenuBar::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);

    const Style normal = getComposedStyle(ElementState::Normal);
    const Style hover = getComposedStyle(ElementState::Hover);
    const SDL_Color normalText = normal.textColor.value_or(constants::kDefaultTextColor);
    const SDL_Color hoverText = hover.textColor.value_or(normalText);
    const SDL_Color hoverBackground = hover.backgroundColor.value_or(constants::kWin95Navy);

    m_font = menu_render::loadFont(m_manager, normal);
    const bool highlightActive = m_menuMode || isMenuOpen();

    for (size_t i = 0; i < m_titleRects.size() && i < m_menus.size(); ++i) {
        const TitleRect& rect = m_titleRects[i];
        const bool highlighted = highlightActive && static_cast<int>(i) == m_activeMenu;
        if (highlighted) {
            SetDrawColor(renderer, hoverBackground);
            RenderFillRect(renderer, SDL_Rect{rect.x, 0, rect.width, m_height});
        }

        const std::string display = m_menus[i]->displayText();
        const int textWidth = menu_render::textWidth(m_font, display);
        const int textX = rect.x + (rect.width - textWidth) / 2;
        const int textY = (m_height - m_font.lineHeight) / 2;
        menu_render::drawTextWithMnemonic(renderer, m_manager, m_font, m_menus[i]->text,
                                          highlighted ? hoverText : normalText, textX, textY);
    }
}

ComponentType MenuBar::getComponentTypeId() const {
    return ComponentType::MenuBar;
}
