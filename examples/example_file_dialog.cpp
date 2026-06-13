/**
 * @file example_file_dialog.cpp
 * @brief Demonstracja FileDialog - złożonego komponentu wyboru plików
 *
 * Pokazuje:
 * - FileDialog::createOpen() - dialog otwierania pliku
 * - FileDialog::createSave() - dialog zapisywania pliku
 * - Nawigację po katalogach, filtrowanie plików
 */

#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "composite/file_dialog.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("FileDialog Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto mainPanel = std::make_unique<Panel>(guiManager, 50, 50, 700, 500);
        Style panelStyle;
        panelStyle.backgroundColor = {250, 250, 250, 255};
        panelStyle.borderColor = {200, 200, 200, 255};
        panelStyle.borderWidth = 1;
        mainPanel->setStyle(ElementState::Normal, panelStyle);

        auto titleLabel = std::make_unique<Label>(guiManager, 20, 20, "FileDialog Demo", 28);
        titleLabel->setPosition(280, 30);
        mainPanel->addChild(std::move(titleLabel));

        auto statusLabel = std::make_unique<Label>(guiManager, 50, 350,
            "Selected file: (none)", 16);
        auto statusLabelRef = guiManager.makeRef(statusLabel.get());
        mainPanel->addChild(std::move(statusLabel));

        // Przycisk: Open File Dialog
        auto btnOpen = std::make_unique<Button>(guiManager, 100, 100, 200, 40, "Open File...");
        btnOpen->setOnClickCallback([&guiManager, statusLabelRef](GUIElement*) {
            FileDialog::createOpen(
                guiManager,
                "Open File",
                [statusLabelRef](const std::string& path) {
                    std::cout << "Open file: " << path << '\n';
                    if (statusLabelRef) {
                        statusLabelRef->setText("Selected file: " + path);
                    }
                }
            );
        });
        mainPanel->addChild(std::move(btnOpen));

        // Przycisk: Save File Dialog
        auto btnSave = std::make_unique<Button>(guiManager, 320, 100, 200, 40, "Save File...");
        btnSave->setOnClickCallback([&guiManager, statusLabelRef](GUIElement*) {
            FileDialog::createSave(
                guiManager,
                "Save File",
                [statusLabelRef](const std::string& path) {
                    std::cout << "Save file: " << path << '\n';
                    if (statusLabelRef) {
                        statusLabelRef->setText("Save to: " + path);
                    }
                }
            );
        });
        mainPanel->addChild(std::move(btnSave));

        // Przycisk: Open with filter
        auto btnFiltered = std::make_unique<Button>(guiManager, 540, 100, 200, 40, "Open *.cpp...");
        btnFiltered->setOnClickCallback([&guiManager, statusLabelRef](GUIElement*) {
            FileDialog::createOpen(
                guiManager,
                "Open C++ Source",
                [statusLabelRef](const std::string& path) {
                    std::cout << "Open .cpp: " << path << '\n';
                    if (statusLabelRef) {
                        statusLabelRef->setText("Selected: " + path);
                    }
                },
                {},
                "*.cpp"
            );
        });
        mainPanel->addChild(std::move(btnFiltered));

        auto infoLabel = std::make_unique<Label>(guiManager, 50, 200,
            "Left panel: directories (double-click to navigate)\n"
            "Right panel: files (double-click to confirm, single-click to select)\n"
            "\"..\" navigates to parent directory\n"
            "ESC closes the dialog, Enter confirms selection", 14);
        mainPanel->addChild(std::move(infoLabel));

        guiManager.addElement(std::move(mainPanel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            guiManager.update();
            guiManager.render();
            guiManager.cleanup();

            SDL_RenderPresent(renderer);
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
