#pragma once

#include "panel.hpp"
#include "slider.hpp"
#include "text_input.hpp"
#include "SDL2/SDL.h"

import std.compat;

// Struktura reprezentująca współrzędne komórki
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

// Struktura reprezentująca zakres zaznaczenia
struct SelectionRange {
    CellCoord start;
    CellCoord end;
    
    [[nodiscard]] bool isValid() const {
        return start.isValid() && end.isValid();
    }
    
    // Zwraca znormalizowany zakres (start <= end)
    [[nodiscard]] SelectionRange normalized() const {
        return {
            {std::min(start.row, end.row), std::min(start.col, end.col)},
            {std::max(start.row, end.row), std::max(start.col, end.col)}
        };
    }
};

class StringGrid : public Panel {
public:
    // Konstruktor
    StringGrid(GUIManager& manager, int x, int y, int width, int height,
               size_t initialRows = 0, size_t initialCols = 0);
    
    ~StringGrid() override = default;
    
    // Zarządzanie danymi
    void setRowCount(size_t rows);
    void setColumnCount(size_t cols);
    [[nodiscard]] size_t getRowCount() const;
    [[nodiscard]] size_t getColumnCount() const;
    void setCellText(size_t row, size_t col, std::string_view text);
    [[nodiscard]] std::string_view getCellText(size_t row, size_t col) const;
    void clear();
    
    // Geometria kolumn
    void setColumnWidth(size_t col, int width);
    void setRowHeight(int height);
    void setHeaderHeight(int height);
    void setRowHeaderWidth(int width);
    
    // Nagłówki
    void setColumnHeader(size_t col, std::string_view text);
    void setShowRowHeaders(bool show);
    void setShowColumnHeaders(bool show);
    
    // Zaznaczanie
    void setSelectedCell(size_t row, size_t col);
    void setSelectionRange(size_t startRow, size_t startCol, size_t endRow, size_t endCol);
    void clearSelection();
    [[nodiscard]] std::optional<CellCoord> getSelectedCell() const;
    [[nodiscard]] std::optional<SelectionRange> getSelectionRange() const;
    
    // Edycja
    void setEditable(bool editable);
    [[nodiscard]] bool isEditable() const;
    void startEditing(size_t row, size_t col);
    void stopEditing();
    [[nodiscard]] bool isEditing() const;
    
    // Callbacki
    using CellCallback = std::function<void(StringGrid*, CellCoord)>;
    using EditCallback = std::function<void(StringGrid*, CellCoord, std::string)>;
    using SelectionCallback = std::function<void(StringGrid*, SelectionRange)>;
    
    void setOnCellClick(CellCallback callback);
    void setOnCellDoubleClick(CellCallback callback);
    void setOnCellEdit(EditCallback callback);
    void setOnSelectionChange(SelectionCallback callback);
    
    // Nadpisane metody GUIElement
    bool handleEvent(const SDL_Event& e) override;
    [[nodiscard]] const char* getComponentType() const override;
    
    // Overlay dla edytora komórki
    [[nodiscard]] bool isOverlay() const override;
    void renderOverlay(SDL_Renderer* renderer) override;

protected:
    [[nodiscard]] bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;

private:
    // Metody pomocnicze
    void setupSliders();
    void updateSliderRanges();
    [[nodiscard]] int getTotalContentWidth() const;
    [[nodiscard]] int getTotalContentHeight() const;
    [[nodiscard]] int getVisibleCellAreaWidth() const;
    [[nodiscard]] int getVisibleCellAreaHeight() const;
    [[nodiscard]] CellCoord getCellAtPosition(int x, int y) const;
    [[nodiscard]] SDL_Rect getCellRect(size_t row, size_t col) const;
    [[nodiscard]] SDL_Rect getHeaderRect(size_t col) const;
    [[nodiscard]] SDL_Rect getRowHeaderRect(size_t row) const;
    void drawCell(SDL_Renderer* renderer, size_t row, size_t col, int screenX, int screenY, int width, int height);
    void drawColumnHeaders(SDL_Renderer* renderer, int offsetX, int offsetY);
    void drawRowHeaders(SDL_Renderer* renderer, int offsetX, int offsetY);
    void drawSelection(SDL_Renderer* renderer, int offsetX, int offsetY);
    void drawGridLines(SDL_Renderer* renderer, int offsetX, int offsetY);
    void ensureCellVisible(size_t row, size_t col);
    
    // Dane
    std::vector<std::vector<std::string>> m_data;
    std::vector<int> m_columnWidths;
    std::vector<std::string> m_columnHeaders;
    
    // Geometria
    int m_rowHeight = 24;
    int m_headerHeight = 28;
    int m_rowHeaderWidth = 50;
    int m_hScrollOffset = 0;  // w pikselach
    int m_vScrollOffset = 0;  // w pikselach
    int m_sliderWidth = 16;
    
    // Opcje wyświetlania
    bool m_showRowHeaders = true;
    bool m_showColumnHeaders = true;
    
    // Zaznaczenie
    std::optional<CellCoord> m_selectedCell;
    std::optional<CellCoord> m_selectionStart;
    std::optional<CellCoord> m_selectionEnd;
    bool m_isSelecting = false;
    
    // Edycja
    std::unique_ptr<TextInput> m_cellEditor;
    CellCoord m_editingCell = CellCoord::invalid();
    bool m_isEditing = false;
    bool m_editable = true;
    
    // Dzieci (slidery)
    Slider* m_vSlider = nullptr;
    Slider* m_hSlider = nullptr;
    
    // Kolory
    SDL_Color m_cellBackgroundColor = {255, 255, 255, 255};
    SDL_Color m_selectionColor = {51, 153, 255, 100};
    SDL_Color m_gridLineColor = {200, 200, 200, 255};
    SDL_Color m_headerBackgroundColor = {240, 240, 240, 255};
    SDL_Color m_headerTextColor = {0, 0, 0, 255};
    SDL_Color m_textColor = {0, 0, 0, 255};
    SDL_Color m_selectedCellBorderColor = {51, 153, 255, 255};
    
    // Callbacki
    CellCallback m_onCellClick;
    CellCallback m_onCellDoubleClick;
    EditCallback m_onCellEdit;
    SelectionCallback m_onSelectionChange;
    
    // Czcionka
    static constexpr int DEFAULT_FONT_SIZE = 14;
};
