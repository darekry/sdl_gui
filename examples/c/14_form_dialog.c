/*
 * 14_form_dialog.c — FormDialog with control ids and a modal runner
 *
 * Builds a small dialog from item descriptors, wires a command callback and
 * runs the library modal loop. Esc (or the Cancel button) ends with 1.
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 14_form_dialog examples/c/14_form_dialog.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

static void on_command(sdlgui_element_t form, int id, void* data) {
    sdlgui_t gui = (sdlgui_t)data;
    if (id == 2) {
        sdlgui_form_end(form, 0);
    } else if (id == 3) {
        sdlgui_form_end(form, 1);
    }
    (void)gui;
}

static void on_frame(void* data) {
    (void)data;
}

int main(void) {
    sdlgui_t gui = sdlgui_create("FormDialog", 640, 480, 1);
    if (!gui) return 1;

    sdlgui_form_item_desc_t items[] = {
        {1, SDLGUI_FORM_ITEM_STATIC, "Your name:", 20, 20, 120, 24},
        {4, SDLGUI_FORM_ITEM_EDIT, "", 150, 20, 180, 26},
        {2, SDLGUI_FORM_ITEM_BUTTON, "OK", 150, 80, 80, 28},
        {3, SDLGUI_FORM_ITEM_BUTTON, "Cancel", 250, 80, 80, 28},
    };

    sdlgui_element_t form = sdlgui_form_create_from_items(
        gui, "Form", 40, 40, 360, 140, items, 4);
    if (!form) {
        sdlgui_destroy(gui);
        return 1;
    }

    sdlgui_form_set_on_command(form, on_command, gui);

    sdlgui_modal_options_t opts = {0};
    opts.keep_background = 0;
    opts.clear_r = 32;
    opts.clear_g = 32;
    opts.clear_b = 32;
    opts.cancel_key = SDLK_ESCAPE;
    opts.cancel_result = 1;
    opts.frame_cb = on_frame;

    int result = -1;
    sdlgui_modal_run(gui, form, &opts, &result);
    printf("dialog result: %d\n", result);

    sdlgui_destroy(gui);
    return 0;
}
