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
        // clear() resets dimensions to 0
        REQUIRE(grid->getRowCount() == 0);
        REQUIRE(grid->getColumnCount() == 0);
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
