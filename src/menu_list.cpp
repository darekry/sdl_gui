#include "menu_list.hpp"

#include "constants.hpp"
#include "gui_manager.hpp"

#include "std.hpp"

MenuList::MenuList(GUIManager& manager, int x, int y, int width, int height,
                   std::string_view title, std::vector<Item> items)
    : Panel(manager, x, y, width, height)
    , m_title(title)
    , m_items(std::move(items)) {
    setCanGetKeyboardFocus(true);
    m_selected = firstSelectable();
}

MenuList::~MenuList() {
    if (m_timerId != 0) {
        stopTimer(m_timerId);
        m_timerId = 0;
    }
}

void MenuList::setItems(std::vector<Item> items) {
    m_items = std::move(items);
    m_selected = firstSelectable();
    m_hovered = -1;
    m_pressed = -1;
    m_selectedId = -1;
    markDirty();
}

void MenuList::addItem(std::string_view text, int id, bool enabled) {
    m_items.push_back(Item{std::string(text), id, enabled});
    if (m_selected < 0) {
        m_selected = firstSelectable();
    }
    markDirty();
}

void MenuList::setColumns(int columns) {
    m_columns = std::max(1, columns);
    markDirty();
}

void MenuList::setPreselect(int item_id) {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == item_id) {
            if (isItemEnabled(i)) {
                m_selected = static_cast<int>(i);
                m_selectedId = item_id;
            }
            markDirty();
            return;
        }
    }
}

void MenuList::setTimerMs(uint32_t ms) {
    if (m_timerId != 0) {
        stopTimer(m_timerId);
        m_timerId = 0;
    }
    m_timerMs = ms;
    if (ms > 0) {
        m_timerId = startTimer(ms, false, [this](GUIElement*) {
            if (m_onTick) m_onTick();
        });
    }
}

bool MenuList::isItemEnabled(size_t index) const {
    if (index >= m_items.size()) {
        return false;
    }
    if (!m_items[index].enabled) {
        return false;
    }
    if (index < 32 && (m_disabledMask & (1u << index)) != 0) {
        return false;
    }
    return true;
}

int MenuList::firstSelectable() const {
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (isItemEnabled(i)) return static_cast<int>(i);
    }
    return -1;
}

int MenuList::nextSelectable(int from, int direction) const {
    const int n = static_cast<int>(m_items.size());
    if (n == 0) return -1;
    const int dir = direction >= 0 ? 1 : -1;
    const int step = std::max(1, std::abs(direction));
    int pos = from;
    if (pos < 0) {
        pos = dir > 0 ? -step : 0;
    }
    for (int i = 0; i < n; ++i) {
        pos = ((pos + dir * step) % n + n) % n;
        if (isItemEnabled(static_cast<size_t>(pos))) {
            return pos;
        }
    }
    return -1;
}

menu_render::Font MenuList::rowFont() const {
    return menu_render::loadFont(m_manager, getComposedStyle(ElementState::Normal));
}

SDL_Rect MenuList::itemRect(size_t index) const {
    const menu_render::Font font = rowFont();
    const int rowHeight = std::max(20, font.lineHeight + 8);
    const int titleHeight = m_title.empty() ? 0 : rowHeight;
    const int cols = std::max(1, m_columns);
    const int cellWidth = std::max(1, m_width / cols);
    const int row = static_cast<int>(index) / cols;
    const int col = static_cast<int>(index) % cols;
    return SDL_Rect{col * cellWidth, titleHeight + row * rowHeight,
                    cellWidth, rowHeight};
}

int MenuList::indexAt(int local_x, int local_y) const {
    for (size_t i = 0; i < m_items.size(); ++i) {
        const SDL_Rect r = itemRect(i);
        if (local_x >= r.x && local_x < r.x + r.w &&
            local_y >= r.y && local_y < r.y + r.h) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void MenuList::selectIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_items.size())) {
        return;
    }
    if (!isItemEnabled(static_cast<size_t>(index))) {
        return;
    }
    m_selected = index;
    markDirty();
}

void MenuList::activate(size_t index) {
    if (!isItemEnabled(index)) {
        return;
    }
    m_selected = static_cast<int>(index);
    m_selectedId = m_items[index].id;
    if (m_onSelect) {
        m_onSelect(m_selectedId, m_items[index].text);
    }
    markDirty();
}

void MenuList::selectCancel() {
    m_selectedId = m_cancelId;
    if (m_onSelect) {
        m_onSelect(m_selectedId, std::string_view());
    }
    markDirty();
}

bool MenuList::handleSelf(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        const SDL_Point local = toLocalCoords(static_cast<int>(e.motion.x),
                                                static_cast<int>(e.motion.y));
        const int hovered = indexAt(local.x, local.y);
        if (hovered != m_hovered) {
            m_hovered = hovered;
            markDirty();
        }
        return hovered >= 0;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        const SDL_Point local = toLocalCoords(static_cast<int>(e.button.x),
                                                static_cast<int>(e.button.y));
        const int index = indexAt(local.x, local.y);
        if (index >= 0 && isItemEnabled(static_cast<size_t>(index))) {
            m_pressed = index;
            m_manager.requestFocus(this);
            selectIndex(index);
            return true;
        }
        return false;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT) {
        const int pressed = m_pressed;
        m_pressed = -1;
        if (pressed < 0) {
            return false;
        }
        const SDL_Point local = toLocalCoords(static_cast<int>(e.button.x),
                                                static_cast<int>(e.button.y));
        const int index = indexAt(local.x, local.y);
        if (index == pressed && index >= 0 && isItemEnabled(static_cast<size_t>(index))) {
            activate(static_cast<size_t>(index));
        }
        return true;
    }

    if (e.type != SDL_EVENT_KEY_DOWN) {
        return false;
    }

    switch (e.key.key) {
        case SDLK_UP:
            selectIndex(nextSelectable(m_selected, -m_columns));
            return true;
        case SDLK_DOWN:
            selectIndex(nextSelectable(m_selected, m_columns));
            return true;
        case SDLK_LEFT:
            selectIndex(nextSelectable(m_selected, -1));
            return true;
        case SDLK_RIGHT:
            selectIndex(nextSelectable(m_selected, 1));
            return true;
        case SDLK_TAB:
            selectIndex(nextSelectable(m_selected, (e.key.mod & SDL_KMOD_SHIFT) ? -1 : 1));
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (m_selected >= 0) {
                activate(static_cast<size_t>(m_selected));
            }
            return true;
        case SDLK_ESCAPE:
            selectCancel();
            return true;
        default:
            break;
    }

    // First-letter matching (case-insensitive), skipping disabled rows.
    if (e.key.key >= 0x20 && e.key.key < 0x7f && !e.key.repeat) {
        const char wanted = static_cast<char>(std::tolower(static_cast<unsigned char>(e.key.key)));
        for (size_t i = 0; i < m_items.size(); ++i) {
            if (!isItemEnabled(i) || m_items[i].text.empty()) continue;
            const char first = static_cast<char>(std::tolower(
                static_cast<unsigned char>(m_items[i].text.front())));
            if (first == wanted) {
                m_selected = static_cast<int>(i);
                markDirty();
                return true;
            }
        }
    }
    return false;
}

ComponentType MenuList::getComponentTypeId() const {
    return ComponentType::MenuList;
}

void MenuList::renderOverlay(SDL_Renderer* renderer) {
    render(renderer);
}

void MenuList::draw(SDL_Renderer* renderer) {
    // Cached path is disabled (direct render); kept for completeness.
    drawDirect(renderer);
}

void MenuList::drawDirect(SDL_Renderer* renderer) {
    // Direct rendering happens in window coordinates: translate the local
    // layout by the element's absolute position.
    const SDL_Point abs = getAbsolutePosition();
    const Style chrome = getComposedStyle(m_state);
    const SDL_FRect bg = SDLRectToFRect(abs.x, abs.y, m_width, m_height);
    if (chrome.backgroundColor && chrome.backgroundColor->a > 0) {
        drawRoundedFilledRect(renderer, bg,
                              static_cast<float>(chrome.borderRadius.value_or(0)),
                              ColorToFColor(*chrome.backgroundColor));
    }
    drawResolvedBorder(renderer, SDL_Rect{abs.x, abs.y, m_width, m_height},
                       chrome, supportsBevel());

    const menu_render::Font font = rowFont();
    const int rowHeight = std::max(20, font.lineHeight + 8);
    const Style normal = getComposedStyle(ElementState::Normal);
    const SDL_Color textColor = normal.textColor.value_or(constants::kDefaultTextColor);
    const Style hover = getComposedStyle(ElementState::Hover);
    const SDL_Color selectedBg = hover.backgroundColor.value_or(constants::kWin95Navy);
    const SDL_Color selectedText = hover.textColor.value_or(SDL_Color{255, 255, 255, 255});
    const SDL_Color disabledText = SDL_Color{128, 128, 128, 255};

    if (!m_title.empty()) {
        const int titleWidth = menu_render::textWidth(font, m_title);
        const int x = abs.x + std::max(8, (m_width - std::max(titleWidth, m_titleWidth)) / 2);
        menu_render::drawText(renderer, m_manager, font, m_title, textColor, x,
                              abs.y + (rowHeight - font.lineHeight) / 2);
    }

    for (size_t i = 0; i < m_items.size(); ++i) {
        SDL_Rect rect = itemRect(i);
        rect.x += abs.x;
        rect.y += abs.y;
        const bool selected = static_cast<int>(i) == m_selected;
        const bool hovered = static_cast<int>(i) == m_hovered;
        const bool enabled = isItemEnabled(i);

        if (selected || hovered) {
            SetDrawColor(renderer, selectedBg);
            RenderFillRect(renderer, rect);
        }

        if (m_rowCallback) {
            m_rowCallback(renderer, rect, m_items[i].text, m_items[i].id,
                          selected, enabled);
            continue;
        }

        const SDL_Color color = !enabled ? disabledText : (selected ? selectedText : textColor);
        menu_render::drawText(renderer, m_manager, font, m_items[i].text, color,
                              rect.x + 8, rect.y + (rowHeight - font.lineHeight) / 2);
    }
}
