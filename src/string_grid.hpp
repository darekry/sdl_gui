#pragma once

#include "panel.hpp"
#include "slider.hpp"
#include "text_input.hpp"
#include "sdl_deleters.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "std.hpp"

enum class SortDirection { None, Ascending, Descending };

struct CellCoord {
    size_t row;
    size_t col;
    
    [[nodiscard]] bool isValid() const { return row != SIZE_MAX && col != SIZE_MAX; }
    static CellCoord invalid() { return {SIZE_MAX, SIZE_MAX}; }
    
    bool operator==(const CellCoord& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const CellCoord& other) const {
        return !(*this == other);
    }
};

struct SelectionRange {
    CellCoord start;
    CellCoord end;
    
    [[nodiscard]] bool isValid() const {
        return start.isValid() && end.isValid();
    }
    
    // Returns the normalized range (start <= end)
    [[nodiscard]] SelectionRange normalized() const {
        return {
            {std::min(start.row, end.row), std::min(start.col, end.col)},
            {std::max(start.row, end.row), std::max(start.col, end.col)}
        };
    }
};

struct VisibleRange {
    int startRow = 0;
    int endRow = 0;
    int startCol = 0;
    int endCol = 0;
};

class StringGrid : public Panel {
public:
    StringGrid(GUIManager& manager, int x, int y, int width, int height,
               size_t initialRows = 0, size_t initialCols = 0);
    
    ~StringGrid() override {
        // Clear pointers to children before Panel destroys them
        m_vSlider = nullptr;
        m_hSlider = nullptr;
        clearLocalTextureCache();
    }
    
    // Data management
    void setRowCount(size_t rows);
    void setColumnCount(size_t cols);
    [[nodiscard]] size_t getRowCount() const;
    [[nodiscard]] size_t getColumnCount() const;
    void setCellText(size_t row, size_t col, std::string_view text);
    [[nodiscard]] std::string_view getCellText(size_t row, size_t col) const;
    void clear();
    
    // Sorting
    void sortByColumn(size_t col, SortDirection dir);
    [[nodiscard]] SortDirection getSortDirection() const { return m_sortDirection; }
    [[nodiscard]] size_t getSortColumn() const { return m_sortColumn; }
    
    // Custom comparison function for sorting
    // Returns true if a should come before b (for ascending sort)
    using CompareFunc = std::function<bool(const std::string& a, const std::string& b)>;
    
    // Custom comparison functions for column sorting
    void setCustomComparator(size_t col, CompareFunc func);
    void clearCustomComparator(size_t col);
    void clearAllCustomComparators();
    [[nodiscard]] bool hasCustomComparator(size_t col) const;
    
    // Column geometry
    void setColumnWidth(size_t col, int width);
    void setRowHeight(int height);
    void setHeaderHeight(int height);
    void setRowHeaderWidth(int width);
    
    // Headers
    void setColumnHeader(size_t col, std::string_view text);
    void setShowRowHeaders(bool show);
    void setShowColumnHeaders(bool show);
    
    // Selection
    void setSelectedCell(size_t row, size_t col);
    void setSelectionRange(size_t startRow, size_t startCol, size_t endRow, size_t endCol);
    void clearSelection();
    [[nodiscard]] std::optional<CellCoord> getSelectedCell() const;
    [[nodiscard]] std::optional<SelectionRange> getSelectionRange() const;
    
    // Editing
    void setEditable(bool editable);
    [[nodiscard]] bool isEditable() const;
    
    // Slider control
    void setHorizontalScrollEnabled(bool enabled);
    void setVerticalScrollEnabled(bool enabled);
    [[nodiscard]] bool isHorizontalScrollEnabled() const;
    [[nodiscard]] bool isVerticalScrollEnabled() const;
    [[nodiscard]] int getVerticalSliderMax() const;
    [[nodiscard]] int getVerticalScrollOffset() const { return m_vScrollOffset; }
    [[nodiscard]] int getRowHeight() const { return m_rowHeight; }
    void startEditing(size_t row, size_t col);
    void stopEditing();
    [[nodiscard]] bool isEditing() const;
    
    // Setters for special colors
    void setSelectionColor(SDL_Color color);
    void setSelectedCellBorderColor(SDL_Color color);
    
    // Getters for special colors
    [[nodiscard]] SDL_Color getSelectionColor() const;
    [[nodiscard]] SDL_Color getSelectedCellBorderColor() const;
    
    // Callbacks
    using CellCallback = std::function<void(StringGrid*, CellCoord)>;
    using EditCallback = std::function<void(StringGrid*, CellCoord, std::string)>;
    using SelectionCallback = std::function<void(StringGrid*, SelectionRange)>;
    
    void setOnCellClick(CellCallback callback);
    void setOnCellDoubleClick(CellCallback callback);
    void setOnCellEdit(EditCallback callback);
    void setOnSelectionChange(SelectionCallback callback);
    
    // GUIElement overrides
    bool handleEvent(const SDL_Event& e) override;
    [[nodiscard]] ComponentType getComponentTypeId() const override;
    
    // Overlay for the cell editor
    [[nodiscard]] bool isOverlay() const override;
    void renderOverlay(SDL_Renderer* renderer) override;

protected:
    [[nodiscard]] bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;

private:
    // Helper methods
    void setupSliders();
    void updateSliderRanges();
    [[nodiscard]] int getTotalContentWidth() const;
    [[nodiscard]] int getTotalContentHeight() const;
    [[nodiscard]] int getVisibleCellAreaWidth() const;
    [[nodiscard]] int getVisibleCellAreaHeight() const;
    [[nodiscard]] CellCoord getCellAtPosition(int x, int y) const;
    [[nodiscard]] SDL_Rect getCellRect(size_t row, size_t col) const;
    void drawCell(SDL_Renderer* renderer, size_t row, size_t col, int screenX, int screenY, int width, int height,
                  SDL_Color cellBackgroundColor, SDL_Color textColor, TTF_Font* font);
    void drawColumnHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                           SDL_Color headerBackgroundColor, SDL_Color headerTextColor, SDL_Color gridLineColor,
                           TTF_Font* font);
    void drawRowHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                        SDL_Color headerBackgroundColor, SDL_Color headerTextColor, SDL_Color gridLineColor,
                        TTF_Font* font);
    void drawSelection(SDL_Renderer* renderer, int offsetX, int offsetY);
    void drawGridLines(SDL_Renderer* renderer, int offsetX, int offsetY, SDL_Color gridLineColor);
    void ensureCellVisible(size_t row, size_t col);
    
    // Helper methods for position calculations
    [[nodiscard]] int getColumnX(size_t col) const;
    [[nodiscard]] int getRowY(size_t row) const;
    [[nodiscard]] int getCellAreaX() const;
    [[nodiscard]] int getCellAreaY() const;
    [[nodiscard]] VisibleRange calculateVisibleRange() const;
    
    // Helper methods for drawing
    void drawCells(SDL_Renderer* renderer, int offsetX, int offsetY, 
                   SDL_Color cellBackgroundColor, SDL_Color textColor,
                   const VisibleRange& range, TTF_Font* font);
    
    // Helper methods for event handling
    bool handleMouseButtonDown(const SDL_Event& e);
    bool handleMouseMotion(const SDL_Event& e);
    bool handleMouseWheel(const SDL_Event& e);
    bool handleKeyboard(const SDL_Event& e);
    void handleHeaderClick(int localX, int localY);
    void copySelectionToClipboard();
    
    // Data
    std::vector<std::vector<std::string>> m_data;
    std::vector<int> m_columnWidths;
    std::vector<std::string> m_columnHeaders;
    
    // Geometry
    int m_rowHeight = 24;
    int m_headerHeight = 28;
    int m_rowHeaderWidth = 50;
    int m_hScrollOffset = 0;  // in pixels
    int m_vScrollOffset = 0;  // in pixels
    int m_sliderWidth = 16;
    
    // Display options
    bool m_showRowHeaders = true;
    bool m_showColumnHeaders = true;
    
    // Selection
    std::optional<CellCoord> m_selectedCell;
    std::optional<CellCoord> m_selectionStart;
    std::optional<CellCoord> m_selectionEnd;
    bool m_isSelecting = false;
    
    // Editing
    std::unique_ptr<TextInput> m_cellEditor;
    CellCoord m_editingCell = CellCoord::invalid();
    bool m_isEditing = false;
    bool m_editable = true;
    
    // Children (sliders)
    Slider* m_vSlider = nullptr;
    Slider* m_hSlider = nullptr;
    
    // Slider control
    bool m_hScrollEnabled = true;
    bool m_vScrollEnabled = true;
    
    // Colors (default values as fallback)
    SDL_Color m_cellBackgroundColor = {255, 255, 255, 255};
    SDL_Color m_selectionColor = {51, 153, 255, 100};
    SDL_Color m_gridLineColor = {200, 200, 200, 255};
    SDL_Color m_headerTextColor = {0, 0, 0, 255};
    SDL_Color m_textColor = {0, 0, 0, 255};
    SDL_Color m_selectedCellBorderColor = {51, 153, 255, 255};
    
    // Callbacks
    CellCallback m_onCellClick;
    CellCallback m_onCellDoubleClick;
    EditCallback m_onCellEdit;
    SelectionCallback m_onSelectionChange;
    
    // Sorting
    SortDirection m_sortDirection = SortDirection::None;
    size_t m_sortColumn = SIZE_MAX;
    
    // Custom comparison functions per column
    std::unordered_map<size_t, CompareFunc> m_customComparators;
    
    // Font
    static constexpr int DEFAULT_FONT_SIZE = 14;
    
    // Local cell texture cache (not in TextureManager)
    std::unordered_map<std::string, SharedTexture, StringHash, std::equal_to<>> m_localTextureCache;
    SharedTexture createLocalTextTexture(std::string_view text, TTF_Font* font, SDL_Color color);
    void clearLocalTextureCache();
};
