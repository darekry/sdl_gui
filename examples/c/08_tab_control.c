/*
 * 08_tab_control.c — TabControl with content panels
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 08_tab_control examples/c/08_tab_control.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>

int main(void) {
    sdlgui_t gui = sdlgui_create("C TabControl", 500, 350, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t tc = sdlgui_tab_control_create(gui, NULL, 20, 20, 460, 300, 30);

    /* Tab 1: basic info */
    sdlgui_element_t tab1 = sdlgui_tab_control_add_tab(tc, "General");
    sdlgui_label_create(gui, tab1, 20, 20, "Name: My Project", -1);
    sdlgui_label_create(gui, tab1, 20, 50, "Version: 1.0.0", -1);
    sdlgui_label_create(gui, tab1, 20, 80, "Author: Kilo", -1);

    /* Tab 2: settings */
    sdlgui_element_t tab2 = sdlgui_tab_control_add_tab(tc, "Settings");
    sdlgui_checkbox_create(gui, tab2, 20, 20, 20, 20);
    sdlgui_label_create(gui, tab2, 50, 20, "Enable logging", -1);
    sdlgui_checkbox_create(gui, tab2, 20, 50, 20, 20);
    sdlgui_label_create(gui, tab2, 50, 50, "Auto-save", -1);

    /* Tab 3: about */
    sdlgui_element_t tab3 = sdlgui_tab_control_add_tab(tc, "About");
    sdlgui_label_create(gui, tab3, 20, 20, "SDL GUI C API Demo", -1);
    sdlgui_label_create(gui, tab3, 20, 50, "Phase 2 Widgets", -1);
    sdlgui_label_create(gui, tab3, 20, 80, "2026-07-14", -1);

    int quit = 0;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = 1;
            sdlgui_process_event(gui, &e);
        }
        sdlgui_update(gui);
        sdlgui_cleanup(gui);
        SDL_SetRenderDrawColor(ren, 40, 42, 54, 255);
        SDL_RenderClear(ren);
        sdlgui_render(gui);
        SDL_RenderPresent(ren);
    }

    sdlgui_destroy(gui);
    return 0;
}
