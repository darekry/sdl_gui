#include "../src/gui_manager.hpp"
#include "../src/context_menu.hpp"
#include "../src/button.hpp"
#include "../src/sdl_app.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        SDLApp app("ContextMenu Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager manager(renderer, Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});

        // Create a button that will trigger the context menu
        auto triggerButton = std::make_unique<Button>(manager, 350, 250, 100, 50, "Right Click Me");
        Button* triggerButtonPtr = triggerButton.get();

        // Create context menu
        auto contextMenu = std::make_unique<ContextMenu>(manager);
        auto contextMenuRef = manager.makeRef(contextMenu.get());

        // Add menu items
        contextMenuRef->addItem("Copy", []() {
            LOG_INFO("ContextMenu", "Copy action triggered!");
        });

        contextMenuRef->addItem("Paste", []() {
            LOG_INFO("ContextMenu", "Paste action triggered!");
        });

        contextMenuRef->addSeparator();

        contextMenuRef->addItem("Delete", []() {
            LOG_INFO("ContextMenu", "Delete action triggered!");
        }, true); // enabled

        contextMenuRef->addItem("Properties", []() {
            LOG_INFO("ContextMenu", "Properties action triggered!");
        });

        // Set up right-click handler for the button (RMB position decides where the menu opens)
        triggerButtonPtr->setOnRightClickCallback([contextMenuRef](GUIElement* element, float x, float y) {
            if (!contextMenuRef) return;
            (void)element;
            contextMenuRef->showAt(static_cast<int>(x), static_cast<int>(y));
        });

        // Add elements to manager
        manager.addElement(std::move(triggerButton));
        manager.addElement(std::move(contextMenu));

        // Main loop
        bool running = true;
        SDL_Event event;

        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }

                manager.processEvent(event);
            }

            manager.update();
            manager.cleanup();

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