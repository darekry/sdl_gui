
#include "SDL_pixels.h"
#include "gui_manager.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "helpers/sdl_app.hpp"

import std.compat;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("RadioButton Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        auto radioGroup = std::make_shared<RadioGroup>();

        auto createRadioButton = [&](int x, int y, const std::string& label) {
            auto rb = std::make_unique<RadioButton>(guiManager, x, y, 20, 20);
            rb->setLabel(label, 24, (SDL_Color){255, 255, 255, 255});
            radioGroup->addRadioButton(rb.get());
            return rb;
        };

        guiManager.addElement(createRadioButton(100, 100, "Option 1"));

        auto rb2 = createRadioButton(100, 150, "Option 2");
        rb2->setSelected(true);
        guiManager.addElement(std::move(rb2));

        guiManager.addElement(createRadioButton(100, 200, "Option 3"));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

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