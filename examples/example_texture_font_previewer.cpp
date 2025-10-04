#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "combobox.hpp"
#include "text_input.hpp"
#include "label.hpp"
#include "texture_manager.hpp"
#include "font_manager.hpp"
#include <iostream>
#include <memory>

int main() {
    try {
        SDLApp app("Texture & Font Previewer", 1200, 800);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer);

        // Główny panel
        auto main_panel = std::make_unique<Panel>(gui, 50, 50, 1100, 700);

        // Panel dla tekstur
        auto texture_panel = std::make_unique<Panel>(gui, 20, 20, 500, 300);
        texture_panel->addChild(std::make_unique<Label>(gui, 20, 20, "Texture Preview"));
        main_panel->addChild(std::move(texture_panel));

        // Panel dla czcionek
        auto font_panel = std::make_unique<Panel>(gui, 550, 20, 500, 300);
        font_panel->addChild(std::make_unique<Label>(gui, 20, 20, "Font Preview"));
        main_panel->addChild(std::move(font_panel));

        // Panel kontrolny
        auto control_panel = std::make_unique<Panel>(gui, 20, 350, 1030, 300);
        control_panel->addChild(std::make_unique<Label>(gui, 20, 20, "Controls"));

        // TextInput dla tekstu
        auto text_input = std::make_unique<TextInput>(gui, 20, 50, 300, 40);
        text_input->setText(std::string("Hello World!"));
        control_panel->addChild(std::move(text_input));

        // ComboBox dla czcionek
        auto font_combo = std::make_unique<ComboBox>(gui, 350, 50, 200, 40);
        font_combo->addItem("DejaVuSans.ttf");
        font_combo->addItem("DejaVuSansMono.ttf");
        font_combo->setSelectedIndex(0);
        control_panel->addChild(std::move(font_combo));

        // ComboBox dla rozmiarów
        auto size_combo = std::make_unique<ComboBox>(gui, 580, 50, 150, 40);
        size_combo->addItem("12");
        size_combo->addItem("16");
        size_combo->addItem("20");
        size_combo->addItem("24");
        size_combo->addItem("32");
        size_combo->setSelectedIndex(2); // 20
        control_panel->addChild(std::move(size_combo));

        // Label dla wymiarów
        auto size_label = std::make_unique<Label>(gui, 760, 55, "Size: 0x0");
        control_panel->addChild(std::move(size_label));

        // Button do odświeżenia
        auto refresh_btn = std::make_unique<Button>(gui, 20, 120, 100, 40, "Refresh");
        refresh_btn->setOnClickCallback([&](GUIElement*) {
            // Odświeżenie podglądu
            std::cout << "Refresh clicked\n";
        });
        control_panel->addChild(std::move(refresh_btn));

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