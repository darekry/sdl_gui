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
        Label* statusLabelPtr = statusLabel.get();
        guiManager.addElement(std::move(statusLabel));

        // --- StringGrid ---
        auto grid = std::make_unique<StringGrid>(guiManager, 20, 60, 660, 480, 10, 5);
        StringGrid* gridPtr = grid.get();

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
        grid->setOnCellClick([statusLabelPtr](StringGrid*, CellCoord cell) {
            if (cell.isValid()) {
                std::string msg = "Clicked: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1);
                statusLabelPtr->setText(msg);
            }
        });

        // Callback: podwójne kliknięcie (edycja)
        grid->setOnCellDoubleClick([statusLabelPtr](StringGrid*, CellCoord cell) {
            if (cell.isValid()) {
                std::string msg = "Double-clicked: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1) + " (editing)";
                statusLabelPtr->setText(msg);
            }
        });

        // Callback: edycja zakończona
        grid->setOnCellEdit([statusLabelPtr](StringGrid* grid, CellCoord cell, std::string newText) {
            if (cell.isValid()) {
                std::string msg = "Edited: Row " + std::to_string(cell.row + 1) + 
                                  ", Col " + std::to_string(cell.col + 1) + 
                                  " -> \"" + newText + "\"";
                statusLabelPtr->setText(msg);
                // Aktualizacja danych w gridzie
                grid->setCellText(cell.row, cell.col, newText);
            }
        });

        // Callback: zmiana zaznaczenia
        grid->setOnSelectionChange([statusLabelPtr](StringGrid*, SelectionRange range) {
            if (range.isValid()) {
                auto norm = range.normalized();
                std::string msg = "Selected: Rows " + std::to_string(norm.start.row + 1) + 
                                  "-" + std::to_string(norm.end.row + 1) + 
                                  ", Cols " + std::to_string(norm.start.col + 1) + 
                                  "-" + std::to_string(norm.end.col + 1);
                statusLabelPtr->setText(msg);
            }
        });

        guiManager.addElement(std::move(grid));

        // --- Przycisk "Add Row" ---
        auto addButton = std::make_unique<Button>(guiManager, 700, 60, 180, 40, "Add Row");
        addButton->setOnClickCallback([gridPtr, statusLabelPtr](GUIElement*) {
            size_t currentRows = gridPtr->getRowCount();
            gridPtr->setRowCount(currentRows + 1);
            // Wypełnij nowy wiersz danymi
            gridPtr->setCellText(currentRows, 0, std::to_string(currentRows + 1));
            gridPtr->setCellText(currentRows, 1, "Item " + std::to_string(currentRows + 1));
            gridPtr->setCellText(currentRows, 2, std::to_string((currentRows + 1) * 100));
            gridPtr->setCellText(currentRows, 3, currentRows % 2 == 0 ? "Active" : "Inactive");
            gridPtr->setCellText(currentRows, 4, "Notes for row " + std::to_string(currentRows + 1));
            statusLabelPtr->setText("Added row " + std::to_string(currentRows + 1));
        });
        guiManager.addElement(std::move(addButton));

        // --- Przycisk "Clear" ---
        auto clearButton = std::make_unique<Button>(guiManager, 700, 110, 180, 40, "Clear");
        clearButton->setOnClickCallback([gridPtr, statusLabelPtr](GUIElement*) {
            gridPtr->clear();
            statusLabelPtr->setText("Grid cleared");
        });
        guiManager.addElement(std::move(clearButton));

        // --- Przycisk "Reset Data" ---
        auto resetButton = std::make_unique<Button>(guiManager, 700, 160, 180, 40, "Reset Data");
        resetButton->setOnClickCallback([gridPtr, statusLabelPtr](GUIElement*) {
            gridPtr->setRowCount(10);
            for (size_t row = 0; row < 10; ++row) {
                gridPtr->setCellText(row, 0, std::to_string(row + 1));
                gridPtr->setCellText(row, 1, "Item " + std::to_string(row + 1));
                gridPtr->setCellText(row, 2, std::to_string((row + 1) * 100));
                gridPtr->setCellText(row, 3, row % 2 == 0 ? "Active" : "Inactive");
                gridPtr->setCellText(row, 4, "Notes for row " + std::to_string(row + 1));
            }
            statusLabelPtr->setText("Data reset to default");
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
                if (e.type == SDL_QUIT) {
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
