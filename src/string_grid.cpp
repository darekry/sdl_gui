#include "string_grid.hpp"
#include "gui_manager.hpp"
#include "font_manager.hpp"
#include "texture_manager.hpp"
#include "SDL2/SDL_ttf.h"

import std.compat;

// Konstruktor
StringGrid::StringGrid(GUIManager& manager, int x, int y, int width, int height,
                       size_t initialRows, size_t initialCols)
    : Panel(manager, x, y, width, height) {
    
    // Inicjalizacja danych - setColumnCount i setRowCount ustawiają szerokości i nagłówki
    setColumnCount(initialCols);
    setRowCount(initialRows);
    
    // Utwórz slidery
    setupSliders();
    
    // Ustaw przycinanie dzieci
    setClipChildren(true);
}

// Zarządzanie danymi
void StringGrid::setRowCount(size_t rows) {
    m_data.resize(rows);
    // Upewnij się, że każdy wiersz ma odpowiednią liczbę kolumn
    for (auto& row : m_data) {
        row.resize(m_columnWidths.size());
    }
    updateSliderRanges();
}

void StringGrid::setColumnCount(size_t cols) {
    m_columnWidths.resize(cols, 100);  // Domyślna szerokość 100
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
    m_columnWidths.clear();
    m_columnHeaders.clear();
    clearSelection();
}

// Geometria kolumn
void StringGrid::setColumnWidth(size_t col, int width) {
    if (col < m_columnWidths.size()) {
        m_columnWidths[col] = std::max(20, width);  // Minimalna szerokość 20
        updateSliderRanges();
    }
}

void StringGrid::setRowHeight(int height) {
    m_rowHeight = std::max(16, height);  // Minimalna wysokość 16
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
    
    stopEditing();  // Zatrzymaj poprzednią edycję
    
    m_editingCell = {row, col};
    m_isEditing = true;
    
    // Utwórz TextInput dla edycji
    SDL_Rect cellRect = getCellRect(row, col);
    m_cellEditor = std::make_unique<TextInput>(m_manager, cellRect.x, cellRect.y, 
                                                 cellRect.w, cellRect.h);
    m_cellEditor->setText(getCellText(row, col));
    m_cellEditor->setOnEnterPressed([this](TextInput*) {
        stopEditing();
    });
    
    // Przekaż fokus do edytora
    m_manager.setKeyboardFocus(m_cellEditor.get());
}

void StringGrid::stopEditing() {
    if (m_isEditing && m_cellEditor && m_editingCell.isValid()) {
        // Zapisz tekst
        std::string newText = m_cellEditor->getText();
        if (m_editingCell.row < m_data.size() && m_editingCell.col < m_data[m_editingCell.row].size()) {
            m_data[m_editingCell.row][m_editingCell.col] = newText;
            if (m_onCellEdit) {
                m_onCellEdit(this, m_editingCell, newText);
            }
        }
    }
    
    // Wyczyść keyboard focus przed zniszczeniem TextInput, aby uniknąć use-after-free
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

// Obsługa zdarzeń
bool StringGrid::handleEvent(const SDL_Event& e) {
    if (!m_enabled || !m_visible) {
        return false;
    }
    
    // Najpierw przekaż do sliderów
    if (m_vSlider && m_vSlider->handleEvent(e)) {
        return true;
    }
    if (m_hSlider && m_hSlider->handleEvent(e)) {
        return true;
    }
    
    // Przekaż do edytora komórki jeśli jest aktywny
    if (m_isEditing && m_cellEditor) {
        if (m_cellEditor->handleEvent(e)) {
            return true;
        }
        // Kliknięcie poza edytorem kończy edycję
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            SDL_Rect cellRect = getCellRect(m_editingCell.row, m_editingCell.col);
            auto absPos = getAbsolutePosition();
            SDL_Rect absCellRect = {absPos.x + cellRect.x, absPos.y + cellRect.y, cellRect.w, cellRect.h};
            SDL_Point mousePoint = {e.button.x, e.button.y};
            if (!SDL_PointInRect(&mousePoint, &absCellRect)) {
                stopEditing();
            }
        }
    }
    
    // Obsługa zdarzeń myszy
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        auto absPos = getAbsolutePosition();
        int localX = e.button.x - absPos.x;
        int localY = e.button.y - absPos.y;
        
        // Sprawdź czy kliknięcie jest w obszarze komórek
        int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
        int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
        
        if (localX >= cellAreaX && localY >= cellAreaY) {
            CellCoord cell = getCellAtPosition(localX, localY);
            if (cell.isValid()) {
                if (m_onCellClick) {
                    m_onCellClick(this, cell);
                }
                
                // Sprawdź podwójne kliknięcie
                static Uint32 lastClickTime = 0;
                static CellCoord lastClickCell = CellCoord::invalid();
                Uint32 currentTime = SDL_GetTicks();
                
                if (currentTime - lastClickTime < 500 && cell == lastClickCell) {
                    // Podwójne kliknięcie - rozpocznij edycję
                    if (m_editable) {
                        startEditing(cell.row, cell.col);
                        if (m_onCellDoubleClick) {
                            m_onCellDoubleClick(this, cell);
                        }
                    }
                } else {
                    // Pojedyncze kliknięcie - zaznacz komórkę
                    setSelectedCell(cell.row, cell.col);
                    m_isSelecting = true;
                }
                
                lastClickTime = currentTime;
                lastClickCell = cell;
                return true;
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        m_isSelecting = false;
    } else if (e.type == SDL_MOUSEMOTION && m_isSelecting) {
        // Drag selection
        auto absPos = getAbsolutePosition();
        int localX = e.motion.x - absPos.x;
        int localY = e.motion.y - absPos.y;
        
        CellCoord cell = getCellAtPosition(localX, localY);
        if (cell.isValid() && m_selectedCell) {
            m_selectionEnd = cell;
            if (m_onSelectionChange) {
                m_onSelectionChange(this, {m_selectionStart.value(), m_selectionEnd.value()});
            }
        }
    } else if (e.type == SDL_MOUSEWHEEL) {
        // Przewijanie kółkiem myszy
        int scrollAmount = e.wheel.y * 20;  // 20 pikseli na "kliknięcie" kółka
        
        if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            scrollAmount = -scrollAmount;
        }
        
        // Sprawdź czy mysz jest nad obszarem komórek
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        auto absPos = getAbsolutePosition();
        
        if (mouseX >= absPos.x && mouseX < absPos.x + m_width &&
            mouseY >= absPos.y && mouseY < absPos.y + m_height) {
            
            // Przewijanie pionowe
            int maxVScroll = std::max(0, getTotalContentHeight() - getVisibleCellAreaHeight());
            m_vScrollOffset = std::clamp(m_vScrollOffset - scrollAmount, 0, maxVScroll);
            
            if (m_vSlider) {
                m_vSlider->setValue(m_vScrollOffset);
            }
            return true;
        }
    }
    
    // Obsługa klawiatury
    if (e.type == SDL_KEYDOWN && m_selectedCell) {
        bool handled = false;
        size_t newRow = m_selectedCell->row;
        size_t newCol = m_selectedCell->col;
        
        switch (e.key.keysym.sym) {
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
    
    return Panel::handleEvent(e);
}

// Renderowanie bezpośrednie
void StringGrid::drawDirect(SDL_Renderer* renderer) {
    // Pobierz pozycję absolutną elementu - wymagana dla renderowania bezpośredniego
    auto absPos = getAbsolutePosition();
    int offsetX = absPos.x;
    int offsetY = absPos.y;
    
    // Rysuj tło panelu
    drawBackgroundAndBorder(renderer);
    
    // Oblicz obszar komórek
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    // Ustaw przycinanie do obszaru komórek (w współrzędnych absolutnych)
    SDL_Rect cellClipRect = {offsetX + cellAreaX, offsetY + cellAreaY, cellAreaWidth, cellAreaHeight};
    SDL_RenderSetClipRect(renderer, &cellClipRect);
    
    // Oblicz widoczny zakres komórek
    int startRow = 0, endRow = static_cast<int>(m_data.size());
    int startCol = 0, endCol = static_cast<int>(m_columnWidths.size());
    
    // Znajdź pierwszą widoczną kolumnę
    int colOffset = -m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = m_columnWidths[col];
        if (colOffset + colWidth >= cellAreaX) {
            startCol = static_cast<int>(col);
            break;
        }
        colOffset += colWidth;
    }
    
    // Znajdź ostatnią widoczną kolumnę
    colOffset = -m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = static_cast<int>(m_columnWidths[col]);
        colOffset += colWidth;
        if (colOffset > cellAreaX + cellAreaWidth) {
            endCol = static_cast<int>(col);
            break;
        }
    }
    
    // Znajdź pierwszy widoczny wiersz
    int rowY = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        if (rowY + m_rowHeight >= cellAreaY) {
            startRow = static_cast<int>(row);
            break;
        }
        rowY += m_rowHeight;
    }
    
    // Znajdź ostatni widoczny wiersz
    rowY = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        rowY += m_rowHeight;
        if (rowY > cellAreaY + cellAreaHeight) {
            endRow = static_cast<int>(row);
            break;
        }
    }
    
    // Rysuj komórki
    rowY = cellAreaY - m_vScrollOffset + (startRow * m_rowHeight);
    for (int row = startRow; row < endRow && static_cast<size_t>(row) < m_data.size(); ++row) {
        int colX = cellAreaX - m_hScrollOffset;
        
        // Dodaj szerokość kolumn przed startCol
        for (int c = 0; c < startCol; ++c) {
            colX += static_cast<int>(m_columnWidths[static_cast<size_t>(c)]);
        }
        
        for (int col = startCol; col < endCol && static_cast<size_t>(col) < m_columnWidths.size(); ++col) {
            int colWidth = static_cast<int>(m_columnWidths[static_cast<size_t>(col)]);
            
            // Rysuj komórkę (w współrzędnych absolutnych)
            drawCell(renderer, static_cast<size_t>(row), static_cast<size_t>(col), 
                    offsetX + colX, offsetY + rowY, colWidth, m_rowHeight);
            
            colX += colWidth;
        }
        rowY += m_rowHeight;
    }
    
    // Przywróć przycinanie
    SDL_RenderSetClipRect(renderer, nullptr);
    
    // Rysuj nagłówki kolumn (nie podlegają przewijaniu pionowemu)
    if (m_showColumnHeaders) {
        drawColumnHeaders(renderer, offsetX, offsetY);
    }
    
    // Rysuj nagłówki wierszy (nie podlegają przewijaniu poziomemu)
    if (m_showRowHeaders) {
        drawRowHeaders(renderer, offsetX, offsetY);
    }
    
    // Rysuj zaznaczenie
    drawSelection(renderer, offsetX, offsetY);
    
    // Rysuj linie siatki
    drawGridLines(renderer, offsetX, offsetY);
}

// Metody pomocnicze
void StringGrid::setupSliders() {
    // Slider pionowy
    int sliderX = m_width - m_sliderWidth;
    int sliderY = m_showColumnHeaders ? m_headerHeight : 0;
    int sliderHeight = m_height - sliderY - m_sliderWidth;
    
    auto vSlider = std::make_unique<Slider>(m_manager, sliderX, sliderY, 
                                             m_sliderWidth, sliderHeight,
                                             0, 1000, 0, Orientation::Vertical);
    m_vSlider = vSlider.get();
    m_vSlider->setOnChangeCallback([this](GUIElement*) {
        m_vScrollOffset = m_vSlider->getValue();
    });
    addChild(std::move(vSlider));
    
    // Slider poziomy
    int hSliderY = m_height - m_sliderWidth;
    int hSliderX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int sliderWidth = m_width - hSliderX - m_sliderWidth;
    
    auto hSlider = std::make_unique<Slider>(m_manager, hSliderX, hSliderY,
                                             sliderWidth, m_sliderWidth,
                                             0, 1000, 0, Orientation::Horizontal);
    m_hSlider = hSlider.get();
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
    
    if (m_vSlider) {
        int maxVScroll = std::max(0, totalHeight - visibleHeight);
        m_vSlider->setValue(std::min(m_vScrollOffset, maxVScroll));
        // Slider nie ma metody setMaxValue, więc musimy go odtworzyć jeśli zakres się zmienił
        // Dla uproszczenia używamy stałego zakresu i normalizujemy
    }
    
    if (m_hSlider) {
        int maxHScroll = std::max(0, totalWidth - visibleWidth);
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
    if (m_vSlider) {
        width -= m_sliderWidth;
    }
    return std::max(0, width);
}

int StringGrid::getVisibleCellAreaHeight() const {
    int height = m_height;
    if (m_showColumnHeaders) {
        height -= m_headerHeight;
    }
    if (m_hSlider) {
        height -= m_sliderWidth;
    }
    return std::max(0, height);
}

CellCoord StringGrid::getCellAtPosition(int x, int y) const {
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    
    if (x < cellAreaX || y < cellAreaY) {
        return CellCoord::invalid();
    }
    
    // Znajdź kolumnę - sprawdzamy czy x jest wewnątrz kolumny
    int colX = cellAreaX - m_hScrollOffset;
    size_t col = 0;
    for (; col < m_columnWidths.size(); ++col) {
        int colEnd = colX + static_cast<int>(m_columnWidths[col]);
        if (x >= colX && x < colEnd) {
            break;  // znaleziono kolumnę
        }
        colX = colEnd;
    }
    
    if (col >= m_columnWidths.size()) {
        return CellCoord::invalid();
    }
    
    // Znajdź wiersz - sprawdzamy czy y jest wewnątrz wiersza
    int rowY = cellAreaY - m_vScrollOffset;
    size_t row = 0;
    for (; row < m_data.size(); ++row) {
        int rowEnd = rowY + m_rowHeight;
        if (y >= rowY && y < rowEnd) {
            break;  // znaleziono wiersz
        }
        rowY = rowEnd;
    }
    
    if (row >= m_data.size()) {
        return CellCoord::invalid();
    }
    
    return {row, col};
}

SDL_Rect StringGrid::getCellRect(size_t row, size_t col) const {
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    
    // Oblicz pozycję X kolumny
    int x = cellAreaX - m_hScrollOffset;
    for (size_t c = 0; c < col; ++c) {
        x += m_columnWidths[c];
    }
    
    // Oblicz pozycję Y wiersza
    int y = cellAreaY - m_vScrollOffset + static_cast<int>(row) * m_rowHeight;
    
    return {x, y, m_columnWidths[col], m_rowHeight};
}

SDL_Rect StringGrid::getHeaderRect(size_t col) const {
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    
    int x = cellAreaX - m_hScrollOffset;
    for (size_t c = 0; c < col; ++c) {
        x += m_columnWidths[c];
    }
    
    return {x, 0, m_columnWidths[col], m_headerHeight};
}

SDL_Rect StringGrid::getRowHeaderRect(size_t row) const {
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    int y = cellAreaY - m_vScrollOffset + static_cast<int>(row) * m_rowHeight;
    
    return {0, y, m_rowHeaderWidth, m_rowHeight};
}

void StringGrid::drawCell(SDL_Renderer* renderer, size_t row, size_t col, 
                          int screenX, int screenY, int width, int height) {
    // Tło komórki
    SDL_SetRenderDrawColor(renderer, m_cellBackgroundColor.r, m_cellBackgroundColor.g,
                           m_cellBackgroundColor.b, m_cellBackgroundColor.a);
    SDL_Rect cellRect = {screenX, screenY, width, height};
    SDL_RenderFillRect(renderer, &cellRect);
    
    // Tekst komórki
    if (row < m_data.size() && col < m_data[row].size() && !m_data[row][col].empty()) {
        auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", DEFAULT_FONT_SIZE);
        if (font) {
            auto texture = m_manager.getTextureManager().createTextureFromText(
                m_data[row][col], font, m_textColor);
            if (texture) {
                int textWidth, textHeight;
                SDL_QueryTexture(texture.get(), nullptr, nullptr, &textWidth, &textHeight);
                
                // Wyśrodkuj tekst w pionie, wyrównaj do lewej z paddingiem
                int textX = screenX + 4;
                int textY = screenY + (height - textHeight) / 2;
                
                // Przycinanie tekstu do szerokości komórki
                SDL_Rect textClip = {screenX + 2, screenY, width - 4, height};
                SDL_RenderSetClipRect(renderer, &textClip);
                
                SDL_Rect destRect = {textX, textY, textWidth, textHeight};
                SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
                
                SDL_RenderSetClipRect(renderer, nullptr);
            }
        }
    }
}

void StringGrid::drawColumnHeaders(SDL_Renderer* renderer, int offsetX, int offsetY) {
    // Tło nagłówków (w współrzędnych absolutnych)
    SDL_SetRenderDrawColor(renderer, m_headerBackgroundColor.r, m_headerBackgroundColor.g,
                           m_headerBackgroundColor.b, m_headerBackgroundColor.a);
    int headerBgX = offsetX + (m_showRowHeaders ? m_rowHeaderWidth : 0);
    SDL_Rect headerBgRect = {headerBgX, offsetY, getVisibleCellAreaWidth(), m_headerHeight};
    SDL_RenderFillRect(renderer, &headerBgRect);
    
    // Rysuj nagłówki kolumn
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", DEFAULT_FONT_SIZE);
    
    int x = (m_showRowHeaders ? m_rowHeaderWidth : 0) - m_hScrollOffset;
    for (size_t col = 0; col < m_columnWidths.size(); ++col) {
        int colWidth = m_columnWidths[col];
        
        if (x + colWidth >= (m_showRowHeaders ? m_rowHeaderWidth : 0) && 
            x < m_width - (m_vSlider ? m_sliderWidth : 0)) {
            
            // Tło nagłówka kolumny (w współrzędnych absolutnych)
            SDL_Rect headerRect = {offsetX + x, offsetY, colWidth, m_headerHeight};
            SDL_RenderFillRect(renderer, &headerRect);
            
            // Tekst nagłówka
            if (font && col < m_columnHeaders.size() && !m_columnHeaders[col].empty()) {
                auto texture = m_manager.getTextureManager().createTextureFromText(
                    m_columnHeaders[col], font, m_headerTextColor);
                if (texture) {
                    int textWidth, textHeight;
                    SDL_QueryTexture(texture.get(), nullptr, nullptr, &textWidth, &textHeight);
                    
                    int textX = offsetX + x + (colWidth - textWidth) / 2;
                    int textY = offsetY + (m_headerHeight - textHeight) / 2;
                    
                    SDL_Rect destRect = {textX, textY, textWidth, textHeight};
                    SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
                }
            }
            
            // Linia oddzielająca (w współrzędnych absolutnych)
            SDL_SetRenderDrawColor(renderer, m_gridLineColor.r, m_gridLineColor.g,
                                   m_gridLineColor.b, m_gridLineColor.a);
            SDL_RenderDrawLine(renderer, offsetX + x + colWidth - 1, offsetY, 
                              offsetX + x + colWidth - 1, offsetY + m_headerHeight);
        }
        
        x += colWidth;
    }
    
    // Dolna linia nagłówka (w współrzędnych absolutnych)
    SDL_SetRenderDrawColor(renderer, m_gridLineColor.r, m_gridLineColor.g,
                           m_gridLineColor.b, m_gridLineColor.a);
    int lineStartX = offsetX + (m_showRowHeaders ? m_rowHeaderWidth : 0);
    int lineEndX = offsetX + m_width - (m_vSlider ? m_sliderWidth : 0);
    SDL_RenderDrawLine(renderer, lineStartX, offsetY + m_headerHeight - 1,
                       lineEndX, offsetY + m_headerHeight - 1);
}

void StringGrid::drawRowHeaders(SDL_Renderer* renderer, int offsetX, int offsetY) {
    // Tło nagłówków wierszy (w współrzędnych absolutnych)
    SDL_SetRenderDrawColor(renderer, m_headerBackgroundColor.r, m_headerBackgroundColor.g,
                           m_headerBackgroundColor.b, m_headerBackgroundColor.a);
    int headerBgY = offsetY + (m_showColumnHeaders ? m_headerHeight : 0);
    SDL_Rect headerBgRect = {offsetX, headerBgY, m_rowHeaderWidth, getVisibleCellAreaHeight()};
    SDL_RenderFillRect(renderer, &headerBgRect);
    
    // Rysuj nagłówki wierszy
    auto font = m_manager.getFontManager().loadFont("assets/fonts/font.ttf", DEFAULT_FONT_SIZE);
    
    int y = (m_showColumnHeaders ? m_headerHeight : 0) - m_vScrollOffset;
    for (size_t row = 0; row < m_data.size(); ++row) {
        if (y + m_rowHeight >= (m_showColumnHeaders ? m_headerHeight : 0) &&
            y < m_height - (m_hSlider ? m_sliderWidth : 0)) {
            
            // Tło nagłówka wiersza (w współrzędnych absolutnych)
            SDL_Rect headerRect = {offsetX, offsetY + y, m_rowHeaderWidth, m_rowHeight};
            SDL_RenderFillRect(renderer, &headerRect);
            
            // Tekst nagłówka (numer wiersza)
            if (font) {
                std::string rowLabel = std::to_string(row + 1);
                auto texture = m_manager.getTextureManager().createTextureFromText(
                    rowLabel, font, m_headerTextColor);
                if (texture) {
                    int textWidth, textHeight;
                    SDL_QueryTexture(texture.get(), nullptr, nullptr, &textWidth, &textHeight);
                    
                    int textX = offsetX + (m_rowHeaderWidth - textWidth) / 2;
                    int textY = offsetY + y + (m_rowHeight - textHeight) / 2;
                    
                    SDL_Rect destRect = {textX, textY, textWidth, textHeight};
                    SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
                }
            }
            
            // Linia oddzielająca (w współrzędnych absolutnych)
            SDL_SetRenderDrawColor(renderer, m_gridLineColor.r, m_gridLineColor.g,
                                   m_gridLineColor.b, m_gridLineColor.a);
            SDL_RenderDrawLine(renderer, offsetX, offsetY + y + m_rowHeight - 1, 
                              offsetX + m_rowHeaderWidth, offsetY + y + m_rowHeight - 1);
        }
        y += m_rowHeight;
    }
    
    // Prawa linia nagłówka (w współrzędnych absolutnych)
    SDL_SetRenderDrawColor(renderer, m_gridLineColor.r, m_gridLineColor.g,
                           m_gridLineColor.b, m_gridLineColor.a);
    int lineStartY = offsetY + (m_showColumnHeaders ? m_headerHeight : 0);
    int lineEndY = offsetY + m_height - (m_hSlider ? m_sliderWidth : 0);
    SDL_RenderDrawLine(renderer, offsetX + m_rowHeaderWidth - 1, lineStartY,
                       offsetX + m_rowHeaderWidth - 1, lineEndY);
}

void StringGrid::drawSelection(SDL_Renderer* renderer, int offsetX, int offsetY) {
    if (!m_selectedCell) {
        return;
    }
    
    // Rysuj zaznaczenie (zakres lub pojedyncza komórka)
    SelectionRange range;
    if (m_selectionStart && m_selectionEnd) {
        range = SelectionRange{m_selectionStart.value(), m_selectionEnd.value()}.normalized();
    } else {
        range = {m_selectedCell.value(), m_selectedCell.value()};
    }
    
    // Oblicz prostokąt zaznaczenia
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    
    // Pozycja X początku
    int startX = cellAreaX - m_hScrollOffset;
    for (size_t col = 0; col < range.start.col; ++col) {
        startX += m_columnWidths[col];
    }
    
    // Pozycja Y początku
    int startY = cellAreaY - m_vScrollOffset + static_cast<int>(range.start.row) * m_rowHeight;
    
    // Szerokość
    int selWidth = 0;
    for (size_t col = range.start.col; col <= range.end.col; ++col) {
        selWidth += m_columnWidths[col];
    }
    
    // Wysokość
    int selHeight = static_cast<int>(range.end.row - range.start.row + 1) * m_rowHeight;
    
    // Rysuj półprzezroczyste tło zaznaczenia (w współrzędnych absolutnych)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, m_selectionColor.r, m_selectionColor.g,
                           m_selectionColor.b, m_selectionColor.a);
    SDL_Rect selRect = {offsetX + startX, offsetY + startY, selWidth, selHeight};
    SDL_RenderFillRect(renderer, &selRect);
    
    // Rysuj ramkę zaznaczonej komórki (w współrzędnych absolutnych)
    if (m_selectedCell) {
        SDL_Rect activeCellRect = getCellRect(m_selectedCell->row, m_selectedCell->col);
        SDL_Rect absCellRect = {offsetX + activeCellRect.x, offsetY + activeCellRect.y, 
                                activeCellRect.w, activeCellRect.h};
        SDL_SetRenderDrawColor(renderer, m_selectedCellBorderColor.r, m_selectedCellBorderColor.g,
                               m_selectedCellBorderColor.b, m_selectedCellBorderColor.a);
        SDL_RenderDrawRect(renderer, &absCellRect);
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void StringGrid::drawGridLines(SDL_Renderer* renderer, int offsetX, int offsetY) {
    int cellAreaX = m_showRowHeaders ? m_rowHeaderWidth : 0;
    int cellAreaY = m_showColumnHeaders ? m_headerHeight : 0;
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    SDL_SetRenderDrawColor(renderer, m_gridLineColor.r, m_gridLineColor.g,
                           m_gridLineColor.b, m_gridLineColor.a);
    
    // Linie pionowe (w współrzędnych absolutnych)
    int x = cellAreaX - m_hScrollOffset;
    for (size_t col = 0; col <= m_columnWidths.size(); ++col) {
        if (x >= cellAreaX && x <= cellAreaX + cellAreaWidth) {
            SDL_RenderDrawLine(renderer, offsetX + x, offsetY + cellAreaY, 
                              offsetX + x, offsetY + cellAreaY + cellAreaHeight);
        }
        if (col < m_columnWidths.size()) {
            x += m_columnWidths[col];
        }
    }
    
    // Linie poziome (w współrzędnych absolutnych)
    int y = cellAreaY - m_vScrollOffset;
    for (size_t row = 0; row <= m_data.size(); ++row) {
        if (y >= cellAreaY && y <= cellAreaY + cellAreaHeight) {
            SDL_RenderDrawLine(renderer, offsetX + cellAreaX, offsetY + y, 
                              offsetX + cellAreaX + cellAreaWidth, offsetY + y);
        }
        y += m_rowHeight;
    }
}

void StringGrid::ensureCellVisible(size_t row, size_t col) {
    int cellAreaWidth = getVisibleCellAreaWidth();
    int cellAreaHeight = getVisibleCellAreaHeight();
    
    // Oblicz pozycję komórki
    int cellX = 0;
    for (size_t c = 0; c < col; ++c) {
        cellX += m_columnWidths[c];
    }
    int cellY = static_cast<int>(row) * m_rowHeight;
    int cellWidth = m_columnWidths[col];
    int cellHeight = m_rowHeight;
    
    // Przewiń poziomo jeśli potrzeba
    if (cellX < m_hScrollOffset) {
        m_hScrollOffset = cellX;
    } else if (cellX + cellWidth > m_hScrollOffset + cellAreaWidth) {
        m_hScrollOffset = cellX + cellWidth - cellAreaWidth;
    }
    
    // Przewiń pionowo jeśli potrzeba
    if (cellY < m_vScrollOffset) {
        m_vScrollOffset = cellY;
    } else if (cellY + cellHeight > m_vScrollOffset + cellAreaHeight) {
        m_vScrollOffset = cellY + cellHeight - cellAreaHeight;
    }
    
    // Zaktualizuj slidery
    if (m_hSlider) {
        m_hSlider->setValue(m_hScrollOffset);
    }
    if (m_vSlider) {
        m_vSlider->setValue(m_vScrollOffset);
    }
}
