/*
 * 03_slider_label.c — Slider updates label via on_change callback
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 03_slider_label examples/c/03_slider_label.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>   /* snprintf */

typedef struct {
    sdlgui_element_t label;
} SliderCtx;

static void on_slider_change(sdlgui_element_t sld, void* data) {
    SliderCtx* ctx = (SliderCtx*)data;
    int value = sdlgui_slider_get_value(sld);
    char buf[32];
    snprintf(buf, sizeof(buf), "Value: %d%%", value);
    sdlgui_label_set_text(ctx->label, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C Slider + Label", 400, 200, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 30, 110, "Value: 50%", -1);
    sdlgui_element_t sld = sdlgui_slider_create(gui, NULL, 30, 50, 340, 40,
                                                  0, 100, 50, SDLGUI_ORIENTATION_HORIZONTAL);

    SliderCtx ctx = {lbl};
    sdlgui_slider_set_on_change(sld, on_slider_change, &ctx);

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
