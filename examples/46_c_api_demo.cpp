/*
 * 46_c_api_demo.cpp — Side-by-side C and C++ API usage
 *
 * Demonstrates that C API handles work with C++ widgets and vice versa.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "range_slider.hpp"
#include "cursor.hpp"
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

        GUIManager guiManager(renderer, Viewport{500, 200});
        guiManager.setTheme(Theme::createDefaultTheme());
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

        /* Phase 3 widgets created via C++ API, driven via C API */
        auto rangeCpp = std::make_unique<RangeSlider>(guiManager, 10, 60, 200, 40,
                                                      0, 100, 20, 80, Orientation::Horizontal);
        auto* rangePtr = rangeCpp.get();
        guiManager.addElement(std::move(rangeCpp));

        auto cursorCpp = std::make_unique<Cursor>(guiManager);
        auto* cursorPtr = cursorCpp.get();
        guiManager.setCursor(std::move(cursorCpp));

        sdlgui_element_t rangeHandle = (sdlgui_element_t)rangePtr;
        sdlgui_range_slider_set_on_change(rangeHandle,
            [](sdlgui_element_t elem, void*) {
                SDL_Log("Range: %d..%d",
                        sdlgui_range_slider_get_lower_value(elem),
                        sdlgui_range_slider_get_upper_value(elem));
            },
            nullptr);
        sdlgui_range_slider_set_lower_value(rangeHandle, 30);
        SDL_Log("RangeSlider type via C API: %s", sdlgui_element_get_type(rangeHandle));

        sdlgui_element_t cursorHandle = (sdlgui_element_t)cursorPtr;
        /* Textures are required: without them nothing is drawn for a state
           (the system cursor is hidden while a custom cursor is installed). */
        sdlgui_cursor_set_texture(cursorHandle, SDLGUI_CURSOR_NORMAL,
                                  "assets/button1.png", 8, 8);
        sdlgui_cursor_set_texture(cursorHandle, SDLGUI_CURSOR_HOVER,
                                  "assets/button_bg.png", 16, 16);
        sdlgui_cursor_set_scale(cursorHandle, 0.5f);
        sdlgui_cursor_set_state(cursorHandle, SDLGUI_CURSOR_HOVER);
        SDL_Log("Cursor state via C API: %d (type %s)",
                sdlgui_cursor_get_state(cursorHandle),
                sdlgui_element_get_type(cursorHandle));

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
