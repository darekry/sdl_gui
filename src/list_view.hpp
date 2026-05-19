#pragma once

#include "string_grid.hpp"

import std.compat;

class ListView : public StringGrid {
public:
    using RowCallback = std::function<void(ListView*, size_t)>;

    ListView(GUIManager& manager, int x, int y, int width, int height);

    ~ListView() override = default;

    void addItem(const std::string& text);
    void insertItem(size_t index, const std::string& text);
    void removeItem(size_t index);
    void clearItems();
    [[nodiscard]] size_t getItemCount() const;
    [[nodiscard]] std::string getItem(size_t index) const;
    void setItem(size_t index, const std::string& text);

    void setSelectedRow(size_t row);
    [[nodiscard]] std::optional<size_t> getSelectedRow() const;
    void clearSelection();

    void setOnRowClick(RowCallback callback);
    void setOnRowDoubleClick(RowCallback callback);
    void setOnRowActivate(RowCallback callback);

    [[nodiscard]] const char* getComponentType() const override;
    bool handleEvent(const SDL_Event& e) override;

private:
    RowCallback m_onRowClick;
    RowCallback m_onRowDoubleClick;
    RowCallback m_onRowActivate;
};