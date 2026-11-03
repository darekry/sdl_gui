/*
 * 16_menu_list.c — MenuZ-style modal list with columns, disabled rows
 * and a periodic tick
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 16_menu_list examples/c/16_menu_list.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

typedef struct {
    sdlgui_t gui;
    sdlgui_element_t list;
    int selected;
    int done;
} AppCtx;

static void on_select(sdlgui_element_t list, int id, const char* text, void* data) {
    (void)list;
    (void)text;
    AppCtx* app = (AppCtx*)data;
    app->selected = id;
    app->done = 1;
}

static void on_tick(sdlgui_element_t list, void* data) {
    (void)list;
    (void)data;
}

int main(void) {
    AppCtx app = {0};
    app.gui = sdlgui_create("MenuList", 520, 360, 1);
    if (!app.gui) return 1;

    const sdlgui_menu_list_item_t items[] = {
        {"New Game", 1, 1},
        {"Load Game", 2, 1},
        {"Preferences", 3, 0},
        {"Quit", 4, 1},
        {"Custom Game", 5, 1},
        {"Hall of Fame", 6, 1},
    };

    app.list = sdlgui_menu_list_create(app.gui, NULL, 40, 40, 300, 200,
                                       "Main Menu", items, 6);
    sdlgui_menu_list_set_columns(app.list, 2);
    sdlgui_menu_list_set_cancel_id(app.list, 99);
    sdlgui_menu_list_set_on_select(app.list, on_select, &app);
    sdlgui_menu_list_set_on_tick(app.list, on_tick, &app);
    sdlgui_menu_list_set_timer_ms(app.list, 100);

    sdlgui_element_request_focus(app.list);

    SDL_Renderer* renderer = sdlgui_get_renderer(app.gui);
    SDL_Event event;
    while (!app.done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                app.done = 1;
                break;
            }
            sdlgui_process_event(app.gui, &event);
        }
        sdlgui_update(app.gui);
        sdlgui_cleanup(app.gui);

        SDL_SetRenderDrawColor(renderer, 24, 24, 24, 255);
        SDL_RenderClear(renderer);
        sdlgui_render(app.gui);
        SDL_RenderPresent(renderer);
    }

    printf("selected id: %d\n", app.selected);
    sdlgui_destroy(app.gui);
    return 0;
}
