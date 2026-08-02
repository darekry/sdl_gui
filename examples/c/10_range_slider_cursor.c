/*
 * 10_range_slider_cursor.c — RangeSlider + Cursor + ShaderPanel (Phase 3)
 *
 * Shows the dual-thumb RangeSlider and a custom cursor with per-state
 * textures. On GPU-capable systems the same window also shows a
 * ShaderPanel (Vulkan/SPIR-V); without Vulkan it falls back to a CPU
 * context where the ShaderPanel renders as a plain panel.
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 10_range_slider_cursor examples/c/10_range_slider_cursor.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>   /* snprintf */

typedef struct {
    sdlgui_element_t label;
} RangeCtx;

static void on_range_change(sdlgui_element_t rs, void* data) {
    RangeCtx* ctx = (RangeCtx*)data;
    int lo = sdlgui_range_slider_get_lower_value(rs);
    int hi = sdlgui_range_slider_get_upper_value(rs);
    char buf[64];
    snprintf(buf, sizeof(buf), "Range: [%d .. %d]", lo, hi);
    sdlgui_label_set_text(ctx->label, buf);
}

static void on_cursor_state(sdlgui_element_t cur, sdlgui_cursor_state_t state, void* data) {
    (void)cur;
    (void)state;
    (void)data;
}

int main(void) {
    /* Prefer a GPU context (ShaderPanel needs it); fall back to CPU. */
    sdlgui_t gui = sdlgui_create_gpu("C RangeSlider + Cursor", 640, 420, 0);
    if (!gui) {
        gui = sdlgui_create("C RangeSlider + Cursor", 640, 420, 0);
    }
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    /* --- RangeSlider ------------------------------------------------- */
    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 30, 130, "Range: [20 .. 80]", -1);
    sdlgui_element_t rs = sdlgui_range_slider_create(gui, NULL, 30, 60, 420, 40,
                                                        0, 100, 20, 80,
                                                        SDLGUI_ORIENTATION_HORIZONTAL);

    RangeCtx ctx = {lbl};
    sdlgui_range_slider_set_on_change(rs, on_range_change, &ctx);

    /* --- ShaderPanel (plain panel on CPU contexts) ------------------- */
    sdlgui_element_t sp = sdlgui_shader_panel_create(gui, NULL, 30, 180, 420, 200);
    if (sp) {
        /* No SPIR-V here: shader stays disabled, panel renders plain. */
        sdlgui_shader_panel_set_uniform_time(sp, 0.5f);
        sdlgui_shader_panel_set_uniform_mouse(sp, 320.0f, 210.0f);
    }

    /* --- Cursor ------------------------------------------------------ */
    sdlgui_element_t cur = sdlgui_cursor_create(gui);
    if (cur) {
        sdlgui_cursor_set_texture(cur, SDLGUI_CURSOR_NORMAL,
                                  "assets/button1.png", 8, 8);
        sdlgui_cursor_set_texture(cur, SDLGUI_CURSOR_HOVER,
                                  "assets/button_bg.png", 16, 16);
        sdlgui_cursor_set_animated_texture(cur, SDLGUI_CURSOR_BUSY,
                                           "assets/button1.png", 4, 2, 8.0f, 16, 16);
        sdlgui_cursor_set_scale(cur, 0.5f);
        sdlgui_cursor_set_on_state_changed(cur, on_cursor_state, NULL);
    }

    /* --- Main loop ---------------------------------------------------- */
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
