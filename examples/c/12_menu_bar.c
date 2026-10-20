/*
 * 12_menu_bar.c — Win32-style menu bar from pure C
 *
 * File/Options menu with mnemonics, accelerators (Ctrl+S/Ctrl+Q), a check
 * item and a command callback (the C equivalent of WM_COMMAND). The app
 * content panel is anchored below the bar using sdlgui_menu_bar_get_height().
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 12_menu_bar examples/c/12_menu_bar.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    sdlgui_t gui;
    sdlgui_element_t status;
    sdlgui_menu_item_t sound;
    sdlgui_menu_item_t auto_save;
    int quit;
} AppCtx;

static void on_menu_command(sdlgui_element_t bar, const char* command, void* data) {
    (void)bar;
    AppCtx* app = (AppCtx*)data;
    char buf[160];

    if (command == NULL) command = "";

    if (strcmp(command, "file.quit") == 0) {
        app->quit = 1;
        snprintf(buf, sizeof buf, "Command: %s (quitting)", command);
    } else if (strcmp(command, "options.sound") == 0) {
        snprintf(buf, sizeof buf, "Sound: %s",
                 sdlgui_menu_item_is_checked(app->sound) ? "On" : "Off");
    } else if (strcmp(command, "options.autosave") == 0) {
        snprintf(buf, sizeof buf, "Auto Save: %s",
                 sdlgui_menu_item_is_checked(app->auto_save) ? "On" : "Off");
    } else {
        snprintf(buf, sizeof buf, "Command: %s", command);
    }
    sdlgui_label_set_text(app->status, buf);
}

int main(void) {
    sdlgui_t gui = sdlgui_create("C MenuBar", 820, 520, 1);
    if (!gui) return 1;

    SDL_Renderer* ren = sdlgui_get_renderer(gui);

    /* ── Menu bar (auto height from the theme font) ─────────────────── */
    sdlgui_element_t bar = sdlgui_menu_bar_create(gui, NULL, 0, 0, 820, 0);
    sdlgui_element_set_anchor(bar,
        (sdlgui_anchor_t){SDLGUI_H_STRETCH, SDLGUI_V_TOP, 0, 0, 0, 0});

    sdlgui_menu_item_t file = sdlgui_menu_bar_add_menu(bar, "&File");
    sdlgui_menu_item_t save = sdlgui_menu_item_add_item(file, "&Save Game", "file.save");
    sdlgui_menu_item_set_shortcut(save, "Ctrl+S");
    sdlgui_menu_item_add_separator(file);

    sdlgui_menu_item_t options = sdlgui_menu_item_add_submenu(file, "&Options");
    sdlgui_menu_item_t sound = sdlgui_menu_item_add_item(options, "&Sound", "options.sound");
    sdlgui_menu_item_set_check_type(sound, SDLGUI_MENU_CHECK);
    sdlgui_menu_item_set_checked(sound, 1);
    sdlgui_menu_item_t auto_save = sdlgui_menu_item_add_item(options, "&Auto Save", "options.autosave");
    sdlgui_menu_item_set_check_type(auto_save, SDLGUI_MENU_CHECK);
    sdlgui_menu_item_set_checked(auto_save, 1);

    sdlgui_menu_item_t view = sdlgui_menu_bar_add_menu(bar, "&View");
    sdlgui_menu_item_t tiny = sdlgui_menu_item_add_item(view, "&Tiny", "view.tiny");
    sdlgui_menu_item_set_check_type(tiny, SDLGUI_MENU_RADIO);
    sdlgui_menu_item_set_checked(tiny, 1);
    sdlgui_menu_item_t large = sdlgui_menu_item_add_item(view, "&Large", "view.large");
    sdlgui_menu_item_set_check_type(large, SDLGUI_MENU_RADIO);

    sdlgui_menu_item_add_separator(file);
    sdlgui_menu_item_t quit = sdlgui_menu_item_add_item(file, "&Quit", "file.quit");
    sdlgui_menu_item_set_shortcut(quit, "Ctrl+Q");

    sdlgui_menu_item_t help = sdlgui_menu_bar_add_menu(bar, "&Help");
    sdlgui_menu_item_add_item(help, "&About", "help.about");

    /* ── Content panel anchored below the bar ───────────────────────── */
    int bar_h = sdlgui_menu_bar_get_height(bar);
    sdlgui_element_t content = sdlgui_panel_create(gui, NULL, 0, bar_h, 820, 520 - bar_h);
    sdlgui_element_set_anchor(content,
        (sdlgui_anchor_t){SDLGUI_H_STRETCH, SDLGUI_V_STRETCH, 0, bar_h, 0, 0});
    sdlgui_element_set_background_color(content, SDLGUI_STATE_NORMAL,
                                        (SDL_Color){0, 0, 0, 255});

    sdlgui_element_t status = sdlgui_label_create(gui, content, 24, 24,
                                                  "Ready. Alt+F / F10 opens the menu.", 18);
    sdlgui_element_set_text_color(status, SDLGUI_STATE_NORMAL,
                                  (SDL_Color){235, 235, 235, 255});

    sdlgui_element_t hint = sdlgui_label_create(gui, content, 24, 60,
        "Ctrl+S saves, Ctrl+Q quits, check items update themselves.", 14);
    sdlgui_element_set_text_color(hint, SDLGUI_STATE_NORMAL,
                                  (SDL_Color){150, 150, 150, 255});

    AppCtx app = {gui, status, sound, auto_save, 0};
    sdlgui_menu_bar_set_on_command(bar, on_menu_command, &app);

    SDL_Event e;
    while (!app.quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                app.quit = 1;
                break;
            }
            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                sdlgui_handle_resize(gui, e.window.data1, e.window.data2);
                continue;
            }
            sdlgui_process_event(gui, &e);
        }

        sdlgui_update(gui);
        sdlgui_cleanup(gui);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        sdlgui_render(gui);
        SDL_RenderPresent(ren);
    }

    sdlgui_destroy(gui);
    return 0;
}
