#include "helpers/sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "animation_manager.hpp"
#include "easing.hpp"
import std.compat;

int main(int argc, char* argv[]) {
    try {
        SDLApp app("Przykład - Animacje", 800, 600);
        GUIManager gui(app.getRenderer());

        // Zmienna przechowująca animowaną pozycję. Będzie modyfikowana przez AnimationManager.
        float animated_x = 50.0f;

        // 1. Tworzenie panelu do animowania
        auto panel = std::make_unique<Panel>(gui, static_cast<int>(animated_x), 200, 100, 100);
        panel->setBackgroundColor(ElementState::Normal, {200, 100, 100, 255});
        auto* animatable_panel = gui.addElement(std::move(panel));

        // 2. Tworzenie przycisku uruchamiającego animację
        auto button = std::make_unique<Button>(gui, 350, 50, 100, 40, "Animuj!");
button->setOnClickCallback([&gui, &animated_x, animatable_panel](GUIElement*) {
    std::cout << "Rozpoczynam animację..." << std::endl;

    const float start_pos_x = static_cast<float>(animatable_panel->getX());
    const float target_pos_x = 500.0f;

    auto on_update_callback = [&animated_x, animatable_panel]() {
        animatable_panel->setPosition(static_cast<int>(animated_x), animatable_panel->getY());
    };

    // Przechwytujemy zmienne lokalne `start_pos_x` i `target_pos_x` przez wartość (kopię),
    // aby uniknąć problemu z czasem życia, gdy `on_click` zakończy działanie.
    Animation::CompleteCallback on_forward_complete = [
        &gui, &animated_x, on_update_callback,
        start_pos_x = start_pos_x,
        target_pos_x = target_pos_x
    ]() {
        std::cout << "Animacja do celu zakończona. Powrót..." << std::endl;
        gui.getAnimationManager()->createAnimation(
            &animated_x,
            target_pos_x,        // start (używa skopiowanej wartości)
            start_pos_x,         // end (używa skopiowanej wartości)
            2000,                // duration
            Easing::easeOutQuad, // easing
            [](){ std::cout << "Animacja powrotna zakończona!" << std::endl; }, // on_complete
            on_update_callback   // on_update
        );
    };
    
    gui.getAnimationManager()->createAnimation(
        &animated_x,
        start_pos_x,           // start
        target_pos_x,          // end
        2000,                  // duration
        Easing::easeOutQuad,   // easing
        on_forward_complete,   // on_complete
        on_update_callback     // on_update
    );
});
        gui.addElement(std::move(button));

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

            // Aktualizacja logiki (w tym animacji)
            gui.cleanup();

            // Renderowanie
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