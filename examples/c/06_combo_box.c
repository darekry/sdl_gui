/*
 * 06_combo_box.c — ComboBox with selection feedback
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 06_combo_box examples/c/06_combo_box.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_element_t label;
} ComboCtx;

static void on_combo_select(sdlgui_element_t elem, int index, const char* text, void* data) {
    (void)elem;
    ComboCtx* ctx = (ComboCtx*)data;
    char buf[64];
    snprintf(buf, sizeof(buf), "Choice: %s (index %d)", text, index);
    sdlgui_label_set_text(ctx->label, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C ComboBox", 400, 200, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t cb = sdlgui_combo_box_create(gui, NULL, 30, 30, 200, 30);
    sdlgui_combo_box_add_item(cb, "Apple");
    sdlgui_combo_box_add_item(cb, "Banana");
    sdlgui_combo_box_add_item(cb, "Cherry");
    sdlgui_combo_box_add_item(cb, "Date");
    sdlgui_combo_box_add_item(cb, "Elderberry");

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 30, 110, "Choice: Apple (index 0)", -1);

    ComboCtx ctx = {lbl};
    sdlgui_combo_box_set_on_select(cb, on_combo_select, &ctx);

    sdlgui_element_t clearBtn = sdlgui_button_create(gui, NULL, 250, 30, 100, 30, "Clear");

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
