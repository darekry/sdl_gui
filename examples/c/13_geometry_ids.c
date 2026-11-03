/*
 * 13_geometry_ids.c — geometry, control ids, visibility and focus from C
 *
 * Demonstrates the parity P0 additions: local/absolute rects, effective
 * visibility, DFS lookups, control ids and the text metrics helpers.
 *
 * Compile (requires dist/ artifacts from ./nob release):
 *   gcc -std=c11 -o 13_geometry_ids examples/c/13_geometry_ids.c \
 *       -Idist dist/libsdl_gui.a $(pkg-config --cflags --libs sdl3 sdl3-image sdl3-ttf)
 */

#include <sdl_gui.h>
#include <stdio.h>

int main(void) {
    sdlgui_t gui = sdlgui_create("Geometry & ids", 640, 480, 1);
    if (!gui) return 1;

    sdlgui_element_t panel = sdlgui_panel_create(gui, NULL, 40, 60, 300, 200);
    sdlgui_element_t child = sdlgui_panel_create(gui, panel, 10, 20, 120, 40);
    sdlgui_element_set_id(child, "inside");
    sdlgui_element_set_control_id(panel, 1000);
    sdlgui_element_set_control_id(child, 1001);

    sdlgui_rect_t local = {0, 0, 0, 0};
    sdlgui_rect_t absolute = {0, 0, 0, 0};
    sdlgui_element_get_rect(child, &local);
    sdlgui_element_get_absolute_rect(child, &absolute);
    printf("local: %d,%d %dx%d  absolute: %d,%d %dx%d\n",
           local.x, local.y, local.w, local.h,
           absolute.x, absolute.y, absolute.w, absolute.h);

    printf("find_by_id: %s\n",
           sdlgui_element_find_by_id(panel, "inside") == child ? "ok" : "miss");
    printf("control id 1001 visible: %s\n",
           sdlgui_element_find_by_control_id(panel, 1001) == child ? "yes" : "no");

    /* Hiding the parent hides the subtree for focus purposes. */
    sdlgui_element_set_visible(panel, 0);
    printf("child shown while parent hidden: %s\n",
           sdlgui_element_is_shown(child) ? "yes" : "no");
    sdlgui_element_set_visible(panel, 1);

    int width = 0, height = 0, line_height = 0;
    if (sdlgui_measure_text(gui, "Hello\nWorld", 0, &width, &height, &line_height)) {
        printf("text: %dx%d (line %d)\n", width, height, line_height);
    } else {
        printf("text: font fixture unavailable (expected in a bare checkout)\n");
    }

    sdlgui_rect_t window = {0, 0, 0, 0};
    sdlgui_get_window_rect(gui, &window);
    printf("window: %dx%d\n", window.w, window.h);

    sdlgui_destroy(gui);
    return 0;
}
