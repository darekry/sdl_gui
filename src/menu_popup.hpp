#pragma once

#include "gui.hpp"
#include "menu_item.hpp"

// Shared text helpers for MenuBar/MenuPopup (font resolution + mnemonic
// underline). Kept here so both widgets measure and draw identically.
namespace menu_render {

struct Font {
    SharedFont handle;
    int fontSize = 16;
    int lineHeight = 0;
};

Font loadFont(GUIManager& manager, const Style& style);
int textWidth(const Font& font, std::string_view text);
int textHeight(const Font& font);
void drawText(SDL_Renderer* renderer, GUIManager& manager, const Font& font,
              std::string_view text, SDL_Color color, int x, int y);
// Draws menu_text::displayText(sourceText) and underlines its mnemonic char.
void drawTextWithMnemonic(SDL_Renderer* renderer, GUIManager& manager, const Font& font,
                          std::string_view sourceText, SDL_Color color, int x, int y);

} // namespace menu_render

// ---------------------------------------------------------------------------
// MenuPopup: one dropdown level of a menu tree (overlay top-level widget).
//
// The popup owns no model data: it points at a MenuItem::submenu vector owned
// by MenuBar. MenuBar pre-creates one popup per node-with-children at setup
// time, so opening/closing a menu never adds/removes elements from
// GUIManager's top-level vector during event propagation (which would
// invalidate the manager's reverse iteration).
//
// Interaction is delegated to callbacks set by MenuBar:
//   onActivate(index)      - item clicked / Enter pressed
//   onHover(index|-1)      - hovered row changed (submenu open/close)
//   onNavigate(direction)  - Left/Right pressed (-1 / +1)
//   onDismiss()            - Esc pressed
//   onOutsideClick(x,y)    - click outside; return true to consume
// ---------------------------------------------------------------------------
class MenuPopup : public GUIElement {
public:
    using ItemList = std::vector<std::unique_ptr<MenuItem>>;

    MenuPopup(GUIManager& manager, ItemList* items);

    void openAt(int x, int y);
    void hide();
    [[nodiscard]] bool isOpen() const { return m_open; }

    bool isOverlay() const override { return true; }
    // Overlay pass calls renderOverlay(); menus draw like normal widgets.
    void renderOverlay(SDL_Renderer* renderer) override;
    bool wantsFocusOutline() const override { return false; }
    ComponentType getComponentTypeId() const override;

    // Recomputes width/height from the current model (call after edits).
    void refresh();
    // Drops the model pointer before the owner destroys its MenuItem tree.
    // Safe to call while the popup is still registered (manager-owned).
    void detachItems();

    [[nodiscard]] MenuItem* itemAt(size_t index) const;
    [[nodiscard]] size_t itemCount() const;
    [[nodiscard]] SDL_Rect getItemRect(size_t index) const;
    [[nodiscard]] int getHoveredIndex() const { return m_hoveredIndex; }
    void setHoveredIndex(int index);
    [[nodiscard]] int getPressedIndex() const { return m_pressedIndex; }

    // First/last selectable row (-1 when none).
    [[nodiscard]] int firstSelectableIndex() const;
    [[nodiscard]] int lastSelectableIndex() const;
    [[nodiscard]] int nextSelectableIndex(int from, int direction) const;
    // Index of a selectable item whose mnemonic matches (case-insensitive), or -1.
    [[nodiscard]] int mnemonicIndex(char key) const;

    void setOnActivate(std::function<void(int)> callback) { m_onActivate = std::move(callback); }
    void setOnHover(std::function<void(int)> callback) { m_onHover = std::move(callback); }
    void setOnNavigate(std::function<void(int)> callback) { m_onNavigate = std::move(callback); }
    void setOnDismiss(std::function<void()> callback) { m_onDismiss = std::move(callback); }
    void setOnOutsideClick(std::function<bool(float, float)> callback) {
        m_onOutsideClick = std::move(callback);
    }
    void clearCallbacks();

    bool handleSelf(const SDL_Event& e) override;

protected:
    void draw(SDL_Renderer* renderer) override;
    bool canShareRenderCache() const override { return false; }

private:
    [[nodiscard]] int itemIndexAt(int globalX, int globalY) const;
    [[nodiscard]] int selectableIndexAt(int globalX, int globalY) const;
    void toggleCheckState(size_t index);

    ItemList* m_items = nullptr;
    bool m_open = false;
    int m_hoveredIndex = -1;
    int m_pressedIndex = -1;

    // Metrics (recomputed in refresh()).
    menu_render::Font m_font;
    int m_rowHeight = 22;
    int m_separatorHeight = 8;
    int m_leftPad = 6;
    int m_rightPad = 8;
    int m_checkGutter = 22;
    int m_shortcutGap = 24;
    int m_arrowWidth = 0;
    std::vector<int> m_rowY;   // local y of every row
    std::vector<int> m_rowH;   // local height of every row

    std::function<void(int)> m_onActivate;
    std::function<void(int)> m_onHover;
    std::function<void(int)> m_onNavigate;
    std::function<void()> m_onDismiss;
    std::function<bool(float, float)> m_onOutsideClick;
};
