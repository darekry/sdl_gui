#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "animation_manager.hpp"
#include "easing.hpp"

import std.compat;

int main() {
    try {
        SDLApp app("Przykład - Animacje", 800, 600);
        GUIManager gui(app.getRenderer());

        // 1. Tworzenie panelu do animowania
        auto panel = std::make_unique<Panel>(gui, 50, 200, 100, 100);
        panel->setBackgroundColor(ElementState::Normal, {200, 100, 100, 255});
        auto* animatable_panel = gui.addElement(std::move(panel));
        auto panelRef = gui.makeRef(animatable_panel);

        // 2. Tworzenie przycisku uruchamiającego animację
        auto button = std::make_unique<Button>(gui, 350, 50, 100, 40, "Animuj!");
        button->setOnClickCallback([&gui, panelRef](GUIElement*) {
            if (!panelRef) return;
            std::cout << "Rozpoczynam animację..." << std::endl;

            const float start_pos_y = static_cast<float>(panelRef->getY());
            const float target_pos_y = 400.0f;

            Animation::CompleteCallback on_forward_complete = [
                &gui, panelRef,
                start_pos_y,
                target_pos_y
            ]() {
                if (!panelRef) return;
                std::cout << "Animacja do celu zakończona. Powrót..." << std::endl;
                gui.getAnimationManager()->createAnimation(
                    &panelRef->m_y,
                    target_pos_y,
                    start_pos_y,
                    2000,
                    Easing::easeOutQuad,
                    [](){ std::cout << "Animacja powrotna zakończona!" << std::endl; }
                );
            };
            
            gui.getAnimationManager()->createAnimation(
                &panelRef->m_y,
                start_pos_y,
                target_pos_y,
                2000,
                Easing::easeOutQuad,
                on_forward_complete
            );
        });
        gui.addElement(std::move(button));

        // Główna pętla aplikacji
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
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