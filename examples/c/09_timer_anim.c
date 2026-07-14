/*
 * 09_timer_anim.c — Timer + animation driving a ProgressBar
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 09_timer_anim examples/c/09_timer_anim.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_element_t progress;
    sdlgui_element_t label;
    float pct;
    int direction;
} AnimCtx;

static void on_timer(sdlgui_element_t elem, void* data) {
    (void)elem;
    AnimCtx* ctx = (AnimCtx*)data;

    ctx->pct += ctx->direction * 2.0f;
    if (ctx->pct >= 100.0f) { ctx->pct = 100.0f; ctx->direction = -1; }
    if (ctx->pct <= 0.0f)   { ctx->pct = 0.0f;   ctx->direction = 1;  }

    sdlgui_progress_bar_set_value(ctx->progress, ctx->pct);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f%%", ctx->pct);
    sdlgui_label_set_text(ctx->label, buf);
}

static void on_anim_done(void* data) {
    (void)data;
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C Timer + Anim", 400, 200, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t pb = sdlgui_progress_bar_create(gui, NULL, 30, 40, 340, 30);
    sdlgui_progress_bar_set_range(pb, 0.0f, 100.0f);

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 30, 90, "0%", -1);

    /* Label showing animation state */
    sdlgui_element_t status = sdlgui_label_create(gui, NULL, 30, 120, "Timer running...", -1);

    AnimCtx ctx = {pb, lbl, 0.0f, 1};

    /* Repeating timer — fires every 50ms */
    sdlgui_add_timer(gui, NULL, 50, 0, on_timer, &ctx);

    /* Tween animation on the status label opacity (just for demo) */
    float dummy = 0.0f;
    sdlgui_animate_float(gui, &dummy, 0.0f, 1.0f, 2000, SDLGUI_EASING_IN_OUT_QUAD,
        on_anim_done, NULL);

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
