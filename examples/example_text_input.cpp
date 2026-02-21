#include "../src/gui.hpp"
#include "../src/gui_manager.hpp"
#include "../src/text_input.hpp"
#include "sdl_app.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("TextInput Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        // Utwórz pole tekstowe
        auto textInput = std::make_unique<TextInput>(guiManager, 100, 100, 300, 40);
        textInput->setOnEnterPressed([](TextInput* ti) {
            std::cout << "Text submitted: " << ti->getText() << std::endl;
        });
        guiManager.addElement(std::move(textInput));

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

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}