/*
 * 11_dialogs.c — Dialogs from pure C (confirm / color / file / message box)
 *
 * Four buttons open a DialogBox confirm, a ColorDialog picker, a
 * FileDialog opener and a MessageBox info popup. A status label shows
 * the last result.
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 11_dialogs examples/c/11_dialogs.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_t gui;
    sdlgui_element_t status;
} AppCtx;

static void on_confirm_result(sdlgui_element_t dlg, int confirmed, void* data) {
    (void)dlg;
    AppCtx* app = (AppCtx*)data;
    sdlgui_label_set_text(app->status, confirmed ? "Confirmed: YES" : "Confirmed: NO");
}

static void on_ask_confirm(sdlgui_element_t btn, void* data) {
    (void)btn;
    AppCtx* app = (AppCtx*)data;
    sdlgui_dialog_box_create_confirm(app->gui, "Delete this file?",
        "Yes", "No", on_confirm_result, app, 0, 0);
}

static void on_color_result(sdlgui_element_t dlg, SDL_Color color, void* data) {
    (void)dlg;
    AppCtx* app = (AppCtx*)data;
    char buf[64];
    snprintf(buf, sizeof buf, "Color: #%02X%02X%02X", color.r, color.g, color.b);
    sdlgui_label_set_text(app->status, buf);
}

static void on_ask_color(sdlgui_element_t btn, void* data) {
    (void)btn;
    AppCtx* app = (AppCtx*)data;
    SDL_Color initial = {80, 120, 200, 255};
    sdlgui_color_dialog_create(app->gui, "Pick a color", initial, on_color_result, app);
}

static void on_file_result(sdlgui_element_t dlg, const char* path, void* data) {
    (void)dlg;
    AppCtx* app = (AppCtx*)data;
    char buf[512];
    snprintf(buf, sizeof buf, "File: %s", path ? path : "(null)");
    sdlgui_label_set_text(app->status, buf);
}

static void on_ask_file(sdlgui_element_t btn, void* data) {
    (void)btn;
    AppCtx* app = (AppCtx*)data;
    sdlgui_file_dialog_create_open(app->gui, "Open file", NULL, "*.*", on_file_result, app);
}

static void on_info_dismissed(void* data) {
    AppCtx* app = (AppCtx*)data;
    sdlgui_label_set_text(app->status, "Info dismissed");
}

static void on_ask_info(sdlgui_element_t btn, void* data) {
    (void)btn;
    AppCtx* app = (AppCtx*)data;
    sdlgui_message_box_show_info(app->gui, "Saved successfully.", on_info_dismissed, app);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C Dialogs", 480, 260, 0);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    sdlgui_element_t status = sdlgui_label_create(gui, NULL, 20, 200, "No dialog yet", -1);
    AppCtx app = {gui, status};

    sdlgui_element_t b1 = sdlgui_button_create(gui, NULL, 20, 20, 200, 40, "Confirm...");
    sdlgui_element_t b2 = sdlgui_button_create(gui, NULL, 240, 20, 200, 40, "Color...");
    sdlgui_element_t b3 = sdlgui_button_create(gui, NULL, 20, 80, 200, 40, "Open file...");
    sdlgui_element_t b4 = sdlgui_button_create(gui, NULL, 240, 80, 200, 40, "Info");

    sdlgui_button_set_on_click(b1, on_ask_confirm, &app);
    sdlgui_button_set_on_click(b2, on_ask_color, &app);
    sdlgui_button_set_on_click(b3, on_ask_file, &app);
    sdlgui_button_set_on_click(b4, on_ask_info, &app);

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
