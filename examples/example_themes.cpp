#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "sdl_app.hpp"
#include <iostream>
#include <optional>

// Definicja jasnego motywu
Theme createLightTheme() {
    Theme theme;
    
    Style default_style;
    default_style.backgroundColor = {240, 240, 240, 255};
    default_style.textColor = {20, 20, 20, 255};
    default_style.borderColor = {180, 180, 180, 255};
    default_style.borderWidth = 1;
    theme.setDefaultStyle(default_style);

    SDL_Color accent = {0, 120, 215, 255};
    SDL_Color text_on_accent = {255, 255, 255, 255};

    Style button_style;
    button_style.backgroundColor = accent;
    button_style.textColor = text_on_accent;
    button_style.borderColor = accent;
    theme.setStyle("Button", button_style);

    Style checkbox_style;
    checkbox_style.textColor = accent; // Kolor "fajki" w stanie Pressed
    theme.setStyle("Checkbox", checkbox_style);

    Style panel_style;
    panel_style.backgroundColor = {220, 220, 220, 255};
    theme.setStyle("Panel", panel_style);
    theme.setStyle("Window", panel_style); // Window to alias dla Panel w tym kontekście

    return theme;
}

// Definicja ciemnego motywu
Theme createDarkTheme() {
    Theme theme;

    Style default_style;
    default_style.backgroundColor = {50, 50, 50, 255};
    default_style.textColor = {220, 220, 220, 255};
    default_style.borderColor = {90, 90, 90, 255};
    default_style.borderWidth = 1;
    theme.setDefaultStyle(default_style);

    SDL_Color accent = {20, 140, 235, 255};

    Style button_style;
    button_style.backgroundColor = accent;
    button_style.borderColor = accent;
    theme.setStyle("Button", button_style);

    Style checkbox_style;
    checkbox_style.textColor = accent;
    theme.setStyle("Checkbox", checkbox_style);

    Style panel_style;
    panel_style.backgroundColor = {35, 35, 35, 255};
    theme.setStyle("Panel", panel_style);
    theme.setStyle("Window", panel_style);

    return theme;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        SDLApp app("Theme Switching Example", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager manager(renderer);

        Theme light_theme = createLightTheme();
        Theme dark_theme = createDarkTheme();
        manager.setTheme(light_theme);

        auto main_panel = std::make_unique<Panel>(manager, 50, 50, 700, 500);
        // main_panel->setDraggable(true);

        main_panel->addChild(std::make_unique<Label>(manager, 20, 20, "Hello Themed World!"));
        
        auto button = std::make_unique<Button>(manager, 20, 60, 150, 40, "Themed Button");
        Style button_hover_style;
        button_hover_style.backgroundColor = {0, 100, 185, 255}; // Jasny motyw - hover
        button->setStyle(ElementState::Hover, button_hover_style);
        main_panel->addChild(std::move(button));

        auto checkbox = std::make_unique<Checkbox>(manager, 20, 120, 30, 30);
        checkbox->setChecked(true);
        main_panel->addChild(std::move(checkbox));

        main_panel->addChild(std::make_unique<Label>(manager, 60, 125, "Themed Checkbox"));
        main_panel->addChild(std::make_unique<Label>(manager, 20, 165, "Themed Slider"));
        main_panel->addChild(std::make_unique<Slider>(manager, 20, 190, 200, 30, 0, 100, 50, Orientation::Horizontal));

        auto radio_group = std::make_unique<RadioGroup>(manager, 480, 20, 200, 100);
        
        auto radio_light = std::make_unique<RadioButton>(manager, 10, 10, 180, 30);
        radio_light->addChild(std::make_unique<Label>(manager, 5, 5, "Light Theme"));
        radio_light->setOnChange([&, light_theme]([[maybe_unused]] RadioButton* rb, bool selected){
            if (selected) manager.setTheme(light_theme);
        });

        auto radio_dark = std::make_unique<RadioButton>(manager, 10, 50, 180, 30);
        radio_dark->addChild(std::make_unique<Label>(manager, 5, 5, "Dark Theme"));
        radio_dark->setOnChange([&, dark_theme]([[maybe_unused]] RadioButton* rb, bool selected){
            if (selected) manager.setTheme(dark_theme);
        });

        radio_light->setSelected(true);
        radio_group->addChild(std::move(radio_light));
        radio_group->addChild(std::move(radio_dark));
        
        main_panel->addChild(std::move(radio_group));
        manager.addElement(std::move(main_panel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) quit = true;
                manager.processEvent(e);
            }

            const auto& bg_style = manager.getTheme().getDefaultStyle();
            SDL_Color bg_color = bg_style.backgroundColor.value_or(SDL_Color{255,255,255,255});

            SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
            SDL_RenderClear(renderer);

            manager.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}