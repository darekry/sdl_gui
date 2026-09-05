#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/string_grid.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("StringGrid - Construction", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Default construction creates empty grid") {
        StringGrid grid(manager, 10, 10, 400, 300);
        REQUIRE(grid.getRowCount() == 0);
        REQUIRE(grid.getColumnCount() == 0);
    }

    SECTION("Construction with initial rows and columns") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 3);
        REQUIRE(grid.getRowCount() == 5);
        REQUIRE(grid.getColumnCount() == 3);
    }

    SECTION("Construction with zero initial dimensions") {
        StringGrid grid(manager, 10, 10, 400, 300, 0, 0);
        REQUIRE(grid.getRowCount() == 0);
        REQUIRE(grid.getColumnCount() == 0);
    }

    SECTION("getComponentTypeIdId returns StringGrid") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getComponentTypeId() == ComponentType::StringGrid);
    }
}

TEST_CASE("StringGrid - Data Management", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setRowCount increases row count") {
        StringGrid grid(manager, 10, 10, 400, 300, 2, 2);
        grid.setRowCount(5);
        REQUIRE(grid.getRowCount() == 5);
        REQUIRE(grid.getColumnCount() == 2);
    }

    SECTION("setRowCount decreases row count") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 2);
        grid.setCellText(0, 0, "Data");
        grid.setRowCount(2);
        REQUIRE(grid.getRowCount() == 2);
    }

    SECTION("setRowCount to zero clears rows") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setRowCount(0);
        REQUIRE(grid.getRowCount() == 0);
        REQUIRE(grid.getColumnCount() == 3);
    }

    SECTION("setColumnCount increases column count") {
        StringGrid grid(manager, 10, 10, 400, 300, 2, 2);
        grid.setColumnCount(5);
        REQUIRE(grid.getColumnCount() == 5);
        REQUIRE(grid.getRowCount() == 2);
    }

    SECTION("setColumnCount decreases column count") {
        StringGrid grid(manager, 10, 10, 400, 300, 2, 5);
        grid.setCellText(0, 0, "Data");
        grid.setColumnCount(2);
        REQUIRE(grid.getColumnCount() == 2);
    }

    SECTION("setColumnCount to zero clears columns") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setColumnCount(0);
        REQUIRE(grid.getColumnCount() == 0);
        REQUIRE(grid.getRowCount() == 3);
    }

    SECTION("getRowCount returns current value") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getRowCount() == 3);
        grid.setRowCount(7);
        REQUIRE(grid.getRowCount() == 7);
    }

    SECTION("getColumnCount returns current value") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getColumnCount() == 3);
        grid.setColumnCount(7);
        REQUIRE(grid.getColumnCount() == 7);
    }

    SECTION("setCellText stores cell content") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setCellText(1, 2, "Hello");
        REQUIRE(grid.getCellText(1, 2) == "Hello");
    }

    SECTION("getCellText returns empty for unset cells") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getCellText(1, 1).empty());
    }

    SECTION("setCellText overwrites previous content") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setCellText(0, 0, "First");
        grid.setCellText(0, 0, "Second");
        REQUIRE(grid.getCellText(0, 0) == "Second");
    }

    SECTION("clear removes all data but preserves column count") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 3);
        grid.setCellText(0, 0, "Data1");
        grid.setCellText(1, 1, "Data2");
        grid.setCellText(2, 2, "Data3");
        
        grid.clear();
        
        REQUIRE(grid.getRowCount() == 0);
        REQUIRE(grid.getColumnCount() == 3);
    }

    SECTION("clear removes selection") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 3);
        grid.setSelectedCell(2, 1);
        REQUIRE(grid.getSelectedCell().has_value());
        
        grid.clear();
        
        REQUIRE_FALSE(grid.getSelectedCell().has_value());
    }
}

TEST_CASE("StringGrid - Headers", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setColumnHeader stores header text") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setColumnHeader(0, "Name");
        grid.setColumnHeader(1, "Age");
        grid.setColumnHeader(2, "City");
    }

    SECTION("setShowColumnHeaders toggles visibility") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setShowColumnHeaders(true);
        grid.setShowColumnHeaders(false);
    }

    SECTION("setShowRowHeaders toggles visibility") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setShowRowHeaders(true);
        grid.setShowRowHeaders(false);
    }
}

TEST_CASE("StringGrid - Selection", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelectedCell marks cell as selected") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setSelectedCell(2, 3);
        
        auto selected = grid.getSelectedCell();
        REQUIRE(selected.has_value());
        REQUIRE(selected->row == 2);
        REQUIRE(selected->col == 3);
    }

    SECTION("getSelectedCell returns empty when no selection") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE_FALSE(grid.getSelectedCell().has_value());
    }

    SECTION("setSelectionRange marks range as selected") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setSelectionRange(1, 1, 3, 3);
        
        auto range = grid.getSelectionRange();
        REQUIRE(range.has_value());
        REQUIRE(range->start.row == 1);
        REQUIRE(range->start.col == 1);
        REQUIRE(range->end.row == 3);
        REQUIRE(range->end.col == 3);
    }

    SECTION("getSelectionRange returns empty when no range selected") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE_FALSE(grid.getSelectionRange().has_value());
    }

    SECTION("clearSelection removes single cell selection") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setSelectedCell(2, 3);
        grid.clearSelection();
        
        REQUIRE_FALSE(grid.getSelectedCell().has_value());
    }

    SECTION("clearSelection removes range selection") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setSelectionRange(1, 1, 3, 3);
        grid.clearSelection();
        
        REQUIRE_FALSE(grid.getSelectionRange().has_value());
    }

    SECTION("Selection range stored as-is (start and end preserved)") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setSelectionRange(3, 3, 1, 1);
        
        auto range = grid.getSelectionRange();
        REQUIRE(range.has_value());
        REQUIRE(range->start.row == 3);
        REQUIRE(range->start.col == 3);
        REQUIRE(range->end.row == 1);
        REQUIRE(range->end.col == 1);
    }
}

TEST_CASE("StringGrid - Editing", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setEditable enables editing") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setEditable(true);
        REQUIRE(grid.isEditable());
    }

    SECTION("setEditable disables editing") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.setEditable(false);
        REQUIRE_FALSE(grid.isEditable());
    }

    SECTION("Default editable state is true") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE(grid.isEditable());
    }

    SECTION("startEditing begins cell edit mode") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.startEditing(2, 2);
        REQUIRE(grid.isEditing());
    }

    SECTION("stopEditing ends edit mode") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.startEditing(2, 2);
        grid.stopEditing();
        REQUIRE_FALSE(grid.isEditing());
    }

    SECTION("isEditing returns false initially") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE_FALSE(grid.isEditing());
    }

    SECTION("isOverlay returns true when editing") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        grid.startEditing(2, 2);
        REQUIRE(grid.isOverlay());
    }

    SECTION("isOverlay returns false when not editing") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE_FALSE(grid.isOverlay());
    }
}

TEST_CASE("StringGrid - Sorting", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("sortByColumn Ascending sorts text alphabetically") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        grid.setCellText(0, 0, "Charlie");
        grid.setCellText(1, 0, "Alpha");
        grid.setCellText(2, 0, "Bravo");
        grid.setCellText(3, 0, "Delta");
        
        grid.sortByColumn(0, SortDirection::Ascending);
        
        REQUIRE(grid.getCellText(0, 0) == "Alpha");
        REQUIRE(grid.getCellText(1, 0) == "Bravo");
        REQUIRE(grid.getCellText(2, 0) == "Charlie");
        REQUIRE(grid.getCellText(3, 0) == "Delta");
    }

    SECTION("sortByColumn Descending sorts text reverse alphabetically") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        grid.setCellText(0, 0, "Charlie");
        grid.setCellText(1, 0, "Alpha");
        grid.setCellText(2, 0, "Bravo");
        grid.setCellText(3, 0, "Delta");
        
        grid.sortByColumn(0, SortDirection::Descending);
        
        REQUIRE(grid.getCellText(0, 0) == "Delta");
        REQUIRE(grid.getCellText(1, 0) == "Charlie");
        REQUIRE(grid.getCellText(2, 0) == "Bravo");
        REQUIRE(grid.getCellText(3, 0) == "Alpha");
    }

    SECTION("sortByColumn Ascending sorts numbers numerically") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        grid.setCellText(0, 1, "30");
        grid.setCellText(1, 1, "10");
        grid.setCellText(2, 1, "20");
        grid.setCellText(3, 1, "5");
        
        grid.sortByColumn(1, SortDirection::Ascending);
        
        REQUIRE(grid.getCellText(0, 1) == "5");
        REQUIRE(grid.getCellText(1, 1) == "10");
        REQUIRE(grid.getCellText(2, 1) == "20");
        REQUIRE(grid.getCellText(3, 1) == "30");
    }

    SECTION("sortByColumn Descending sorts numbers reverse numerically") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        grid.setCellText(0, 1, "30");
        grid.setCellText(1, 1, "10");
        grid.setCellText(2, 1, "20");
        grid.setCellText(3, 1, "5");
        
        grid.sortByColumn(1, SortDirection::Descending);
        
        REQUIRE(grid.getCellText(0, 1) == "30");
        REQUIRE(grid.getCellText(1, 1) == "20");
        REQUIRE(grid.getCellText(2, 1) == "10");
        REQUIRE(grid.getCellText(3, 1) == "5");
    }

    SECTION("sortByColumn None does nothing") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 2);
        grid.setCellText(0, 0, "Charlie");
        grid.setCellText(1, 0, "Alpha");
        grid.setCellText(2, 0, "Bravo");
        
        grid.sortByColumn(0, SortDirection::None);
        
        REQUIRE(grid.getCellText(0, 0) == "Charlie");
        REQUIRE(grid.getCellText(1, 0) == "Alpha");
        REQUIRE(grid.getCellText(2, 0) == "Bravo");
    }

    SECTION("getSortDirection returns current direction") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE(grid.getSortDirection() == SortDirection::None);
        
        grid.sortByColumn(0, SortDirection::Ascending);
        REQUIRE(grid.getSortDirection() == SortDirection::Ascending);
        
        grid.sortByColumn(0, SortDirection::Descending);
        REQUIRE(grid.getSortDirection() == SortDirection::Descending);
        
        grid.sortByColumn(0, SortDirection::None);
        REQUIRE(grid.getSortDirection() == SortDirection::None);
    }

    SECTION("getSortColumn returns current sorted column") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE(grid.getSortColumn() == SIZE_MAX);
        
        grid.sortByColumn(1, SortDirection::Ascending);
        REQUIRE(grid.getSortColumn() == 1);
        
        grid.sortByColumn(2, SortDirection::Descending);
        REQUIRE(grid.getSortColumn() == 2);
    }
}

TEST_CASE("StringGrid - Custom Comparators", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setCustomComparator registers comparator") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE_FALSE(grid.hasCustomComparator(0));
        
        grid.setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        
        REQUIRE(grid.hasCustomComparator(0));
        REQUIRE_FALSE(grid.hasCustomComparator(1));
    }

    SECTION("clearCustomComparator removes specific comparator") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        
        grid.setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        
        REQUIRE(grid.hasCustomComparator(0));
        
        grid.clearCustomComparator(0);
        
        REQUIRE_FALSE(grid.hasCustomComparator(0));
    }

    SECTION("clearAllCustomComparators removes all comparators") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        
        grid.setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        grid.setCustomComparator(1, [](const std::string& a, const std::string& b) {
            return a > b;
        });
        
        REQUIRE(grid.hasCustomComparator(0));
        REQUIRE(grid.hasCustomComparator(1));
        
        grid.clearAllCustomComparators();
        
        REQUIRE_FALSE(grid.hasCustomComparator(0));
        REQUIRE_FALSE(grid.hasCustomComparator(1));
    }

    SECTION("Custom comparator is used for Ascending sort") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        
        grid.setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        
        grid.setCellText(0, 0, "Elephant");
        grid.setCellText(1, 0, "Cat");
        grid.setCellText(2, 0, "Dog");
        grid.setCellText(3, 0, "Hippopotamus");
        
        grid.sortByColumn(0, SortDirection::Ascending);
        
        REQUIRE(grid.getCellText(0, 0).length() <= grid.getCellText(1, 0).length());
        REQUIRE(grid.getCellText(1, 0).length() <= grid.getCellText(2, 0).length());
        REQUIRE(grid.getCellText(2, 0).length() <= grid.getCellText(3, 0).length());
    }

    SECTION("Custom comparator is used for Descending sort") {
        StringGrid grid(manager, 10, 10, 400, 300, 4, 2);
        
        grid.setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        
        grid.setCellText(0, 0, "Elephant");
        grid.setCellText(1, 0, "Cat");
        grid.setCellText(2, 0, "Hippopotamus");
        grid.setCellText(3, 0, "Dog");
        
        grid.sortByColumn(0, SortDirection::Descending);
        
        REQUIRE(grid.getCellText(0, 0).length() >= grid.getCellText(1, 0).length());
        REQUIRE(grid.getCellText(1, 0).length() >= grid.getCellText(2, 0).length());
        REQUIRE(grid.getCellText(2, 0).length() >= grid.getCellText(3, 0).length());
    }

    SECTION("Custom comparator overrides numeric detection") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 2);
        
        grid.setCustomComparator(1, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        
        grid.setCellText(0, 1, "100");
        grid.setCellText(1, 1, "20");
        grid.setCellText(2, 1, "3");
        
        grid.sortByColumn(1, SortDirection::Ascending);
        
        REQUIRE(grid.getCellText(0, 1) == "100");
        REQUIRE(grid.getCellText(1, 1) == "20");
        REQUIRE(grid.getCellText(2, 1) == "3");
    }

    SECTION("No custom comparator uses default numeric sorting") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 2);
        
        grid.setCellText(0, 1, "30");
        grid.setCellText(1, 1, "10");
        grid.setCellText(2, 1, "20");
        
        grid.sortByColumn(1, SortDirection::Ascending);
        
        REQUIRE(grid.getCellText(0, 1) == "10");
        REQUIRE(grid.getCellText(1, 1) == "20");
        REQUIRE(grid.getCellText(2, 1) == "30");
    }
}

TEST_CASE("StringGrid - Geometry", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setColumnWidth sets column width") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setColumnWidth(0, 100);
        grid.setColumnWidth(1, 150);
        grid.setColumnWidth(2, 200);
    }

    SECTION("setRowHeight sets default row height") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setRowHeight(30);
        REQUIRE(grid.getRowHeight() == 30);
    }

    SECTION("setHeaderHeight sets header row height") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setHeaderHeight(40);
    }

    SECTION("setRowHeaderWidth sets row header column width") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setRowHeaderWidth(60);
    }

    SECTION("getRowHeight returns current value") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getRowHeight() == 24);
        grid.setRowHeight(32);
        REQUIRE(grid.getRowHeight() == 32);
    }
}

TEST_CASE("StringGrid - Scrolling", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setHorizontalScrollEnabled toggles horizontal slider") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setHorizontalScrollEnabled(true);
        REQUIRE(grid.isHorizontalScrollEnabled());
        
        grid.setHorizontalScrollEnabled(false);
        REQUIRE_FALSE(grid.isHorizontalScrollEnabled());
    }

    SECTION("setVerticalScrollEnabled toggles vertical slider") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        grid.setVerticalScrollEnabled(true);
        REQUIRE(grid.isVerticalScrollEnabled());
        
        grid.setVerticalScrollEnabled(false);
        REQUIRE_FALSE(grid.isVerticalScrollEnabled());
    }

    SECTION("Horizontal scroll enabled by default") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.isHorizontalScrollEnabled());
    }

    SECTION("Vertical scroll enabled by default") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.isVerticalScrollEnabled());
    }

    SECTION("getVerticalScrollOffset returns current offset") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid.getVerticalScrollOffset() == 0);
    }
}

TEST_CASE("CellCoord - Structure", "[string_grid]") {
    SECTION("Valid CellCoord returns isValid true") {
        CellCoord coord{2, 3};
        REQUIRE(coord.isValid());
    }

    SECTION("Invalid CellCoord returns isValid false") {
        CellCoord coord = CellCoord::invalid();
        REQUIRE_FALSE(coord.isValid());
    }

    SECTION("CellCoord equality comparison") {
        CellCoord a{2, 3};
        CellCoord b{2, 3};
        CellCoord c{3, 2};
        
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }

    SECTION("CellCoord inequality comparison") {
        CellCoord a{2, 3};
        CellCoord b{3, 2};
        
        REQUIRE(a != b);
    }

    SECTION("invalid() creates invalid coordinate") {
        CellCoord coord = CellCoord::invalid();
        REQUIRE(coord.row == SIZE_MAX);
        REQUIRE(coord.col == SIZE_MAX);
        REQUIRE_FALSE(coord.isValid());
    }
}

TEST_CASE("SelectionRange - Structure", "[string_grid]") {
    SECTION("Valid SelectionRange returns isValid true") {
        SelectionRange range{{1, 1}, {3, 3}};
        REQUIRE(range.isValid());
    }

    SECTION("Invalid start makes SelectionRange invalid") {
        SelectionRange range{CellCoord::invalid(), {3, 3}};
        REQUIRE_FALSE(range.isValid());
    }

    SECTION("Invalid end makes SelectionRange invalid") {
        SelectionRange range{{1, 1}, CellCoord::invalid()};
        REQUIRE_FALSE(range.isValid());
    }

    SECTION("normalized() returns ordered range") {
        SelectionRange range{{3, 3}, {1, 1}};
        SelectionRange normalized = range.normalized();
        
        REQUIRE(normalized.start.row == 1);
        REQUIRE(normalized.start.col == 1);
        REQUIRE(normalized.end.row == 3);
        REQUIRE(normalized.end.col == 3);
    }

    SECTION("normalized() preserves already ordered range") {
        SelectionRange range{{1, 1}, {3, 3}};
        SelectionRange normalized = range.normalized();
        
        REQUIRE(normalized.start.row == 1);
        REQUIRE(normalized.start.col == 1);
        REQUIRE(normalized.end.row == 3);
        REQUIRE(normalized.end.col == 3);
    }
}

TEST_CASE("StringGrid - Callbacks", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setOnCellClick stores callback") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        
        bool callbackCalled = false;
        CellCoord clickedCell = CellCoord::invalid();
        
        grid.setOnCellClick([&](StringGrid*, CellCoord coord) {
            callbackCalled = true;
            clickedCell = coord;
        });
    }

    SECTION("setOnCellDoubleClick stores callback") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        
        grid.setOnCellDoubleClick([](StringGrid*, CellCoord) {});
    }

    SECTION("setOnCellEdit stores callback") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        
        grid.setOnCellEdit([](StringGrid*, CellCoord, std::string) {});
    }

    SECTION("setOnSelectionChange stores callback") {
        StringGrid grid(manager, 10, 10, 400, 300, 5, 5);
        
        grid.setOnSelectionChange([](StringGrid*, SelectionRange) {});
    }
}

TEST_CASE("StringGrid - Colors", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setSelectionColor changes selection color") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        SDL_Color color{100, 200, 50, 128};
        grid.setSelectionColor(color);
        
        SDL_Color retrieved = grid.getSelectionColor();
        REQUIRE(retrieved.r == color.r);
        REQUIRE(retrieved.g == color.g);
        REQUIRE(retrieved.b == color.b);
        REQUIRE(retrieved.a == color.a);
    }

    SECTION("setSelectedCellBorderColor changes border color") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        SDL_Color color{255, 0, 0, 255};
        grid.setSelectedCellBorderColor(color);
        
        SDL_Color retrieved = grid.getSelectedCellBorderColor();
        REQUIRE(retrieved.r == color.r);
        REQUIRE(retrieved.g == color.g);
        REQUIRE(retrieved.b == color.b);
        REQUIRE(retrieved.a == color.a);
    }

    SECTION("getSelectionColor returns default initially") {
        StringGrid grid(manager, 10, 10, 400, 300, 3, 3);
        SDL_Color color = grid.getSelectionColor();
        REQUIRE(color.r == 51);
        REQUIRE(color.g == 153);
        REQUIRE(color.b == 255);
        REQUIRE(color.a == 100);
    }
}

TEST_CASE("StringGrid - Scroll State", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getVerticalScrollOffset returns initial offset of 0") {
        StringGrid grid(manager, 10, 10, 400, 300, 50, 3);
        REQUIRE(grid.getVerticalScrollOffset() == 0);
    }

    SECTION("Scroll enabled state persists after toggle") {
        StringGrid grid(manager, 10, 10, 400, 300, 50, 3);
        
        grid.setVerticalScrollEnabled(false);
        REQUIRE_FALSE(grid.isVerticalScrollEnabled());
        
        grid.setVerticalScrollEnabled(true);
        REQUIRE(grid.isVerticalScrollEnabled());
        
        grid.setHorizontalScrollEnabled(false);
        REQUIRE_FALSE(grid.isHorizontalScrollEnabled());
        
        grid.setHorizontalScrollEnabled(true);
        REQUIRE(grid.isHorizontalScrollEnabled());
    }
}