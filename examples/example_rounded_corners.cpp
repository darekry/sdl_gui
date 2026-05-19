#include "button.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "text_input.hpp"
#include "checkbox.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Rounded Corners Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Tytuł ---
        auto titleLabel = std::make_unique<Label>(guiManager, 50, 20, "Zaokrąglone rogi (borderRadius) - demonstracja");
        guiManager.addElement(std::move(titleLabel));

        // --- Panel z zaokrąglonymi rogami ---
        auto panelRounded = std::make_unique<Panel>(guiManager, 50, 60, 300, 200);
        panelRounded->setBackgroundColor(ElementState::Normal, {70, 130, 180, 255}); // Steel blue
        panelRounded->setBorderRadius(ElementState::Normal, 12);
        panelRounded->setBorder(ElementState::Normal, {100, 150, 200, 255}, 2);
        
        auto panelLabel = std::make_unique<Label>(guiManager, 10, 10, "Panel (radius=12)");
        panelRounded->addChild(std::move(panelLabel));
        guiManager.addElement(std::move(panelRounded));

        // --- Panel bez zaokrąglenia (porównanie) ---
        auto panelSharp = std::make_unique<Panel>(guiManager, 380, 60, 300, 200);
        panelSharp->setBackgroundColor(ElementState::Normal, {70, 130, 180, 255}); // Steel blue
        panelSharp->setBorderRadius(ElementState::Normal, 0);
        panelSharp->setBorder(ElementState::Normal, {100, 150, 200, 255}, 2);
        
        auto panelSharpLabel = std::make_unique<Label>(guiManager, 10, 10, "Panel (radius=0, ostre)");
        panelSharp->addChild(std::move(panelSharpLabel));
        guiManager.addElement(std::move(panelSharp));

        // --- Przyciski z różnymi radiusami ---
        auto radius4Label = std::make_unique<Label>(guiManager, 50, 280, "Radius = 4 (domyślny)");
        guiManager.addElement(std::move(radius4Label));
        
        auto buttonRadius4 = std::make_unique<Button>(guiManager, 50, 310, 200, 40, "Default Button");
        // Domyślny radius z Theme (4px)
        guiManager.addElement(std::move(buttonRadius4));

        auto radius8Label = std::make_unique<Label>(guiManager, 280, 280, "Radius = 8");
        guiManager.addElement(std::move(radius8Label));
        
        auto buttonRadius8 = std::make_unique<Button>(guiManager, 280, 310, 200, 40, "Medium Round");
        buttonRadius8->setBorderRadius(ElementState::Normal, 8);
        buttonRadius8->setBorderRadius(ElementState::Hover, 10);
        guiManager.addElement(std::move(buttonRadius8));

        auto radius16Label = std::make_unique<Label>(guiManager, 510, 280, "Radius = 16");
        guiManager.addElement(std::move(radius16Label));
        
        auto buttonRadius16 = std::make_unique<Button>(guiManager, 510, 310, 200, 40, "Very Round");
        buttonRadius16->setBorderRadius(ElementState::Normal, 16);
        guiManager.addElement(std::move(buttonRadius16));

        // --- Kolorowe przyciski z zaokrąglonymi rogami ---
        auto buttonGreen = std::make_unique<Button>(guiManager, 50, 370, 150, 40, "Green");
        buttonGreen->setBackgroundColor(ElementState::Normal, {46, 139, 87, 255}); // Sea green
        buttonGreen->setBackgroundColor(ElementState::Hover, {60, 179, 113, 255}); // Medium sea green
        buttonGreen->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        buttonGreen->setBorderRadius(ElementState::Normal, 20); // Bardzo zaokrąglony
        guiManager.addElement(std::move(buttonGreen));

        auto buttonRed = std::make_unique<Button>(guiManager, 220, 370, 150, 40, "Red");
        buttonRed->setBackgroundColor(ElementState::Normal, {178, 34, 34, 255}); // Firebrick
        buttonRed->setBackgroundColor(ElementState::Hover, {220, 20, 60, 255}); // Crimson
        buttonRed->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        buttonRed->setBorderRadius(ElementState::Normal, 20);
        guiManager.addElement(std::move(buttonRed));

        auto buttonBlue = std::make_unique<Button>(guiManager, 390, 370, 150, 40, "Blue");
        buttonBlue->setBackgroundColor(ElementState::Normal, {30, 144, 255, 255}); // Dodger blue
        buttonBlue->setBackgroundColor(ElementState::Hover, {65, 105, 225, 255}); // Royal blue
        buttonBlue->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        buttonBlue->setBorderRadius(ElementState::Normal, 20);
        guiManager.addElement(std::move(buttonBlue));

        // --- TextInput z zaokrąglonymi rogami ---
        auto inputLabel = std::make_unique<Label>(guiManager, 50, 430, "TextInput (radius=6)");
        guiManager.addElement(std::move(inputLabel));
        
        auto textInput = std::make_unique<TextInput>(guiManager, 50, 460, 300, 35);
        textInput->setBorderRadius(ElementState::Normal, 6);
        textInput->setText(std::string("Zaokrąglone pole tekstowe"));
        guiManager.addElement(std::move(textInput));

        // --- TextInput bez zaokrąglenia ---
        auto inputSharpLabel = std::make_unique<Label>(guiManager, 400, 430, "TextInput (radius=0)");
        guiManager.addElement(std::move(inputSharpLabel));
        
        auto textInputSharp = std::make_unique<TextInput>(guiManager, 400, 460, 300, 35);
        textInputSharp->setBorderRadius(ElementState::Normal, 0);
        textInputSharp->setText(std::string("Ostre pole tekstowe"));
        guiManager.addElement(std::move(textInputSharp));

        // --- Checkbox z zaokrąglonymi rogami ---
        auto checkboxLabel = std::make_unique<Label>(guiManager, 50, 510, "Checkboxi:");
        guiManager.addElement(std::move(checkboxLabel));
        
        auto checkboxRounded = std::make_unique<Checkbox>(guiManager, 50, 540, 200, 30);
        checkboxRounded->setBorderRadius(ElementState::Normal, 6);
        guiManager.addElement(std::move(checkboxRounded));

        auto checkboxSharp = std::make_unique<Checkbox>(guiManager, 280, 540, 200, 30);
        checkboxSharp->setBorderRadius(ElementState::Normal, 0);
        guiManager.addElement(std::move(checkboxSharp));

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

            SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
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