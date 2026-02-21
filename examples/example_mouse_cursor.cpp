#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "cursor.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"

import std.compat;

int main() {
    try {
        SDLApp app("Przykład - MouseCursor", 800, 600);
        GUIManager gui(app.getRenderer());

        auto cursorPtr = std::make_unique<Cursor>(gui);
        Cursor* cursor = cursorPtr.get();
        gui.setCursor(std::move(cursorPtr));

        cursor->setCursorTexture(CursorState::Normal, "assets/button1.png", 8, 8);
        cursor->setCursorTexture(CursorState::Hover, "assets/button2.png", 16, 16);
        cursor->setAnimatedCursor(CursorState::Busy, "assets/button1.png", 4, 2, 8.0f, 16, 16);
        cursor->setScale(0.5f);

        auto infoPanel = std::make_unique<Panel>(gui, 50, 50, 300, 250);
        infoPanel->setBackgroundColor(ElementState::Normal, SDL_Color{200, 200, 220, 255});
        infoPanel->setBorder(ElementState::Normal, SDL_Color{0, 0, 0, 255}, 2);

        auto titleLabel = std::make_unique<Label>(gui, 10, 10, "Przykład Kursora Myszy", 16);
        infoPanel->addChild(std::move(titleLabel));

        auto currentStateLabel = std::make_unique<Label>(gui, 10, 40, "Stan: Normal", 14);
        Label* statePtr = currentStateLabel.get();
        infoPanel->addChild(std::move(currentStateLabel));

        cursor->setOnStateChanged([statePtr](CursorState state) {
            std::string stateName;
            switch (state) {
                case CursorState::Normal: stateName = "Normal"; break;
                case CursorState::Hover: stateName = "Hover"; break;
                case CursorState::Pressed: stateName = "Pressed"; break;
                case CursorState::Disabled: stateName = "Disabled"; break;
                case CursorState::Busy: stateName = "Busy"; break;
                case CursorState::Text: stateName = "Text"; break;
                default: stateName = "Custom"; break;
            }
            statePtr->setText("Stan: " + stateName);
        });

        auto btnNormal = std::make_unique<Button>(gui, 10, 70, 130, 30, "Normal");
        btnNormal->setOnClickCallback([cursor](GUIElement*) {
            cursor->setState(CursorState::Normal);
            std::cout << "Kursor: Normal\n";
        });
        infoPanel->addChild(std::move(btnNormal));

        auto btnHover = std::make_unique<Button>(gui, 10, 110, 130, 30, "Hover");
        btnHover->setOnClickCallback([cursor](GUIElement*) {
            cursor->setState(CursorState::Hover);
            std::cout << "Kursor: Hover\n";
        });
        infoPanel->addChild(std::move(btnHover));

        auto btnBusy = std::make_unique<Button>(gui, 10, 150, 130, 30, "Busy (animowany)");
        btnBusy->setOnClickCallback([cursor](GUIElement*) {
            cursor->setState(CursorState::Busy);
            std::cout << "Kursor: Busy (animowany)\n";
        });
        infoPanel->addChild(std::move(btnBusy));

        auto btnToggle = std::make_unique<Button>(gui, 10, 190, 130, 30, "Pokaż/Ukryj");
        btnToggle->setOnClickCallback([cursor](GUIElement*) {
            cursor->setVisible(!cursor->isVisible());
            std::cout << "Widoczność kursora: " << (cursor->isVisible() ? "TAK" : "NIE") << "\n";
        });
        infoPanel->addChild(std::move(btnToggle));

        gui.addElement(std::move(infoPanel));

        auto interactionPanel = std::make_unique<Panel>(gui, 400, 50, 350, 300);
        interactionPanel->setBackgroundColor(ElementState::Normal, SDL_Color{220, 240, 220, 255});
        interactionPanel->setBorder(ElementState::Normal, SDL_Color{0, 100, 0, 255}, 2);

        auto interactionTitle = std::make_unique<Label>(gui, 10, 10, "Panel Interakcji", 16);
        interactionPanel->addChild(std::move(interactionTitle));

        auto hoverLabel = std::make_unique<Label>(gui, 10, 40, "Najedź na przyciski aby zobaczyć\nzmianę kursora.", 12);
        interactionPanel->addChild(std::move(hoverLabel));

        auto testButton1 = std::make_unique<Button>(gui, 10, 90, 150, 40, "Test 1");
        testButton1->setOnClickCallback([](GUIElement*) {
            std::cout << "Kliknięto Test 1\n";
        });
        interactionPanel->addChild(std::move(testButton1));

        auto testButton2 = std::make_unique<Button>(gui, 170, 90, 150, 40, "Test 2");
        testButton2->setOnClickCallback([](GUIElement*) {
            std::cout << "Kliknięto Test 2\n";
        });
        interactionPanel->addChild(std::move(testButton2));

        auto testButton3 = std::make_unique<Button>(gui, 10, 140, 150, 40, "Test 3");
        testButton3->setOnClickCallback([](GUIElement*) {
            std::cout << "Kliknięto Test 3\n";
        });
        interactionPanel->addChild(std::move(testButton3));

        auto scaleSliderLabel = std::make_unique<Label>(gui, 10, 200, "Skaluj kursor:", 12);
        interactionPanel->addChild(std::move(scaleSliderLabel));

        auto btnScaleSmall = std::make_unique<Button>(gui, 10, 220, 80, 30, "Mały");
        btnScaleSmall->setOnClickCallback([cursor](GUIElement*) {
            cursor->setScale(0.25f);
            std::cout << "Skala kursora: 0.25\n";
        });
        interactionPanel->addChild(std::move(btnScaleSmall));

        auto btnScaleMedium = std::make_unique<Button>(gui, 100, 220, 80, 30, "Średni");
        btnScaleMedium->setOnClickCallback([cursor](GUIElement*) {
            cursor->setScale(0.5f);
            std::cout << "Skala kursora: 0.5\n";
        });
        interactionPanel->addChild(std::move(btnScaleMedium));

        auto btnScaleLarge = std::make_unique<Button>(gui, 190, 220, 80, 30, "Duży");
        btnScaleLarge->setOnClickCallback([cursor](GUIElement*) {
            cursor->setScale(1.0f);
            std::cout << "Skala kursora: 1.0\n";
        });
        interactionPanel->addChild(std::move(btnScaleLarge));

        gui.addElement(std::move(interactionPanel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                gui.processEvent(e);
            }

            gui.update();
            gui.cleanup();

            SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
            SDL_RenderClear(app.getRenderer());
            gui.render();
            SDL_RenderPresent(app.getRenderer());
        }

    } catch (const std::exception& e) {
        std::cerr << "Wyjątek: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
