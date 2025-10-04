#include "../src/gui_manager.hpp"
#include "../src/context_menu.hpp"
#include "../src/button.hpp"
#include "../src/sdl_app.hpp"
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        SDLApp app("ContextMenu Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager manager(renderer);

        // Create a button that will trigger the context menu
        auto triggerButton = std::make_unique<Button>(manager, 350, 250, 100, 50, "Right Click Me");
        Button* triggerButtonPtr = triggerButton.get();

        // Create context menu
        auto contextMenu = std::make_unique<ContextMenu>(manager);
        ContextMenu* contextMenuPtr = contextMenu.get();

        // Add menu items
        contextMenuPtr->addItem("Copy", []() {
            std::cout << "Copy action triggered!" << std::endl;
        });

        contextMenuPtr->addItem("Paste", []() {
            std::cout << "Paste action triggered!" << std::endl;
        });

        contextMenuPtr->addSeparator();

        contextMenuPtr->addItem("Delete", []() {
            std::cout << "Delete action triggered!" << std::endl;
        }, true); // enabled

        contextMenuPtr->addItem("Properties", []() {
            std::cout << "Properties action triggered!" << std::endl;
        });

        // Set up right-click handler for the button
        triggerButtonPtr->setOnClickCallback([&](GUIElement* element) {
            // For this example, we'll simulate right-click with left-click
            // In real usage, you'd check for SDL_BUTTON_RIGHT
          
                auto pos = element->getAbsolutePosition();
                contextMenuPtr->showAt(pos.x, pos.y + element->getHeight());
            
        });

        // Add elements to manager
        manager.addElement(std::move(triggerButton));
        manager.addElement(std::move(contextMenu));

        // Main loop
        bool running = true;
        SDL_Event event;

        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }

                manager.processEvent(event);
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            manager.render();

            SDL_RenderPresent(renderer);
            SDL_Delay(16); // ~60 FPS
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}