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
        auto statusLabelRef = guiManager.makeRef(statusLabel.get());
        guiManager.addElement(std::move(statusLabel));

        auto listView = std::make_unique<ListView>(guiManager, 20, 60, 300, 400);
        auto listViewRef = guiManager.makeRef(listView.get());

        for (size_t i = 1; i <= 20; ++i) {
            listView->addItem("Element " + std::to_string(i));
        }

        listView->setOnRowClick([statusLabelRef](ListView* lv, size_t row) {
            std::string item = lv->getItem(row);
            if (statusLabelRef) statusLabelRef->setText("Selected: " + item + " (row " + std::to_string(row + 1) + ")");
        });

        listView->setOnRowDoubleClick([statusLabelRef](ListView* lv, size_t row) {
            std::string item = lv->getItem(row);
            if (statusLabelRef) statusLabelRef->setText("Activated: " + item);
        });

        guiManager.addElement(std::move(listView));

        auto addButton = std::make_unique<Button>(guiManager, 340, 60, 150, 40, "Add Item");
        addButton->setOnClickCallback([listViewRef, statusLabelRef](GUIElement*) {
            if (!listViewRef || !statusLabelRef) return;
            size_t count = listViewRef->getItemCount();
            listViewRef->addItem("Element " + std::to_string(count + 1));
            statusLabelRef->setText("Added Element " + std::to_string(count + 1));
        });
        guiManager.addElement(std::move(addButton));

        auto removeButton = std::make_unique<Button>(guiManager, 340, 110, 150, 40, "Remove Selected");
        removeButton->setOnClickCallback([listViewRef, statusLabelRef](GUIElement*) {
            if (!listViewRef || !statusLabelRef) return;
            auto selected = listViewRef->getSelectedRow();
            if (selected.has_value()) {
                std::string item = listViewRef->getItem(selected.value());
                listViewRef->removeItem(selected.value());
                statusLabelRef->setText("Removed: " + item);
            } else {
                statusLabelRef->setText("No item selected");
            }
        });
        guiManager.addElement(std::move(removeButton));

        auto clearButton = std::make_unique<Button>(guiManager, 340, 160, 150, 40, "Clear All");
        clearButton->setOnClickCallback([listViewRef, statusLabelRef](GUIElement*) {
            if (!listViewRef || !statusLabelRef) return;
            listViewRef->clearItems();
            statusLabelRef->setText("List cleared");
        });
        guiManager.addElement(std::move(clearButton));

        auto instructionsLabel = std::make_unique<Label>(guiManager, 340, 220, 
            "Click to select, Double-click to activate");
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