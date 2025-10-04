#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "animated_image.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "combobox.hpp"
#include "slider.hpp"
#include "label.hpp"
#include <iostream>
#include <memory>

int main() {
    try {
        SDLApp app("Sprite Animator: Controls & FPS", 1000, 700);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer);

        // Główny panel dla animacji
        auto main_panel = std::make_unique<Panel>(gui, 50, 50, 900, 600);

        // AnimatedImage z sprite-sheet
        auto anim_uptr = std::make_unique<AnimatedImage>(gui, 50, 50, 256, 256);
        AnimatedImage* anim_ptr = anim_uptr.get();
        anim_uptr->setSpriteSheet("assets/sprites/placeholder.png", 4, 4); // Zakładamy 4x4 klatki
        anim_uptr->setFPS(12.0f);
        anim_uptr->setLoop(true);
        anim_uptr->setUseCache(false); // Direct render dla płynności
        main_panel->addChild(std::move(anim_uptr));

        // Panel kontrolny
        auto control_panel = std::make_unique<Panel>(gui, 350, 50, 500, 250);
        control_panel->addChild(std::make_unique<Label>(gui, 20, 20, "Animation Controls"));

        // Przycisk Start/Stop
        auto start_stop_btn = std::make_unique<Button>(gui, 20, 50, 120, 40, "Start");
        bool is_playing = false;
        start_stop_btn->setOnClickCallback([&](GUIElement*) {
            if (is_playing) {
                anim_ptr->pause();
                //start_stop_btn->setText("Resume");
            } else {
                anim_ptr->play();
               // start_stop_btn->setText("Pause");
            }
            is_playing = !is_playing;
        });
        control_panel->addChild(std::move(start_stop_btn));

        // Przycisk Stop
        auto stop_btn = std::make_unique<Button>(gui, 160, 50, 100, 40, "Stop");
        stop_btn->setOnClickCallback([&](GUIElement*) {
            anim_ptr->stop();
            is_playing = false;
          //  start_stop_btn->setText("Start");
        });
        control_panel->addChild(std::move(stop_btn));

        // ComboBox dla preset prędkości
        auto speed_combo = std::make_unique<ComboBox>(gui, 20, 110, 150, 40);
        speed_combo->addItem("Slow (6 FPS)");
        speed_combo->addItem("Normal (12 FPS)");
        speed_combo->addItem("Fast (24 FPS)");
        speed_combo->addItem("Very Fast (60 FPS)");
        speed_combo->setSelectedIndex(1); // Normal
        speed_combo->on_selection_changed = [&](int index, [[maybe_unused]] const std::string& item) {
            float fps = 12.0f;
            switch (index) {
                case 0: fps = 6.0f; break;
                case 1: fps = 12.0f; break;
                case 2: fps = 24.0f; break;
                case 3: fps = 60.0f; break;
            }
            anim_ptr->setFPS(fps);
        };
        control_panel->addChild(std::move(speed_combo));
        control_panel->addChild(std::make_unique<Label>(gui, 180, 115, "Preset Speed"));

        // Slider dla regulacji FPS
        auto fps_slider = std::make_unique<Slider>(gui, 20, 170, 200, 30, 1, 120, 12, Orientation::Horizontal);
        fps_slider->setOnChangeCallback([&](GUIElement*) {
            int fps = fps_slider->getValue();
            anim_ptr->setFPS(static_cast<float>(fps));
        });
        control_panel->addChild(std::move(fps_slider));
        control_panel->addChild(std::make_unique<Label>(gui, 230, 175, "Custom FPS"));

        main_panel->addChild(std::move(control_panel));
        gui.addElement(std::move(main_panel));

        // Główna pętla
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) quit = true;
                gui.processEvent(e);
            }

            gui.cleanup();

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);
            gui.render();
            SDL_RenderPresent(renderer);
        }

    } catch (const std::exception& e) {
        std::cerr << "Wyjątek: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}