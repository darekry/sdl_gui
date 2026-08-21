#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "text_input.hpp"
#include "theme.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main() {
    try {
        SDLApp app("Rounded Corners Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());

        // Demonstration 1: Panel with rounded corners and a border
        auto roundedPanel = std::make_unique<Panel>(guiManager, 50, 60, 320, 160);
        roundedPanel->setBackgroundColor(ElementState::Normal, {40, 42, 54, 255});
        roundedPanel->setBorder(ElementState::Normal, {98, 114, 164, 255}, 2);
        roundedPanel->setBorderRadius(ElementState::Normal, 12);
        auto panelLabel = std::make_unique<Label>(guiManager, 100, 70, "borderRadius = 12", 16);
        panelLabel->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        roundedPanel->addChild(std::move(panelLabel));
        guiManager.addElement(std::move(roundedPanel));

        auto desc1 = std::make_unique<Label>(guiManager, 50, 230, "Rounded panels are ideal for cards and containers.", 14);
        guiManager.addElement(std::move(desc1));

        // Demonstration 2: Pill-shaped button with a large border radius
        auto pillBtn = std::make_unique<Button>(guiManager, 50, 280, 220, 50, "Pill Button");
        pillBtn->setBorderRadius(ElementState::Normal, 20);
        pillBtn->setBackgroundColor(ElementState::Normal, {70, 130, 180, 255});
        pillBtn->setBackgroundColor(ElementState::Hover, {100, 150, 200, 255});
        pillBtn->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        pillBtn->setTooltip("A fully rounded pill-shaped button");
        guiManager.addElement(std::move(pillBtn));

        auto desc2 = std::make_unique<Label>(guiManager, 50, 340, "Pill button with borderRadius = 20", 14);
        guiManager.addElement(std::move(desc2));

        // Demonstration 3: Text input with subtle rounded corners
        auto textInput = std::make_unique<TextInput>(guiManager, 50, 390, 320, 40);
        textInput->setBorderRadius(ElementState::Normal, 8);
        textInput->setText("Rounded text input");
        guiManager.addElement(std::move(textInput));

        auto desc3 = std::make_unique<Label>(guiManager, 50, 440, "Text input with borderRadius = 8", 14);
        guiManager.addElement(std::move(desc3));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 30, 30, 46, 255);
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
