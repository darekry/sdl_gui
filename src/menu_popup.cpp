#include "menu_popup.hpp"

#include "constants.hpp"
#include "gui_manager.hpp"
#include "sdl_rect_helpers.hpp"

#include <SDL3_ttf/SDL_ttf.h>

#include "std.hpp"

namespace menu_render {

Font loadFont(GUIManager& manager, const Style& style) {
    Font font;
    font.fontSize = style.fontSize.value_or(
        manager.getTheme().getDefaultStyle().fontSize.value_or(16));
    const std::string fontName = style.fontName.value_or(constants::kDefaultFontPath);
    font.handle = manager.getFontManager().loadFont(fontName, font.fontSize);
    font.lineHeight = font.handle ? TTF_GetFontHeight(font.handle.get()) : font.fontSize + 4;
    return font;
}

int textWidth(const Font& font, std::string_view text) {
    if (!font.handle || text.empty()) return 0;
    int width = 0;
    int height = 0;
    if (!TTF_GetStringSize(font.handle.get(), text.data(), text.size(), &width, &height)) {
        return 0;
    }
    return width;
}

int textHeight(const Font& font) {
    return font.lineHeight;
}

void drawText(SDL_Renderer* renderer, GUIManager& manager, const Font& font,
              std::string_view text, SDL_Color color, int x, int y) {
    if (text.empty() || !font.handle) return;
    auto texture = manager.getTextureManager().createTextureFromText(text, font.handle, color);
    if (!texture) return;
    RenderTexture(renderer, texture.get(), SDL_Rect{x, y, TextureWidth(texture.get()), TextureHeight(texture.get())});
}

void drawTextWithMnemonic(SDL_Renderer* renderer, GUIManager& manager, const Font& font,
                          std::string_view sourceText, SDL_Color color, int x, int y) {
    const std::string display = menu_text::displayText(sourceText);
    drawText(renderer, manager, font, display, color, x, y);

    const size_t mnemonicIndex = menu_text::mnemonicDisplayIndex(sourceText);
    if (mnemonicIndex == std::string::npos || mnemonicIndex >= display.size()) return;

    const std::string_view displayView(display);
    const int prefixWidth = textWidth(font, displayView.substr(0, mnemonicIndex));
    const int charWidth = std::max(1, textWidth(font, displayView.substr(mnemonicIndex, 1)));
    const int underlineY = y + font.lineHeight - 1;
    SetDrawColor(renderer, color);
    RenderLine(renderer, x + prefixWidth, underlineY, x + prefixWidth + charWidth, underlineY);
}

} // namespace menu_render

namespace {

SDL_Color lighten(SDL_Color color, int amount) {
    auto bump = [amount](Uint8 channel) {
        const int value = static_cast<int>(channel) + amount;
        return static_cast<Uint8>(std::clamp(value, 0, 255));
    };
    return SDL_Color{bump(color.r), bump(color.g), bump(color.b), color.a};
}

} // namespace

MenuPopup::MenuPopup(GUIManager& manager, ItemList* items)
    : GUIElement(manager, 0, 0, 1, 1), m_items(items) {
    setCanGetKeyboardFocus(true);
    setVisible(false);
    refresh();
}

void MenuPopup::detachItems() {
    m_items = nullptr;
    hide();
}

void MenuPopup::refresh() {
    if (!m_items) {
        setSize(1, 1);
        return;
    }

    const Style normal = getComposedStyle(ElementState::Normal);
    m_font = menu_render::loadFont(m_manager, normal);
    m_rowHeight = std::max(20, m_font.lineHeight + 8);

    m_rowY.clear();
    m_rowH.clear();
    m_rowY.reserve(m_items->size());
    m_rowH.reserve(m_items->size());

    int maxTextWidth = 0;
    int maxShortcutWidth = 0;
    bool hasSubmenu = false;
    bool hasCheck = false;
    int totalHeight = 0;

    for (const auto& item : *m_items) {
        if (!item) continue;
        const int height = item->separator ? m_separatorHeight : m_rowHeight;
        m_rowY.push_back(totalHeight);
        m_rowH.push_back(height);
        totalHeight += height;

        if (item->separator) continue;
        maxTextWidth = std::max(maxTextWidth, menu_render::textWidth(m_font, item->displayText()));
        maxShortcutWidth = std::max(maxShortcutWidth, menu_render::textWidth(m_font, item->shortcut));
        if (item->hasSubmenu()) hasSubmenu = true;
        if (item->check != MenuCheck::None) hasCheck = true;
    }

    m_checkGutter = hasCheck ? 22 : 14;
    m_arrowWidth = hasSubmenu ? 16 : 0;

    int width = m_leftPad + m_checkGutter + maxTextWidth + m_arrowWidth + m_rightPad;
    if (maxShortcutWidth > 0) {
        width += m_shortcutGap + maxShortcutWidth;
    }
    width = std::max(width, 110);

    setSize(width, std::max(totalHeight, m_rowHeight));
    markDirty();
}

void MenuPopup::openAt(int x, int y) {
    setPosition(x, y);
    m_open = true;
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
    setVisible(true);
    markDirty();
}

void MenuPopup::hide() {
    m_open = false;
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
    setVisible(false);
    markDirty();
}

ComponentType MenuPopup::getComponentTypeId() const {
    return ComponentType::MenuPopup;
}

void MenuPopup::renderOverlay(SDL_Renderer* renderer) {
    render(renderer);
}

MenuItem* MenuPopup::itemAt(size_t index) const {
    if (!m_items || index >= m_items->size()) return nullptr;
    return (*m_items)[index].get();
}

size_t MenuPopup::itemCount() const {
    return m_items ? m_items->size() : 0;
}

SDL_Rect MenuPopup::getItemRect(size_t index) const {
    if (index >= m_rowY.size()) return SDL_Rect{0, 0, 0, 0};
    return SDL_Rect{0, m_rowY[index], m_width, m_rowH[index]};
}

int MenuPopup::itemIndexAt(int globalX, int globalY) const {
    const SDL_Point local = toLocalCoords(globalX, globalY);
    if (local.x < 0 || local.y < 0 || local.x >= m_width || local.y >= m_height) return -1;
    for (size_t i = 0; i < m_rowY.size(); ++i) {
        if (local.y >= m_rowY[i] && local.y < m_rowY[i] + m_rowH[i]) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int MenuPopup::selectableIndexAt(int globalX, int globalY) const {
    const int index = itemIndexAt(globalX, globalY);
    const MenuItem* item = itemAt(static_cast<size_t>(index));
    return (item && item->isSelectable()) ? index : -1;
}

void MenuPopup::setHoveredIndex(int index) {
    int next = -1;
    if (index >= 0) {
        if (const MenuItem* item = itemAt(static_cast<size_t>(index)); item && item->isSelectable()) {
            next = index;
        }
    }
    if (next == m_hoveredIndex) return;
    m_hoveredIndex = next;
    markDirty();
    if (m_onHover) m_onHover(m_hoveredIndex);
}

int MenuPopup::firstSelectableIndex() const {
    for (size_t i = 0; i < itemCount(); ++i) {
        const MenuItem* item = itemAt(i);
        if (item && item->isSelectable()) return static_cast<int>(i);
    }
    return -1;
}

int MenuPopup::lastSelectableIndex() const {
    for (size_t i = itemCount(); i > 0; --i) {
        const MenuItem* item = itemAt(i - 1);
        if (item && item->isSelectable()) return static_cast<int>(i - 1);
    }
    return -1;
}

int MenuPopup::mnemonicIndex(char key) const {
    const char lowerKey = static_cast<char>(std::tolower(static_cast<unsigned char>(key)));
    for (size_t i = 0; i < itemCount(); ++i) {
        const MenuItem* item = itemAt(i);
        if (!item || !item->isSelectable()) continue;
        const char candidate = menu_text::mnemonic(item->text);
        if (candidate == '\0') continue;
        if (std::tolower(static_cast<unsigned char>(candidate)) == lowerKey) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int MenuPopup::nextSelectableIndex(int from, int direction) const {
    const int count = static_cast<int>(itemCount());
    if (direction > 0) {
        const int start = (from < 0) ? -1 : from;
        for (int i = start + 1; i < count; ++i) {
            const MenuItem* item = itemAt(static_cast<size_t>(i));
            if (item && item->isSelectable()) return i;
        }
    } else {
        const int start = (from < 0) ? count : from;
        for (int i = start - 1; i >= 0; --i) {
            const MenuItem* item = itemAt(static_cast<size_t>(i));
            if (item && item->isSelectable()) return i;
        }
    }
    return from;
}

void MenuPopup::toggleCheckState(size_t index) {
    MenuItem* item = itemAt(index);
    if (!item || item->check == MenuCheck::None) return;

    if (item->check == MenuCheck::Radio) {
        // Radio group = siblings of the same submenu.
        for (auto& sibling : *m_items) {
            if (sibling && sibling->check == MenuCheck::Radio) {
                sibling->checked = false;
            }
        }
        item->checked = true;
    } else {
        item->checked = !item->checked;
    }
    markDirty();
}

void MenuPopup::clearCallbacks() {
    m_onActivate = nullptr;
    m_onHover = nullptr;
    m_onNavigate = nullptr;
    m_onDismiss = nullptr;
    m_onOutsideClick = nullptr;
}

bool MenuPopup::handleSelf(const SDL_Event& e) {
    if (!m_open || !m_visible) return false;

    switch (e.type) {
        case SDL_EVENT_MOUSE_MOTION: {
            const bool inside = contains(e.motion.x, e.motion.y);
            if (!inside) {
                setHoveredIndex(-1);
                return false;
            }
            setHoveredIndex(selectableIndexAt(static_cast<int>(e.motion.x), static_cast<int>(e.motion.y)));
            return true;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) return false;
            if (contains(e.button.x, e.button.y)) {
                const int index = selectableIndexAt(static_cast<int>(e.button.x), static_cast<int>(e.button.y));
                m_pressedIndex = index;
                setHoveredIndex(index);
                m_manager.requestFocus(this);
                return true;
            }
            if (m_onOutsideClick) return m_onOutsideClick(e.button.x, e.button.y);
            hide();
            return true;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (e.button.button != SDL_BUTTON_LEFT) return false;
            const bool inside = contains(e.button.x, e.button.y);
            if (m_pressedIndex >= 0) {
                const int index = inside
                    ? selectableIndexAt(static_cast<int>(e.button.x), static_cast<int>(e.button.y))
                    : -1;
                const int pressed = m_pressedIndex;
                m_pressedIndex = -1;
                if (pressed == index && index >= 0) {
                    if (m_onActivate) m_onActivate(index);
                    else hide();
                }
                return true;
            }
            return inside;
        }
        case SDL_EVENT_KEY_DOWN: {
            if (e.key.repeat) return true;
            switch (e.key.key) {
                case SDLK_UP:
                    setHoveredIndex(nextSelectableIndex(m_hoveredIndex, -1));
                    return true;
                case SDLK_DOWN:
                    setHoveredIndex(nextSelectableIndex(m_hoveredIndex, +1));
                    return true;
                case SDLK_HOME:
                    setHoveredIndex(firstSelectableIndex());
                    return true;
                case SDLK_END:
                    setHoveredIndex(lastSelectableIndex());
                    return true;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                case SDLK_SPACE: {
                    int index = m_hoveredIndex >= 0 ? m_hoveredIndex : firstSelectableIndex();
                    if (index >= 0) {
                        if (m_onActivate) m_onActivate(index);
                        else hide();
                    }
                    return true;
                }
                case SDLK_ESCAPE:
                    if (m_onDismiss) m_onDismiss();
                    else hide();
                    return true;
                case SDLK_LEFT:
                    if (m_onNavigate) m_onNavigate(-1);
                    return true;
                case SDLK_RIGHT:
                    if (m_onNavigate) m_onNavigate(+1);
                    return true;
                default: {
                    // Win32 item mnemonics: with the menu open, the underlined
                    // letter activates the item directly (Alt+letter included).
                    if (e.key.mod & SDL_KMOD_CTRL) return false;
                    const SDL_Keycode key = e.key.key;
                    const bool letter = key >= SDLK_A && key <= SDLK_Z;
                    const bool digit = key >= SDLK_0 && key <= SDLK_9;
                    if (!letter && !digit) return false;
                    const int index = mnemonicIndex(static_cast<char>(key));
                    if (index < 0) return false;
                    if (m_onActivate) m_onActivate(index);
                    else hide();
                    return true;
                }
            }
        }
        default:
            return false;
    }
}

void MenuPopup::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    if (!m_items) return;

    const Style normal = getComposedStyle(ElementState::Normal);
    const Style hoverStyle = getComposedStyle(ElementState::Hover);
    const Style disabledStyle = getComposedStyle(ElementState::Disabled);

    const SDL_Color normalText = normal.textColor.value_or(constants::kDefaultTextColor);
    const SDL_Color hoverText = hoverStyle.textColor.value_or(normalText);
    const SDL_Color disabledText = disabledStyle.textColor.value_or(normalText);
    const SDL_Color hoverBackground = hoverStyle.backgroundColor.value_or(constants::kWin95Navy);
    const SDL_Color separatorShadow = normal.borderColor.value_or(constants::kWin95Shadow);
    const SDL_Color separatorLight = lighten(separatorShadow, 64);

    for (size_t i = 0; i < m_rowY.size() && i < m_items->size(); ++i) {
        const MenuItem* item = (*m_items)[i].get();
        if (!item) continue;

        const int rowY = m_rowY[i];
        const int rowHeight = m_rowH[i];

        if (item->separator) {
            const int lineY = rowY + m_separatorHeight / 2;
            SetDrawColor(renderer, separatorShadow);
            RenderLine(renderer, 6, lineY, m_width - 6, lineY);
            SetDrawColor(renderer, separatorLight);
            RenderLine(renderer, 6, lineY + 1, m_width - 6, lineY + 1);
            continue;
        }

        const bool highlighted = static_cast<int>(i) == m_hoveredIndex;
        const SDL_Color textColor = !item->enabled
            ? disabledText
            : (highlighted ? hoverText : normalText);

        if (highlighted) {
            SetDrawColor(renderer, hoverBackground);
            RenderFillRect(renderer, SDL_Rect{1, rowY, m_width - 2, rowHeight});
        }

        const int textX = m_leftPad + m_checkGutter;
        const int textY = rowY + (m_rowHeight - m_font.lineHeight) / 2;
        menu_render::drawTextWithMnemonic(renderer, m_manager, m_font, item->text, textColor, textX, textY);

        if (!item->shortcut.empty()) {
            const int shortcutWidth = menu_render::textWidth(m_font, item->shortcut);
            const int shortcutX = m_width - m_rightPad - m_arrowWidth - shortcutWidth;
            menu_render::drawText(renderer, m_manager, m_font, item->shortcut, textColor,
                                  shortcutX, textY);
        }

        const int checkCenterX = m_leftPad + m_checkGutter / 2;
        const int checkCenterY = rowY + m_rowHeight / 2;
        if (item->check != MenuCheck::None && item->checked) {
            SetDrawColor(renderer, textColor);
            if (item->check == MenuCheck::Radio) {
                drawRoundedFilledRect(renderer,
                    SDL_FRect{static_cast<float>(checkCenterX - 3), static_cast<float>(checkCenterY - 3), 6.0f, 6.0f},
                    3.0f, ColorToFColor(textColor));
            } else {
                // 2px checkmark: two parallel strokes for the Win95 look.
                for (int offset = 0; offset < 2; ++offset) {
                    RenderLine(renderer, checkCenterX - 4, checkCenterY + offset,
                               checkCenterX - 1, checkCenterY + 3 + offset);
                    RenderLine(renderer, checkCenterX - 1, checkCenterY + 3 + offset,
                               checkCenterX + 4, checkCenterY - 4 + offset);
                }
            }
        }

        if (item->hasSubmenu()) {
            const int arrowTipX = m_width - m_rightPad - 2;
            const int arrowX = arrowTipX - 4;
            const int arrowY = rowY + m_rowHeight / 2;
            SDL_FPoint points[4] = {
                {static_cast<float>(arrowX), static_cast<float>(arrowY - 4)},
                {static_cast<float>(arrowTipX), static_cast<float>(arrowY)},
                {static_cast<float>(arrowX), static_cast<float>(arrowY + 4)},
                {static_cast<float>(arrowX), static_cast<float>(arrowY - 4)},
            };
            SetDrawColor(renderer, textColor);
            SDL_RenderLines(renderer, points, 4);
        }
    }
}
