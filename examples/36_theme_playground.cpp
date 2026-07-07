#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "theme_presets.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "slider.hpp"
#include "label.hpp"
#include "radio_button.hpp"
#include "radio_group.hpp"
#include "std.hpp"

int main(int, char*[]) {
    try {
        SDLApp app("Theme Playground", 800, 500);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer);

        Theme light = ThemePresets::createLightTheme();
        Theme dark = ThemePresets::createDarkTheme();
        gui.setTheme(light);

        auto mainPanel = std::make_unique<Panel>(gui, 20, 20, 760, 460);

        // Demo panel — widgets styled by the current theme
        auto demoPanel = std::make_unique<Panel>(gui, 10, 10, 350, 240);
        demoPanel->addChild(std::make_unique<Label>(gui, 10, 10, "Demo Controls"));
        demoPanel->addChild(std::make_unique<Button>(gui, 10, 50, 150, 40, "Click Me"));
        demoPanel->addChild(std::make_unique<Checkbox>(gui, 10, 110, 30, 30));
        demoPanel->addChild(std::make_unique<Label>(gui, 50, 115, "Checkbox"));
        demoPanel->addChild(std::make_unique<Slider>(gui, 10, 160, 200, 30, 0, 100, 50,
                                                     Orientation::Horizontal));
        demoPanel->addChild(std::make_unique<Label>(gui, 10, 200, "Slider"));
        mainPanel->addChild(std::move(demoPanel));

        // Theme switcher — RadioGroup toggles between Light and Dark themes
        auto themePanel = std::make_unique<Panel>(gui, 380, 10, 350, 240);
        themePanel->addChild(std::make_unique<Label>(gui, 10, 10, "Switch Theme"));

        auto radioGroup = std::make_unique<RadioGroup>(gui, 10, 50, 300, 80);
        auto radioLight = std::make_unique<RadioButton>(gui, 10, 10, 250, 30);
        radioLight->addChild(std::make_unique<Label>(gui, 5, 5, "Light Theme"));
        radioLight->setSelected(true);
        radioLight->setOnChange([&, light](RadioButton*, bool sel) { if (sel) gui.setTheme(light); });
        auto radioDark = std::make_unique<RadioButton>(gui, 10, 40, 250, 30);
        radioDark->addChild(std::make_unique<Label>(gui, 5, 5, "Dark Theme"));
        radioDark->setOnChange([&, dark](RadioButton*, bool sel) { if (sel) gui.setTheme(dark); });
        radioGroup->addChild(std::move(radioLight));
        radioGroup->addChild(std::move(radioDark));
        themePanel->addChild(std::move(radioGroup));
        mainPanel->addChild(std::move(themePanel));
        gui.addElement(std::move(mainPanel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }
            gui.update();
            gui.cleanup();
            const auto& bg = gui.getTheme().getDefaultStyle();
            SDL_Color bgColor = bg.backgroundColor.value_or(SDL_Color{240, 240, 240, 255});
            SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
            SDL_RenderClear(renderer);
            gui.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
