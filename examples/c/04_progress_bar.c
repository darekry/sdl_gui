/*
 * 04_progress_bar.c — ProgressBar with value display
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 04_progress_bar examples/c/04_progress_bar.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_element_t progress;
    sdlgui_element_t label;
    float value;
    int direction;
} PBCtx;

static void on_animate(sdlgui_element_t btn, void* data) {
    (void)btn;
    PBCtx* ctx = (PBCtx*)data;

    ctx->value += ctx->direction * 2.0f;
    if (ctx->value >= 100.0f) { ctx->value = 100.0f; ctx->direction = -1; }
    if (ctx->value <= 0.0f)   { ctx->value = 0.0f;   ctx->direction = 1;  }

    sdlgui_progress_bar_set_value(ctx->progress, ctx->value);

    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f%%", ctx->value);
    sdlgui_label_set_text(ctx->label, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C ProgressBar", 400, 200, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t pb = sdlgui_progress_bar_create(gui, NULL, 30, 40, 340, 40);
    sdlgui_progress_bar_set_value(pb, 0.0f);
    sdlgui_progress_bar_set_range(pb, 0.0f, 100.0f);

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 30, 90, "0%", -1);

    sdlgui_element_t btn = sdlgui_button_create(gui, NULL, 140, 140, 120, 40, "Step");

    PBCtx ctx = {pb, lbl, 0.0f, 1};
    sdlgui_button_set_on_click(btn, on_animate, &ctx);

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
