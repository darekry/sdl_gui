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

import std.compat;

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
    checkbox_style.textColor = accent;
    theme.setStyle("Checkbox", checkbox_style);

    Style panel_style;
    panel_style.backgroundColor = {220, 220, 220, 255};
    theme.setStyle("Panel", panel_style);
    theme.setStyle("Window", panel_style);

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
        SDLApp app("Theme Playground: States & Colors", 1270, 900);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager manager(renderer);

        Theme light_theme = createLightTheme();
        Theme dark_theme = createDarkTheme();
        manager.setTheme(light_theme);

        auto main_panel = std::make_unique<Panel>(manager, 50, 50, 900, 800);

        // Panel z kontrolkami demonstracyjnymi
        auto demo_panel = std::make_unique<Panel>(manager, 20, 20, 400, 400);
        demo_panel->addChild(std::make_unique<Label>(manager, 20, 20, "Demo Controls"));
        demo_panel->addChild(std::make_unique<Button>(manager, 20, 60, 150, 40, "Hover Me"));
        demo_panel->addChild(std::make_unique<Checkbox>(manager, 20, 120, 30, 30));
        demo_panel->addChild(std::make_unique<Label>(manager, 60, 125, "Checkbox"));
        demo_panel->addChild(std::make_unique<Slider>(manager, 20, 180, 200, 30, 0, 100, 50, Orientation::Horizontal));
        demo_panel->addChild(std::make_unique<Label>(manager, 20, 220, "Slider"));

        main_panel->addChild(std::move(demo_panel));

        // Panel z kontrolkami motywów
        auto theme_panel = std::make_unique<Panel>(manager, 450, 20, 400, 900);
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 20, "Theme Controls"));

        // Przełącznik motywów
        auto radio_group = std::make_unique<RadioGroup>(manager, 20, 50, 200, 80);
        auto radio_light = std::make_unique<RadioButton>(manager, 10, 10, 180, 30);
        radio_light->addChild(std::make_unique<Label>(manager, 5, 5, "Light Theme"));
        radio_light->setOnChange([&, light_theme]([[maybe_unused]] RadioButton* rb, bool selected){
            if (selected) manager.setTheme(light_theme);
        });
        auto radio_dark = std::make_unique<RadioButton>(manager, 10, 40, 180, 30);
        radio_dark->addChild(std::make_unique<Label>(manager, 5, 5, "Dark Theme"));
        radio_dark->setOnChange([&, dark_theme]([[maybe_unused]] RadioButton* rb, bool selected){
            if (selected) manager.setTheme(dark_theme);
        });
        radio_light->setSelected(true);
        radio_group->addChild(std::move(radio_light));
        radio_group->addChild(std::move(radio_dark));
        theme_panel->addChild(std::move(radio_group));

        // Suwaki RGB dla akcentu
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 150, "Accent Color RGB"));
        auto slider_r = std::make_unique<Slider>(manager, 20, 180, 150, 30, 0, 255, 120, Orientation::Horizontal);
        auto slider_g = std::make_unique<Slider>(manager, 20, 220, 150, 30, 0, 255, 120, Orientation::Horizontal);
        auto slider_b = std::make_unique<Slider>(manager, 20, 260, 150, 30, 0, 255, 215, Orientation::Horizontal);

        Slider* slider_r_ptr = slider_r.get();
        Slider* slider_g_ptr = slider_g.get();
        Slider* slider_b_ptr = slider_b.get();

        // Deklaracje raw pointerów dla nowych sliderów
        Slider* slider_border_r_ptr = nullptr;
        Slider* slider_border_g_ptr = nullptr;
        Slider* slider_border_b_ptr = nullptr;
        Slider* slider_border_width_ptr = nullptr;
        Slider* slider_bg_r_ptr = nullptr;
        Slider* slider_bg_g_ptr = nullptr;
        Slider* slider_bg_b_ptr = nullptr;
        Slider* slider_text_r_ptr = nullptr;
        Slider* slider_text_g_ptr = nullptr;
        Slider* slider_text_b_ptr = nullptr;

        theme_panel->addChild(std::make_unique<Label>(manager, 180, 185, "R"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 225, "G"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 265, "B"));

        // Callback do aktualizacji motywu na podstawie wszystkich suwaków
        auto update_all_styles = [&]() {
            // Aktualizacja accent
            int r = slider_r_ptr->getValue();
            int g = slider_g_ptr->getValue();
            int b = slider_b_ptr->getValue();
            SDL_Color new_accent = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), 255};

            // Aktualizacja border
            int border_r = slider_border_r_ptr->getValue();
            int border_g = slider_border_g_ptr->getValue();
            int border_b = slider_border_b_ptr->getValue();
            SDL_Color new_border_color = {static_cast<Uint8>(border_r), static_cast<Uint8>(border_g), static_cast<Uint8>(border_b), 255};
            int border_width = slider_border_width_ptr->getValue();

            // Aktualizacja background
            int bg_r = slider_bg_r_ptr->getValue();
            int bg_g = slider_bg_g_ptr->getValue();
            int bg_b = slider_bg_b_ptr->getValue();
            SDL_Color new_bg_color = {static_cast<Uint8>(bg_r), static_cast<Uint8>(bg_g), static_cast<Uint8>(bg_b), 255};

            // Aktualizacja text color
            int text_r = slider_text_r_ptr->getValue();
            int text_g = slider_text_g_ptr->getValue();
            int text_b = slider_text_b_ptr->getValue();
            SDL_Color new_text_color = {static_cast<Uint8>(text_r), static_cast<Uint8>(text_g), static_cast<Uint8>(text_b), 255};

            Theme current_theme = manager.getTheme();

            // Aktualizacja default_style
            Style default_style = current_theme.getDefaultStyle();
            default_style.backgroundColor = new_bg_color;
            default_style.textColor = new_text_color;
            default_style.borderColor = new_border_color;
            default_style.borderWidth = border_width;
            current_theme.setDefaultStyle(default_style);

            // Aktualizacja button_style
            Style button_style = current_theme.getStyle("Button");
            button_style.backgroundColor = new_accent;
            button_style.borderColor = new_accent;
            button_style.textColor = new_text_color; // lub z accent jak wcześniej
            button_style.borderWidth = border_width;
            current_theme.setStyle("Button", button_style);

            // Aktualizacja checkbox_style
            Style checkbox_style = current_theme.getStyle("Checkbox");
            checkbox_style.textColor = new_accent;
            checkbox_style.borderColor = new_border_color;
            checkbox_style.borderWidth = border_width;
            current_theme.setStyle("Checkbox", checkbox_style);

            // Aktualizacja panel_style
            Style panel_style = current_theme.getStyle("Panel");
            panel_style.backgroundColor = new_bg_color; // lub dostosować
            panel_style.borderColor = new_border_color;
            panel_style.borderWidth = border_width;
            panel_style.textColor = new_text_color;
            current_theme.setStyle("Panel", panel_style);
            current_theme.setStyle("Window", panel_style);

            manager.setTheme(current_theme);
        };

        // Slidery dla Border Color
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 300, "Border Color RGB"));
        auto slider_border_r = std::make_unique<Slider>(manager, 20, 330, 150, 30, 0, 255, 180, Orientation::Horizontal);
        slider_border_r_ptr = slider_border_r.get();
        slider_border_r->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_border_r));

        auto slider_border_g = std::make_unique<Slider>(manager, 20, 370, 150, 30, 0, 255, 180, Orientation::Horizontal);
        slider_border_g_ptr = slider_border_g.get();
        slider_border_g->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_border_g));

        auto slider_border_b = std::make_unique<Slider>(manager, 20, 410, 150, 30, 0, 255, 180, Orientation::Horizontal);
        slider_border_b_ptr = slider_border_b.get();
        slider_border_b->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_border_b));

        theme_panel->addChild(std::make_unique<Label>(manager, 180, 335, "R"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 375, "G"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 415, "B"));

        // Slider dla Border Width
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 450, "Border Width"));
        auto slider_border_width = std::make_unique<Slider>(manager, 20, 480, 150, 30, 0, 10, 1, Orientation::Horizontal);
        slider_border_width_ptr = slider_border_width.get();
        slider_border_width->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_border_width));

        // Slidery dla Background Color (default_style)
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 520, "Background Color RGB"));
        auto slider_bg_r = std::make_unique<Slider>(manager, 20, 550, 150, 30, 0, 255, 240, Orientation::Horizontal);
        slider_bg_r_ptr = slider_bg_r.get();
        slider_bg_r->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_bg_r));

        auto slider_bg_g = std::make_unique<Slider>(manager, 20, 590, 150, 30, 0, 255, 240, Orientation::Horizontal);
        slider_bg_g_ptr = slider_bg_g.get();
        slider_bg_g->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_bg_g));

        auto slider_bg_b = std::make_unique<Slider>(manager, 20, 630, 150, 30, 0, 255, 240, Orientation::Horizontal);
        slider_bg_b_ptr = slider_bg_b.get();
        slider_bg_b->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_bg_b));

        theme_panel->addChild(std::make_unique<Label>(manager, 180, 555, "R"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 595, "G"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 635, "B"));

        // Slidery dla Text Color (default_style)
        theme_panel->addChild(std::make_unique<Label>(manager, 20, 670, "Text Color RGB"));
        auto slider_text_r = std::make_unique<Slider>(manager, 20, 700, 150, 30, 0, 255, 20, Orientation::Horizontal);
        slider_text_r_ptr = slider_text_r.get();
        slider_text_r->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_text_r));

        auto slider_text_g = std::make_unique<Slider>(manager, 20, 740, 150, 30, 0, 255, 20, Orientation::Horizontal);
        slider_text_g_ptr = slider_text_g.get();
        slider_text_g->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_text_g));

        auto slider_text_b = std::make_unique<Slider>(manager, 20, 780, 150, 30, 0, 255, 20, Orientation::Horizontal);
        slider_text_b_ptr = slider_text_b.get();
        slider_text_b->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        theme_panel->addChild(std::move(slider_text_b));

        theme_panel->addChild(std::make_unique<Label>(manager, 180, 705, "R"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 745, "G"));
        theme_panel->addChild(std::make_unique<Label>(manager, 180, 785, "B"));


        slider_r->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        slider_g->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });
        slider_b->setOnChangeCallback([&, update_all_styles]([[maybe_unused]] GUIElement* s) { update_all_styles(); });

        theme_panel->addChild(std::move(slider_r));
        theme_panel->addChild(std::move(slider_g));
        theme_panel->addChild(std::move(slider_b));

        main_panel->addChild(std::move(theme_panel));
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