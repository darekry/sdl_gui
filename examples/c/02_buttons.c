/*
 * 02_buttons.c — Buttons with click callbacks that change label text
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 02_buttons examples/c/02_buttons.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <string.h>

/* Userdata struct for callbacks */
typedef struct {
    sdlgui_element_t label;
} ButtonCtx;

static void on_increment(sdlgui_element_t btn, void* data) {
    (void)btn;
    ButtonCtx* ctx = (ButtonCtx*)data;
    sdlgui_label_set_text(ctx->label, "Incremented!");
}

static void on_reset(sdlgui_element_t btn, void* data) {
    (void)btn;
    ButtonCtx* ctx = (ButtonCtx*)data;
    sdlgui_label_set_text(ctx->label, "Reset!");
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C Buttons", 400, 200, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 20, 120, "No clicks yet", -1);
    sdlgui_element_t btn1 = sdlgui_button_create(gui, NULL, 20, 20, 140, 40, "Increment");
    sdlgui_element_t btn2 = sdlgui_button_create(gui, NULL, 180, 20, 140, 40, "Reset");

    ButtonCtx ctx = {lbl};

    sdlgui_button_set_on_click(btn1, on_increment, &ctx);
    sdlgui_button_set_on_click(btn2, on_reset, &ctx);

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
