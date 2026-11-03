/*
 * 15_custom_control.c — owner-draw control with draw/event hooks
 *
 * The control paints itself through a DRAW_REPLACE hook and swallows clicks
 * in an event hook, then reports a command id to its owning form.
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 15_custom_control examples/c/15_custom_control.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_t gui;
    sdlgui_element_t form;
    int clicks;
} AppCtx;

static void paint(sdlgui_element_t e, SDL_Renderer* renderer,
                  const sdlgui_rect_t* rect, const sdlgui_rect_t* clip,
                  void* data) {
    (void)e;
    (void)clip;
    AppCtx* app = (AppCtx*)data;
    SDL_SetRenderDrawColor(renderer, 40, 120, 200, 255);
    SDL_FRect r = {(float)rect->x, (float)rect->y,
                   (float)rect->w, (float)rect->h};
    SDL_RenderFillRect(renderer, &r);
    (void)app;
}

static int on_event(sdlgui_element_t e, const SDL_Event* event,
                    int local_x, int local_y, void* data) {
    AppCtx* app = (AppCtx*)data;
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        (void)local_x;
        (void)local_y;
        ++app->clicks;
        sdlgui_element_notify_command(e, 42);
        return 1;
    }
    return 0;
}

static void on_command(sdlgui_element_t form, int id, void* data) {
    AppCtx* app = (AppCtx*)data;
    if (id == 42) {
        sdlgui_form_end(form, 42);
    }
}

int main(void) {
    AppCtx app = {0};
    app.gui = sdlgui_create("CustomControl", 480, 320, 1);
    if (!app.gui) return 1;

    app.form = sdlgui_form_create(app.gui, "Owner draw", 40, 40, 320, 180);
    sdlgui_form_set_on_command(app.form, on_command, &app);

    sdlgui_element_t control = sdlgui_form_add_custom(app.form, 42, 20, 40, 280, 100);
    sdlgui_element_set_draw_hook(control, SDLGUI_DRAW_REPLACE, paint, &app);
    sdlgui_element_set_event_hook(control, on_event, &app);

    sdlgui_modal_options_t opts = {0};
    opts.keep_background = 0;
    opts.cancel_key = SDLK_ESCAPE;
    opts.cancel_result = -1;

    int result = -1;
    sdlgui_modal_run(app.gui, app.form, &opts, &result);
    printf("clicks: %d, result: %d\n", app.clicks, result);

    sdlgui_destroy(app.gui);
    return 0;
}
