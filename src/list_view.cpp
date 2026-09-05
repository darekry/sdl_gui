#include "list_view.hpp"
#include "gui_manager.hpp"

#include "std.hpp"

ListView::ListView(GUIManager& manager, int x, int y, int width, int height)
    : StringGrid(manager, x, y, width, height, 0, 1) {
    
    setShowRowHeaders(false);
    setShowColumnHeaders(false);
    setHorizontalScrollEnabled(false);
    
    // Calculate proper column width accounting for slider
    // StringGrid has slider width ~16px on the right side
    int actualWidth = width - 16;  // Leave space for vertical slider
    setColumnWidth(0, actualWidth);
    
    setOnCellClick([this](StringGrid*, CellCoord cell) {
        if (m_onRowClick) {
            m_onRowClick(this, cell.row);
        }
    });
    
    setOnCellDoubleClick([this](StringGrid*, CellCoord cell) {
        if (m_onRowDoubleClick) {
            m_onRowDoubleClick(this, cell.row);
        }
        if (m_onRowActivate) {
            m_onRowActivate(this, cell.row);
        }
    });
}

void ListView::addItem(const std::string& text) {
    size_t newRow = getRowCount();
    setRowCount(newRow + 1);
    setCellText(newRow, 0, text);
}

void ListView::insertItem(size_t index, const std::string& text) {
    size_t count = getRowCount();
    index = std::min(index, count);
    
    setRowCount(count + 1);
    
    for (size_t row = count; row > index; --row) {
        setCellText(row, 0, getCellText(row - 1, 0));
    }
    
    setCellText(index, 0, text);
}

void ListView::removeItem(size_t index) {
    size_t count = getRowCount();
    if (index >= count) {
        return;
    }
    
    for (size_t row = index; row < count - 1; ++row) {
        setCellText(row, 0, getCellText(row + 1, 0));
    }
    
    setRowCount(count - 1);
    
    auto selected = getSelectedRow();
    if (selected) {
        if (*selected == index) {
            clearSelection();
        } else if (*selected > index) {
            setSelectedRow(*selected - 1);
        }
    }
}

void ListView::clearItems() {
    clear();
}

size_t ListView::getItemCount() const {
    return getRowCount();
}

std::string_view ListView::getItem(size_t index) const {
    return getCellText(index, 0);
}

void ListView::setItem(size_t index, const std::string& text) {
    setCellText(index, 0, text);
}

void ListView::setSelectedRow(size_t row) {
    if (row < getRowCount()) {
        StringGrid::setSelectedCell(row, 0);
    }
}

std::optional<size_t> ListView::getSelectedRow() const {
    auto cell = StringGrid::getSelectedCell();
    if (cell && cell->row != SIZE_MAX) {
        return cell->row;
    }
    return std::nullopt;
}

void ListView::clearSelection() {
    StringGrid::clearSelection();
}

void ListView::setOnRowClick(RowCallback callback) {
    m_onRowClick = std::move(callback);
}

void ListView::setOnRowDoubleClick(RowCallback callback) {
    m_onRowDoubleClick = std::move(callback);
}

void ListView::setOnRowActivate(RowCallback callback) {
    m_onRowActivate = std::move(callback);
}

ComponentType ListView::getComponentTypeId() const {
    return ComponentType::ListView;
}

bool ListView::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                return false;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                if (m_onRowActivate) {
                    auto selected = getSelectedRow();
                    if (selected) {
                        m_onRowActivate(this, *selected);
                        return true;
                    }
                }
                break;
            default:
                break;
        }
    }
    
    return StringGrid::handleEvent(e);
}