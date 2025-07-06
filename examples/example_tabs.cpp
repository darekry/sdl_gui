#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <memory>

#include "SDL_render.h"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "tab_control.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* args[]) {
    (void)argc; // Unused
    (void)args; // Unused

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("TabControl Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    GUIManager guiManager(renderer);

    // Tworzenie TabControl
    auto tabControlPtr = std::make_unique<TabControl>(guiManager, 50, 50, 700, 500);
    TabControl* tabControl = tabControlPtr.get(); // Pobierz surowy wskaźnik
    guiManager.addElement(std::move(tabControlPtr)); // Dodaj do menedżera, aby ustawić wskaźnik GUIManager

    // Dodawanie zakładek
    // Załaduj czcionkę
    SharedFont font = guiManager.getFontManager().loadFont("assets/ARIAL.TTF", 16);
    if (!font) {
        std::cerr << "Failed to load font." << std::endl;
        return 1;
    }
    SDL_Color textColor = { 0, 255, 255, 255 };

    // Dodawanie zakładek
    Panel* tab1Panel = tabControl->addTab("Tab 1");
    Panel* tab2Panel = tabControl->addTab("Tab 2");
    Panel* tab3Panel = tabControl->addTab("Tab 3");

 tab1Panel->setBorderColor(255, 0, 0, 128); // Czerwony, półprzezroczysty kolor ramki
    tab1Panel->setBorderThickness(5);

     tab2Panel->setBorderColor(0, 255, 0, 128); // Czerwony, półprzezroczysty kolor ramki
    tab2Panel->setBorderThickness(5);

     tab3Panel->setBorderColor(0, 0, 255, 128); // Czerwony, półprzezroczysty kolor ramki
    tab3Panel->setBorderThickness(5);


    // Dodawanie zawartości do zakładek
    // Zakładka 1
    if (tab1Panel) {
        auto button1 = std::make_unique<Button>(guiManager, 50, 50, 150, 50);
        button1->setTexture(guiManager.getTextureManager().createTextureFromText("Click Me", font, textColor));
        button1->setOnClickCallback([](GUIElement*){ std::cout << "Button 1 clicked!" << std::endl; });
        tab1Panel->addChild(std::move(button1));
    }

    // Zakładka 2
    if (tab2Panel) {
        auto checkbox1 = std::make_unique<Checkbox>(guiManager, 50, 50, 20, 20);
        checkbox1->setLabel("Check me!", 24, textColor);
        tab2Panel->addChild(std::move(checkbox1));
    }
    // Zakładka 3
    if (tab3Panel) {
        auto textInput1 = std::make_unique<TextInput>(guiManager, 50, 50, 300, 40);
        tab3Panel->addChild(std::move(textInput1));
    }
    bool quit = false;

    while (!quit) {

   
       quit = guiManager.handleEvents();

        

        SDL_SetRenderDrawColor(renderer, 0x22, 0, 0x22, 0xFF);
        SDL_RenderClear(renderer);

        guiManager.render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}