#include "string_grid.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "button.hpp"
#include "label.hpp"

import std.compat;




const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 600;

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    try {
        SDLApp app("StringGrid Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Etykieta statusu ---
        auto statusLabel = std::make_unique<Label>(guiManager, 20, 20, "Selected: None");
        auto statusLabelRef = guiManager.makeRef(statusLabel.get());
        guiManager.addElement(std::move(statusLabel));

        // --- StringGrid ---
        auto grid = std::make_unique<StringGrid>(guiManager, 20, 60, 660, 480, 10, 5);
        auto gridRef = guiManager.makeRef(grid.get());

        // Ustawienie nagłówków kolumn
        grid->setColumnHeader(0, "ID");
        grid->setColumnHeader(1, "Name");
        grid->setColumnHeader(2, "Value");
        grid->setColumnHeader(3, "Status");
        grid->setColumnHeader(4, "Notes");

        // Ustawienie szerokości kolumn
        grid->setColumnWidth(0, 50);
        grid->setColumnWidth(1, 150);
        grid->setColumnWidth(2, 100);
        grid->setColumnWidth(3, 100);
        grid->setColumnWidth(4, 200);

        // Wypełnienie danymi testowymi
        for (size_t row = 0; row < 10; ++row) {
            grid->setCellText(row, 0, std::to_string(row + 1));
            grid->setCellText(row, 1, "Item " + std::to_string(row + 1));
            grid->setCellText(row, 2, std::to_string((row + 1) * 100));
            grid->setCellText(row, 3, row % 2 == 0 ? "Active" : "Inactive");
            grid->setCellText(row, 4, "Notes for row " + std::to_string(row + 1));
        }

        // Callback: kliknięcie komórki
        grid->setOnCellClick([statusLabelRef](StringGrid*, CellCoord cell) {
            if (cell.isValid() && statusLabelRef) {
                std::string msg = "Clicked: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1);
                statusLabelRef->setText(msg);
            }
        });

        // Callback: podwójne kliknięcie (edycja)
        grid->setOnCellDoubleClick([statusLabelRef](StringGrid*, CellCoord cell) {
            if (cell.isValid() && statusLabelRef) {
                std::string msg = "Double-clicked: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1) + " (editing)";
                statusLabelRef->setText(msg);
            }
        });

        // Callback: edycja zakończona
        grid->setOnCellEdit([statusLabelRef](StringGrid* grid, CellCoord cell, std::string newText) {
            if (cell.isValid() && statusLabelRef) {
                std::string msg = "Edited: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1) + 
                                  " -> \"" + newText + "\"";
                statusLabelRef->setText(msg);
                grid->setCellText(cell.row, cell.col, newText);
            }
        });

        // Callback: zmiana zaznaczenia
        grid->setOnSelectionChange([statusLabelRef](StringGrid*, SelectionRange range) {
            if (range.isValid() && statusLabelRef) {
                auto norm = range.normalized();
                std::string msg = "Selected: Rows " + std::to_string(norm.start.row + 1) + 
                                  "-" + std::to_string(norm.end.row + 1) + 
                                  ", Cols " + std::to_string(norm.start.col + 1) + 
                                  "-" + std::to_string(norm.end.col + 1);
                statusLabelRef->setText(msg);
            }
        });

        // --- Przycisk "Add Row" ---
        auto addButton = std::make_unique<Button>(guiManager, 700, 60, 180, 40, "Add Row");
        addButton->setOnClickCallback([gridRef, statusLabelRef](GUIElement*) {
            if (!gridRef || !statusLabelRef) return;
            size_t currentRows = gridRef->getRowCount();
            gridRef->setRowCount(currentRows + 1);
            gridRef->setCellText(currentRows, 0, std::to_string(currentRows + 1));
            gridRef->setCellText(currentRows, 1, "Item " + std::to_string(currentRows + 1));
            gridRef->setCellText(currentRows, 2, std::to_string((currentRows + 1) * 100));
            gridRef->setCellText(currentRows, 3, currentRows % 2 == 0 ? "Active" : "Inactive");
            gridRef->setCellText(currentRows, 4, "Notes for row " + std::to_string(currentRows + 1));
            statusLabelRef->setText("Added row " + std::to_string(currentRows + 1));
        });
        guiManager.addElement(std::move(addButton));

        // --- Przycisk "Clear" ---
        auto clearButton = std::make_unique<Button>(guiManager, 700, 110, 180, 40, "Clear");
        clearButton->setOnClickCallback([gridRef, statusLabelRef](GUIElement*) {
            if (!gridRef || !statusLabelRef) return;
            gridRef->clear();
            statusLabelRef->setText("Grid cleared");
        });
        guiManager.addElement(std::move(clearButton));

        // --- Przycisk "Reset Data" ---
        auto resetButton = std::make_unique<Button>(guiManager, 700, 160, 180, 40, "Reset Data");
        resetButton->setOnClickCallback([gridRef, statusLabelRef](GUIElement*) {
            if (!gridRef || !statusLabelRef) return;
            gridRef->setRowCount(10);
            for (size_t row = 0; row < 10; ++row) {
                gridRef->setCellText(row, 0, std::to_string(row + 1));
                gridRef->setCellText(row, 1, "Item " + std::to_string(row + 1));
                gridRef->setCellText(row, 2, std::to_string((row + 1) * 100));
                gridRef->setCellText(row, 3, row % 2 == 0 ? "Active" : "Inactive");
                gridRef->setCellText(row, 4, "Notes for row " + std::to_string(row + 1));
            }
            statusLabelRef->setText("Data reset to default");
        });
        guiManager.addElement(std::move(resetButton));

        // --- Instrukcje ---
        auto instructionsLabel = std::make_unique<Label>(guiManager, 700, 220, 
            "Instructions: Click to select, Double-click to edit, Drag to select range");
        guiManager.addElement(std::move(instructionsLabel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
