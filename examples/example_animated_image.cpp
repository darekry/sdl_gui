#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"
#include "panel.hpp"
#include "button.hpp"
#include <iostream>
#include <cstddef>
#include <memory>

int main() {
    try {
        SDLApp app("Przykład - AnimatedImage", 800, 600);
        GUIManager gui(app.getRenderer());

        // Przykład użycia AnimatedImage (sprite-sheet)
        // Plik przykładowy: assets/anim.png (9 klatek ułożonych poziomo)
        auto pan = std::make_unique<Panel>(gui, 100, 100, 256, 128);

        // Stwórz AnimatedImage jako unique_ptr, zachowaj surowy wskaźnik do sterowania z przycisku
        auto anim_uptr = std::make_unique<AnimatedImage>(gui, 0, 0, 256, 128);
        AnimatedImage* anim_ptr = anim_uptr.get();
        anim_uptr->setSpriteSheet("assets/anim.png", 9, 1);
        anim_uptr->setFPS(12.0f);
        anim_uptr->setLoop(true);
        anim_uptr->setUseCache(false); // domyślnie direct render w tym przykładzie
        anim_uptr->play();

        pan->addChild(std::move(anim_uptr));
        gui.addElement(std::move(pan));

        // Mały przycisk, który przełącza tryb cache/direct render dla animacji
        auto toggleBtn = std::make_unique<Button>(gui, 380, 110, 140, 32, "Toggle Cache");
        auto useCache = std::make_shared<bool>(false);
        toggleBtn->setOnClickCallback([anim_ptr, useCache](GUIElement*) {
            *useCache = !*useCache;
            anim_ptr->setUseCache(*useCache);
            std::cout << "cache" << (*useCache ? "ON" : "OFF") << "\n";
        });
        gui.addElement(std::move(toggleBtn));

        // Główna pętla aplikacji
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                gui.processEvent(e);
            }

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