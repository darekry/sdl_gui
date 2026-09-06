#include "button.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
int main(int, char**) {
    try {
        SDLApp app("Button Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());

        // Default themed button
        auto buttonDefault = std::make_unique<Button>(guiManager, 50, 50, 200, 50, "Default Button");
        buttonDefault->setTooltip("A simple button with default theme styling");
        buttonDefault->setOnClickCallback([](GUIElement*) {
            LOG_INFO("Button", "Default Button clicked!");
        });
        guiManager.addElement(std::move(buttonDefault));

        // Colored button with hover/pressed styles (green scheme)
        auto buttonColored = std::make_unique<Button>(guiManager, 50, 130, 200, 50, "Colored Button");
        buttonColored->setBackgroundColor(ElementState::Normal, {0, 120, 0, 255});
        buttonColored->setBackgroundColor(ElementState::Hover, {0, 180, 0, 255});
        buttonColored->setBackgroundColor(ElementState::Pressed, {0, 80, 0, 255});
        buttonColored->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        buttonColored->setTooltip("Green color scheme with hover and pressed states");
        buttonColored->setOnClickCallback([](GUIElement*) {
            LOG_INFO("Button", "Colored Button clicked!");
        });
        guiManager.addElement(std::move(buttonColored));

        // Textured button with border radius
        auto buttonTextured = std::make_unique<Button>(guiManager, 50, 210, 200, 50, "Textured Button");
        SharedTexture buttonTexture = guiManager.getTextureManager().loadTexture("assets/button_bg.png");

        Style tex_normal;
        tex_normal.texture = buttonTexture;
        tex_normal.textColor = {255, 255, 255, 255};
        tex_normal.borderRadius = 8;
        buttonTextured->setStyle(ElementState::Normal, tex_normal);

        Style tex_hover;
        tex_hover.texture = buttonTexture;
        tex_hover.textColor = {255, 255, 100, 255};
        tex_hover.borderRadius = 8;
        buttonTextured->setStyle(ElementState::Hover, tex_hover);

        Style tex_pressed;
        tex_pressed.texture = buttonTexture;
        tex_pressed.textColor = {150, 150, 150, 255};
        tex_pressed.borderRadius = 8;
        buttonTextured->setStyle(ElementState::Pressed, tex_pressed);

        buttonTextured->setTooltip("Textured button with rounded corners");
        buttonTextured->setOnClickCallback([](GUIElement*) {
            LOG_INFO("Button", "Textured Button clicked!");
        });
        guiManager.addElement(std::move(buttonTextured));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
