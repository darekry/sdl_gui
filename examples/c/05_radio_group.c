/*
 * 05_radio_group.c — RadioGroup with TextArea showing selection history
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 05_radio_group examples/c/05_radio_group.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    sdlgui_element_t text_area;
} RadioCtx;

static void on_selection(sdlgui_element_t elem, int index, const char* text, void* data) {
    (void)elem;
    RadioCtx* ctx = (RadioCtx*)data;
    char buf[128];
    snprintf(buf, sizeof(buf), "Selected: [%d] %s\n", index, text);

    const char* old = sdlgui_text_area_get_text(ctx->text_area);
    char combined[512];
    snprintf(combined, sizeof(combined), "%s%s", old, buf);
    sdlgui_text_area_set_text(ctx->text_area, combined);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C RadioGroup", 500, 350, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t rg = sdlgui_radio_group_create(gui, NULL, 20, 20, 200, 150);
    sdlgui_radio_group_add_option(rg, "Option A", 1);
    sdlgui_radio_group_add_option(rg, "Option B", 0);
    sdlgui_radio_group_add_option(rg, "Option C", 0);

    sdlgui_element_t ta = sdlgui_text_area_create(gui, NULL, 240, 20, 230, 200, "", 14);
    sdlgui_text_area_set_locked(ta, 1);

    RadioCtx ctx = {ta};
    sdlgui_radio_group_set_on_selection_change(rg, on_selection, &ctx);

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
