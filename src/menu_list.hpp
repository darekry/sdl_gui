#pragma once

#include "gui.hpp"
#include "panel.hpp"
#include "menu_popup.hpp"   // menu_render::Font + helpers

#include "std.hpp"

/**
 * @file menu_list.hpp
 * @brief MenuList - modal MenuZ-style list (title + grid, disabled rows).
 *
 * Not a MenuBar/MenuPopup: a flat item list with an optional title, a row-major
 * grid (columns items per row), disabled entries, preselect/cancel ids and an
 * optional periodic tick (CivLin's 100 ms timer).
 *
 * Keyboard: arrows, Tab/Shift+Tab, Enter, Esc and first-letter matching.
 * Disabled rows are skipped and navigation wraps. Mouse: hover/press/click;
 * clicking a disabled row does not select it.
 */
class MenuList : public Panel {
public:
    struct Item {
        std::string text;
        int id = 0;
        bool enabled = true;
    };

    using SelectCallback = std::function<void(int item_id, std::string_view text)>;
    using TickCallback = std::function<void()>;
    using RowCallback = std::function<void(SDL_Renderer* renderer,
                                           const SDL_Rect& row_rect,
                                           std::string_view text, int item_id,
                                           bool selected, bool enabled)>;

    MenuList(GUIManager& manager, int x, int y, int width, int height,
             std::string_view title = "", std::vector<Item> items = {});
    ~MenuList() override;

    void setItems(std::vector<Item> items);
    void addItem(std::string_view text, int id, bool enabled = true);
    [[nodiscard]] size_t getItemCount() const { return m_items.size(); }
    [[nodiscard]] const Item* getItem(size_t index) const {
        return index < m_items.size() ? &m_items[index] : nullptr;
    }

    // Items per row (row-major). Clamped to >= 1.
    void setColumns(int columns);
    [[nodiscard]] int getColumns() const { return m_columns; }
    // Minimum header width in pixels (0 = text width only).
    void setTitleWidth(int px) { m_titleWidth = std::max(0, px); markDirty(); }
    // Bit i disables item index i (CivLin max 24 items -> 32-bit mask).
    void setDisabledMask(uint32_t mask) { m_disabledMask = mask; markDirty(); }
    [[nodiscard]] uint32_t getDisabledMask() const { return m_disabledMask; }

    // Preselect by item id (-1 leaves the first selectable item).
    void setPreselect(int item_id);
    // Item id reported on Esc; -1 when unset.
    void setCancelId(int item_id) { m_cancelId = item_id; }
    // Periodic tick interval (0 disables the timer).
    void setTimerMs(uint32_t ms);
    [[nodiscard]] uint32_t getTimerMs() const { return m_timerMs; }

    void setOnSelect(SelectCallback callback) { m_onSelect = std::move(callback); }
    void setOnTick(TickCallback callback) { m_onTick = std::move(callback); }
    void setRowCallback(RowCallback callback) { m_rowCallback = std::move(callback); markDirty(); }

    // Last selected id (Enter) or cancel id (Esc); -1 before any activation.
    [[nodiscard]] int getSelectedId() const { return m_selectedId; }
    [[nodiscard]] int getSelectedIndex() const { return m_selected; }

    bool isOverlay() const override { return true; }
    void renderOverlay(SDL_Renderer* renderer) override;
    bool wantsFocusOutline() const override { return false; }
    bool handleSelf(const SDL_Event& e) override;
    ComponentType getComponentTypeId() const override;

protected:
    void draw(SDL_Renderer* renderer) override;
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;
    bool canShareRenderCache() const override { return false; }

    [[nodiscard]] bool isItemEnabled(size_t index) const;
    [[nodiscard]] int  nextSelectable(int from, int direction) const;
    [[nodiscard]] int  firstSelectable() const;
    [[nodiscard]] int  indexAt(int local_x, int local_y) const;
    [[nodiscard]] SDL_Rect itemRect(size_t index) const;
    void activate(size_t index);
    void selectIndex(int index);
    void selectCancel();

private:
    [[nodiscard]] menu_render::Font rowFont() const;

    std::string m_title;
    std::vector<Item> m_items;
    int m_columns = 1;
    int m_titleWidth = 0;
    uint32_t m_disabledMask = 0;
    int m_selected = -1;
    int m_selectedId = -1;
    int m_hovered = -1;
    int m_pressed = -1;
    int m_cancelId = -1;
    uint32_t m_timerMs = 0;
    uint32_t m_timerId = 0;

    SelectCallback m_onSelect;
    TickCallback m_onTick;
    RowCallback m_rowCallback;
};
