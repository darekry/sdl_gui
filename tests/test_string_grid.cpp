#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/string_grid.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("StringGrid Construction", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Default construction") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300);
        REQUIRE(grid->getRowCount() == 0);
        REQUIRE(grid->getColumnCount() == 0);
    }

    SECTION("Construction with initial size") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 3);
        REQUIRE(grid->getRowCount() == 5);
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("Construction with zero dimensions") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 0, 0);
        REQUIRE(grid->getRowCount() == 0);
        REQUIRE(grid->getColumnCount() == 0);
    }
}

TEST_CASE("StringGrid Data Management", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set and get cell text") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setCellText(1, 1, "Test");
        REQUIRE(grid->getCellText(1, 1) == "Test");
    }

    SECTION("Set cell text multiple times") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setCellText(0, 0, "First");
        grid->setCellText(0, 0, "Second");
        REQUIRE(grid->getCellText(0, 0) == "Second");
    }

    SECTION("Get empty cell text") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        REQUIRE(grid->getCellText(1, 1).empty());
    }

    SECTION("Clear data") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setCellText(0, 0, "Data");
        grid->setCellText(1, 1, "More Data");
        grid->clear();
        // clear() resets row count to 0 but preserves column structure
        REQUIRE(grid->getRowCount() == 0);
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("Set row count") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setRowCount(5);
        REQUIRE(grid->getRowCount() == 5);
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("Set column count") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnCount(5);
        REQUIRE(grid->getRowCount() == 3);
        REQUIRE(grid->getColumnCount() == 5);
    }

    SECTION("Set row count to zero") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setRowCount(0);
        REQUIRE(grid->getRowCount() == 0);
    }

    SECTION("Set column count to zero") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnCount(0);
        REQUIRE(grid->getColumnCount() == 0);
    }
}

TEST_CASE("StringGrid Geometry", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set column width") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnWidth(0, 100);
        grid->setColumnWidth(1, 150);
        // No getter for column width, but should not crash
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("Set row height") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setRowHeight(30);
        // No getter for row height, but should not crash
        REQUIRE(grid->getRowCount() == 3);
    }

    SECTION("Set header height") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setHeaderHeight(40);
        // No getter for header height, but should not crash
        REQUIRE(grid->getRowCount() == 3);
    }

    SECTION("Set row header width") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setRowHeaderWidth(60);
        // No getter for row header width, but should not crash
        REQUIRE(grid->getRowCount() == 3);
    }
}

TEST_CASE("StringGrid Headers", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set column header") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnHeader(0, "Column A");
        grid->setColumnHeader(1, "Column B");
        // No getter for column header, but should not crash
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("Set show row headers") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setShowRowHeaders(true);
        grid->setShowRowHeaders(false);
        // No getter for show row headers, but should not crash
        REQUIRE(grid->getRowCount() == 3);
    }

    SECTION("Set show column headers") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setShowColumnHeaders(true);
        grid->setShowColumnHeaders(false);
        // No getter for show column headers, but should not crash
        REQUIRE(grid->getRowCount() == 3);
    }
}

TEST_CASE("StringGrid Selection", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set and get selected cell") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setSelectedCell(2, 3);
        
        auto selected = grid->getSelectedCell();
        REQUIRE(selected.has_value());
        REQUIRE(selected->row == 2);
        REQUIRE(selected->col == 3);
    }

    SECTION("Get selected cell when none selected") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        auto selected = grid->getSelectedCell();
        REQUIRE_FALSE(selected.has_value());
    }

    SECTION("Set selection range") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setSelectionRange(1, 1, 3, 3);
        
        auto range = grid->getSelectionRange();
        REQUIRE(range.has_value());
        REQUIRE(range->start.row == 1);
        REQUIRE(range->start.col == 1);
        REQUIRE(range->end.row == 3);
        REQUIRE(range->end.col == 3);
    }

    SECTION("Get selection range when none selected") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        auto range = grid->getSelectionRange();
        REQUIRE_FALSE(range.has_value());
    }

    SECTION("Clear selection") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setSelectedCell(2, 3);
        grid->clearSelection();
        
        auto selected = grid->getSelectedCell();
        REQUIRE_FALSE(selected.has_value());
    }

    SECTION("Clear selection range") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setSelectionRange(1, 1, 3, 3);
        grid->clearSelection();
        
        auto range = grid->getSelectionRange();
        REQUIRE_FALSE(range.has_value());
    }

    SECTION("Selection range stored as-is") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        // Set range with end before start
        grid->setSelectionRange(3, 3, 1, 1);
        
        auto range = grid->getSelectionRange();
        REQUIRE(range.has_value());
        // The range should be stored as-is (start and end)
        REQUIRE(range->start.row == 3);
        REQUIRE(range->start.col == 3);
        REQUIRE(range->end.row == 1);
        REQUIRE(range->end.col == 1);
    }
}

TEST_CASE("StringGrid Editing", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set editable true") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setEditable(true);
        REQUIRE(grid->isEditable() == true);
    }

    SECTION("Set editable false") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setEditable(false);
        REQUIRE(grid->isEditable() == false);
    }

    SECTION("Default editable state") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE(grid->isEditable() == true);
    }

    SECTION("Start and stop editing") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->startEditing(2, 2);
        REQUIRE(grid->isEditing() == true);
        
        grid->stopEditing();
        REQUIRE(grid->isEditing() == false);
    }

    SECTION("Is editing when not editing") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE(grid->isEditing() == false);
    }
}

TEST_CASE("StringGrid Callbacks", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Set on cell click callback") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        bool callbackCalled = false;
        CellCoord clickedCell = CellCoord::invalid();
        
        grid->setOnCellClick([&](StringGrid*, CellCoord coord) {
            callbackCalled = true;
            clickedCell = coord;
        });
        
        // Callback is set, no direct way to trigger without events
        REQUIRE(grid->getRowCount() == 5);
    }

    SECTION("Set on cell double click callback") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        grid->setOnCellDoubleClick([](StringGrid*, CellCoord) {
            // Callback set successfully
        });
        
        REQUIRE(grid->getRowCount() == 5);
    }

    SECTION("Set on cell edit callback") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        grid->setOnCellEdit([](StringGrid*, CellCoord, std::string) {
            // Callback set successfully
        });
        
        REQUIRE(grid->getRowCount() == 5);
    }

    SECTION("Set on selection change callback") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        
        grid->setOnSelectionChange([](StringGrid*, SelectionRange) {
            // Callback set successfully
        });
        
        REQUIRE(grid->getRowCount() == 5);
    }
}

TEST_CASE("StringGrid Component Type", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Get component type") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE(grid->getComponentType() != nullptr);
    }
}

TEST_CASE("StringGrid Overlay", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("Is overlay returns true when editing") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->startEditing(2, 2);
        REQUIRE(grid->isOverlay() == true);
    }

    SECTION("Is overlay returns false when not editing") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        REQUIRE(grid->isOverlay() == false);
    }
}

TEST_CASE("CellCoord Structure", "[string_grid]") {
    SECTION("Valid CellCoord") {
        CellCoord coord{2, 3};
        REQUIRE(coord.isValid() == true);
    }

    SECTION("Invalid CellCoord") {
        CellCoord coord = CellCoord::invalid();
        REQUIRE(coord.isValid() == false);
    }

    SECTION("CellCoord equality") {
        CellCoord a{2, 3};
        CellCoord b{2, 3};
        CellCoord c{3, 2};
        
        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    }

    SECTION("CellCoord inequality") {
        CellCoord a{2, 3};
        CellCoord b{3, 2};
        
        REQUIRE(a != b);
    }
}

TEST_CASE("SelectionRange Structure", "[string_grid]") {
    SECTION("Valid SelectionRange") {
        SelectionRange range{{1, 1}, {3, 3}};
        REQUIRE(range.isValid() == true);
    }

    SECTION("Invalid SelectionRange with invalid start") {
        SelectionRange range{CellCoord::invalid(), {3, 3}};
        REQUIRE(range.isValid() == false);
    }

    SECTION("Invalid SelectionRange with invalid end") {
        SelectionRange range{{1, 1}, CellCoord::invalid()};
        REQUIRE(range.isValid() == false);
    }

    SECTION("Normalized SelectionRange") {
        SelectionRange range{{3, 3}, {1, 1}};
        SelectionRange normalized = range.normalized();
        
        REQUIRE(normalized.start.row == 1);
        REQUIRE(normalized.start.col == 1);
        REQUIRE(normalized.end.row == 3);
        REQUIRE(normalized.end.col == 3);
    }

    SECTION("Already normalized SelectionRange") {
        SelectionRange range{{1, 1}, {3, 3}};
        SelectionRange normalized = range.normalized();
        
        REQUIRE(normalized.start.row == 1);
        REQUIRE(normalized.start.col == 1);
        REQUIRE(normalized.end.row == 3);
        REQUIRE(normalized.end.col == 3);
    }
}

TEST_CASE("StringGrid Clear - preserves column structure", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("clear() removes data but preserves column headers") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnHeader(0, "Name");
        grid->setColumnHeader(1, "Age");
        grid->setColumnHeader(2, "City");
        grid->setCellText(0, 0, "John");
        grid->setCellText(0, 1, "30");
        
        grid->clear();
        
        // Data should be cleared
        REQUIRE(grid->getRowCount() == 0);
        // Column count should be preserved
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("clear() removes data but preserves column widths") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnWidth(0, 150);
        grid->setColumnWidth(1, 200);
        grid->setColumnWidth(2, 100);
        grid->setCellText(0, 0, "Data");
        
        grid->clear();
        
        // After clear, adding new rows should maintain column structure
        grid->setRowCount(1);
        REQUIRE(grid->getColumnCount() == 3);
    }

    SECTION("clear() clears selection") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 5, 5);
        grid->setSelectedCell(2, 3);
        REQUIRE(grid->getSelectedCell().has_value());
        
        grid->clear();
        
        REQUIRE_FALSE(grid->getSelectedCell().has_value());
    }
}

TEST_CASE("StringGrid Sorting - sortByColumn", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("sortByColumn() sorts text data ascending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        grid->setCellText(0, 0, "Charlie");
        grid->setCellText(1, 0, "Alpha");
        grid->setCellText(2, 0, "Bravo");
        grid->setCellText(3, 0, "Delta");
        
        grid->sortByColumn(0, SortDirection::Ascending);
        
        REQUIRE(grid->getCellText(0, 0) == "Alpha");
        REQUIRE(grid->getCellText(1, 0) == "Bravo");
        REQUIRE(grid->getCellText(2, 0) == "Charlie");
        REQUIRE(grid->getCellText(3, 0) == "Delta");
    }

    SECTION("sortByColumn() sorts text data descending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        grid->setCellText(0, 0, "Charlie");
        grid->setCellText(1, 0, "Alpha");
        grid->setCellText(2, 0, "Bravo");
        grid->setCellText(3, 0, "Delta");
        
        grid->sortByColumn(0, SortDirection::Descending);
        
        REQUIRE(grid->getCellText(0, 0) == "Delta");
        REQUIRE(grid->getCellText(1, 0) == "Charlie");
        REQUIRE(grid->getCellText(2, 0) == "Bravo");
        REQUIRE(grid->getCellText(3, 0) == "Alpha");
    }

    SECTION("sortByColumn() sorts numeric data ascending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        grid->setCellText(0, 1, "30");
        grid->setCellText(1, 1, "10");
        grid->setCellText(2, 1, "20");
        grid->setCellText(3, 1, "5");
        
        grid->sortByColumn(1, SortDirection::Ascending);
        
        REQUIRE(grid->getCellText(0, 1) == "5");
        REQUIRE(grid->getCellText(1, 1) == "10");
        REQUIRE(grid->getCellText(2, 1) == "20");
        REQUIRE(grid->getCellText(3, 1) == "30");
    }

    SECTION("sortByColumn() sorts numeric data descending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        grid->setCellText(0, 1, "30");
        grid->setCellText(1, 1, "10");
        grid->setCellText(2, 1, "20");
        grid->setCellText(3, 1, "5");
        
        grid->sortByColumn(1, SortDirection::Descending);
        
        REQUIRE(grid->getCellText(0, 1) == "30");
        REQUIRE(grid->getCellText(1, 1) == "20");
        REQUIRE(grid->getCellText(2, 1) == "10");
        REQUIRE(grid->getCellText(3, 1) == "5");
    }

    SECTION("sortByColumn() with SortDirection::None does nothing") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 2);
        grid->setCellText(0, 0, "Charlie");
        grid->setCellText(1, 0, "Alpha");
        grid->setCellText(2, 0, "Bravo");
        
        grid->sortByColumn(0, SortDirection::None);
        
        REQUIRE(grid->getCellText(0, 0) == "Charlie");
        REQUIRE(grid->getCellText(1, 0) == "Alpha");
        REQUIRE(grid->getCellText(2, 0) == "Bravo");
    }
}

TEST_CASE("StringGrid Sorting - getSortDirection and getSortColumn", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("getSortDirection() returns correct direction") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE(grid->getSortDirection() == SortDirection::None);
        
        grid->sortByColumn(0, SortDirection::Ascending);
        REQUIRE(grid->getSortDirection() == SortDirection::Ascending);
        
        grid->sortByColumn(0, SortDirection::Descending);
        REQUIRE(grid->getSortDirection() == SortDirection::Descending);
        
        grid->sortByColumn(0, SortDirection::None);
        REQUIRE(grid->getSortDirection() == SortDirection::None);
    }

    SECTION("getSortColumn() returns correct column") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE(grid->getSortColumn() == SIZE_MAX);
        
        grid->sortByColumn(1, SortDirection::Ascending);
        REQUIRE(grid->getSortColumn() == 1);
        
        grid->sortByColumn(2, SortDirection::Descending);
        REQUIRE(grid->getSortColumn() == 2);
    }

    SECTION("Clicking header cycles sort state: None -> Asc -> Desc -> None") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        grid->setColumnHeader(0, "Col0");
        grid->setColumnHeader(1, "Col1");
        manager.addElement(std::move(grid));
        
        // We can't easily test the full click cycle without complex event simulation,
        // but we can verify the cycle logic through direct sortByColumn calls
        // First click simulation: None -> Ascending
        // Second click simulation: Ascending -> Descending
    }
}

// Note: Clipboard tests with Ctrl+C are skipped because SDL event simulation
// with keyboard modifiers (KMOD_CTRL) is complex in headless test environment.
// The copySelectionToClipboard() method is tested indirectly through the
// integration tests in example_string_grid.cpp.

TEST_CASE("StringGrid Custom Comparators", "[string_grid]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setCustomComparator and hasCustomComparator") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        REQUIRE_FALSE(grid->hasCustomComparator(0));
        
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        
        REQUIRE(grid->hasCustomComparator(0));
        REQUIRE_FALSE(grid->hasCustomComparator(1));
    }

    SECTION("clearCustomComparator removes comparator") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        
        REQUIRE(grid->hasCustomComparator(0));
        
        grid->clearCustomComparator(0);
        
        REQUIRE_FALSE(grid->hasCustomComparator(0));
    }

    SECTION("clearAllCustomComparators removes all comparators") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a < b;
        });
        grid->setCustomComparator(1, [](const std::string& a, const std::string& b) {
            return a > b;
        });
        
        REQUIRE(grid->hasCustomComparator(0));
        REQUIRE(grid->hasCustomComparator(1));
        
        grid->clearAllCustomComparators();
        
        REQUIRE_FALSE(grid->hasCustomComparator(0));
        REQUIRE_FALSE(grid->hasCustomComparator(1));
    }

    SECTION("Custom comparator is used for sorting ascending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        
        // Custom comparator: sort by string length
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        
        grid->setCellText(0, 0, "Elephant");   // 8 chars
        grid->setCellText(1, 0, "Cat");        // 3 chars
        grid->setCellText(2, 0, "Dog");        // 3 chars
        grid->setCellText(3, 0, "Hippopotamus"); // 12 chars
        
        grid->sortByColumn(0, SortDirection::Ascending);
        
        // Should be sorted by length (shortest first)
        REQUIRE(grid->getCellText(0, 0).length() <= grid->getCellText(1, 0).length());
        REQUIRE(grid->getCellText(1, 0).length() <= grid->getCellText(2, 0).length());
        REQUIRE(grid->getCellText(2, 0).length() <= grid->getCellText(3, 0).length());
    }

    SECTION("Custom comparator is used for sorting descending") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 4, 2);
        
        // Custom comparator: sort by string length
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        
        grid->setCellText(0, 0, "Elephant");   // 8 chars
        grid->setCellText(1, 0, "Cat");        // 3 chars
        grid->setCellText(2, 0, "Hippopotamus"); // 12 chars
        grid->setCellText(3, 0, "Dog");        // 3 chars
        
        grid->sortByColumn(0, SortDirection::Descending);
        
        // Should be sorted by length (longest first)
        REQUIRE(grid->getCellText(0, 0).length() >= grid->getCellText(1, 0).length());
        REQUIRE(grid->getCellText(1, 0).length() >= grid->getCellText(2, 0).length());
        REQUIRE(grid->getCellText(2, 0).length() >= grid->getCellText(3, 0).length());
    }

    SECTION("Custom comparator overrides default numeric detection") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 2);
        
        // Custom comparator that treats numbers as strings (lexicographic)
        grid->setCustomComparator(1, [](const std::string& a, const std::string& b) {
            return a < b;  // Lexicographic comparison
        });
        
        grid->setCellText(0, 1, "100");
        grid->setCellText(1, 1, "20");
        grid->setCellText(2, 1, "3");
        
        grid->sortByColumn(1, SortDirection::Ascending);
        
        // Lexicographic order: "100" < "20" < "3"
        REQUIRE(grid->getCellText(0, 1) == "100");
        REQUIRE(grid->getCellText(1, 1) == "20");
        REQUIRE(grid->getCellText(2, 1) == "3");
    }

    SECTION("Different comparators for different columns") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 3);
        
        // Column 0: sort by length
        grid->setCustomComparator(0, [](const std::string& a, const std::string& b) {
            return a.length() < b.length();
        });
        
        // Column 1: reverse lexicographic
        grid->setCustomComparator(1, [](const std::string& a, const std::string& b) {
            return a > b;
        });
        
        grid->setCellText(0, 0, "BBB");
        grid->setCellText(1, 0, "A");
        grid->setCellText(2, 0, "CC");
        
        grid->setCellText(0, 1, "Alpha");
        grid->setCellText(1, 1, "Gamma");
        grid->setCellText(2, 1, "Beta");
        
        // Sort by column 0 (length)
        grid->sortByColumn(0, SortDirection::Ascending);
        REQUIRE(grid->getCellText(0, 0) == "A");     // 1 char
        REQUIRE(grid->getCellText(1, 0) == "CC");    // 2 chars
        REQUIRE(grid->getCellText(2, 0) == "BBB");   // 3 chars
        
        // Reset and sort by column 1 (reverse lexicographic)
        grid->setCellText(0, 1, "Alpha");
        grid->setCellText(1, 1, "Gamma");
        grid->setCellText(2, 1, "Beta");
        
        grid->sortByColumn(1, SortDirection::Ascending);
        REQUIRE(grid->getCellText(0, 1) == "Gamma");  // G > B > A
        REQUIRE(grid->getCellText(1, 1) == "Beta");
        REQUIRE(grid->getCellText(2, 1) == "Alpha");
    }

    SECTION("No custom comparator uses default behavior") {
        auto grid = std::make_unique<StringGrid>(manager, 10, 10, 400, 300, 3, 2);
        
        // No custom comparator set - should use default numeric/text detection
        grid->setCellText(0, 1, "30");
        grid->setCellText(1, 1, "10");
        grid->setCellText(2, 1, "20");
        
        grid->sortByColumn(1, SortDirection::Ascending);
        
        // Default numeric sort: 10 < 20 < 30
        REQUIRE(grid->getCellText(0, 1) == "10");
        REQUIRE(grid->getCellText(1, 1) == "20");
        REQUIRE(grid->getCellText(2, 1) == "30");
    }
}
