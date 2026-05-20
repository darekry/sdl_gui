/**
 * @file example_dialog.cpp
 * @brief Demonstracja DialogBox i MessageBox - złożonych komponentów GUI
 * 
 * Ten przykład pokazuje:
 * - DialogBox::createConfirm() - dialog potwierdzający (Tak/Nie)
 * - DialogBox::createAlert() - dialog alertu (OK)
 * - DialogBox::createWithTitle() - dialog z własnym tytułem
 * - MessageBox::showInfo() - szybki komunikat informacyjny
 * - MessageBox::showError() - komunikat błędu
 * - MessageBox::showQuestion() - dialog pytania
 */

#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "button.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "composite/dialog_box.hpp"
#include "composite/message_box.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Dialog & MessageBox Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Panel główny z przyciskami testowymi
        auto mainPanel = std::make_unique<Panel>(guiManager, 50, 50, 700, 500);
        Style panelStyle;
        panelStyle.backgroundColor = {250, 250, 250, 255};
        panelStyle.borderColor = {200, 200, 200, 255};
        panelStyle.borderWidth = 1;
        mainPanel->setStyle(ElementState::Normal, panelStyle);

        // Label tytułowy
        auto titleLabel = std::make_unique<Label>(guiManager, 20, 20, "Test DialogBox i MessageBox", 28);
        titleLabel->setPosition(250, 20);
        mainPanel->addChild(std::move(titleLabel));

        // === DialogBox Examples ===

        // Przycisk 1: Confirm Dialog
        auto btnConfirm = std::make_unique<Button>(guiManager, 50, 80, 200, 40, "Confirm Dialog");
        btnConfirm->setOnClickCallback([&guiManager](GUIElement*) {
            DialogBox::createConfirm(
                guiManager,
                "Czy na pewno chcesz usunąć ten plik?",
                "Tak",
                "Nie",
                [](bool confirmed) {
                    if (confirmed) {
                        std::cout << "Użytkownik wybrał: TAK (usuń plik)\n";
                    } else {
                        std::cout << "Użytkownik wybrał: NIE (anuluj)\n";
                    }
                }
            );
        });
        mainPanel->addChild(std::move(btnConfirm));

        // Przycisk 2: Alert Dialog
        auto btnAlert = std::make_unique<Button>(guiManager, 260, 80, 200, 40, "Alert Dialog");
        btnAlert->setOnClickCallback([&guiManager](GUIElement*) {
            DialogBox::createAlert(
                guiManager,
                "Operacja została zakończona sukcesem!",
                "OK",
                [](int) {
                    std::cout << "Użytkownik kliknął OK\n";
                }
            );
        });
        mainPanel->addChild(std::move(btnAlert));

        // Przycisk 3: Dialog with Title
        auto btnWithTitle = std::make_unique<Button>(guiManager, 470, 80, 200, 40, "Dialog with Title");
        btnWithTitle->setOnClickCallback([&guiManager](GUIElement*) {
            DialogBox::createWithTitle(
                guiManager,
                "Zapisz zmiany?",
                "Dokument został zmodyfikowany. Czy chcesz zapisać zmiany przed zamknięciem?",
                {"Zapisz", "Nie zapisuj", "Anuluj"},
                [](int buttonIndex) {
                    switch (buttonIndex) {
                        case 0: std::cout << "Wybrano: Zapisz\n"; break;
                        case 1: std::cout << "Wybrano: Nie zapisuj\n"; break;
                        case 2: std::cout << "Wybrano: Anuluj\n"; break;
                    }
                },
                450, 180
            );
        });
        mainPanel->addChild(std::move(btnWithTitle));

        // === MessageBox Examples ===

        // Przycisk 4: MessageBox Info
        auto btnInfo = std::make_unique<Button>(guiManager, 50, 140, 200, 40, "MessageBox Info");
        btnInfo->setOnClickCallback([&guiManager](GUIElement*) {
            MessageBox::showInfo(guiManager, "Plik został zapisany pomyślnie.", []() {
                std::cout << "Info dialog closed\n";
            });
        });
        mainPanel->addChild(std::move(btnInfo));

        // Przycisk 5: MessageBox Error
        auto btnError = std::make_unique<Button>(guiManager, 260, 140, 200, 40, "MessageBox Error");
        btnError->setOnClickCallback([&guiManager](GUIElement*) {
            MessageBox::showError(guiManager, "Błąd: Nie można otworzyć pliku. Sprawdź czy plik istnieje.", []() {
                std::cout << "Error dialog closed\n";
            });
        });
        mainPanel->addChild(std::move(btnError));

        // Przycisk 6: MessageBox Warning
        auto btnWarning = std::make_unique<Button>(guiManager, 470, 140, 200, 40, "MessageBox Warning");
        btnWarning->setOnClickCallback([&guiManager](GUIElement*) {
            MessageBox::showWarning(guiManager, "Ostrzeżenie: Niewystarczająca ilość pamięci.", []() {
                std::cout << "Warning dialog closed\n";
            });
        });
        mainPanel->addChild(std::move(btnWarning));

        // Przycisk 7: MessageBox Question
        auto btnQuestion = std::make_unique<Button>(guiManager, 50, 200, 200, 40, "MessageBox Question");
        btnQuestion->setOnClickCallback([&guiManager](GUIElement*) {
            MessageBox::showQuestion(
                guiManager,
                "Czy chcesz kontynuować instalację?",
                []() { std::cout << "Kontynuuję instalację...\n"; },
                []() { std::cout << "Instalacja anulowana.\n"; }
            );
        });
        mainPanel->addChild(std::move(btnQuestion));

        // Przycisk 8: Custom Dialog
        auto btnCustom = std::make_unique<Button>(guiManager, 260, 200, 200, 40, "Custom Dialog");
        btnCustom->setOnClickCallback([&guiManager](GUIElement*) {
            MessageBox::showCustom(
                guiManager,
                "Custom Title",
                "To jest własny dialog z własnym tekstem przycisku.",
                "Rozumiem",
                MessageBox::IconType::Info,
                []() { std::cout << "Custom dialog closed\n"; }
            );
        });
        mainPanel->addChild(std::move(btnCustom));

        // Label informacyjny
        auto infoLabel = std::make_unique<Label>(guiManager, 50, 280, 
            "Kliknij przyciski aby zobaczyć różne typy dialogów.\nESC zamyka dialog bez wyboru.", 18);
        mainPanel->addChild(std::move(infoLabel));

        // Drugi label
        auto dragLabel = std::make_unique<Label>(guiManager, 50, 340,
            "Dialogi są draggable - możesz przesuwać je myszką.", 18);
        mainPanel->addChild(std::move(dragLabel));

        guiManager.addElement(std::move(mainPanel));

        // Main loop
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
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