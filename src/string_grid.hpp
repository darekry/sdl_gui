#pragma once

#include "panel.hpp"
#include "slider.hpp"
#include "text_input.hpp"
#include "sdl_deleters.hpp"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

import std.compat;

// Enum reprezentujący kierunek sortowania
enum class SortDirection { None, Ascending, Descending };

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

// Struktura reprezentująca widoczny zakres komórek
struct VisibleRange {
    int startRow = 0;
    int endRow = 0;
    int startCol = 0;
    int endCol = 0;
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
    
    // Sortowanie
    void sortByColumn(size_t col, SortDirection dir);
    [[nodiscard]] SortDirection getSortDirection() const { return m_sortDirection; }
    [[nodiscard]] size_t getSortColumn() const { return m_sortColumn; }
    
    // Niestandardowa funkcja porównująca dla sortowania
    // Zwraca true, jeśli a powinno być przed b (dla sortowania rosnącego)
    using CompareFunc = std::function<bool(const std::string& a, const std::string& b)>;
    
    // Niestandardowe funkcje porównujące dla sortowania kolumn
    void setCustomComparator(size_t col, CompareFunc func);
    void clearCustomComparator(size_t col);
    void clearAllCustomComparators();
    [[nodiscard]] bool hasCustomComparator(size_t col) const;
    
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
    
    // Settery dla kolorów specjalnych
    void setSelectionColor(SDL_Color color);
    void setSelectedCellBorderColor(SDL_Color color);
    
    // Gettery dla kolorów specjalnych
    [[nodiscard]] SDL_Color getSelectionColor() const;
    [[nodiscard]] SDL_Color getSelectedCellBorderColor() const;
    
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
    void drawCell(SDL_Renderer* renderer, size_t row, size_t col, int screenX, int screenY, int width, int height,
                  SDL_Color cellBackgroundColor, SDL_Color textColor);
    void drawColumnHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                           SDL_Color headerBackgroundColor, SDL_Color headerTextColor, SDL_Color gridLineColor);
    void drawRowHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                        SDL_Color headerBackgroundColor, SDL_Color headerTextColor, SDL_Color gridLineColor);
    void drawSelection(SDL_Renderer* renderer, int offsetX, int offsetY);
    void drawGridLines(SDL_Renderer* renderer, int offsetX, int offsetY, SDL_Color gridLineColor);
    void ensureCellVisible(size_t row, size_t col);
    
    // Nowe metody pomocnicze do obliczania pozycji
    [[nodiscard]] int getColumnX(size_t col) const;
    [[nodiscard]] int getRowY(size_t row) const;
    [[nodiscard]] int getCellAreaX() const;
    [[nodiscard]] int getCellAreaY() const;
    [[nodiscard]] VisibleRange calculateVisibleRange() const;
    
    // Nowe metody pomocnicze do rysowania
    void drawCells(SDL_Renderer* renderer, int offsetX, int offsetY, 
                   SDL_Color cellBackgroundColor, SDL_Color textColor,
                   const VisibleRange& range);
    void renderText(SDL_Renderer* renderer, std::string_view text, int x, int y, 
                    SDL_Color color, bool centerX = false, bool centerY = false);
    
    // Nowe metody pomocnicze do obsługi zdarzeń
    bool handleMouseButtonDown(const SDL_Event& e);
    bool handleMouseMotion(const SDL_Event& e);
    bool handleMouseWheel(const SDL_Event& e);
    bool handleKeyboard(const SDL_Event& e);
    void handleHeaderClick(int localX, int localY);
    void copySelectionToClipboard();
    
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
    
    // Kolory (wartości domyślne jako fallback)
    SDL_Color m_cellBackgroundColor = {255, 255, 255, 255};
    SDL_Color m_selectionColor = {51, 153, 255, 100};
    SDL_Color m_gridLineColor = {200, 200, 200, 255};
    SDL_Color m_headerTextColor = {0, 0, 0, 255};
    SDL_Color m_textColor = {0, 0, 0, 255};
    SDL_Color m_selectedCellBorderColor = {51, 153, 255, 255};
    
    // Callbacki
    CellCallback m_onCellClick;
    CellCallback m_onCellDoubleClick;
    EditCallback m_onCellEdit;
    SelectionCallback m_onSelectionChange;
    
    // Sortowanie
    SortDirection m_sortDirection = SortDirection::None;
    size_t m_sortColumn = SIZE_MAX;
    
    // Niestandardowe funkcje porównujące dla kolumn
    std::map<size_t, CompareFunc> m_customComparators;
    
    // Czcionka
    static constexpr int DEFAULT_FONT_SIZE = 14;
    
    // Lokalny cache tekstur komórek (nie w TextureManager)
    std::map<std::string, SharedTexture> m_localTextureCache;
    SharedTexture createLocalTextTexture(std::string_view text, TTF_Font* font, SDL_Color color);
    void clearLocalTextureCache();
};
