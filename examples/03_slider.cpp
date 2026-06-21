#include "gui.hpp"
#include "gui_manager.hpp"
#include "slider.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**) {
    try {
        SDLApp app("Slider Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());
        // Panel with border and rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 120, 80, 560, 210);
        Style panelStyle;
        panelStyle.backgroundColor = {50, 52, 64, 255};
        panelStyle.borderColor = {98, 114, 164, 255};
        panelStyle.borderWidth = 2;
        panelStyle.borderRadius = 10;
        panel->setStyle(ElementState::Normal, panelStyle);
        // White text style shared by labels
        Style lightText;
        lightText.textColor = {255, 255, 255, 255};
        // Title label
        auto title = std::make_unique<Label>(guiManager, 20, 20, "Slider Example", 22);
        title->setStyle(ElementState::Normal, lightText);
        panel->addChild(std::move(title));

        // Label showing current slider value
        auto valueLabel = std::make_unique<Label>(guiManager, 260, 120, "50", 18);
        valueLabel->setStyle(ElementState::Normal, lightText);
        auto valueLabelRef = guiManager.makeRef(valueLabel.get());
        panel->addChild(std::move(valueLabel));

        // Slider with tooltip and change callback
        auto slider = std::make_unique<Slider>(guiManager, 20, 70, 520, 30, 0, 100, 50, Orientation::Horizontal);
        slider->setTooltip("Drag to change value");
        slider->setOnChangeCallback([valueLabelRef](GUIElement* element) {
            Slider* slider_ptr = static_cast<Slider*>(element);
            if (slider_ptr && valueLabelRef) {
                valueLabelRef->setText(std::to_string(slider_ptr->getValue()));
            }
        });
        panel->addChild(std::move(slider));
        guiManager.addElement(std::move(panel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
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
