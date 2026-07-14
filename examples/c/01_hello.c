/*
 * 01_hello.c — Minimal C API boilerplate
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 01_hello examples/c/01_hello.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>

int main(void) {
    sdlgui_t gui = sdlgui_create("Hello C API", 800, 600, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t btn = sdlgui_button_create(gui, NULL, 10, 10, 120, 40, "Click Me");
    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 10, 60, "Hello from C!", -1);

    (void)btn;
    (void)lbl;

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
