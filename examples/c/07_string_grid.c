/*
 * 07_string_grid.c — StringGrid with table data
 *
 * Compile (requires dist/ artifacts):
 *   gcc -std=c11 -o 07_string_grid examples/c/07_string_grid.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_element_t label;
} GridCtx;

static const char* kHeaders[] = {"Name", "Size", "Price"};
static const char* kData[][3] = {
    {"Monstera",   "Medium", "$25"},
    {"Ficus",      "Large",  "$45"},
    {"Succulent",  "Small",  "$8"},
    {"Orchid",     "Medium", "$30"},
    {"Snake Plant","Large",  "$35"},
    {"Aloe Vera",  "Small",  "$12"},
};
static const int kRows = 6;
static const int kCols = 3;

static void on_cell_click(sdlgui_element_t elem, size_t row, size_t col, void* data) {
    (void)elem;
    GridCtx* ctx = (GridCtx*)data;
    const char* cell_text = sdlgui_string_grid_get_cell(elem, row, col);
    char buf[128];
    snprintf(buf, sizeof(buf), "Click: [%zu,%zu] = %s", row, col, cell_text ? cell_text : "");
    sdlgui_label_set_text(ctx->label, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C StringGrid", 500, 350, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t sg = sdlgui_string_grid_create(gui, NULL, 20, 20, 460, 240,
                                                      (size_t)kRows, (size_t)kCols);

    /* Populate headers */
    for (int c = 0; c < kCols; ++c) {
        sdlgui_string_grid_set_cell(sg, 0, (size_t)c, kHeaders[c]);
    }

    /* Populate data */
    for (int r = 1; r <= kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            sdlgui_string_grid_set_cell(sg, (size_t)r, (size_t)c, kData[r-1][c]);
        }
    }

    sdlgui_element_t lbl = sdlgui_label_create(gui, NULL, 20, 280, "Click a cell...", -1);

    GridCtx ctx = {lbl};
    sdlgui_string_grid_set_on_cell_click(sg, on_cell_click, &ctx);

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
