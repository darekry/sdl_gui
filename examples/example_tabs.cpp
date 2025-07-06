#include <iostream>
#include <memory>

#include "gui.hpp"
#include "gui_manager.hpp"
#include "tab_control.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "helpers/sdl_app.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("TabControl Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Tworzenie TabControl
        auto tabControl = std::make_unique<TabControl>(guiManager, 50, 50, 700, 500);
        
        // Dodawanie zakładek
        Panel* tab1Panel = tabControl->addTab("Tab 1");
        Panel* tab2Panel = tabControl->addTab("Tab 2");
        Panel* tab3Panel = tabControl->addTab("Tab 3");

        tab1Panel->setBorderColor(255, 0, 0, 128);
        tab1Panel->setBorderThickness(5);

        tab2Panel->setBorderColor(0, 255, 0, 128);
        tab2Panel->setBorderThickness(5);

        tab3Panel->setBorderColor(0, 0, 255, 128);
        tab3Panel->setBorderThickness(5);

        // Dodawanie zawartości do zakładek
        
        auto button1 = std::make_unique<Button>(guiManager, 50, 50, 150, 50);
        button1->setLabel("Click Me", 24, {0, 0, 0, 255});
        button1->setOnClickCallback([](GUIElement*){ std::cout << "Button 1 clicked!" << std::endl; });
        tab1Panel->addChild(std::move(button1));
        

       
        auto checkbox1 = std::make_unique<Checkbox>(guiManager, 50, 50, 20, 20);
        checkbox1->setLabel("Check me!", 24, {0, 0, 0, 255});
        tab2Panel->addChild(std::move(checkbox1));
        
        
       
        auto textInput1 = std::make_unique<TextInput>(guiManager, 50, 50, 300, 40);
        tab3Panel->addChild(std::move(textInput1));
        

        guiManager.addElement(std::move(tabControl));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            SDL_SetRenderDrawColor(renderer, 0x22, 0, 0x22, 0xFF);
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