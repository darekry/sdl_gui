#include "sdl_gui.hpp"

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Standalone SDL GUI Example", 800, 600, 0);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    GUIManager manager(renderer);
    manager.setWindowSize(800, 600);

    // Apply default theme
    Theme theme = Theme::createDefaultTheme();
    manager.setTheme(theme);

    // Create root panel with children
    auto panel = std::make_unique<Panel>(manager, 50, 50, 700, 500);

    auto label = std::make_unique<Label>(manager, 20, 20, "Hello from standalone build!", 32);
    panel->addChild(std::move(label));

    auto button = std::make_unique<Button>(manager, 20, 80, 200, 50, "Click me");
    button->setOnClickCallback([](GUIElement*) {
        SDL_Log("Button clicked!");
    });
    button->setBackgroundColor(ElementState::Normal, {0, 120, 0, 255});
    button->setBackgroundColor(ElementState::Hover,  {0, 180, 0, 255});
    button->setBackgroundColor(ElementState::Pressed, {0, 80, 0, 255});
    panel->addChild(std::move(button));

    manager.addElement(std::move(panel));

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            manager.processEvent(event);
        }
        manager.update();
        manager.cleanup();
        SDL_RenderClear(renderer);
        manager.render();
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
