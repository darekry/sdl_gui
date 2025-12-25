#include "button.hpp"
#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "label.hpp"




const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Button Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // --- Przycisk 1: Domyślny wygląd z motywu ---
        // --- Przycisk 1: Domyślny wygląd z motywu ---
        auto buttonDefault = std::make_unique<Button>(guiManager, 50, 50, 200, 50, "Default Button");
        buttonDefault->setOnClickCallback([](GUIElement*) {
            std::cout << "Default Button clicked!" << std::endl;
        });
        guiManager.addElement(std::move(buttonDefault));


        // --- Przycisk 2: Personalizacja kolorów ---
        auto buttonCustomColor = std::make_unique<Button>(guiManager, 300, 50, 200, 50, "Colored Button");
        buttonCustomColor->setBackgroundColor(ElementState::Normal, {0, 120, 0, 255}); // Ciemnozielony
        buttonCustomColor->setBackgroundColor(ElementState::Hover, {0, 180, 0, 255}); // Jaśniejszy zielony
        buttonCustomColor->setBackgroundColor(ElementState::Pressed, {0, 80, 0, 255}); // Najciemniejszy zielony
        buttonCustomColor->setTextColor(ElementState::Normal, {255, 255, 255, 255});

        buttonCustomColor->setOnClickCallback([](GUIElement*) {
            std::cout << "Colored Button clicked!" << std::endl;
        });
        guiManager.addElement(std::move(buttonCustomColor));


        // --- Przycisk 3: Pełna personalizacja stylu ---
        auto buttonStyled = std::make_unique<Button>(guiManager, 550, 50, 200, 50, "Styled Button");
        SharedTexture buttonTexture = guiManager.getTextureManager().loadTexture("assets/button_bg.png");

        Style normal_style;
        normal_style.borderColor = {255, 255, 0, 255}; // Żółta ramka
        normal_style.borderWidth = 2;
        buttonStyled->setStyle(ElementState::Normal, normal_style);

        Style hover_style = normal_style;
        hover_style.borderColor = {255, 165, 0, 255}; // Pomarańczowa ramka
        buttonStyled->setStyle(ElementState::Hover, hover_style);
        
        buttonStyled->setOnClickCallback([](GUIElement*) {
            std::cout << "Styled Button clicked!" << std::endl;
        });
        guiManager.addElement(std::move(buttonStyled));

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

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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