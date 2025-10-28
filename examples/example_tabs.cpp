#include "gui_manager.hpp"
#include "tab_control.hpp"
#include "checkbox.hpp"
#include "text_input.hpp"
#include "button.hpp" // Dodano
#include "label.hpp" // Dodano
#include "sdl_app.hpp"


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

        tab1Panel->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = std::nullopt, .borderColor={{255, 0, 0, 128}}, .borderWidth=5, .fontSize = std::nullopt, .fontName = std::nullopt});
        tab2Panel->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = std::nullopt, .borderColor={{0, 255, 0, 128}}, .borderWidth=5, .fontSize = std::nullopt, .fontName = std::nullopt});
        tab3Panel->setStyle(ElementState::Normal, {.backgroundColor = std::nullopt, .textColor = std::nullopt, .texture = std::nullopt, .borderColor={{0, 0, 255, 128}}, .borderWidth=5, .fontSize = std::nullopt, .fontName = std::nullopt});

        // Dodawanie zawartości do zakładek
        auto button1 = std::make_unique<Button>(guiManager, 50, 50, 150, 50);
        auto label1 = std::make_unique<Label>(guiManager, 0, 0, "Click Me", 24);
        label1->setPosition((button1->getWidth() - label1->getWidth())/2, (button1->getHeight() - label1->getHeight())/2);
        button1->addChild(std::move(label1));
        button1->setOnClickCallback([](GUIElement*){ std::cout << "Button 1 clicked!" << std::endl; });
        tab1Panel->addChild(std::move(button1));
        
        auto checkbox1 = std::make_unique<Checkbox>(guiManager, 50, 50, 20, 20);
        auto label2 = std::make_unique<Label>(guiManager, 75, 50, "Check me!", 16);
        tab2Panel->addChild(std::move(checkbox1));
        tab2Panel->addChild(std::move(label2));
       
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