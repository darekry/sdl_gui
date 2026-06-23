#include "string_grid.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include <SDL3_ttf/SDL_ttf.h>
#include "constants.hpp"

#include "std.hpp"

// Konstruktor
StringGrid::StringGrid(GUIManager& manager, int x, int y, int width, int height,
                       size_t initialRows, size_t initialCols)
    : Panel(manager, x, y, width, height) {
    
    setColumnCount(initialCols);
    setRowCount(initialRows);
    setupSliders();
    setClipChildren(true);
}

// Zarządzanie danymi
void StringGrid::setRowCount(size_t rows) {
    m_data.resize(rows);
    for (auto& row : m_data) {
        row.resize(m_columnWidths.size());
    }
    updateSliderRanges();
}

void StringGrid::setColumnCount(size_t cols) {
    m_columnWidths.resize(cols, 100);
    m_columnHeaders.resize(cols);
    for (size_t i = 0; i < cols; ++i) {
        if (m_columnHeaders[i].empty()) {
            m_columnHeaders[i] = std::to_string(i + 1);
        }
    }
    for (auto& row : m_data) {
        row.resize(cols);
    }
    updateSliderRanges();
}

size_t StringGrid::getRowCount() const {
    return m_data.size();
}

size_t StringGrid::getColumnCount() const {
    return m_columnWidths.size();
}

void StringGrid::setCellText(size_t row, size_t col, std::string_view text) {
    if (row < m_data.size() && col < m_data[row].size()) {
        m_data[row][col] = text;
    }
}

std::string_view StringGrid::getCellText(size_t row, size_t col) const {
    if (row < m_data.size() && col < m_data[row].size()) {
        return m_data[row][col];
    }
    return "";
}

void StringGrid::clear() {
    m_data.clear();
    clearSelection();
    clearLocalTextureCache();
}

void StringGrid::sortByColumn(size_t col, SortDirection dir) {
    if (col >= m_columnWidths.size()) {
        return;
    }
    
    m_sortColumn = col;
    m_sortDirection = dir;
    
    if (dir == SortDirection::None) {
        return;
    }
    
    // Sprawdzamy czy istnieje niestandardowy komparator dla tej kolumny
    bool hasCustom = m_customComparators.find(col) != m_customComparators.end();
    
    // Sortujemy dane
    std::sort(m_data.begin(), m_data.end(),
        [col, dir, hasCustom, this](const auto& rowA, const auto& rowB) {
            // Handle rows with different sizes - maintain consistent ordering
            // Rows with fewer columns come first (or last depending on direction)
            const size_t sizeA = rowA.size();
            const size_t sizeB = rowB.size();
            
            if (col >= sizeA && col >= sizeB) {
                // Both rows don't have this column - compare by row size
                return (dir == SortDirection::Ascending) ? (sizeA < sizeB) : (sizeA > sizeB);
            }
            if (col >= sizeA) {
                // rowA doesn't have this column - it comes first (ascending) or last (descending)
                return dir == SortDirection::Ascending;
            }
            if (col >= sizeB) {
                // rowB doesn't have this column - rowA comes last (ascending) or first (descending)
                return dir == SortDirection::Descending;
            }
            
            const auto& cellA = rowA[col];
            const auto& cellB = rowB[col];
            
            // Używamy niestandardowego komparatora jeśli istnieje
            if (hasCustom) {
                const auto& comparator = m_customComparators.at(col);
                if (dir == SortDirection::Ascending) {
                    return comparator(cellA, cellB);
                } else {
                    return comparator(cellB, cellA);  // Odwracamy kolejność dla malejącego
                }
            }
            
            // Domyślna logika: próbujemy porównać jako liczby
            try {
                double numA = std::stod(cellA);
                double numB = std::stod(cellB);
                if (dir == SortDirection::Ascending) {
                    return numA < numB;
                } else {
                    return numA > numB;
                }
            } catch (...) {
                // Jeśli nie są liczbami, porównujemy jako tekst
                if (dir == SortDirection::Ascending) {
                    return cellA < cellB;
                } else {
                    return cellA > cellB;
                }
            }
        });
}

// Zarządzanie niestandardowymi komparatorami
void StringGrid::setCustomComparator(size_t col, CompareFunc func) {
    m_customComparators[col] = std::move(func);
}

void StringGrid::clearCustomComparator(size_t col) {
    m_customComparators.erase(col);
}

void StringGrid::clearAllCustomComparators() {
    m_customComparators.clear();
}

bool StringGrid::hasCustomComparator(size_t col) const {
    return m_customComparators.find(col) != m_customComparators.end();
}

// Geometria kolumn
void StringGrid::setColumnWidth(size_t col, int width) {
    if (col < m_columnWidths.size()) {
        m_columnWidths[col] = std::max(20, width);
        updateSliderRanges();
    }
}

void StringGrid::setRowHeight(int height) {
    m_rowHeight = std::max(16, height);
    updateSliderRanges();
}

void StringGrid::setHeaderHeight(int height) {
    m_headerHeight = std::max(16, height);
}

void StringGrid::setRowHeaderWidth(int width) {
    m_rowHeaderWidth = std::max(20, width);
}

// Nagłówki
void StringGrid::setColumnHeader(size_t col, std::string_view text) {
    if (col < m_columnHeaders.size()) {
        m_columnHeaders[col] = text;
    }
}

void StringGrid::setShowRowHeaders(bool show) {
    m_showRowHeaders = show;
}

void StringGrid::setShowColumnHeaders(bool show) {
    m_showColumnHeaders = show;
}

// Zaznaczanie
void StringGrid::setSelectedCell(size_t row, size_t col) {
    if (row < m_data.size() && col < m_columnWidths.size()) {
        m_selectedCell = {row, col};
        m_selectionStart = {row, col};
        m_selectionEnd = {row, col};
        ensureCellVisible(row, col);
        if (m_onSelectionChange) {
            m_onSelectionChange(this, {m_selectedCell.value(), m_selectedCell.value()});
        }
    }
}

void StringGrid::setSelectionRange(size_t startRow, size_t startCol, size_t endRow, size_t endCol) {
    if (startRow < m_data.size() && startCol < m_columnWidths.size() &&
        endRow < m_data.size() && endCol < m_columnWidths.size()) {
        m_selectionStart = {startRow, startCol};
        m_selectionEnd = {endRow, endCol};
        m_selectedCell = {startRow, startCol};
        if (m_onSelectionChange) {
            m_onSelectionChange(this, {m_selectionStart.value(), m_selectionEnd.value()});
        }
    }
}

void StringGrid::clearSelection() {
    m_selectedCell = std::nullopt;
    m_selectionStart = std::nullopt;
    m_selectionEnd = std::nullopt;
    m_isSelecting = false;
}

std::optional<CellCoord> StringGrid::getSelectedCell() const {
    return m_selectedCell;
}

std::optional<SelectionRange> StringGrid::getSelectionRange() const {
    if (m_selectionStart && m_selectionEnd) {
        return SelectionRange{m_selectionStart.value(), m_selectionEnd.value()};
    }
    return std::nullopt;
}

// Edycja
void StringGrid::setEditable(bool editable) {
    m_editable = editable;
    if (!m_editable && m_isEditing) {
        stopEditing();
    }
}

bool StringGrid::isEditable() const {
    return m_editable;
}

void StringGrid::startEditing(size_t row, size_t col) {
    if (!m_editable || row >= m_data.size() || col >= m_columnWidths.size()) {
        return;
    }
    
    stopEditing();
    
    m_editingCell = {row, col};
    m_isEditing = true;
    
    SDL_Rect cellRect = getCellRect(row, col);
    m_cellEditor = std::make_unique<TextInput>(m_manager, cellRect.x, cellRect.y, 
                                                 cellRect.w, cellRect.h);
    m_cellEditor->setText(getCellText(row, col));
    m_cellEditor->setOnEnterPressed([this](TextInput*) {
        stopEditing();
    });
    
    m_manager.setKeyboardFocus(m_cellEditor.get());
}

void StringGrid::stopEditing() {
    if (m_isEditing && m_cellEditor && m_editingCell.isValid()) {
        std::string newText = m_cellEditor->getText();
        if (m_editingCell.row < m_data.size() && m_editingCell.col < m_data[m_editingCell.row].size()) {
            m_data[m_editingCell.row][m_editingCell.col] = newText;
            if (m_onCellEdit) {
                m_onCellEdit(this, m_editingCell, newText);
            }
        }
    }
    
    if (m_cellEditor && m_manager.getKeyboardFocus() == m_cellEditor.get()) {
        m_manager.setKeyboardFocus(nullptr);
    }
    
    m_isEditing = false;
    m_editingCell = CellCoord::invalid();
    m_cellEditor.reset();
}

bool StringGrid::isEditing() const {
    return m_isEditing;
}

// Kontrola sliderów
void StringGrid::setHorizontalScrollEnabled(bool enabled) {
    m_hScrollEnabled = enabled;
    if (m_hSlider) {
        m_hSlider->setVisible(enabled);
    }
    if (!enabled) {
        m_hScrollOffset = 0;
    }
    updateSliderRanges();
}

void StringGrid::setVerticalScrollEnabled(bool enabled) {
    m_vScrollEnabled = enabled;
    if (m_vSlider) {
        m_vSlider->setVisible(enabled);
    }
    if (!enabled) {
        m_vScrollOffset = 0;
    }
    updateSliderRanges();
}

bool StringGrid::isHorizontalScrollEnabled() const {
    return m_hScrollEnabled;
}

bool StringGrid::isVerticalScrollEnabled() const {
    return m_vScrollEnabled;
}

// Settery dla kolorów specjalnych
void StringGrid::setSelectionColor(SDL_Color color) {
    m_selectionColor = color;
}

void StringGrid::setSelectedCellBorderColor(SDL_Color color) {
    m_selectedCellBorderColor = color;
}

// Gettery dla kolorów specjalnych
SDL_Color StringGrid::getSelectionColor() const {
    return m_selectionColor;
}

SDL_Color StringGrid::getSelectedCellBorderColor() const {
    return m_selectedCellBorderColor;
}

// Callbacki
void StringGrid::setOnCellClick(CellCallback callback) {
    m_onCellClick = std::move(callback);
}

void StringGrid::setOnCellDoubleClick(CellCallback callback) {
    m_onCellDoubleClick = std::move(callback);
}

void StringGrid::setOnCellEdit(EditCallback callback) {
    m_onCellEdit = std::move(callback);
}

void StringGrid::setOnSelectionChange(SelectionCallback callback) {
    m_onSelectionChange = std::move(callback);
}

// Nadpisane metody GUIElement
const char* StringGrid::getComponentType() const {
    return "StringGrid";
}

bool StringGrid::isOverlay() const {
    return m_isEditing;
}

void StringGrid::renderOverlay(SDL_Renderer* renderer) {
    if (m_isEditing && m_cellEditor) {
        m_cellEditor->render(renderer);
    }
}

// Event handler helpers
bool StringGrid::handleMouseButtonDown(const SDL_Event& e) {
    auto absPos = getAbsolutePosition();
    int localX = static_cast<int>(e.button.x) - absPos.x;
    int localY = static_cast<int>(e.button.y) - absPos.y;
    
    int cellAreaX = getCellAreaX();
    int cellAreaY = getCellAreaY();
    
    // Sprawdź czy kliknięcie było w nagłówku kolumny
    if (m_showColumnHeaders && localY < m_headerHeight && localX >= cellAreaX) {
        handleHeaderClick(localX, localY);
        return true;
    }
    
    if (localX < cellAreaX || localY < cellAreaY) {
        return false;
    }
    
    CellCoord cell = getCellAtPosition(localX, localY);
    if (!cell.isValid()) {
        return false;
    }
    
    if (m_onCellClick) {
        m_onCellClick(this, cell);
    }
    
    // Check for double click
    static Uint64 lastClickTime = 0;
    static CellCoord lastClickCell = CellCoord::invalid();
    Uint64 currentTime = SDL_GetTicks();
    
    if (currentTime - lastClickTime < 500 && cell == lastClickCell) {
        if (m_editable) {
            startEditing(cell.row, cell.col);
        }
        if (m_onCellDoubleClick) {
            m_onCellDoubleClick(this, cell);
        }
    } else {
        setSelectedCell(cell.row, cell.col);
        m_isSelecting = true;
    }
    
    lastClickTime = currentTime;
    lastClickCell = cell;
    return true;
}

bool StringGrid::handleMouseMotion(const SDL_Event& e) {
    if (!m_isSelecting) {
        return false;
    }
    
    auto absPos = getAbsolutePosition();
    int localX = static_cast<int>(e.motion.x) - absPos.x;
    int localY = static_cast<int>(e.motion.y) - absPos.y;
    
    CellCoord cell = getCellAtPosition(localX, localY);
    if (cell.isValid() && m_selectedCell) {
        m_selectionEnd = cell;
        if (m_onSelectionChange) {
            m_onSelectionChange(this, {m_selectionStart.value(), m_selectionEnd.value()});
        }
    }
    return true;
}

bool StringGrid::handleMouseWheel(const SDL_Event& e) {
    int scrollAmount = static_cast<int>(e.wheel.y * 20.0f);
    
    if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
        scrollAmount = -scrollAmount;
    }
    
    int mouseX = static_cast<int>(e.wheel.mouse_x);
    int mouseY = static_cast<int>(e.wheel.mouse_y);
    auto absPos = getAbsolutePosition();
    
    if (mouseX < absPos.x || mouseX >= absPos.x + m_width ||
        mouseY < absPos.y || mouseY >= absPos.y + m_height) {
        return false;
    }
    
    int maxVScroll = std::max(0, getTotalContentHeight() - getVisibleCellAreaHeight());
    m_vScrollOffset = std::clamp(m_vScrollOffset - scrollAmount, 0, maxVScroll);
    
    if (m_vSlider) {
        m_vSlider->setValue(m_vScrollOffset);
    }
    return true;
}

bool StringGrid::handleKeyboard(const SDL_Event& e) {
    // Obsługa Ctrl+C - kopiowanie zaznaczenia do schowka
    if (e.key.key == SDLK_C && (SDL_GetModState() & SDL_KMOD_CTRL)) {
        copySelectionToClipboard();
        return true;
    }
    
    if (!m_selectedCell) {
        return false;
    }
    
    bool handled = false;
    size_t newRow = m_selectedCell->row;
    size_t newCol = m_selectedCell->col;
    
    switch (e.key.key) {
        case SDLK_UP:
            if (newRow > 0) {
                --newRow;
                handled = true;
            }
            break;
        case SDLK_DOWN:
            if (newRow < m_data.size() - 1) {
                ++newRow;
                handled = true;
            }
            break;
        case SDLK_LEFT:
            if (newCol > 0) {
                --newCol;
                handled = true;
            }
            break;
        case SDLK_RIGHT:
            if (newCol < m_columnWidths.size() - 1) {
                ++newCol;
                handled = true;
            }
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (m_editable && !m_isEditing) {
                startEditing(m_selectedCell->row, m_selectedCell->col);
                handled = true;
            }
            break;
        case SDLK_ESCAPE:
            if (m_isEditing) {
                stopEditing();
                handled = true;
            }
            break;
        default:
            break;
    }
    
    if (handled && (newRow != m_selectedCell->row || newCol != m_selectedCell->col)) {
        setSelectedCell(newRow, newCol);
    }
    
    return handled;
}

void StringGrid::copySelectionToClipboard() {
    if (!m_selectedCell) {
        return;
    }
    
    std::string clipboardText;
    
    // Określamy zakres do skopiowania
    SelectionRange range;
    if (m_selectionStart && m_selectionEnd) {
        range = SelectionRange{m_selectionStart.value(), m_selectionEnd.value()}.normalized();
    } else {
        range = {m_selectedCell.value(), m_selectedCell.value()};
    }
    
    // Budujemy tekst do schowka
    for (size_t row = range.start.row; row <= range.end.row; ++row) {
        for (size_t col = range.start.col; col <= range.end.col; ++col) {
            if (col < m_data[row].size()) {
                clipboardText += m_data[row][col];
            }
            if (col < range.end.col) {
                clipboardText += "\t";  // Tabulacja jako separator kolumn
            }
        }
        if (row < range.end.row) {
            clipboardText += "\n";  // Nowa linia jako separator wierszy
        }
    }
    
    SDL_SetClipboardText(clipboardText.c_str());
}

void StringGrid::handleHeaderClick(int localX, int localY) {
    if (!m_showColumnHeaders) {
        return;
    }
    
    int cellAreaX = getCellAreaX();
    
    // Sprawdzamy czy kliknięcie było w obszarze nagłówków kolumn
    if (localY < 0 || localY >= m_headerHeight || localX < cellAreaX) {
        return;
    }
    
    // Znajdujemy kolumnę
    int x = cellAreaX - m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = m_columnWidths[col];
        if (localX >= x && localX < x + colWidth) {
            // Cykl sortowania: None -> Ascending -> Descending -> None
            if (m_sortColumn == col) {
                switch (m_sortDirection) {
                    case SortDirection::None:
                        m_sortDirection = SortDirection::Ascending;
                        break;
                    case SortDirection::Ascending:
                        m_sortDirection = SortDirection::Descending;
                        break;
                    case SortDirection::Descending:
                        m_sortDirection = SortDirection::None;
                        m_sortColumn = SIZE_MAX;
                        break;
                }
            } else {
                m_sortColumn = col;
                m_sortDirection = SortDirection::Ascending;
            }
            
            // Wykonujemy sortowanie
            if (m_sortDirection != SortDirection::None) {
                sortByColumn(m_sortColumn, m_sortDirection);
            }
            return;
        }
        x += colWidth;
    }
}

// Obsługa zdarzeń
bool StringGrid::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }
    
    if (m_vSlider && m_vScrollEnabled && m_vSlider->handleEvent(e)) {
        return true;
    }
    if (m_hSlider && m_hScrollEnabled && m_hSlider->handleEvent(e)) {
        return true;
    }
    
    // Pass to cell editor if active
    if (m_isEditing && m_cellEditor) {
        if (m_cellEditor->handleEvent(e)) {
            return true;
        }
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            SDL_Rect cellRect = getCellRect(m_editingCell.row, m_editingCell.col);
            auto absPos = getAbsolutePosition();
            SDL_Rect absCellRect = {absPos.x + cellRect.x, absPos.y + cellRect.y, cellRect.w, cellRect.h};
            SDL_Point mousePoint = {static_cast<int>(e.button.x), static_cast<int>(e.button.y)};
            if (!SDL_PointInRect(&mousePoint, &absCellRect)) {
                stopEditing();
            }
        }
    }
    
    // Handle mouse events
    switch (e.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                if (handleMouseButtonDown(e)) {
                    return true;
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button == SDL_BUTTON_LEFT) {
                m_isSelecting = false;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (handleMouseMotion(e)) {
                return true;
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            if (handleMouseWheel(e)) {
                return true;
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if (handleKeyboard(e)) {
                return true;
            }
            break;
    }
    
    return Panel::handleEvent(e);
}

// Renderowanie bezpośrednie
void StringGrid::drawDirect(SDL_Renderer* renderer) {
    Style style = getComposedStyle(m_state);
    
    SDL_Color cellBackgroundColor = style.backgroundColor.value_or(m_cellBackgroundColor);
    SDL_Color textColor = style.textColor.value_or(m_textColor);
    SDL_Color gridLineColor = style.borderColor.value_or(m_gridLineColor);
    SDL_Color headerTextColor = style.textColor.value_or(m_headerTextColor);
    
    SDL_Color headerBackgroundColor = {
        static_cast<Uint8>(std::max(0, cellBackgroundColor.r - 15)),
        static_cast<Uint8>(std::max(0, cellBackgroundColor.g - 15)),
        static_cast<Uint8>(std::max(0, cellBackgroundColor.b - 15)),
        cellBackgroundColor.a
    };
    
    auto absPos = getAbsolutePosition();
    int offsetX = absPos.x;
    int offsetY = absPos.y;

    auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, DEFAULT_FONT_SIZE);
    
    drawBackgroundAndBorder(renderer);
    
    int cellAreaX = getCellAreaX();
    int cellAreaY = getCellAreaY();
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    SDL_Rect cellClipRect = {offsetX + cellAreaX, offsetY + cellAreaY, cellAreaWidth, cellAreaHeight};
    SDL_SetRenderClipRect(renderer, &cellClipRect);
    
    VisibleRange range = calculateVisibleRange();
    drawCells(renderer, offsetX, offsetY, cellBackgroundColor, textColor, range, font.get());
    drawSelection(renderer, offsetX, offsetY);
    
    SDL_SetRenderClipRect(renderer, nullptr);
    
    if (m_showColumnHeaders) {
        drawColumnHeaders(renderer, offsetX, offsetY, headerBackgroundColor, headerTextColor, gridLineColor, font.get());
    }
    
    if (m_showRowHeaders) {
        drawRowHeaders(renderer, offsetX, offsetY, headerBackgroundColor, headerTextColor, gridLineColor, font.get());
    }
    
    drawGridLines(renderer, offsetX, offsetY, gridLineColor);
}

// Drawing helper methods
void StringGrid::drawCells(SDL_Renderer* renderer, int offsetX, int offsetY,
                           SDL_Color cellBackgroundColor, SDL_Color textColor,
                           const VisibleRange& range, TTF_Font* font) {
    int cellAreaY = getCellAreaY();
    int rowY = cellAreaY - m_vScrollOffset + (range.startRow * m_rowHeight);
    
    for (int row = range.startRow; row < range.endRow && static_cast<size_t>(row) < m_data.size(); ++row) {
        int colX = getColumnX(static_cast<size_t>(range.startCol));
        
        for (int col = range.startCol; col < range.endCol && static_cast<size_t>(col) < m_columnWidths.size(); ++col) {
            int colWidth = static_cast<int>(m_columnWidths[static_cast<size_t>(col)]);
            drawCell(renderer, static_cast<size_t>(row), static_cast<size_t>(col),
                    offsetX + colX, offsetY + rowY, colWidth, m_rowHeight,
                    cellBackgroundColor, textColor, font);
            colX += colWidth;
        }
        rowY += m_rowHeight;
    }
}

void StringGrid::renderText(SDL_Renderer* renderer, std::string_view text, int x, int y,
                            SDL_Color color, bool centerX, bool centerY) {
    if (text.empty()) return;
    
    auto font = m_manager.getFontManager().loadFont(constants::kDefaultFontPath, DEFAULT_FONT_SIZE);
    if (!font) return;
    
    auto texture = createLocalTextTexture(text, font.get(), color);
    if (!texture) return;
    
    int textWidth, textHeight;
    { textWidth = TextureWidth(texture.get()); textHeight = TextureHeight(texture.get()); }
    
    int drawX = centerX ? x - textWidth / 2 : x;
    int drawY = centerY ? y - textHeight / 2 : y;
    
    SDL_Rect destRect = {drawX, drawY, textWidth, textHeight};
    RenderTexture(renderer, texture.get(), destRect);
}

// Metody pomocnicze
void StringGrid::setupSliders() {
    int sliderX = m_width - m_sliderWidth;
    int sliderY = m_showColumnHeaders ? m_headerHeight : 0;
    int sliderHeight = m_height - sliderY - m_sliderWidth;
    
    auto vSlider = std::make_unique<Slider>(m_manager, sliderX, sliderY, 
                                             m_sliderWidth, sliderHeight,
                                             0, 1000, 0, Orientation::Vertical);
    m_vSlider = vSlider.get();
    m_vSlider->setVisible(m_vScrollEnabled);
    m_vSlider->setOnChangeCallback([this](GUIElement*) {
        m_vScrollOffset = m_vSlider->getValue();
    });
    addChild(std::move(vSlider));
    
    int hSliderY = m_height - m_sliderWidth;
    int hSliderX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int sliderWidth = m_width - hSliderX - m_sliderWidth;
    
    auto hSlider = std::make_unique<Slider>(m_manager, hSliderX, hSliderY,
                                             sliderWidth, m_sliderWidth,
                                             0, 1000, 0, Orientation::Horizontal);
    m_hSlider = hSlider.get();
    m_hSlider->setVisible(m_hScrollEnabled);
    m_hSlider->setOnChangeCallback([this](GUIElement*) {
        m_hScrollOffset = m_hSlider->getValue();
    });
    addChild(std::move(hSlider));
    
    updateSliderRanges();
}

void StringGrid::updateSliderRanges() {
    int totalWidth = getTotalContentWidth();
    int totalHeight = getTotalContentHeight();
    int visibleWidth = getVisibleCellAreaWidth();
    int visibleHeight = getVisibleCellAreaHeight();
    
    if (m_vSlider && m_vScrollEnabled) {
        int maxVScroll = std::max(0, totalHeight - visibleHeight);
        m_vSlider->setRange(0, maxVScroll);
        m_vSlider->setValue(std::min(m_vScrollOffset, maxVScroll));
    }
    
    if (m_hSlider && m_hScrollEnabled) {
        int maxHScroll = std::max(0, totalWidth - visibleWidth);
        m_hSlider->setRange(0, maxHScroll);
        m_hSlider->setValue(std::min(m_hScrollOffset, maxHScroll));
    }
}

int StringGrid::getTotalContentWidth() const {
    int width = 0;
    for (int w : m_columnWidths) {
        width += w;
    }
    return width;
}

int StringGrid::getTotalContentHeight() const {
    return static_cast<int>(m_data.size()) * m_rowHeight;
}

int StringGrid::getVisibleCellAreaWidth() const {
    int width = m_width;
    if (m_showRowHeaders) {
        width -= m_rowHeaderWidth;
    }
    if (m_vSlider && m_vScrollEnabled) {
        width -= m_sliderWidth;
    }
    return std::max(0, width);
}

int StringGrid::getVisibleCellAreaHeight() const {
    int height = m_height;
    if (m_showColumnHeaders) {
        height -= m_headerHeight;
    }
    if (m_hSlider && m_hScrollEnabled) {
        height -= m_sliderWidth;
    }
    return std::max(0, height);
}

int StringGrid::getVerticalSliderMax() const {
    if (!m_vSlider || !m_vScrollEnabled) {
        return 0;
    }
    return m_vSlider->getMax();
}

// Helper methods for position calculations
int StringGrid::getColumnX(size_t col) const {
    int x = getCellAreaX() - m_hScrollOffset;
    for (size_t c = 0; c < col; ++c) {
        x += m_columnWidths[c];
    }
    return x;
}

int StringGrid::getRowY(size_t row) const {
    return getCellAreaY() - m_vScrollOffset + static_cast<int>(row) * m_rowHeight;
}

int StringGrid::getCellAreaX() const {
    return m_showRowHeaders ? m_rowHeaderWidth : 0;
}

int StringGrid::getCellAreaY() const {
    return m_showColumnHeaders ? m_headerHeight : 0;
}

VisibleRange StringGrid::calculateVisibleRange() const {
    VisibleRange range;
    range.endRow = static_cast<int>(m_data.size());
    range.endCol = static_cast<int>(m_columnWidths.size());
    
    int cellAreaX = getCellAreaX();
    int cellAreaY = getCellAreaY();
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    // Calculate start column
    int colOffset = -m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = m_columnWidths[col];
        if (colOffset + colWidth >= cellAreaX) {
            range.startCol = static_cast<int>(col);
            break;
        }
        colOffset += colWidth;
    }
    
    // Calculate end column
    colOffset = -m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        colOffset += m_columnWidths[col];
        if (colOffset > cellAreaX + cellAreaWidth) {
            range.endCol = static_cast<int>(col);
            break;
        }
    }
    
    // Calculate start row
    int rowY = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        if (rowY + m_rowHeight >= cellAreaY) {
            range.startRow = static_cast<int>(row);
            break;
        }
        rowY += m_rowHeight;
    }
    
    // Calculate end row
    rowY = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        rowY += m_rowHeight;
        if (rowY > cellAreaY + cellAreaHeight) {
            range.endRow = static_cast<int>(row);
            break;
        }
    }
    
    return range;
}

CellCoord StringGrid::getCellAtPosition(int x, int y) const {
    int cellAreaX = getCellAreaX();
    int cellAreaY = getCellAreaY();
    
    if (x < cellAreaX || y < cellAreaY) {
        return CellCoord::invalid();
    }
    
    // Find column
    int colX = cellAreaX - m_hScrollOffset;
    size_t col = 0;
    for (; col < m_columnWidths.size(); ++col) {
        int colEnd = colX + static_cast<int>(m_columnWidths[col]);
        if (x >= colX && x < colEnd) {
            break;
        }
        colX = colEnd;
    }
    
    if (col >= m_columnWidths.size()) {
        return CellCoord::invalid();
    }
    
    // Find row
    int rowY = cellAreaY - m_vScrollOffset;
    size_t row = 0;
    for (; row < m_data.size(); ++row) {
        int rowEnd = rowY + m_rowHeight;
        if (y >= rowY && y < rowEnd) {
            break;
        }
        rowY = rowEnd;
    }
    
    if (row >= m_data.size()) {
        return CellCoord::invalid();
    }
    
    return {row, col};
}

SDL_Rect StringGrid::getCellRect(size_t row, size_t col) const {
    return {getColumnX(col), getRowY(row), m_columnWidths[col], m_rowHeight};
}

SDL_Rect StringGrid::getHeaderRect(size_t col) const {
    return {getColumnX(col), 0, m_columnWidths[col], m_headerHeight};
}

SDL_Rect StringGrid::getRowHeaderRect(size_t row) const {
    return {0, getRowY(row), m_rowHeaderWidth, m_rowHeight};
}

void StringGrid::drawCell(SDL_Renderer* renderer, size_t row, size_t col,
                         int screenX, int screenY, int width, int height,
                         SDL_Color cellBackgroundColor, SDL_Color textColor, TTF_Font* font) {
    SetDrawColor(renderer, cellBackgroundColor);
    SDL_Rect cellRect = {screenX, screenY, width, height};
    RenderFillRect(renderer, cellRect);
    
    if (row < m_data.size() && col < m_data[row].size() && !m_data[row][col].empty()) {
        if (!font) return;
        
        auto texture = createLocalTextTexture(m_data[row][col], font, textColor);
        if (!texture) return;
        
        int textWidth, textHeight;
        { textWidth = TextureWidth(texture.get()); textHeight = TextureHeight(texture.get()); }
        
        int textX = screenX + 4;
        int textY = screenY + (height - textHeight) / 2;
        
        SDL_Rect textClip = {screenX + 2, screenY, width - 4, height};
        SDL_SetRenderClipRect(renderer, &textClip);
        
        SDL_Rect destRect = {textX, textY, textWidth, textHeight};
        RenderTexture(renderer, texture.get(), destRect);
        
        SDL_SetRenderClipRect(renderer, nullptr);
    }
}

void StringGrid::drawColumnHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                                   SDL_Color headerBackgroundColor, SDL_Color headerTextColor,
                                   SDL_Color gridLineColor, TTF_Font* font) {
    // Podświetlenie aktywnej kolumny sortowania
    if (m_sortDirection != SortDirection::None && m_sortColumn < m_columnWidths.size()) {
        int activeX = getCellAreaX() - m_hScrollOffset;
        for (size_t c = 0; c < m_sortColumn; ++c) {
            activeX += m_columnWidths[c];
        }
        if (activeX + static_cast<int>(m_columnWidths[m_sortColumn]) >= getCellAreaX()) {
            SDL_SetRenderDrawColor(renderer,
                static_cast<Uint8>(std::min(255, headerBackgroundColor.r + 30)),
                static_cast<Uint8>(std::min(255, headerBackgroundColor.g + 30)),
                static_cast<Uint8>(std::min(255, headerBackgroundColor.b + 30)),
                headerBackgroundColor.a);
            SDL_Rect activeRect = {offsetX + activeX, offsetY,
                                   static_cast<int>(m_columnWidths[m_sortColumn]), m_headerHeight};
            RenderFillRect(renderer, activeRect);
        }
    }
    
    SetDrawColor(renderer, headerBackgroundColor);
    int headerBgX = offsetX + getCellAreaX();
    SDL_Rect headerBgRect = {headerBgX, offsetY, getVisibleCellAreaWidth(), m_headerHeight};
    RenderFillRect(renderer, headerBgRect);
    
    int x = getCellAreaX() - m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = m_columnWidths[col];
        
        if (x + colWidth >= getCellAreaX() && x < m_width - (m_vSlider ? m_sliderWidth : 0)) {
            SDL_Rect headerRect = {offsetX + x, offsetY, colWidth, m_headerHeight};
            RenderFillRect(renderer, headerRect);
            
            if (font && col < m_columnHeaders.size() && !m_columnHeaders[col].empty()) {
                std::string headerText = m_columnHeaders[col];
                
                if (m_sortColumn == col && m_sortDirection != SortDirection::None) {
                    headerText += (m_sortDirection == SortDirection::Ascending) ? " \u2191" : " \u2193";
                }
                
                auto texture = createLocalTextTexture(headerText, font, headerTextColor);
                if (texture) {
                    int textWidth, textHeight;
                    { textWidth = TextureWidth(texture.get()); textHeight = TextureHeight(texture.get()); }
                    
                    if (textWidth > colWidth - 8) {
                        textWidth = colWidth - 8;
                    }
                    
                    int textX = offsetX + x + (colWidth - textWidth) / 2;
                    int textY = offsetY + (m_headerHeight - textHeight) / 2;
                    
                    SDL_Rect destRect = {textX, textY, textWidth, textHeight};
                    RenderTexture(renderer, texture.get(), destRect);
                }
            }
            
            SDL_SetRenderDrawColor(renderer, gridLineColor.r, gridLineColor.g,
                                   gridLineColor.b, gridLineColor.a);
            RenderLine(renderer, offsetX + x + colWidth - 1, offsetY,
                              offsetX + x + colWidth - 1, offsetY + m_headerHeight);
        }
        
        x += colWidth;
    }
    
    SetDrawColor(renderer, gridLineColor);
    int lineStartX = offsetX + getCellAreaX();
    int lineEndX = offsetX + m_width - (m_vSlider ? m_sliderWidth : 0);
    RenderLine(renderer, lineStartX, offsetY + m_headerHeight - 1,
                       lineEndX, offsetY + m_headerHeight - 1);
}

void StringGrid::drawRowHeaders(SDL_Renderer* renderer, int offsetX, int offsetY,
                                SDL_Color headerBackgroundColor, SDL_Color headerTextColor,
                                SDL_Color gridLineColor, TTF_Font* font) {
    SetDrawColor(renderer, headerBackgroundColor);
    int headerBgY = offsetY + getCellAreaY();
    SDL_Rect headerBgRect = {offsetX, headerBgY, m_rowHeaderWidth, getVisibleCellAreaHeight()};
    RenderFillRect(renderer, headerBgRect);
    
    int y = getCellAreaY() - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        if (y + m_rowHeight >= getCellAreaY() && y < m_height - (m_hSlider ? m_sliderWidth : 0)) {
            SDL_Rect headerRect = {offsetX, offsetY + y, m_rowHeaderWidth, m_rowHeight};
            RenderFillRect(renderer, headerRect);
            
            if (font) {
                std::string rowLabel = std::to_string(row + 1);
                auto texture = createLocalTextTexture(rowLabel, font, headerTextColor);
                if (texture) {
                    int textWidth, textHeight;
                    { textWidth = TextureWidth(texture.get()); textHeight = TextureHeight(texture.get()); }
                    
                    int textX = offsetX + (m_rowHeaderWidth - textWidth) / 2;
                    int textY = offsetY + y + (m_rowHeight - textHeight) / 2;
                    
                    SDL_Rect destRect = {textX, textY, textWidth, textHeight};
                    RenderTexture(renderer, texture.get(), destRect);
                }
            }
            
            SDL_SetRenderDrawColor(renderer, gridLineColor.r, gridLineColor.g,
                                   gridLineColor.b, gridLineColor.a);
            RenderLine(renderer, offsetX, offsetY + y + m_rowHeight - 1, 
                              offsetX + m_rowHeaderWidth, offsetY + y + m_rowHeight - 1);
        }
        y += m_rowHeight;
    }
    
    SetDrawColor(renderer, gridLineColor);
    int lineStartY = offsetY + getCellAreaY();
    int lineEndY = offsetY + m_height - (m_hSlider ? m_sliderWidth : 0);
    RenderLine(renderer, offsetX + m_rowHeaderWidth - 1, lineStartY,
                       offsetX + m_rowHeaderWidth - 1, lineEndY);
}

void StringGrid::drawSelection(SDL_Renderer* renderer, int offsetX, int offsetY) {
    if (!m_selectedCell) {
        return;
    }
    
    SelectionRange range;
    if (m_selectionStart && m_selectionEnd) {
        range = SelectionRange{m_selectionStart.value(), m_selectionEnd.value()}.normalized();
    } else {
        range = {m_selectedCell.value(), m_selectedCell.value()};
    }
    
    int startX = getColumnX(range.start.col);
    int startY = getRowY(range.start.row);
    
    int selWidth = 0;
    for (size_t col = range.start.col; col <= range.end.col; ++col) {
        selWidth += m_columnWidths[col];
    }
    
    int selHeight = static_cast<int>(range.end.row - range.start.row + 1) * m_rowHeight;
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SetDrawColor(renderer, m_selectionColor);
    SDL_Rect selRect = {offsetX + startX, offsetY + startY, selWidth, selHeight};
    RenderFillRect(renderer, selRect);
    
    if (m_selectedCell) {
        SDL_Rect activeCellRect = getCellRect(m_selectedCell->row, m_selectedCell->col);
        SDL_Rect absCellRect = {offsetX + activeCellRect.x, offsetY + activeCellRect.y, 
                                activeCellRect.w, activeCellRect.h};
        SetDrawColor(renderer, m_selectedCellBorderColor);
        RenderRect(renderer, absCellRect);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void StringGrid::drawGridLines(SDL_Renderer* renderer, int offsetX, int offsetY, SDL_Color gridLineColor) {
    int cellAreaX = getCellAreaX();
    int cellAreaY = getCellAreaY();
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    SetDrawColor(renderer, gridLineColor);
    
    // Vertical lines
    int x = cellAreaX - m_hScrollOffset;
    for (size_t col = 0; col <= m_columnWidths.size(); ++col) {
        if (x >= cellAreaX && x <= cellAreaX + cellAreaWidth) {
            RenderLine(renderer, offsetX + x, offsetY + cellAreaY, 
                              offsetX + x, offsetY + cellAreaY + cellAreaHeight);
        }
        if (col < m_columnWidths.size()) {
            x += m_columnWidths[col];
        }
    }
    
    // Horizontal lines
    int y = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row <= m_data.size(); ++row) {
        if (y >= cellAreaY && y <= cellAreaY + cellAreaHeight) {
            RenderLine(renderer, offsetX + cellAreaX, offsetY + y, 
                              offsetX + cellAreaX + cellAreaWidth, offsetY + y);
        }
        y += m_rowHeight;
    }
}

void StringGrid::ensureCellVisible(size_t row, size_t col) {
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    int cellX = 0;
    for (size_t c = 0; c < col; ++c) {
        cellX += m_columnWidths[c];
    }
    int cellY = static_cast<int>(row) * m_rowHeight;
    int cellWidth = m_columnWidths[col];
    int cellHeight = m_rowHeight;
    
    if (cellX < m_hScrollOffset) {
        m_hScrollOffset = cellX;
    } else if (cellX + cellWidth > m_hScrollOffset + cellAreaWidth) {
        m_hScrollOffset = cellX + cellWidth - cellAreaWidth;
    }
    
    if (cellY < m_vScrollOffset) {
        m_vScrollOffset = cellY;
    } else if (cellY + cellHeight > m_vScrollOffset + cellAreaHeight) {
        m_vScrollOffset = cellY + cellHeight - cellAreaHeight;
    }
    
    if (m_hSlider) {
        m_hSlider->setValue(m_hScrollOffset);
    }
    if (m_vSlider) {
        m_vSlider->setValue(m_vScrollOffset);
    }
}

SharedTexture StringGrid::createLocalTextTexture(std::string_view text, TTF_Font* font, SDL_Color color) {
    // Create text string ONCE for both cache key and SDL call
    std::string textStr(text);
    std::string key = textStr + "|" + std::to_string(color.r) + "," + std::to_string(color.g) + "," + std::to_string(color.b) + "," + std::to_string(color.a);
    
    auto it = m_localTextureCache.find(key);
    if (it != m_localTextureCache.end()) {
        return it->second;
    }
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, textStr.c_str(), textStr.length(), color);
    if (!surface) {
        LOG_DEBUG("StringGrid: TTF_RenderText_Blended failed: %s", SDL_GetError());
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_manager.getRenderer(), surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        LOG_DEBUG("StringGrid: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return nullptr;
    }
    
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    auto sharedTexture = SharedTexture(texture, SDLTextureDeleter());
    m_localTextureCache.emplace(std::move(key), sharedTexture);
    
    return sharedTexture;
}

void StringGrid::clearLocalTextureCache() {
    size_t count = m_localTextureCache.size();
    m_localTextureCache.clear();
    if (count > 0) {
        LOG_DEBUG("StringGrid::clearLocalTextureCache(): Cleared %zu local textures.", count);
    }
}
