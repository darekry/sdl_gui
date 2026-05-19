#include "list_view.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "button.hpp"
#include "label.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    try {
        SDLApp app("ListView Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto statusLabel = std::make_unique<Label>(guiManager, 20, 20, "Selected: None");
        Label* statusLabelPtr = statusLabel.get();
        guiManager.addElement(std::move(statusLabel));

        auto listView = std::make_unique<ListView>(guiManager, 20, 60, 300, 400);
        ListView* listViewPtr = listView.get();

        for (size_t i = 1; i <= 20; ++i) {
            listView->addItem("Element " + std::to_string(i));
        }

        listView->setOnRowClick([statusLabelPtr](ListView* lv, size_t row) {
            std::string item = lv->getItem(row);
            statusLabelPtr->setText("Selected: " + item + " (row " + std::to_string(row + 1) + ")");
        });

        listView->setOnRowDoubleClick([statusLabelPtr](ListView* lv, size_t row) {
            std::string item = lv->getItem(row);
            statusLabelPtr->setText("Activated: " + item);
        });

        guiManager.addElement(std::move(listView));

        auto addButton = std::make_unique<Button>(guiManager, 340, 60, 150, 40, "Add Item");
        addButton->setOnClickCallback([listViewPtr, statusLabelPtr](GUIElement*) {
            size_t count = listViewPtr->getItemCount();
            listViewPtr->addItem("Element " + std::to_string(count + 1));
            statusLabelPtr->setText("Added Element " + std::to_string(count + 1));
        });
        guiManager.addElement(std::move(addButton));

        auto removeButton = std::make_unique<Button>(guiManager, 340, 110, 150, 40, "Remove Selected");
        removeButton->setOnClickCallback([listViewPtr, statusLabelPtr](GUIElement*) {
            auto selected = listViewPtr->getSelectedRow();
            if (selected.has_value()) {
                std::string item = listViewPtr->getItem(selected.value());
                listViewPtr->removeItem(selected.value());
                statusLabelPtr->setText("Removed: " + item);
            } else {
                statusLabelPtr->setText("No item selected");
            }
        });
        guiManager.addElement(std::move(removeButton));

        auto clearButton = std::make_unique<Button>(guiManager, 340, 160, 150, 40, "Clear All");
        clearButton->setOnClickCallback([listViewPtr, statusLabelPtr](GUIElement*) {
            listViewPtr->clearItems();
            statusLabelPtr->setText("List cleared");
        });
        guiManager.addElement(std::move(clearButton));

        auto instructionsLabel = std::make_unique<Label>(guiManager, 340, 220, 
            "Click to select, Double-click to activate");
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