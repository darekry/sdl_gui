/*
 * 41_c_api_demo.cpp — Side-by-side C and C++ API usage
 *
 * Demonstrates that C API handles work with C++ widgets and vice versa.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "std.hpp"

extern "C" {
#include "sdl_gui.h"
}

static void c_click_callback(sdlgui_element_t elem, void* data) {
    (void)elem;
    auto* lbl = (Label*)data;
    lbl->setText("Clicked from C callback!");
}

int main(int, char**) {
    try {
        SDLApp app("C API Demo", 500, 200);
        SDL_Renderer* renderer = app.getRenderer();

        GUIManager guiManager(renderer);
        guiManager.setTheme(Theme::createDefaultTheme());
        guiManager.setWindowSize(500, 200);

        /* Create widgets via C++ API */
        auto btnCpp = std::make_unique<Button>(guiManager, 10, 10, 150, 40, "C++ Button");
        auto* btnCppPtr = btnCpp.get();
        guiManager.addElement(std::move(btnCpp));

        auto lblCpp = std::make_unique<Label>(guiManager, 180, 10, "Waiting...");
        Label* lblCppPtr = lblCpp.get();
        guiManager.addElement(std::move(lblCpp));

        /* Use C API on the same widget */
        sdlgui_element_t btnHandle = (sdlgui_element_t)btnCppPtr;
        sdlgui_element_t lblHandle = (sdlgui_element_t)lblCppPtr;

        sdlgui_button_set_on_click(btnHandle, c_click_callback, lblCppPtr);

        /* Verify get_type works on C++-created widget */
        SDL_Log("Button type via C API: %s", sdlgui_element_get_type(btnHandle));
        SDL_Log("Label type via C API: %s", sdlgui_element_get_type(lblHandle));

        /* Create widgets via C API (using raw GUIManager* wrapped in sdlgui_t) */
        /* Note: real sdlgui_create creates its own SDLApp. Here we just test the element API. */

        /* Process events to test */
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

    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    return 0;
}
